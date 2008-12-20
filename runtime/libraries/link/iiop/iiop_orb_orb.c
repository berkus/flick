/*
 * Copyright (c) 1996, 1997, 1998, 1999 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#include "iiop-link.h"

#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#ifdef NEED_GETHOSTNAME_DECL
/*
 * `gethostname' should (?) be in <unistd.h>, but Solaris 2.5 at least doesn't
 * declare `gethostname' in any system header.
 */
int gethostname(char *name, int namelen);
#endif /* NEED_GETHOSTNAME_DECL */

/* needed only for flick_check_byte_order */
#include <flick/encode/cdr.h>

/*****************************************************************************/

char flick_is_little_endian = -1; /* not defined yet */

int flick_seqnum = 0;

#ifdef INTERFACE_DAEMON
/* Information about the simple interface repository object */
char *orb_ir_ref;
CORBA_Object orb_ir_obj;
#endif

static CORBA_ORB *orbs = 0;
static int orb_count = 0;

/*
 * XXX --- Just a stub for now.
 */
CORBA_ORB_ObjectIdList *
CORBA_ORB_list_initial_services(CORBA_ORB obj, CORBA_Environment *ev)
{
	CORBA_ORB_ObjectIdList *list;
	
	fprintf(stderr,
		"Warning: `CORBA_ORB_list_initial_services' is not yet "
		"supported.\n");
	
	list = CORBA_alloc(sizeof(CORBA_ORB_ObjectIdList));
	if (!list) {
		flick_set_exception(0 /* boa */, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	
	list->_maximum = 0;
	list->_length = 0;
	list->_buffer = 0;
	
	ev->_major = CORBA_NO_EXCEPTION;
	
	return list;
}

/*
 * XXX --- Just a stub for now until we have at least a Naming Service.
 */
CORBA_Object
CORBA_ORB_resolve_initial_references(CORBA_ORB ths,
				     CORBA_ORB_ObjectId identifier,
				     CORBA_Environment *ev)
{
	fprintf(stderr,
		"Warning: `CORBA_ORB_resolve_initial_references' is not yet "
		"supported.\n");
	CORBA_BOA_set_exception(0 /*boa*/, ev,
				CORBA_USER_EXCEPTION,
				ex_CORBA_ORB_InvalidName,
				/*
				 * Last arg is null; see Section 17.27.2 of
				 * the CORBA 2.1 spec to see why we don't call
				 * `CORBA_ORB_InvalidName__alloc' here.
				 */
				0);
	return 0;
}

CORBA_ORB CORBA_ORB_init_internal(CORBA_ORBid ORBid,
				  CORBA_Environment *ev)
{
        unsigned int i, one = 1;
	
	flick_is_little_endian = *(char *)(&one);
	flick_check_byte_order();
	
#ifdef INTERFACE_DAEMON
	/* These are the builtin defaults */
	if( !orb_ir_ref )
		orb_ir_ref = "iiop:1.0//localhost:6000/FLICK::"
			     "interface_type/SimpleInterfaceRepository";
#endif
	if (!ORBid)
		ORBid = "FlickORB--";
	for (i = 0; i < (unsigned int) orb_count; ++i)
		if (!strcmp(orbs[i]->ORBid, ORBid))
			break;
	
	/* Set the default value for the ir ref */
	if (((int) i) == orb_count) {
		CORBA_ORB *new_orbs, orb;
				
		orb = t_calloc(struct CORBA_ORB_type, 1);
		if (!orb)
			goto err1;		
		orb->ORBid = t_malloc(char, strlen(ORBid) + 1);
		if (!orb->ORBid)
			goto err2;
		strcpy(orb->ORBid, ORBid);
		orb->OA_count = 0;
		orb->boas = 0;
		
		new_orbs = t_realloc(orbs, CORBA_ORB, (orb_count + 1));
		if (!new_orbs)
			goto err3;
		orbs = new_orbs;
		orbs[orb_count++] = orb;
#ifdef INTERFACE_DAEMON
		/* Make a reference to the simple
		   interface repository object */
		orb_ir_obj = CORBA_ORB_string_to_object( orb, orb_ir_ref, ev );
		if (ev->_major != CORBA_NO_EXCEPTION) {
			printf( "Can't get simple interface "
				"repository object.\n" );
			exit(1);
		}
#endif
		return orb;
		
	  err3:
		free(orb->ORBid);
	  err2:
		free(orb);
	  err1:
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for new ORB.\n");
		return 0;
		
	} else
		/* do nothing.  `i' is set correctly. */
	
	return orbs[i];
}


CORBA_ORB CORBA_ORB_init(int *argc, char **argv,
			 CORBA_ORBid ORBid,
			 CORBA_Environment *ev)
{
	int i, new_argc;
	
	/*****/
	
	/* No ORB --> no BOA --> no `CORBA_BOA_set_exception'. */
	ev->_major = CORBA_NO_EXCEPTION;
	
	if (!argc || !argv) {
		flick_set_exception(0 /* XXX boa */, ev,
				    ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: null `argc' or `argv' passed to "
			"`CORBA_ORB_init'.\n");
		return 0;
	}
	
	for (i = new_argc = 1; i < *argc; i++) {
		if (!strcmp(argv[i], "-ORBid") &&
		    i + 1 < *argc) {
			if (!ORBid)
				ORBid = argv[++i];
			else {
				flick_set_exception(0 /* XXX boa */, ev,
						    ex_CORBA_BAD_PARAM,
						    0, CORBA_COMPLETED_NO);
				fprintf(stderr,
					"Error: too many ORB identifiers "
					"specified.\n");
				return 0;
			}
		}
#ifdef INTERFACE_DAEMON
		else if( !strcmp( argv[i], "-ORBir") && (i + 1 < *argc) ) {
			/* The user has specified an alternative
			   reference to the simple interface repository */
			if( !orb_ir_ref )
				orb_ir_ref = argv[++i];
			else {
				flick_set_exception(0 /* XXX boa */, ev,
						    ex_CORBA_BAD_PARAM,
						    0, CORBA_COMPLETED_NO);
				fprintf(stderr,
					"Error: too many ORB identifiers "
					"specified.\n");
				return 0;
			}
		}
#endif
		else
			/* keep all of the arguments except the -ORB ones */
			argv[new_argc++] = argv[i];
	}
	/* nullify the removed arguments */
	for (i = *argc-1; i >= new_argc; i--)
		argv[i] = 0;
	/* export the modified argc */
	*argc = new_argc;
	return CORBA_ORB_init_internal(ORBid, ev);
}

CORBA_BOA CORBA_ORB_BOA_init_internal(CORBA_ORB ths,
				      CORBA_ORB_OAid OAid,
				      char *OAaddr,
				      unsigned short host_port,
				      CORBA_Environment *ev)
{
	int i;
	CORBA_BOA boa, *new_boas;
	int socket_reuseaddr_p = 1;		
	struct hostent *he; /* not on heap */
	
	/*
	 * Convert `OAaddr' into a FQDN.  If we didn't do this, the
	 * IORs for `iiop:1.0/host/foo' and `iiop:1.0/host.dom/foo'
	 * would be different.  We want them to be the same so that our
	 * server doesn't think that the two IORs refer to objects in
	 * different ORBs.  (The server expects the hostnames to be
	 * identical.)
	 */
	if (!(he = gethostbyname(OAaddr))) {
		flick_set_exception(boa /* XXX not initialized yet */, ev,
				    ex_CORBA_INV_IDENT,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: unable to identify host `%s'.\n",
			OAaddr);
		return 0;
	}
	
	/* Check to see if this BOA is already here... */
	for (i = 0; i < ths->OA_count; i++)
		if ((ths->boas[i]->hostport == host_port) &&
		    (!strcmp(ths->boas[i]->hostname, he->h_name))) {
			/* Found it! */
			if ((OAid) && !!strcmp(ths->boas[i]->OAid, OAid))
				fprintf(stderr,
					"Warning: OA id `%s' will not replace"
					" `%s' in ORB `%s'\n",
					OAid, ths->boas[i]->OAid,
					ths->ORBid);
			return ths->boas[i];
		}
	
	boa = t_calloc(struct CORBA_BOA_type, 1);
	if (!boa)
		goto err1;
	
	boa->ipaddr = t_calloc(struct sockaddr_in, 1);
	if (!boa->ipaddr)
		goto err2;
	
	boa->ipaddr->sin_addr.s_addr = *(int *)*he->h_addr_list;
	boa->ipaddr->sin_family = AF_INET;
	boa->ipaddr->sin_port = htons(host_port);
	
	/*
	 * Use the FQDN returned by `gethostbyname', not `OAaddr',
	 * because `OAaddr' may not be fully qualified.
	 */
	boa->hostname = t_malloc(char, strlen(he->h_name) + 1);
	if (!boa->hostname)
		goto err3;
	strcpy(boa->hostname, he->h_name);
	
	boa->hostport = host_port;
	boa->orb = ths;
	
	if (!OAid)
		OAid = "FlickBOA--";
	boa->OAid = t_malloc(char, strlen(OAid) + 1);
	if (!boa->OAid)
		goto err5;
	strcpy(boa->OAid, OAid);
	
	boa->in = t_calloc(FLICK_BUFFER, 1);
	if (!boa->in)
		goto err6;
	boa->in->real_buf_start = boa->in->buf_read = boa->in->buf_start
				= boa->in->buf_current = t_calloc(char, 8192);
	if (!boa->in->buf_start)
		goto err7;
	
	boa->in->real_buf_end = boa->in->buf_end
			      = ((char *) boa->in->buf_start) + 8192;
	
	boa->out = t_calloc(FLICK_BUFFER, 1);
	if (!boa->out)
		goto err8;
	boa->out->real_buf_start = boa->out->buf_read = boa->out->buf_start
				= boa->out->buf_current = t_calloc(char, 8192);
	if (!boa->out->buf_start)
		goto err9;
	
	boa->out->real_buf_end = boa->out->buf_end
			       = ((char *) boa->out->buf_start) + 8192;
	
	boa->out->buf_end = ((char *) boa->out->buf_start) + 8192;
	
	boa->count_servers = 0;
	boa->refs = 0;
	boa->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (boa->socket_fd < 0) {
		perror("socket");
		flick_set_exception(boa, ev, ex_CORBA_COMM_FAILURE,
				    0, CORBA_COMPLETED_NO);
		goto err10;
	}
	if (setsockopt(boa->socket_fd,
		       SOL_SOCKET, SO_REUSEADDR,
		       ((void *) &socket_reuseaddr_p),
		       sizeof(socket_reuseaddr_p))
	    != 0) {
		perror("setsockopt");
		flick_set_exception(boa, ev, ex_CORBA_COMM_FAILURE,
				    0, CORBA_COMPLETED_NO);
		goto err11;
	}
	boa->connected = 0;
	
	new_boas = t_realloc(ths->boas, CORBA_BOA, ths->OA_count + 1);
	if (!new_boas)
		goto err11;
	ths->boas = new_boas;
	ths->boas[ths->OA_count++] = boa;
	
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return boa;
	
  err11:
	close(boa->socket_fd);
  err10:
	free(boa->out->buf_start);
  err9:
	free(boa->out);
  err8:
	free(boa->in->buf_start);
  err7:
	free(boa->in);
  err6:
	free(boa->OAid);
  err5:
	free(boa->hostname);
  err3:
	free(boa->ipaddr);
  err2:
	free(boa);
  err1:
	if (ev->_major == CORBA_NO_EXCEPTION)
		flick_set_exception(boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
 	/* set exception */
	return 0;
}

	
CORBA_BOA CORBA_ORB_BOA_init(CORBA_ORB ths, int *argc, char **argv,
			     CORBA_ORB_OAid OAid,
			     CORBA_Environment *ev)
{
	int i, new_argc;
	char *prt = 0;
	unsigned short host_port;
	char *OAaddr = 0;
	char buf[256];
	
	/*****/
	
	if (!argc || !argv) {
		flick_set_exception(0 /* XXX boa */, ev,
				    ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: null `argc' or `argv' passed to "
			"`CORBA_ORB_BOA_init'.\n");
		return 0;
	}
	
	for (i = new_argc = 1; i < *argc; i++) {
                /* XXX should change name to OAipaddr */
		if (!strcmp(argv[i], "-ORBipaddr") &&
		    (i + 1 < *argc)) {
			if (!OAaddr)
				OAaddr = argv[++i];
			else {
				flick_set_exception(0 /* XXX boa */, ev,
						    ex_CORBA_BAD_PARAM,
						    0, CORBA_COMPLETED_NO);
				fprintf(stderr,
					"Error: too many ORB addresses "
					"specified.\n");
				return 0;
			}
		} else if (!strcmp(argv[i], "-OAport") &&
			   i + 1 < *argc) {
			if (!prt)
				prt = argv[++i];
			else {
				flick_set_exception(boa, ev,
						    ex_CORBA_BAD_PARAM,
						    0, CORBA_COMPLETED_NO);
				fprintf(stderr,
					"Error: too many OA ports "
					"specified.\n");
				return 0;
			}
		} else if (!strcmp(argv[i], "-OAid") &&
			   i + 1 < *argc) {
			if (!OAid)
				OAid = argv[++i];
			else {
				flick_set_exception(boa, ev,
						    ex_CORBA_BAD_PARAM,
						    0, CORBA_COMPLETED_NO);
				fprintf(stderr,
					"Error: too many OA id's "
					"specified.\n");
				return 0;
			}
		}
		else
			/* keep all of the arguments except the -OA ones */
			argv[new_argc++] = argv[i];
	}
	/* nullify the removed arguments */
	for (i = *argc-1; i >= new_argc; i--)
		argv[i] = 0;
	
	/* export the modified argc */
	*argc = new_argc;
	
	if (!OAaddr) {
		if (gethostname(buf, 256)) {
			flick_set_exception(0 /* XXX boa */, ev,
					    ex_CORBA_INV_IDENT,
					    0, CORBA_COMPLETED_NO);
			fprintf(stderr,
				"Error: unable to get name of local host.\n");
			return 0;
		}
		OAaddr = &buf[0];
		fprintf(stderr,
			"Warning: no `-ORBipaddr' specified; using `%s'.\n",
			OAaddr);
	}
	
	if (!prt) {
		flick_set_exception(boa, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr, "Error: No `-OAport' specified.\n");
		return 0;
	}
	host_port = atoi(prt);
	return CORBA_ORB_BOA_init_internal(ths, OAid, OAaddr, host_port, ev);
}

CORBA_BOA CORBA_ORB_BOA_create(CORBA_ORB ths,
			       CORBA_ORB_OAid OAid,
			       char *OAaddr,
			       unsigned short host_port,
			       CORBA_Environment *ev)
{
	CORBA_BOA ret;
	
	if (!ths && orb_count)
		ths = orbs[0];
	if (!OAid)
		OAid = "FlickBOA--";
	
	ret = CORBA_ORB_BOA_init_internal(ths, OAid, OAaddr, host_port, ev);
	
	/* Bind to the port. */
	if (!ret->connected) {
		struct sockaddr_in server;
		int len = sizeof(struct sockaddr_in);
		
		if (bind(ret->socket_fd,
			 (struct sockaddr *)ret->ipaddr,
			 sizeof(*ret->ipaddr)) != 0) {
			perror("cannot `bind' to socket");
			flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
					    0, CORBA_COMPLETED_NO);
			return ret;
		}
		
		if ((getsockname(ret->socket_fd,
				 (struct sockaddr *)&server, &len) != 0)
		    || (listen(ret->socket_fd, 8) != 0)) {
			perror("cannot `getsockname' or `listen', "
			       "port may be in use");
			flick_set_exception(ths, ev, ex_CORBA_COMM_FAILURE,
					    0, CORBA_COMPLETED_NO);
			return ret;
		}
		ret->connected = 1;
	}
	
	return ret;
}

CORBA_char *CORBA_ORB_object_to_readable_string(CORBA_ORB orb,
						FLICK_TARGET obj,
						CORBA_Environment *ev)
{
	char buf[1024], *res;
	int text = 1;
	unsigned int pos;
	
	if (!obj) {
		res = t_malloc(char, strlen("iiop:1.0//") + 1);
		if (!res) {
			/* No object --> no BOA. */
			flick_set_exception(0 /*XXX*/, ev, ex_CORBA_NO_MEMORY,
					    0, CORBA_COMPLETED_NO);
			return 0;
		}
		strcpy(res, "iiop:1.0//");
		
		/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
		ev->_major = CORBA_NO_EXCEPTION;
		
		return res;
	}
	
	for (pos = 0; pos < obj->key._length && text; pos++)
		text = ((obj->key._buffer[pos] >= ' ')
			&& (obj->key._buffer[pos] <= 126));
	if (!text)
		return CORBA_ORB_object_to_string(orb, obj, ev);
	sprintf(buf, "iiop:1.0//%s:%d/%s/",
		obj->boa->hostname,
		obj->boa->hostport,
		obj->type_id);
	pos = strlen(buf);
	memcpy(&buf[pos], obj->key._buffer, obj->key._length);
	buf[pos + obj->key._length] = 0;
	
	res = t_malloc(char, strlen(buf) + 1);
	if (!res) {
		flick_set_exception(obj->boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
	} else {
		strcpy(res, buf);
		CORBA_BOA_set_exception(obj->boa, ev, CORBA_NO_EXCEPTION,
					0, 0);
	}
	
	return res;
}

/*
 * This converts an object into a stringified IOR, using hex digits.
 */
CORBA_char *CORBA_ORB_object_to_string(CORBA_ORB orb,
				       FLICK_TARGET obj,
				       CORBA_Environment *ev)
{
	char *buffer = 0, *res;
	unsigned int buffer_len = 0;
	unsigned int i;
	
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
	if (!obj) {
		/* make a NULL object IOR */
		buffer = (char *)null_obj_ior;
		buffer_len = null_obj_ior_len;
	} else {
		buffer = obj->ior;
		buffer_len = obj->ior_len;
	}
	/* now convert the IOR into a HEX representation */
	res = t_malloc(char, (buffer_len * 2) + 13);
	if (!res) {
		flick_set_exception((obj ? obj->boa : 0) /* XXX */,
				    ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for stringified IOR.\n");
		return 0;
	}
	
	res[0] = 'I';
	res[1] = 'O';
	res[2] = 'R';
	res[3] = ':';
	sprintf(&(res[4]), "%02x000000", native_endianness & 0xff);
	for (i = 0; i < buffer_len; ++i)
		sprintf(&(res[i * 2 + 12]), "%02x", ((int) buffer[i]) & 0xff);
	res[(buffer_len * 2) + 12] = 0;
	
	CORBA_BOA_set_exception((obj ? obj->boa : 0) /* XXX */,
				ev, CORBA_NO_EXCEPTION, 0, 0);
	return res;
}

static int hexbits(char a)
{
	if (a >= '0' && a <= '9')
		return a - '0';
	if ((a >= 'a' && a < 'g') ||
	    (a >= 'A' && a < 'G'))
		return (a & 15) + 9;
	return -1;
}

static int hexoctet(char x, char y)
{
	return hexbits(x) * 16 + hexbits(y);
}

/* extracts the data from an object string.  Returns the length of the data */
int flick_parse_readable_object_string(char *instr, char **out_OAaddr,
				       unsigned short *out_boa_port,
				       char **out_obj_type_id,
				       unsigned int *out_obj_type_id_len,
				       CORBA_octet **out_obj_key,
				       unsigned int *out_obj_key_len,
				       CORBA_Environment *ev) 
{
	/* We're using a readable stringified object. */
	int len, res;
	char *host, *port, *type, *str, *obj_id, *orig;
	
        /* return the total size of the ior */
	res = strlen(instr) + 1; 
	
	if (!!strncmp("iiop:1.0//", instr, 10)) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: malformed URL-style object "
			"reference.\n");
		return res;
	}
	
	if (res == 10 + 1) {
		/* NULL Object reference */
		*out_OAaddr = *out_obj_type_id = 0;
		*out_obj_key = 0;
		*out_obj_type_id_len = *out_obj_key_len = 0;
		*out_boa_port = 0;
		
		/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
		ev->_major = CORBA_NO_EXCEPTION;
		return res;
	}
		
	orig = str = t_malloc(char, res);
	if (!str) {
		flick_set_exception(0 /* boa */, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	strcpy(str, instr + 10); /* skip the header */
	
	/* get the host */
	host = str;
	while(*str != ':' && *str != 0) {
		str++;
	}
	if (*str == 0)
		goto bail;
	*str = 0;
	str++;
	
	/* get the port */
	port = str;

	while(*str != '/' && *str != 0) {
		str++;
	}
	if (*str == 0)
		goto bail;
	*str = 0;
	str++;
	
	/* get the type */
	type = str;
	
	while(*str != '/' && *str != 0) {
		str++;
	}
	if (*str == 0)
		goto bail;
	*str = 0;
	str++;
	
	/* str points to the object id */
	obj_id = str;
	
	len = strlen(host);
	*out_OAaddr = t_malloc(char, (len + 1));
	if (!*out_OAaddr) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for ORB address.\n");
		free(orig);
		return res;
	}
	strncpy(*out_OAaddr,
		host,
		(len + 1));
	
	*out_boa_port = atoi(port);
	
	*out_obj_type_id_len = strlen(type);
	*out_obj_type_id = t_malloc(char, (*out_obj_type_id_len + 1));
	if (!*out_obj_type_id) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for object "
			"type.\n");
		free(orig);
		return res;
	}
	strncpy(*out_obj_type_id,
		type,
		(*out_obj_type_id_len + 1));
	
	*out_obj_key_len = strlen(obj_id);
	*out_obj_key = t_malloc(CORBA_octet, *out_obj_key_len);
	if (!*out_obj_key) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for object "
			"key.\n");
		if (*out_obj_type_id) free(*out_obj_type_id);
		free(orig);
		return res;
	}
	memcpy(*out_obj_key, obj_id, *out_obj_key_len);
	
	/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
	ev->_major = CORBA_NO_EXCEPTION;
	free(orig);
	return res;
	
  bail:
	flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
			    0, CORBA_COMPLETED_NO);
	fprintf(stderr,
		"Error: malformed URL-style object "
		"reference.\n");
	free(orig);
	return res;
	
}

/* extracts the data from an object string.  Returns the length of the data */
int flick_parse_hex_ior(char *str, char **out_OAaddr,
			unsigned short *out_boa_port,
			char **out_obj_type_id,
			unsigned int *out_obj_type_id_len,
			CORBA_octet **out_obj_key,
			unsigned int *out_obj_key_len,
			CORBA_Environment *ev) 
{
	/* We're using the CORBA 2.0 p10.17 stringified object. */
	
	char buf[1024]; /* XXX --- should dynamically alloc `buf'. */
	unsigned int len;     
	
	unsigned i, res;
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
	char encapsulation_endianness;
	
	len = strlen(str);
	res = len + 1;
	
	if (!!strncmp("IOR:", str, 4)) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: malformed IOR.\n");
		return res;
	}
	
	/* Skip the IOR: prefix */
	str += 4;
	len -= 4;
	
	/*
	 * Because every byte in the IOR is represented by two hex
	 * digits in the string, the string must contain an even number
	 * of characters.
	 */
	if (len % 2) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: invalid IOR (odd number of hex "
			"digits).\n");
		return res;
	}
	if ((len / 2) > sizeof(buf)) {
		/* XXX */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_IMP_LIMIT,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: IOR too long.\n");
		return res;
	}
	
	/* Decode the hex string into a sequence of bytes in `buf'. */
	for (i = 0; i < len; i += 2) {
		int val = hexoctet(str[i], str[i + 1]);
		if (val < 0) {
			flick_set_exception(0 /*boa*/, ev,
					    ex_CORBA_BAD_PARAM,
					    0, CORBA_COMPLETED_NO);
			fprintf(stderr,
				"Error: invalid IOR (contains non-hex "
				"digits).\n");
			return res;
		} else
			buf[(i / 2)] = val;
	}	
	
	/*
	 * `buf' now holds a CORBA encapsulation of an IOR as described
	 * in Section 10.6.2 of the CORBA 2.0 specification.
	 */
	encapsulation_endianness = buf[0];
	
	flick_cdr_parse_IOR(&(buf[4]),
			    (encapsulation_endianness != native_endianness),
			    out_OAaddr,
			    out_boa_port,
			    out_obj_type_id,
			    out_obj_type_id_len,
			    out_obj_key, out_obj_key_len, ev);
	return res;
}

/* extracts the data from an object string.  Returns the length of the data */
/* The semantics of this function is that it will allocate the space for the
   out strings and the caller will free them. */
int flick_parse_object_string(char *str, char **out_OAaddr,
			      unsigned short *out_boa_port,
			      char **out_obj_type_id,
			      unsigned int *out_obj_type_id_len,
			      CORBA_octet **out_obj_key,
			      unsigned int *out_obj_key_len,
			      CORBA_Environment *ev) 
{
	assert(out_OAaddr && out_boa_port && out_obj_type_id
	       && out_obj_type_id_len && out_obj_key && out_obj_key_len);
	if (!str || !str[0]) {
		/* Null string object reference */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Warning: empty object reference string.\n");
		return 0;
	}
	
	if (!strncmp("iiop:1.0//", str, 10)) {
		return flick_parse_readable_object_string(str, out_OAaddr,
							  out_boa_port,
							  out_obj_type_id,
							  out_obj_type_id_len,
							  out_obj_key,
							  out_obj_key_len, ev);
	} else if (!strncmp("IOR:", str, 4)) {
		return flick_parse_hex_ior(str, out_OAaddr,
					   out_boa_port,
					   out_obj_type_id,
					   out_obj_type_id_len,
					   out_obj_key, out_obj_key_len, ev);
	} else {
		/* I have no idea what kind of object reference this is. */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: unrecognized object reference format.\n");
		return 0;
	}
}

FLICK_TARGET flick_create_object(char *ORBid, char *OAid, char *OAaddr,
				 unsigned short boa_port,
				 char *obj_type_id,
				 unsigned int obj_type_id_len,
				 CORBA_octet *obj_key,
				 unsigned int obj_key_len,
				 CORBA_Environment *ev)
{
	CORBA_ORB orb;
	
	CORBA_BOA boa;
	
	FLICK_TARGET res;
	
	/*****/
	
	/* return a NULL object if insufficient information is given */
	if ((!obj_type_id) || (!obj_key)) {
		ev->_major = CORBA_NO_EXCEPTION;
		return 0;
	}
	
	orb = CORBA_ORB_init_internal(ORBid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	boa = CORBA_ORB_BOA_init_internal(orb, OAid,
					  OAaddr, boa_port, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	res = t_malloc(FLICK_TARGET_STRUCT, 1);
	if (!res) {
		flick_set_exception(boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for Object.\n");
		return 0;
	}
	
	res->boa = boa;
	res->key._buffer = obj_key;
	res->key._length = obj_key_len;
	res->key._maximum = obj_key_len;
	res->type_id = obj_type_id;
	res->type_id_len = obj_type_id_len;
	res->ior_len = 0;
	res->ior = 0;
	res->server_func = 0;
	if (!flick_cdr_make_ior(res, &res->ior, &res->ior_len, 0)) {
		flick_set_exception(boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for IOR.\n");
		return 0;
	}
	return res;
}

FLICK_TARGET CORBA_ORB_string_to_object(CORBA_ORB ths,
					CORBA_char *str,
					CORBA_Environment *ev)
{
	FLICK_TARGET res;
	char *OAaddr = 0;	
	unsigned short boa_port;	
	char *obj_type_id = 0;
	CORBA_octet *obj_key = 0; 	
	unsigned int obj_type_id_len, obj_key_len;
	
	/*****/
	
	if (!ths) {
		flick_set_exception(0 /* XXX boa */, ev,
				    ex_CORBA_INV_OBJREF,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	
	/* extract the useful data from the object string */
	flick_parse_object_string(str, &OAaddr, &boa_port, &obj_type_id,
				  &obj_type_id_len, &obj_key, &obj_key_len,
				  ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	
	/* create the object (but do not connect it yet) */
	res = flick_create_object(ths->ORBid,
				  0 /*use default OAid*/,
				  OAaddr, boa_port, obj_type_id,
				  obj_type_id_len,
				  obj_key, obj_key_len, ev);
        /* was copied by CORBA_ORB_BOA_init_internal */
	if (OAaddr) free(OAaddr); 
	if (ev->_major != CORBA_NO_EXCEPTION) {
		if (obj_type_id) free(obj_type_id);
		if (obj_key) free(obj_key);
		return 0;
	}
	
	return res;
}

/* End of file. */

