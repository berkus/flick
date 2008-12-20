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

#define FLICK_PROTOCOL_TRAPEZE
#include "trapeze-link.h"
#include "assert.h"

/* needed only for flick_check_byte_order */
#include <flick/encode/cdr.h>

const int one = 1;
char flick_is_little_endian = -1; /* not defined yet */

int flick_seqnum = 0;

CORBA_ORB *orbs = 0;
int orb_count = 0;

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
	CORBA_ORB orb = t_calloc(struct CORBA_ORB_type, 1);
	if (!orb) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
	}
	else
		/* initialize the environment */
		CORBA_BOA_set_exception(0 /*boa*/, ev,
					CORBA_NO_EXCEPTION, 0, 0);
	/* XXX Assumes that boas is statically allocated in CORBA_ORB_type */
	assert(orb->boas);
	orb->boas[0] = 0;
	return orb;
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
	CORBA_BOA boa = 0;
	
	/* initialize the environment */
	CORBA_BOA_set_exception(boa, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	if (!OAid)
		OAid = "FlickBOA--";
	
	boa = t_calloc(struct CORBA_BOA_type, 1);
	if (!boa)
		goto err0;
	
	boa->orb = ths;
	boa->orb->boas[0] = boa;
	boa->count_servers = 0;
	boa->refs = 0;
	boa->OAid = t_malloc(char, strlen(OAid) + 1);
	if (!boa->OAid)
		goto err4;
	strcpy(boa->OAid, OAid);
	
	ths->boas[0] = boa;
	return boa;
	
  err4:
	free(boa);
  err0:
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

CORBA_char *CORBA_ORB_object_to_readable_string(CORBA_ORB orb,
						FLICK_TARGET obj,
						CORBA_Environment *ev)
{
	CORBA_char *str = (char *)malloc(50);
	sprintf(str, "%d", obj->u.info.key);
	return str;
}

/*
 * XXX --- The IIOP runtime should cache objects' IORs so that we can just
 * blast them out.
 *
 * XXX --- If there is a bug in this code, you should also check `flick_cdr_
 * encode_IOR_internal' to see if has the same bug.  Someday we should combine
 * the marshaling guts of these two functions.
 */
CORBA_char *CORBA_ORB_object_to_string(CORBA_ORB orb,
				       FLICK_TARGET obj,
				       CORBA_Environment *ev)
{
	CORBA_char *str = (char *)malloc(50);
	sprintf(str, "%d", obj->u.info.key);
	return str;
}

/* str must be an int which is the object key */
FLICK_TARGET CORBA_ORB_string_to_object(CORBA_ORB orb,
					CORBA_char *str,
					CORBA_Environment *ev)
{
	FLICK_TARGET res;
	
	assert(ev);
	if (!str || !str[0]) {
		/* Null string object reference */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Warning: empty object reference string.\n");
		return 0;
	}
	
	res = t_malloc(FLICK_TARGET_STRUCT, 1);
	if (!res) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for Object.\n");
		return 0;
	}
	
	res->dest = atoi(strtok(str,"/"));
	res->u.info.key = atoi(strtok(NULL,""));
	res->host = 1;
	if (orb)
		res->u.info.boa = orb->boas[0];
	else
		res->u.info.boa = 0;

	res->u.info.type_id = 0;
	res->u.info.type_id_len = 0;
	
	CORBA_BOA_set_exception(res->u.info.boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	
	trapeze_mcp_init(0);
	
	return res;
	
}

/* End of file. */

