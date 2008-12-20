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

#ifndef __flick_pres_corbaxx_h
#define __flick_pres_corbaxx_h

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <flick/pres/all.h>
#include <tao/corba.h>
#include <tao/IIOP_Profile.h>

/* This is a pseudo-CORBA allocator -- It doesn't implement freeing of ports */

#define CORBA_flick_alloc(a) CORBA::alloc(a)
#define CORBA_flick_free(a) CORBA::free(a)

#define CORBA_free(a) CORBA::alloc(a)
#define CORBA_alloc(a)  CORBA::free(a)

#define CORBA_string_flick_alloc(a) CORBA::string_alloc(a)
#define CORBA_string_flick_free(a) CORBA::string_free(a)

#define CORBA_seq_flick_alloc(a) operator new[] (a)
#define CORBA_seq_flick_free(a) delete [] a

#define CORBA_object_flick_free(a) CORBA::release(a)

struct flick_corbaxx_env {
	char *_id;
	void *_user_except;
};

/*
 * Here are the CORBA basic type specifiers.  See Section 17.7 of the CORBA 2.1
 * specification.
 */
typedef short CORBA_short;
typedef int CORBA_long;
typedef long long CORBA_long_long;
typedef unsigned short CORBA_unsigned_short;
typedef unsigned int CORBA_unsigned_long;
typedef unsigned long long CORBA_unsigned_long_long;
typedef float CORBA_float;
typedef double CORBA_double;
typedef long double CORBA_long_double;
typedef char CORBA_char;
/*
 * typedef wchar CORBA_wchar;
 */
typedef char CORBA_boolean;
typedef unsigned char CORBA_octet;
typedef unsigned int CORBA_enum;
/*
 * typedef struct CORBA_any {
 *         CORBA_TypeCode_type _type;
 *         void *_value;
 * } CORBA_any;
 */

/*
 * This definition is requested by CORBA V2.0 Spec in section 7.2.3
 * XXX Should it be CORBA_Object_OBJECT_NIL or CORBA_ORB_OBJECT_NIL instead?
 */   
#define OBJECT_NIL 0

/*
 * Here are other typedefs that are part of a CORBA presentation.  These are
 * not basic CORBA types; they are part of the ORB/BOA/Object interface.
 */

/*
 * The `struct CORBA_ORB_type' and `struct CORBA_BOA_type' are defined in
 * <flick/link/ORB.h>.
 */
typedef struct CORBA_BOA_type *CORBA_BOA;

#ifndef TRAPEZE
typedef struct {
	CORBA_unsigned_long _length, _maximum;
	CORBA_octet *_buffer;
} CORBA_ReferenceData;
#endif

typedef CORBA_char *CORBA_ORBid;
typedef CORBA_char *CORBA_ORB_OAid;


/*****************************************************************************/

/* Should only be called from Flick generated code */
#define flick_corbaxx_init_environment() { \
	_ev = _tao_env; \
}

void *CORBA_exception_value(CORBA_Environment *ev);

#define CORBA_exception_id(_ev) (_ev).exception_id()

/* Frees memory taken by a potential exception */
void flick_system_exception_free(void *system_except);

#define CORBA_exception_free(_ev) {					\
	_ev.clear();							\
}

/* Try to match the name to the predefined system exceptions. */
char *find_system_exception_id(char *name, int size);

/*
 * Here are the ORB/BOA user exceptions.
 */

#ifndef ex_CORBA_ORB_InvalidName
#define ex_CORBA_ORB_InvalidName "IDL:omg.org/CORBA/ORB/InvalidName:1.0"

typedef struct CORBA_ORB_InvalidName {
	/*
	 * There is no actual exception data, but a C struct must have at least
	 * one member.  So, we have a `_dummy'.
	 */
	CORBA_long _dummy;
} CORBA_ORB_InvalidName;

CORBA_ORB_InvalidName *CORBA_ORB_InvalidName__alloc();

#endif /* ex_CORBA_ORB_InvalidName */

/* Translate Flick internal errors to CORBA exception ids. */
char *flick_error_to_CORBA_exception_id(int errval);


/*****************************************************************************/

/********
 * Here's the ORB interface that applies (non-DII/DSI/IR stuff...)
 ********/

/*
 * The following typedefs are for `CORBA::ORB::list_initial_services' and
 * `CORBA::ORB::resolve_initial_references'.  See Section 5.6 of the CORBA 2.1
 * specification.
 */
#ifndef _typedef___CORBA_ORB_ObjectId
#define _typedef___CORBA_ORB_ObjectId
typedef CORBA_char *CORBA_ORB_ObjectId;
#endif /* _typedef___CORBA_ORB_ObjectId */

#ifndef _typedef___CORBA_sequence_string
#define _typedef___CORBA_sequence_string
typedef struct CORBA_sequence_string {
  CORBA_unsigned_long _maximum;
  CORBA_unsigned_long _length;
  CORBA_char **_buffer;
} CORBA_sequence_string;
#endif /* _typedef___CORBA_sequence_string */

/********
 * Here's the BOA stuff that applies...
 ********/

/*
 * Returns true if the object referred to no longer exists.
 * XXX --- Currently just returns true if the reference is OBJECT_NIL.
 */
CORBA_boolean
CORBA_Object_non_existent(CORBA_Object ths, CORBA_Environment *ev);
	

/*****************************************************************************/

#define ex_CORBA_UNKNOWN  "IDL:omg.org/CORBA/UNKNOWN:1.0"
#define ex_CORBA_BAD_PARAM  "IDL:omg.org/CORBA/BAD_PARAM:1.0"
#define ex_CORBA_NO_MEMORY  "IDL:omg.org/CORBA/NO_MEMORY:1.0"
#define ex_CORBA_IMP_LIMIT  "IDL:omg.org/CORBA/IMP_LIMIT:1.0"
#define ex_CORBA_COMM_FAILURE  "IDL:omg.org/CORBA/COMM_FAILURE:1.0"
#define ex_CORBA_INV_OBJREF  "IDL:omg.org/CORBA/INV_OBJREF:1.0"
#define ex_CORBA_NO_PERMISSION  "IDL:omg.org/CORBA/NO_PERMISSION:1.0"
#define ex_CORBA_INTERNAL  "IDL:omg.org/CORBA/INTERNAL:1.0"
#define ex_CORBA_MARSHAL  "IDL:omg.org/CORBA/MARSHAL:1.0"
#define ex_CORBA_INITIALIZE  "IDL:omg.org/CORBA/INITIALIZE:1.0"
#define ex_CORBA_NO_IMPLEMENT  "IDL:omg.org/CORBA/NO_IMPLEMENT:1.0"
#define ex_CORBA_BAD_TYPECODE  "IDL:omg.org/CORBA/BAD_TYPECODE:1.0"
#define ex_CORBA_BAD_OPERATION  "IDL:omg.org/CORBA/BAD_OPERATION:1.0"
#define ex_CORBA_NO_RESOURCES  "IDL:omg.org/CORBA/NO_RESOURCES:1.0"
#define ex_CORBA_NO_RESPONSE  "IDL:omg.org/CORBA/NO_RESPONSE:1.0"
#define ex_CORBA_PERSIST_STORE  "IDL:omg.org/CORBA/PERSIST_STORE:1.0"
#define ex_CORBA_BAD_INV_ORDER  "IDL:omg.org/CORBA/BAD_INV_ORDER:1.0"
#define ex_CORBA_TRANSIENT  "IDL:omg.org/CORBA/TRANSIENT:1.0"
#define ex_CORBA_FREE_MEM  "IDL:omg.org/CORBA/FREE_MEM:1.0"
#define ex_CORBA_INV_IDENT  "IDL:omg.org/CORBA/INV_IDENT:1.0"
#define ex_CORBA_INV_FLAG  "IDL:omg.org/CORBA/INV_FLAG:1.0"
#define ex_CORBA_INTF_REPOS  "IDL:omg.org/CORBA/INTF_REPOS:1.0"
#define ex_CORBA_BAD_CONTEXT  "IDL:omg.org/CORBA/BAD_CONTEXT:1.0"
#define ex_CORBA_OBJ_ADAPTER  "IDL:omg.org/CORBA/OBJ_ADAPTER:1.0"
#define ex_CORBA_DATA_CONVERSION  "IDL:omg.org/CORBA/DATA_CONVERSION:1.0"
#define ex_CORBA_OBJECT_NOT_EXIST  "IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0"
#define ex_CORBA_TRANSACTION_REQUIRED  "IDL:omg.org/CORBA/TRANSACTION_REQUIRED:1.0"
#define ex_CORBA_TRANSACTION_ROLLEDBACK  "IDL:omg.org/CORBA/TRANSACTION_ROLLEDBACK:1.0"
#define ex_CORBA_INVALID_TRANSACTION  "IDL:omg.org/CORBA/INVALID_TRANSACTION:1.0"

#define flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, _completed, _name) \
	if( !strcmp( _ex_id, "IDL:omg.org/CORBA/" #_name ":1.0") ) \
		_the_ev.exception(new CORBA::##_name(_minor, _completed));

#define flick_corbaxx_set_exception(_the_ev, _ex_id, _minor, _completed) { \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, UNKNOWN); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, BAD_PARAM); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, NO_MEMORY); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, IMP_LIMIT); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, COMM_FAILURE); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INV_OBJREF); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, OBJECT_NOT_EXIST); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, NO_PERMISSION); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INTERNAL); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, MARSHAL); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INITIALIZE); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, NO_IMPLEMENT); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, BAD_TYPECODE); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, BAD_OPERATION); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, NO_RESOURCES); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, NO_RESPONSE); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, PERSIST_STORE); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, BAD_INV_ORDER); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, TRANSIENT); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, FREE_MEM); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INV_IDENT); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INV_FLAG); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, INTF_REPOS); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, BAD_CONTEXT); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, OBJ_ADAPTER); \
	flick_corbaxx_cmp_ex(_the_ev, _ex_id, _minor, \
			     _completed, DATA_CONVERSION); \
}

#define flick_error_to_CORBAXX_exception_id(dest, errval) {	\
	switch(errval) {					\
	default:						\
		dest = ex_CORBA_UNKNOWN;			\
		break;						\
	case FLICK_ERROR_CONSTANT:				\
		dest = ex_CORBA_MARSHAL;			\
		break;						\
	case FLICK_ERROR_VIRTUAL_UNION:				\
		dest = ex_CORBA_UNKNOWN;			\
		break;						\
	case FLICK_ERROR_STRUCT_UNION:				\
		dest = ex_CORBA_BAD_PARAM;			\
		break;						\
	case FLICK_ERROR_DECODE_SWITCH:				\
		dest = ex_CORBA_BAD_OPERATION;			\
		break;						\
	case FLICK_ERROR_COLLAPSED_UNION:			\
		dest = ex_CORBA_BAD_OPERATION;			\
		break;						\
	case FLICK_ERROR_VOID_UNION:				\
		dest = ex_CORBA_UNKNOWN;			\
		break;						\
	case FLICK_ERROR_COMMUNICATION:				\
		dest = ex_CORBA_COMM_FAILURE;			\
		break;						\
	case FLICK_ERROR_OUT_OF_BOUNDS:				\
		dest = ex_CORBA_BAD_PARAM;			\
		break;						\
	case FLICK_ERROR_INVALID_TARGET:			\
		dest = ex_CORBA_INV_OBJREF;			\
		break;						\
	case FLICK_ERROR_NO_MEMORY:				\
		dest = ex_CORBA_NO_MEMORY;			\
		break;						\
	}							\
}

/*
 * The encode/decode system exceptions don't have to deal with the _major
 * field because it has already been marshaled/unmarshaled.
 */
#define flick_corbaxx_decode_system_exception(loc, ENCNAME, LINKNAME, _onerror) \
{										\
	int _temp_len;								\
	char *_ex_id;								\
	CORBA::ULong _minor;							\
	CORBA::CompletionStatus _completed;					\
										\
	flick_##ENCNAME##_decode_stringlen(_temp_len, LINKNAME, _onerror);	\
	flick_##LINKNAME##_check_span(_temp_len, _onerror);			\
	flick_##ENCNAME##_decode_auto_string(_ex_id,				\
					     null_flick_free,			\
					     LINKNAME,				\
		                             _temp_len);			\
	flick_##LINKNAME##_decode_new_glob(					\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);			\
	flick_##LINKNAME##_decode_new_chunk_align(				\
		(flick_##ENCNAME##_unsigned32_size) * 2, 2, 0, 0);		\
	flick_##LINKNAME##_check_span(flick_##ENCNAME##_unsigned32_size * 2, _onerror); \
	flick_##ENCNAME##_decode_unsigned32					\
		(0, _minor, unsigned int);					\
	flick_##ENCNAME##_decode_unsigned32					\
		((flick_##ENCNAME##_unsigned32_size),				\
		 _completed, CORBA::CompletionStatus);				\
	flick_##LINKNAME##_decode_end_chunk(					\
		(flick_##ENCNAME##_unsigned32_size) * 2);			\
	flick_##LINKNAME##_decode_end_glob(					\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);			\
	flick_corbaxx_set_exception((loc), _ex_id, _minor, _completed);		\
}

#define flick_corbaxx_encode_system_exception(loc, ENCNAME, LINKNAME, _onerror)	\
{										\
	flick_##ENCNAME##_encode_longstring(CORBA_exception_id(loc),		\
					    null_flick_free, LINKNAME,		\
					    strlen(CORBA_exception_id(loc)),	\
					    _onerror);				\
	flick_##LINKNAME##_encode_new_glob(					\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3, _onerror);		\
	flick_##LINKNAME##_encode_new_chunk_align(				\
		(flick_##ENCNAME##_unsigned32_size) * 2, 2, 0, 0);		\
	flick_##ENCNAME##_encode_unsigned32					\
		(0, ((CORBA_SystemException *)(loc).exception())->minor(), unsigned int);	\
	flick_##ENCNAME##_encode_unsigned32					\
		((flick_##ENCNAME##_unsigned32_size),				\
		 ((CORBA_SystemException *)(loc).exception())->completed(), unsigned int);			\
	flick_##LINKNAME##_encode_end_chunk(					\
		(flick_##ENCNAME##_unsigned32_size) * 2);			\
	flick_##LINKNAME##_encode_end_glob(					\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);			\
										\
	CORBA_exception_free(loc);						\
}


/* These are all the error macros */

#define FLICK_CLIENT_STATE_TO_CORBA_COMPLETE(fstate, complete) {	\
	switch(fstate.state) {						\
	case FLICK_STATE_PROLOGUE:					\
	case FLICK_STATE_MARSHAL:					\
	case FLICK_STATE_SEND:						\
		complete = CORBA::COMPLETED_NO;				\
		break;							\
	case FLICK_STATE_SEND_RECEIVE:					\
	case FLICK_STATE_RECEIVE:					\
		complete = CORBA::COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_UNMARSHAL:					\
		complete = CORBA::COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_EPILOGUE:					\
		complete = CORBA::COMPLETED_YES;			\
		break;							\
	}								\
}

#define FLICK_SERVER_STATE_TO_CORBAXX_COMPLETE(fstate, complete) {	\
	switch(fstate.state) {						\
	case FLICK_STATE_PROLOGUE:					\
	case FLICK_STATE_RECEIVE:					\
	case FLICK_STATE_UNMARSHAL:					\
		complete = CORBA::COMPLETED_NO;				\
		break;							\
	case FLICK_STATE_FUNCTION_CALL:					\
		complete = CORBA::COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_FUNCTION_RETURN:				\
	case FLICK_STATE_MARSHAL:					\
	case FLICK_STATE_SEND:						\
	case FLICK_STATE_EPILOGUE:					\
		complete = CORBA::COMPLETED_YES;			\
		break;							\
	}								\
}

#define flick_corbaxx_server_error(ENCNAME, LINKNAME, _onerror, _finish) {\
	unsigned int _temp_len;						\
	int complete = CORBA::COMPLETED_MAYBE;				\
	char *corba_error;						\
	flick_error_to_CORBAXX_exception_id(corba_error,		\
					    _stub_state.error_number);	\
	FLICK_STATE_TO_SERVER_START(_stub_state,			\
				    LINKNAME);				\
	FLICK_SERVER_STATE_TO_CORBAXX_COMPLETE(_stub_state,		\
						complete);		\
	flick_##LINKNAME##_encode_new_glob(				\
		flick_##ENCNAME##_unsigned32_size, _onerror);		\
	flick_##LINKNAME##_encode_new_chunk(				\
		flick_##ENCNAME##_unsigned32_size);			\
	flick_##ENCNAME##_encode_unsigned32(0, CORBA::SYSTEM_EXCEPTION, unsigned int);\
	flick_##LINKNAME##_encode_end_chunk(				\
		flick_##ENCNAME##_unsigned32_size);			\
	flick_##LINKNAME##_encode_end_glob(				\
		flick_##ENCNAME##_unsigned32_size);			\
        _temp_len = strlen(corba_error);				\
        flick_##LINKNAME##_encode_new_glob(				\
		flick_##ENCNAME##_unsigned32_size, _onerror);		\
        flick_##LINKNAME##_encode_new_chunk(				\
		flick_##ENCNAME##_unsigned32_size);			\
        flick_##ENCNAME##_encode_unsigned32(0, _temp_len, unsigned int);		\
        flick_##LINKNAME##_encode_end_chunk(				\
		flick_##ENCNAME##_unsigned32_size);			\
        flick_##LINKNAME##_encode_end_glob(				\
		flick_##ENCNAME##_unsigned32_size);			\
	/* XXX --- We should pass in the system exception type! */	\
	flick_##ENCNAME##_encode_longstring(				\
                corba_error,						\
		null_flick_free,					\
		LINKNAME,						\
                _temp_len,						\
		_onerror);						\
	flick_##LINKNAME##_encode_new_glob(				\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3, _onerror);	\
	flick_##LINKNAME##_encode_new_chunk_align(8, 2, 0, 0);		\
	flick_##ENCNAME##_encode_unsigned32(0, 0, unsigned int); /* minor code */	\
	flick_##ENCNAME##_encode_unsigned32(				\
		flick_##ENCNAME##_unsigned32_size, complete, unsigned int);		\
	flick_##LINKNAME##_encode_end_chunk(				\
		(flick_##ENCNAME##_unsigned32_size) * 2);		\
	flick_##LINKNAME##_encode_end_glob(				\
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);		\
	flick_##LINKNAME##_server_end_encode();				\
	goto _finish;							\
}

#define flick_corbaxx_client_error(ENCNAME, \
				   LINKNAME, _onerror) \
{ \
	int complete = CORBA::COMPLETED_MAYBE; \
\
	FLICK_CLIENT_STATE_TO_CORBA_COMPLETE( \
		_stub_state, complete); \
	switch(_stub_state.error_number) { \
	case FLICK_ERROR_CONSTANT: \
		_ev.exception(new CORBA::MARSHAL); \
		break; \
	case FLICK_ERROR_VIRTUAL_UNION: \
		_ev.exception(new CORBA::UNKNOWN); \
		break; \
	case FLICK_ERROR_STRUCT_UNION: \
		_ev.exception(new CORBA::BAD_PARAM); \
		break; \
	case FLICK_ERROR_DECODE_SWITCH: \
		_ev.exception(new CORBA::BAD_OPERATION); \
		break; \
	case FLICK_ERROR_COLLAPSED_UNION: \
		_ev.exception(new CORBA::BAD_OPERATION); \
		break; \
	case FLICK_ERROR_VOID_UNION: \
		_ev.exception(new CORBA::UNKNOWN); \
		break; \
	case FLICK_ERROR_COMMUNICATION: \
		_ev.exception(new CORBA::COMM_FAILURE); \
		break; \
	case FLICK_ERROR_OUT_OF_BOUNDS: \
		_ev.exception(new CORBA::BAD_PARAM); \
		break; \
	case FLICK_ERROR_INVALID_TARGET: \
		_ev.exception(new CORBA::INV_OBJREF); \
		break; \
	case FLICK_ERROR_NO_MEMORY: \
		_ev.exception(new CORBA::NO_MEMORY); \
		break; \
	default: \
		_ev.exception(new CORBA::UNKNOWN); \
		break; \
	} \
	goto _onerror; \
}

#define flick_corbaxx_mu_error(ENCNAME, LINKNAME, _onerror) {	\
	goto _onerror;						\
}

#define flick_corbaxx_msg_error(ENCNAME, LINKNAME, _onerror)	\
	flick_corba_client_error(ENCNAME, LINKNAME, _onerror)

#define flick_corbaxx_send_error(ENCNAME, LINKNAME, _onerror)	\
	flick_corba_client_error(ENCNAME, LINKNAME, _onerror)

/*****************************************************************************/

/*
 * Following are all the temporary variable macros.
 */

#define flick_corbaxx_ptr_not_nil(o_type, o_expr, t_type, t_expr) \
	(t_expr) = ((o_expr) != 0)

#define flick_corbaxx_exception_to_env(o_type, o_expr, t_type, t_expr) \
	o_expr.exception((CORBA_Exception *)t_expr._user_except)

#define flick_corbaxx_env_to_exception(o_type, o_expr,			\
					      t_type, t_expr) {		\
	t_expr._id = ACE_const_cast(char *, o_expr.exception_id());	\
	t_expr._user_except = o_expr.exception();			\
}

#define flick_corbaxx_get_major(o_type, o_expr, t_type, t_expr) \
{ \
	t_expr = CORBA::NO_EXCEPTION; \
	if( o_expr.exception() ) \
		t_expr = o_expr.exception_type(); \
}

#endif /* __flick_pres_corba_h */

/* End of file. */

