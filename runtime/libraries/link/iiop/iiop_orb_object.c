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

#include <assert.h>
#include "iiop-link.h"

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
	CORBA_BOA_set_exception((ths ? ths->boa : 0) /* XXX */, ev,
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
		flick_set_exception(ths->boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0 /* nil FLICK_TARGET */;
	}
	
	res->boa = ths->boa;
	res->server_func = ths->server_func;
	
	res->key._buffer = t_malloc(CORBA_octet, ths->key._length);
	if (!res->key._buffer) {
		flick_set_exception(ths->boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		free(res);
		return 0 /* nil FLICK_TARGET */;
	}
	res->key._maximum = ths->key._length;
	res->key._length = ths->key._length;
	memcpy(res->key._buffer, ths->key._buffer, ths->key._length);
	
	res->type_id = t_malloc(char, (ths->type_id_len + 1));
	if (!res->type_id) {
		flick_set_exception(ths->boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		free(res->key._buffer);
		free(res);
		return 0 /* nil FLICK_TARGET */;
	}
	strncpy((char *) res->type_id, ths->type_id, (ths->type_id_len + 1));
	
	res->type_id_len = ths->type_id_len;
	
	res->ior = t_malloc(char, ths->ior_len);
	if (!res->ior) {
		flick_set_exception(ths->boa, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		free((char *) res->type_id);
		free(res->key._buffer);
		free(res);
		return 0 /* nil FLICK_TARGET */;
	}
	memcpy((char *) res->ior, ths->ior, ths->ior_len);
	
	res->ior_len = ths->ior_len;
	CORBA_BOA_set_exception(ths->boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return res;
}

void CORBA_Object_release(FLICK_TARGET ths, CORBA_Environment *ev)
{
	CORBA_BOA_set_exception((ths ? ths->boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	if (ths) {
		if (ths->key._buffer)
			free(ths->key._buffer);
		if (ths->type_id)
			free((char *) ths->type_id);
		if (ths->ior)
			free((char *) ths->ior);
		free(ths);
	}
}

CORBA_unsigned_long CORBA_Object_hash(FLICK_TARGET ths,
				      CORBA_unsigned_long maximum,
				      CORBA_Environment *ev)
{
	unsigned int hash = 0;
	unsigned int i;
	
	if (!ths) {
		/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
		ev->_major = CORBA_NO_EXCEPTION;
		return hash;
	}
	
	for (i = 0; i < ths->key._length; i++) {
		hash = hash + (unsigned int)ths->key._buffer[i];
		hash = hash << 8;
		hash = hash % (maximum + 1);
	}
	CORBA_BOA_set_exception(ths->boa, ev, CORBA_NO_EXCEPTION, 0, 0);
	return hash;
}

CORBA_boolean CORBA_Object_is_equivalent(FLICK_TARGET ths,
					 FLICK_TARGET other_object,
					 CORBA_Environment *ev)
{
	unsigned int i;
	
	/*
	 * XXX --- should null object references cause us to signal an error?
	 */
	CORBA_BOA_set_exception((ths ? ths->boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
	
	if (ths == other_object)
		return 1;
	if (!ths || !other_object)
		return 0;
	if (ths->boa != other_object->boa)
		return 0;
	if (ths->server_func != other_object->server_func)
		return 0;
	if (ths->type_id_len != other_object->type_id_len)
		return 0;
	for (i = 0; i < ths->type_id_len; i++)
		if (ths->type_id[i] != other_object->type_id[i])
			return 0;
	if (ths->key._length != other_object->key._length)
		return 0;
	for (i = 0; i < ths->key._length; i++)
		if (ths->key._buffer[i] != other_object->key._buffer[i])
			return 0;
	return 1;
}

/* not implemented yet. currently causes an exception */
CORBA_ImplementationDef CORBA_Object_get_implementation(FLICK_TARGET ths,
							CORBA_Environment *ev) 
{
	flick_set_exception((ths ? (ths->boa) : 0) /*XXX*/, ev,
			    ex_CORBA_BAD_OPERATION,
			    0, CORBA_COMPLETED_NO);
	return 0;
}

/* not implemented yet. currently causes an exception */
CORBA_InterfaceDef CORBA_Object_get_interface(FLICK_TARGET ths,
					      CORBA_Environment *ev) 
{
	flick_set_exception((ths ? (ths->boa) : 0) /*XXX*/, ev,
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
	
	CORBA_BOA_set_exception((ths ? ths->boa : 0), ev,
				CORBA_NO_EXCEPTION, 0, 0);
#ifdef INTERFACE_DAEMON
	if (ths && supertype && orb_ir_obj) {
		char *type;
		
		type = alloca(strlen(ths->type_id) + strlen("IDL::1.0") + 1);
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

CORBA_boolean CORBA_Object_is_a(FLICK_TARGET _obj,
				char *logical_id,
				CORBA_Environment *_ev)
{
	FLICK_BUFFER *_stream;
	
	flick_iiop_client_start_encode();
	if (_obj && logical_id) {
		/* XXX - The rest of this was made by flick, EXCEPT
		   for checking if the object is local and then calling
		   the dispatch function directly. */
		CORBA_boolean _return;
		
		if (!strcmp(logical_id, "IDL:CORBA/Object:1.0"))
			return 1;
		_return = 0;
		{
			_stream->stub_state.state = FLICK_STATE_PROLOGUE;
			_stream->stub_state.state = FLICK_STATE_MARSHAL;
			flick_iiop_client_encode_target(_obj, 0, cdr, error);
		}
		{
			int _logical_id_string_len;
			
			flick_iiop_encode_new_glob(20, error);
			flick_iiop_encode_new_chunk(20);
			flick_cdr_encode_unsigned32(0, 6, unsigned int);
			flick_cdr_encode_char8(4, '_', char);
			flick_cdr_encode_char8(5, 'i', char);
			flick_cdr_encode_char8(6, 's', char);
			flick_cdr_encode_char8(7, '_', char);
			flick_cdr_encode_char8(8, 'a', char);
			flick_cdr_encode_char8(9, 0, char);
			/* Encode the requesting `Principal' pseudo object (security ID). */
			flick_cdr_encode_unsigned32(12, (unsigned int) 0, unsigned int);
			/* Begin encode phase on parameters */
			_logical_id_string_len = strlen(logical_id) + 1;
			flick_cdr_encode_unsigned32(16, _logical_id_string_len, unsigned int);
			flick_iiop_encode_end_chunk(20);
			flick_iiop_encode_end_glob(20);
			flick_cdr_encode_longstring(logical_id, null_flick_free, iiop, _logical_id_string_len, error);
			_stream->stub_state.state = FLICK_STATE_SEND;
		}
		flick_iiop_client_end_encode();
		
		/* XXX - The following if/else stmt was generated by hand
		   and must be updated with any updates to flick */
		/* We check if the object is local or not so that we
		   can just call the dispatch function ourselves instead
		   of trying to send a message to ourselves which would
		   never be answered since we're the server */
		if (_obj->server_func) {
			FLICK_TARGET obj;
			unsigned int request_id;
			
			obj = find_implementation(
				_obj->boa, _stream,
				&request_id, _ev);
			(*obj->server_func)(obj->boa->in, obj->boa->out,
					    request_id, obj);
		} else {
			/* We aren't the server so send it out as usual */
			flick_iiop_client_set_response_expected(1);
			if (!flick_client_send_request(_obj, _obj->boa->in))
				flick_stub_error(FLICK_ERROR_COMMUNICATION, error);
			_stream->stub_state.state = FLICK_STATE_RECEIVE;
			if (!flick_client_get_reply(_obj, _obj->boa->out))
				flick_stub_error(FLICK_ERROR_COMMUNICATION, error);
		}
		flick_iiop_client_start_decode();
		if (flick_cdr_swap()) {
			{
				_stream->stub_state.state = FLICK_STATE_UNMARSHAL;
				flick_iiop_check_span(4,error);
				flick_iiop_decode_new_glob(5);
				flick_iiop_decode_new_chunk(4);
				flick_cdr_swap_decode_signed32(0, _ev->_major, unsigned int);
				flick_iiop_decode_end_chunk(4);
				switch (_ev->_major)
					{
					case CORBA_NO_EXCEPTION:
						{
							/* Begin decode phase on parameters */
							flick_iiop_check_span(1,error);
							flick_iiop_decode_new_chunk(1);
							flick_cdr_swap_decode_unsigned8(0, _return, char);
							flick_iiop_decode_end_chunk(1);
							flick_iiop_decode_end_glob(5);
							break;
						}
					case CORBA_SYSTEM_EXCEPTION:
						{
							flick_iiop_decode_end_glob(5);
							flick_corba_decode_system_exception(_ev, cdr_swap, iiop, error);
							break;
						}
					default:
					{
						flick_stub_error(FLICK_ERROR_VOID_UNION, error);
						flick_iiop_decode_end_glob(5);
					}
					}
			}
		} else {
			{
				_stream->stub_state.state = 6;
				flick_iiop_decode_new_glob(5);
				flick_iiop_check_span(4, error);
				flick_iiop_decode_new_chunk(4);
				flick_cdr_decode_signed32(0, _ev->_major, unsigned int);
				flick_iiop_decode_end_chunk(4);
				switch (_ev->_major)
					{
					case CORBA_NO_EXCEPTION:
						{
							/* Begin decode phase on parameters */
							flick_iiop_check_span(1, error);
							flick_iiop_decode_new_chunk(1);
							flick_cdr_decode_unsigned8(0, _return, char);
							flick_iiop_decode_end_chunk(1);
							flick_iiop_decode_end_glob(5);
							break;
						}
					case CORBA_SYSTEM_EXCEPTION:
						{
							flick_iiop_decode_end_glob(5);
							flick_corba_decode_system_exception(_ev, cdr, iiop, error);
							break;
						}
					default:
						{
							flick_stub_error(FLICK_ERROR_VOID_UNION, error);
							flick_iiop_decode_end_glob(5);
						}
					}
			}
		}
		return _return;
	}
	else
		return 0;
	flick_iiop_client_end_decode();
  error:
	flick_corba_client_error(cdr, iiop, error_reaper);
  error_reaper:
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

