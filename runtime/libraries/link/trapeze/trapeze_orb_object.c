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
#include <assert.h>
#include "trapeze-link.h"

/* needed only for flick_check_byte_order */
#include <flick/encode/cdr.h>

/* XXX - INTERFACE_DAEMON is used to specify if you want to use a
   separate interface repository daemon to answer is_a queries.  However,
   the library must also be linked with the proper client code to make
   the call to the interface daemon.  The idl file is
   runtime/daemon/ird/ird.idl and the client stuff can be generated from it.
*/
#ifdef INTERFACE_DAEMON
#include "ird-client.h"
extern CORBA_Object orb_ir_obj;
#endif

CORBA_boolean CORBA_Object_is_nil(FLICK_TARGET ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception((ths ? ths->u.info.boa : 0) /* XXX */, ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	return !ths;
}

FLICK_TARGET CORBA_Object_duplicate(FLICK_TARGET ths, CORBA_Environment *ev)
{
	FLICK_TARGET res;
	
	if (!ths) {
		/*
		 * We can't use `CORBA_BOA_set_exception' here, because our
		 * nil object doesn't have a BOA.  (Is this wrong?)
		 */
		ev->_major = CORBA_NO_EXCEPTION;
		return ths;
	}
	
	res = t_malloc(FLICK_TARGET_STRUCT, 1);
	if (!res) {			
		flick_set_exception(ths->u.info.boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0 /* nil FLICK_TARGET */;
	}
	
	res->u.info.boa = ths->u.info.boa;
	
	res->dest = ths->dest;
	res->u.info.key = ths->u.info.key;
	res->server_func = ths->server_func;
	
	res->u.info.type_id = t_malloc(char, (ths->u.info.type_id_len + 1));
	if (!res->u.info.type_id) {
		flick_set_exception(ths->u.info.boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		free(res);
		return 0 /* nil FLICK_TARGET */;
	}
	strncpy((char *) res->u.info.type_id, ths->u.info.type_id, (ths->u.info.type_id_len + 1));
	
	res->u.info.type_id_len = ths->u.info.type_id_len;
	
	CORBA_BOA_set_exception(ths->u.info.boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return res;
}

void CORBA_Object_release(FLICK_TARGET ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception((ths ? ths->u.info.boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	if (ths && ths->u.info.type_id)
		free((char *) ths->u.info.type_id);
	if (ths)
		free(ths);
}

CORBA_unsigned_long CORBA_Object_hash(FLICK_TARGET ths,
				      CORBA_unsigned_long maximum,
				      CORBA_Environment *ev)
{
	unsigned int hash = 0;
	
	if (!ths) {
		/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
		ev->_major = CORBA_NO_EXCEPTION;
		return hash;
	}
	
	hash = ths->u.info.key % maximum;
	CORBA_BOA_set_exception(ths->u.info.boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return hash;
}

CORBA_boolean CORBA_Object_is_equivalent(FLICK_TARGET ths,
					 FLICK_TARGET other_object,
					 CORBA_Environment *ev)
{
	/*
	 * XXX --- should null object references cause us to signal an error?
	 */
	CORBA_BOA_set_exception((ths ? ths->u.info.boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	if (ths == other_object)
		return 1;
	if (!ths || !other_object)
		return 0;
	if (ths->u.info.boa != other_object->u.info.boa)
		return 0;
	if (ths->server_func != other_object->server_func)
		return 0;
	if (ths->u.info.key != other_object->u.info.key)
		return 0;
	return 1;
}

/* not implemented yet. currently causes an exception */
CORBA_ImplementationDef CORBA_Object_get_implementation(FLICK_TARGET ths,
							CORBA_Environment *ev) 
{
	flick_set_exception((ths ? ths->u.info.boa : 0), ev,
			    ex_CORBA_BAD_OPERATION,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/* not implemented yet. currently causes an exception */
CORBA_InterfaceDef CORBA_Object_get_interface(FLICK_TARGET ths,
					      CORBA_Environment *ev) 
{
	flick_set_exception((ths ? ths->u.info.boa : 0), ev,
			    ex_CORBA_BAD_OPERATION,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/*
 * tests to see if an object is descended from a given type
 */
CORBA_boolean CORBA_Object_is_a_internal(FLICK_TARGET ths,
					 CORBA_char *supertype,
					 char *flick_interface_parents[],
					 CORBA_Environment *ev)
{
	CORBA_boolean retval = 0;
	
	CORBA_BOA_set_exception((ths ? ths->u.info.boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
#ifdef INTERFACE_DAEMON
	if (ths && supertype && orb_ir_obj) {
		char *type;
		
		type = alloca(strlen(ths->u.info.type_id) + strlen("IDL::1.0")
			      + 1);
		strcpy(type, "IDL:");
		/* Get the right type of id */
		flick_cpptype_to_omg(rightid, ths->type_id);
		strcat(type, ":1.0");
		retval = flick_internal_type(type, supertype);
	}
#else
	/* Make sure we have everything */
	if (ths && supertype) {
		int i;
			
		for (i = 0; flick_interface_parents[i] && !retval; i++) {
			if (!strcmp(supertype, flick_interface_parents[i]))
				retval = 1;
		}
	}
#endif
	return retval;
}

CORBA_boolean CORBA_Object_is_a(FLICK_TARGET ths,
				CORBA_char *logical_type_id,
				CORBA_Environment *ev)
{
	CORBA_BOA_set_exception((ths ? ths->u.info.boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
	/*
	 * XXX --- This implementation is very much out of date.  See the
	 * implementation in the IIOP runtime library for a more up-to-date
	 * version.
	 */
	/* Stupid test until we have figured out what to really do. */
	if (ths
	    && logical_type_id
	    && !strcmp(logical_type_id, ths->u.info.type_id)) {
		return 1;
	} else
		return 0;
}

/* returns true if the object referred to no longer exists */
/* XXX Currently just returns true if the reference is CORBA_NIL */
/* XXX needs to be implemented to use a remote call */
CORBA_boolean CORBA_Object_non_existent(FLICK_TARGET ths,
					CORBA_Environment *ev)
{
	return CORBA_Object_is_nil(ths, ev);
}

/* End of file. */

