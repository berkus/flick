/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include "mach3mig-link.h"

int flick_seqnum = 0;

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
        unsigned int i;
	
	/* No ORB --> no BOA --> no `CORBA_BOA_set_exception'. */
	ev->_major = CORBA_NO_EXCEPTION;
	
	if (!ORBid)
		ORBid = "FlickORB--";
	for (i = 0; i < (unsigned int) orb_count; ++i)
		if (!strcmp(orbs[i]->ORBid, ORBid))
			break;
	
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
		
	}
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
				      CORBA_Environment *ev)
{
	int i;
	CORBA_BOA boa, *new_boas;
	mach_port_t server_port = 0;
	
	/* Check to see if this BOA is already here... */
	if (!OAid) OAid = "NameServer";
	
	for (i = 0; i < ths->OA_count; i++) {
		if (!strcmp(ths->boas[i]->OAid, OAid)) {
			return ths->boas[i];
		}
	}
	
	boa = t_calloc(struct CORBA_BOA_type, 1);
	if (!boa)
		goto err1;
	
	/*
	 * The OAid is a string version of the name server port (in
	 * hex).  Check here to see if it's valid; if not, or if we
	 * weren't given one, simply create one from the default
	 * name_server_port defined by the mach headers.
	 */
	if (server_port)
		boa->name_server_port = server_port;
	else
		boa->name_server_port = name_server_port;
	
	boa->OAid = t_malloc(char, strlen(OAid) + 1);
	if (!boa->OAid)
		goto err2;
	strcpy(boa->OAid, OAid);
	
	if (netname_look_up(name_server_port, "",
			    boa->OAid, &boa->name_server_port)
	    != KERN_SUCCESS)
		goto err3;
	
	boa->obj_count = 0;
	boa->objs = 0;
	boa->names = 0;
	boa->types = 0;
	boa->impls = 0;
	boa->orb = ths;
	
	new_boas = t_realloc(ths->boas, CORBA_BOA, ths->OA_count + 1);
	if (!new_boas)
		goto err3;
	ths->boas = new_boas;
	ths->boas[ths->OA_count++] = boa;
	
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return boa;
	
  err3:
	free(boa->OAid);
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
	
	/*****/
	
	/* No ORB --> no BOA --> no `CORBA_BOA_set_exception'. */
	ev->_major = CORBA_NO_EXCEPTION;
	
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
		if (!strcmp(argv[i], "-OAid") &&
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
	
	return CORBA_ORB_BOA_init_internal(ths, OAid, ev);
}

CORBA_char *CORBA_ORB_object_to_string(CORBA_ORB orb,
				       CORBA_Object obj,
				       CORBA_Environment *ev)
{
	char buf[1024], *res;
	unsigned int i, j, obj_ref;
	CORBA_BOA boa;
	
	if (!obj) {
		res = t_malloc(char, strlen("miop:1.0//") + 1);
		if (!res) {
			/* No object --> no BOA. */
			flick_set_exception(0 /*XXX*/, ev, ex_CORBA_NO_MEMORY,
					    0, CORBA_COMPLETED_NO);
			return 0;
		}
		strcpy(res, "miop:1.0//");
		
		/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
		ev->_major = CORBA_NO_EXCEPTION;
		
		return res;
	}

	/* need to locate the boa that this object is in */
	obj_ref = (unsigned int) -1;
	for (j = 0; j < orb->OA_count; j++) {
		boa = orb->boas[j];
		for (i = 0; i < boa->obj_count; i++) {
			if (boa->objs[i] == obj) {
				obj_ref = i;
				break;
			}
		}
	}
	/* make sure we found the object!! */
	if (obj_ref == (unsigned int) -1) {
		flick_set_exception(boa, ev, ex_CORBA_OBJECT_NOT_EXIST, 0, 0);
		return NULL;
	}
	
	sprintf(buf, "miop:1.0//%s/%s/%s",
		boa->OAid,
		boa->types[obj_ref],
		boa->names[obj_ref]);
	
	res = t_malloc(char, strlen(buf) + 1);
	if (!res) {
		flick_set_exception(boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	
	strcpy(res, buf);
	CORBA_BOA_set_exception(boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	
	return res;
}

/* extracts the data from an object string.  Returns the length of the data */
/* The semantics of this function is that it will allocate the space for the
   out strings and the caller will free them. */
int flick_parse_object_string(char *instr,
			      char **out_OAid,
			      char **out_obj_type,
			      char **out_obj_key,
			      unsigned *out_obj_key_len,
			      CORBA_Environment *ev) 
{
	char *OAid, *obj_type, *obj_key, *str, *orig;
	int res;
	
	if (!out_OAid || !out_obj_key || !out_obj_type)
		goto bail;
	
	if (!instr || !instr[0]) {
		/* Null string object reference */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Warning: empty object reference string.\n");
		return 0;
	}
	
	orig = str = t_malloc(char, strlen(instr) + 1);
	if (!str) {
		flick_set_exception(0 /* boa */, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	strcpy(str, instr);
	
	if (!!strncmp("miop:1.0//", str, 10)) {
		/* I have no idea what kind of object reference this is. */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: unrecognized object reference format.\n");
		free(orig);
		return 0;
	}
	
	/* skip "miop:1.0//" header */
	str += 10;
	res = 10;
	
	/* get the OAid */
	OAid = str;
	while(*str != '/' && *str != 0) {
		str++;
		res++;
	}
	if (*str == 0)
		goto bail;
	*str = 0;
	str++;
	res++;
	
	/* get the type */
	obj_type = str;

	while(*str != '/' && *str != 0) {
		str++;
		res++;
	}
	if (*str == 0)
		goto bail;
	*str = 0;
	str++;
	res++;
	
	/* get the object name */
	obj_key = str;
	
	while(*str != '/' && *str != 0) {
		str++;
		res++;
	}
	if (*str != 0)
		goto bail;
	
	*out_OAid = t_malloc(char, strlen(OAid) + 1);
	*out_obj_type = t_malloc(char, strlen(obj_type) + 1);
	*out_obj_key_len = strlen(obj_key);
	*out_obj_key = t_malloc(char, *out_obj_key_len);
	
	if (!*out_OAid || !*out_obj_type || !*out_obj_key) {
		if (*out_OAid) free(*out_OAid);
		if (*out_obj_type) free(*out_obj_type);
		if (*out_obj_key) free(*out_obj_key);
		flick_set_exception(0 /* boa */, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		free(orig);
		return 0;
	}
	
	strcpy(*out_OAid, OAid);
	strcpy(*out_obj_type, obj_type);
	memcpy(*out_obj_key, obj_key, *out_obj_key_len);
	
	CORBA_BOA_set_exception(0 /* boa */, ev, CORBA_NO_EXCEPTION, 0, 0);
	free(orig);
	return res;
  bail:
	flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
			    0, CORBA_COMPLETED_NO);
	fprintf(stderr,
		"Error: ill-formed object reference.\n");
	free(orig);
	return 0;
}

CORBA_Object flick_create_object(char *ORBid, char *OAid,
				 char *obj_type,
				 char *obj_key, unsigned obj_key_len,
				 CORBA_Environment *ev)
{
	CORBA_ORB orb;
	CORBA_BOA boa;
	CORBA_Object res;
	CORBA_Object *new_objs;
	char **new_names;
	char **new_types;
	char *new_name;
	char *new_type;
	unsigned int i;
	
	/*****/
	
	/* return a NULL object if insufficient information is given */
	if ((!obj_key) || (!obj_type)) {
		ev->_major = CORBA_NO_EXCEPTION;
		return 0;
	}
	
	orb = CORBA_ORB_init_internal(ORBid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	boa = CORBA_ORB_BOA_init_internal(orb, OAid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	for (i = 0; i < boa->obj_count; i++) {
		if (!memcmp(boa->names[i], obj_key, obj_key_len)
		    && !strcmp(boa->types[i], obj_type)) {
			/* already here! */
			return boa->objs[i];
		}
	}
	
	/* Not found, so create one... */
	new_name = t_malloc(char, obj_key_len + 1);
	new_type = t_malloc(char, strlen(obj_type) + 1);
	
	if (!new_name || !new_type) {
		if (new_name) free(new_name);
		if (new_type) free(new_type);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't realloc memory for CORBA_BOA object "
			"references.\n");
		return 0;
	}
	
	memcpy(new_name, obj_key, obj_key_len);
	strcpy(new_type, obj_type);
	
	if (netname_look_up(boa->name_server_port,
			    "" /* hostname */,
			    new_name,
			    &res)
	    != KERN_SUCCESS) {
		flick_set_exception(ths, ev, ex_CORBA_OBJECT_NOT_EXIST,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't find object from nameserver.\n");
		return 0;
	}
	
	/* Reallocate our BOA's vector of references. */
	new_objs = t_realloc(boa->objs,
			     CORBA_Object,
			     (boa->obj_count + 1));
	new_names = t_realloc(boa->names,
			      char *,
			      (boa->obj_count + 1));
	new_types = t_realloc(boa->types,
			      char *,
			      (boa->obj_count + 1));

	if (!new_objs || !new_names || !new_types) {
		if (new_objs) free(new_objs);
		if (new_names) free(new_names);
		if (new_types) free(new_types);
		free(new_name);
		free(new_type);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't realloc memory for CORBA_BOA object "
			"references.\n");
		return 0;
	}
	
	boa->objs = new_objs;
	boa->names = new_names;
	boa->types = new_types;
	
	boa->objs[boa->obj_count] = res;
	boa->names[boa->obj_count] = new_name;
	boa->types[boa->obj_count] = new_type;
	boa->obj_count++;
	
	CORBA_BOA_set_exception(0 /* boa */, ev, CORBA_NO_EXCEPTION, 0, 0);
	return res;
}

CORBA_Object CORBA_ORB_string_to_object(CORBA_ORB ths,
					CORBA_char *str,
					CORBA_Environment *ev)
{
	CORBA_Object res;
	char *obj_key = 0, *obj_type = 0;
	char *OAid = 0;
	unsigned obj_key_len;
	
	/*****/
	
	if (!ths) {
		flick_set_exception(0 /* XXX boa */, ev,
				    ex_CORBA_INV_OBJREF,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	
	/* extract the useful data from the object string */
	flick_parse_object_string(str, &OAid, &obj_type,
				  &obj_key, &obj_key_len, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return 0;
	
	/* create the object (but do not connect it yet) */
	res = flick_create_object(ths->ORBid,
				  OAid, obj_type,
				  obj_key, obj_key_len, ev);
	
	if (OAid) free(OAid); 
	if (obj_key) free(obj_key);
	if (obj_type) free(obj_type);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		return 0;
	}
	
	return res;
}

/* End of file. */
