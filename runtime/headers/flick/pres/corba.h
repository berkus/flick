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

#ifndef __flick_pres_corba_h
#define __flick_pres_corba_h

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <flick/pres/all.h>

/* This is a pseudo-CORBA allocator -- It doesn't implement freeing of ports */

#define CORBA_flick_alloc(a) (void *)malloc(a)
#define CORBA_flick_free(a) free(a)

#define CORBA_free(a) free(a)
#define CORBA_alloc(a) malloc(a)

#define CORBA_object_flick_free(a) { \
	CORBA_Environment tmp_ev; \
	\
	CORBA_Object_release(a, &tmp_ev); \
}

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

/* FLICK_TARGET is defined by whatever link layer header you are using. */
typedef FLICK_TARGET CORBA_Object;

/* not implemented yet */
#ifndef _typedef___CORBA_ImplementationDef
#define _typedef___CORBA_ImplementationDef
typedef CORBA_Object CORBA_ImplementationDef;
#endif /* _typedef___CORBA_ImplementationDef */

typedef CORBA_Object CORBA_InterfaceDef;

/*
 * Special types used in our special ``decomposed stub'' presentations.
 *
 * The internal `flick' types must be defined by the link layer header.
 */
typedef FLICK_PSEUDO_CLIENT CORBA_Client;
typedef flick_invocation_id CORBA_Invocation_id;

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
typedef struct CORBA_ORB_type *CORBA_ORB;
typedef struct CORBA_BOA_type *CORBA_BOA;

#ifndef TRAPEZE
#ifndef _typedef___CORBA_ReferenceData
#define _typedef___CORBA_ReferenceData
typedef struct {
	CORBA_unsigned_long _maximum;
	CORBA_unsigned_long _length;
	CORBA_octet *_buffer;
	CORBA_boolean _release;
} CORBA_ReferenceData;
#endif /* _typedef___CORBA_ReferenceData */
#endif /* TRAPEZE */

typedef CORBA_char *CORBA_ORBid;
typedef CORBA_char *CORBA_ORB_OAid;


/*****************************************************************************/

/* These are the CORBA_exception_type constants */
typedef CORBA_unsigned_long CORBA_exception_type;
#define CORBA_NO_EXCEPTION ((CORBA_exception_type) 0)
#define CORBA_USER_EXCEPTION ((CORBA_exception_type) 1)
#define CORBA_SYSTEM_EXCEPTION ((CORBA_exception_type) 2)

/* These are the CORBA_completion_status constants */
typedef CORBA_unsigned_long CORBA_completion_status;
#define CORBA_COMPLETED_YES ((CORBA_completion_status) 0)
#define CORBA_COMPLETED_NO ((CORBA_completion_status) 1)
#define CORBA_COMPLETED_MAYBE ((CORBA_completion_status) 2)

typedef struct flick_system_exception {
	CORBA_unsigned_long minor;
	CORBA_completion_status completed;
} flick_system_exception;

typedef struct CORBA_Environment {
	CORBA_exception_type _major;
	union {
		flick_system_exception _system_except;
		void *_user_except;
	} _value;
	char *_id;
} CORBA_Environment;

/* Should only be called from Flick generated code */
#define flick_corba_init_environment() {		\
	(_ev)->_major = CORBA_NO_EXCEPTION;		\
	(_ev)->_value._system_except.minor = 0;		\
	(_ev)->_value._system_except.completed = 0;	\
	(_ev)->_id = 0;					\
}

void *CORBA_exception_value(CORBA_Environment *ev);

#define CORBA_exception_id(_ev) (_ev)->_id

/* Frees memory taken by a potential exception */
void flick_system_exception_free(void *system_except);

#define CORBA_exception_free(_ev) {					 \
	if (((_ev)->_major == CORBA_USER_EXCEPTION)			 \
	    && ((_ev)->_value._user_except)) {				 \
		CORBA_free((_ev)->_value._user_except);			 \
		(_ev)->_value._user_except = 0;				 \
	}								 \
	(_ev)->_id = 0;							 \
	(_ev)->_major = CORBA_NO_EXCEPTION;				 \
}

/* Try to match the name to the predefined system exceptions. */
char *find_system_exception_id(char *name, int size);

/* Sets a system exception without allocating memory */
#define flick_set_exception(boa, ev, exceptname, errno, completion)	\
{									\
	(ev)->_major = CORBA_SYSTEM_EXCEPTION;				\
	(ev)->_value._system_except.minor = (errno);			\
	(ev)->_value._system_except.completed = (completion);		\
	(ev)->_id = (exceptname);					\
}

#define corba_except(name)					\
typedef flick_system_exception name;				\
name *##name##__alloc();

/*
 * Here are the standard CORBA system exception.
 *
 * If you update this list, you should also update the CORBA runtime library.
 * See `corba_excepts.c'.
 */
corba_except(CORBA_UNKNOWN)
#define ex_CORBA_UNKNOWN  "IDL:omg.org/CORBA/UNKNOWN:1.0"
corba_except(CORBA_BAD_PARAM)
#define ex_CORBA_BAD_PARAM  "IDL:omg.org/CORBA/BAD_PARAM:1.0"
corba_except(CORBA_NO_MEMORY)
#define ex_CORBA_NO_MEMORY  "IDL:omg.org/CORBA/NO_MEMORY:1.0"
corba_except(CORBA_IMP_LIMIT)
#define ex_CORBA_IMP_LIMIT  "IDL:omg.org/CORBA/IMP_LIMIT:1.0"
corba_except(CORBA_COMM_FAILURE)
#define ex_CORBA_COMM_FAILURE  "IDL:omg.org/CORBA/COMM_FAILURE:1.0"
corba_except(CORBA_INV_OBJREF)
#define ex_CORBA_INV_OBJREF  "IDL:omg.org/CORBA/INV_OBJREF:1.0"
corba_except(CORBA_NO_PERMISSION)
#define ex_CORBA_NO_PERMISSION  "IDL:omg.org/CORBA/NO_PERMISSION:1.0"
corba_except(CORBA_INTERNAL)
#define ex_CORBA_INTERNAL  "IDL:omg.org/CORBA/INTERNAL:1.0"
corba_except(CORBA_MARSHAL)
#define ex_CORBA_MARSHAL  "IDL:omg.org/CORBA/MARSHAL:1.0"
corba_except(CORBA_INITIALIZE)
#define ex_CORBA_INITIALIZE  "IDL:omg.org/CORBA/INITIALIZE:1.0"
corba_except(CORBA_NO_IMPLEMENT)
#define ex_CORBA_NO_IMPLEMENT  "IDL:omg.org/CORBA/NO_IMPLEMENT:1.0"
corba_except(CORBA_BAD_TYPECODE)
#define ex_CORBA_BAD_TYPECODE  "IDL:omg.org/CORBA/BAD_TYPECODE:1.0"
corba_except(CORBA_BAD_OPERATION)
#define ex_CORBA_BAD_OPERATION  "IDL:omg.org/CORBA/BAD_OPERATION:1.0"
corba_except(CORBA_NO_RESOURCES)
#define ex_CORBA_NO_RESOURCES  "IDL:omg.org/CORBA/NO_RESOURCES:1.0"
corba_except(CORBA_NO_RESPONSE)
#define ex_CORBA_NO_RESPONSE  "IDL:omg.org/CORBA/NO_RESPONSE:1.0"
corba_except(CORBA_PERSIST_STORE)
#define ex_CORBA_PERSIST_STORE  "IDL:omg.org/CORBA/PERSIST_STORE:1.0"
corba_except(CORBA_BAD_INV_ORDER)
#define ex_CORBA_BAD_INV_ORDER  "IDL:omg.org/CORBA/BAD_INV_ORDER:1.0"
corba_except(CORBA_TRANSIENT)
#define ex_CORBA_TRANSIENT  "IDL:omg.org/CORBA/TRANSIENT:1.0"
corba_except(CORBA_FREE_MEM)
#define ex_CORBA_FREE_MEM  "IDL:omg.org/CORBA/FREE_MEM:1.0"
corba_except(CORBA_INV_IDENT)
#define ex_CORBA_INV_IDENT  "IDL:omg.org/CORBA/INV_IDENT:1.0"
corba_except(CORBA_INV_FLAG)
#define ex_CORBA_INV_FLAG  "IDL:omg.org/CORBA/INV_FLAG:1.0"
corba_except(CORBA_INTF_REPOS)
#define ex_CORBA_INTF_REPOS  "IDL:omg.org/CORBA/INTF_REPOS:1.0"
corba_except(CORBA_BAD_CONTEXT)
#define ex_CORBA_BAD_CONTEXT  "IDL:omg.org/CORBA/BAD_CONTEXT:1.0"
corba_except(CORBA_OBJ_ADAPTER)
#define ex_CORBA_OBJ_ADAPTER  "IDL:omg.org/CORBA/OBJ_ADAPTER:1.0"
corba_except(CORBA_DATA_CONVERSION)
#define ex_CORBA_DATA_CONVERSION  "IDL:omg.org/CORBA/DATA_CONVERSION:1.0"
corba_except(CORBA_OBJECT_NOT_EXIST)
#define ex_CORBA_OBJECT_NOT_EXIST  "IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0"
corba_except(CORBA_TRANSACTION_REQUIRED)
#define ex_CORBA_TRANSACTION_REQUIRED			\
	"IDL:omg.org/CORBA/TRANSACTION_REQUIRED:1.0"
corba_except(CORBA_TRANSACTION_ROLLEDBACK)
#define ex_CORBA_TRANSACTION_ROLLEDBACK			\
	"IDL:omg.org/CORBA/TRANSACTION_ROLLEDBACK:1.0"
corba_except(CORBA_INVALID_TRANSACTION)
#define ex_CORBA_INVALID_TRANSACTION			\
	"IDL:omg.org/CORBA/INVALID_TRANSACTION:1.0"

#undef corba_except

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

CORBA_ORB
CORBA_ORB_init(int *argc, char **argv,
	       CORBA_ORBid orb_identifier,
	       CORBA_Environment *ev);

CORBA_BOA
CORBA_ORB_BOA_init(CORBA_ORB ths,
		   int *argc, char **argv,
		   CORBA_ORB_OAid boa_identifier,
		   CORBA_Environment *ev);

CORBA_BOA
CORBA_ORB_BOA_create(CORBA_ORB ths,
		     CORBA_ORB_OAid OAid,
		     char *OAaddr,
		     unsigned short host_port,
		     CORBA_Environment *ev);

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
  CORBA_boolean _release;
} CORBA_sequence_string;
#endif /* _typedef___CORBA_sequence_string */

#ifndef _typedef___CORBA_ORB_ObjectIdList
#define _typedef___CORBA_ORB_ObjectIdList
typedef struct CORBA_sequence_string CORBA_ORB_ObjectIdList;
#endif /* _typedef___CORBA_ORB_ObjectIdList */

CORBA_ORB_ObjectIdList *
CORBA_ORB_list_initial_services(CORBA_ORB obj, CORBA_Environment *ev);

CORBA_Object
CORBA_ORB_resolve_initial_references(CORBA_ORB obj,
				     CORBA_ORB_ObjectId identifier,
				     CORBA_Environment *ev);

CORBA_char *
CORBA_ORB_object_to_readable_string(CORBA_ORB orb,
				    CORBA_Object obj,
				    CORBA_Environment *ev);

CORBA_char *
CORBA_ORB_object_to_string(CORBA_ORB orb,
			   CORBA_Object obj,
			   CORBA_Environment *ev);

CORBA_Object
CORBA_ORB_string_to_object(CORBA_ORB orb,
			   CORBA_char *str,
			   CORBA_Environment *ev);

/********
 * Here's the BOA stuff that applies...
 ********/

void
CORBA_BOA_set_exception(CORBA_BOA boa, CORBA_Environment *ev,
			CORBA_exception_type major,
			CORBA_char *exceptname, void *param);

CORBA_ReferenceData *
CORBA_BOA_get_id(CORBA_BOA ths, CORBA_Object obj, CORBA_Environment *ev);

#ifdef _typedef___FLICK_SERVER
/* Functions only needed on the server side */
CORBA_Object
CORBA_BOA_create(CORBA_BOA ths,
		 CORBA_ReferenceData *obj_key,
		 const char *type_id,
		 FLICK_SERVER,
		 CORBA_Environment *ev);

void
CORBA_BOA_impl_is_ready(CORBA_BOA ths, CORBA_Environment *ev);
#endif

CORBA_Client
CORBA_BOA_Client_create(CORBA_BOA ths,
			void *client_data,
			CORBA_Environment *ev);

void
CORBA_BOA_handle_replies(CORBA_BOA ths,
			 int num_handle,
			 CORBA_Environment *ev);
/*
 * I'm not doing this stuff right now:
 *
 * void CORBA_BOA_dispose(CORBA_BOA ths, CORBA_Object obj);
 */

/*
 * Here's the Object interface that applies (non-DII/DSI/IR stuff...)
 */
CORBA_boolean
CORBA_Object_is_nil(CORBA_Object ths, CORBA_Environment *ev);

CORBA_Object
CORBA_Object_duplicate(CORBA_Object ths, CORBA_Environment *ev);

void
CORBA_Object_release(CORBA_Object ths, CORBA_Environment *ev);

CORBA_unsigned_long
CORBA_Object_hash(CORBA_Object ths,
		  CORBA_unsigned_long maximum,
		  CORBA_Environment *ev);

CORBA_boolean
CORBA_Object_is_equivalent(CORBA_Object ths,
			   CORBA_Object other_object,
			   CORBA_Environment *ev);

/* XXX --- Not implemented yet.  Currently throws an exception. */
CORBA_ImplementationDef
CORBA_Object_get_implementation(CORBA_Object ths,
				CORBA_Environment *ev);

/* XXX --- Not implemented yet.  Currently throws an exception. */
CORBA_InterfaceDef
CORBA_Object_get_interface(CORBA_Object ths, CORBA_Environment *ev);

/*
 * Tests to see if an object is descended from a given type.
 */
CORBA_boolean
CORBA_Object_is_a(CORBA_Object ths,
		  CORBA_char *logical_type_id,
		  CORBA_Environment *ev);

/*
 * Returns true if the object referred to no longer exists.
 * XXX --- Currently just returns true if the reference is OBJECT_NIL.
 */
CORBA_boolean
CORBA_Object_non_existent(CORBA_Object ths, CORBA_Environment *ev);
	
/*
 * Here's the Client Object interface that applies.
 */
void *
CORBA_Client_get_data(CORBA_Client cli,
		      CORBA_Environment *ev);

void
CORBA_Client_set_data(CORBA_Client cli,
		      void *data,
		      CORBA_Environment *ev);

CORBA_Client
CORBA_Client_duplicate(CORBA_Client cli, CORBA_Environment *ev);

void
CORBA_Client_release(CORBA_Client cli, CORBA_Environment *ev);

CORBA_boolean
CORBA_Client_is_equivalent(CORBA_Client cli,
			   CORBA_Client other_cli,
			   CORBA_Environment *ev);

/* Decomposed message runtime functions. */
void CORBA_msg_wakeup(flick_msg_t msg, CORBA_Environment *ev);

/********
 * Other miscellaneous presented functions....
 ********/

void CORBA_sequence_set_release(void *seq, CORBA_boolean rel);

CORBA_boolean CORBA_sequence_get_release(void *seq);



/*****************************************************************************/

/*
 * The encode/decode system exceptions don't have to deal with the _major
 * field because it has already been marshaled/unmarshaled.
 */
#define flick_corba_decode_system_exception(loc, ENCNAME, LINKNAME, _onerror) \
{									      \
        int _temp_len;							      \
									      \
	flick_##ENCNAME##_decode_stringlen(_temp_len, LINKNAME, _onerror);    \
	flick_##LINKNAME##_check_span(_temp_len, _onerror);		      \
	flick_##ENCNAME##_decode_auto_string((loc)->_id,		      \
					     null_flick_free,		      \
					     LINKNAME,			      \
		                             _temp_len);		      \
	(loc)->_id = find_system_exception_id((loc)->_id,		      \
					      strlen((loc)->_id) + 1);	      \
	flick_##LINKNAME##_decode_new_glob(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);		      \
	flick_##LINKNAME##_decode_new_chunk_align(			      \
		(flick_##ENCNAME##_unsigned32_size) * 2, 2, 0, 0);	      \
	flick_##LINKNAME##_check_span(flick_##ENCNAME##_unsigned32_size * 2,  \
				      _onerror);			      \
	flick_##ENCNAME##_decode_unsigned32				      \
		(0, (loc)->_value._system_except.minor, unsigned int);	      \
	flick_##ENCNAME##_decode_unsigned32				      \
		((flick_##ENCNAME##_unsigned32_size),			      \
		 (loc)->_value._system_except.completed, unsigned int);	      \
	flick_##LINKNAME##_decode_end_chunk(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2);		      \
	flick_##LINKNAME##_decode_end_glob(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);		      \
}

#define flick_corba_encode_system_exception(loc, ENCNAME, LINKNAME, _onerror) \
{									      \
	flick_##ENCNAME##_encode_longstring(CORBA_exception_id(loc),	      \
					    null_flick_free, LINKNAME,	      \
					    strlen((loc)->_id),		      \
					    _onerror);			      \
	flick_##LINKNAME##_encode_new_glob(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3, _onerror);	      \
	flick_##LINKNAME##_encode_new_chunk_align(			      \
		(flick_##ENCNAME##_unsigned32_size) * 2, 2, 0, 0);	      \
	flick_##ENCNAME##_encode_unsigned32				      \
		(0, (loc)->_value._system_except.minor, unsigned int);	      \
	flick_##ENCNAME##_encode_unsigned32				      \
		((flick_##ENCNAME##_unsigned32_size),			      \
		 (loc)->_value._system_except.completed, unsigned int);	      \
	flick_##LINKNAME##_encode_end_chunk(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2);		      \
	flick_##LINKNAME##_encode_end_glob(				      \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);		      \
									      \
	CORBA_exception_free(loc);					      \
}


/* These are all the error macros */

#define FLICK_CLIENT_STATE_TO_CORBA_COMPLETE(fstate, complete) {	\
	switch(fstate.state) {						\
	case FLICK_STATE_PROLOGUE:					\
	case FLICK_STATE_MARSHAL:					\
	case FLICK_STATE_SEND:						\
		complete = CORBA_COMPLETED_NO;				\
		break;							\
	case FLICK_STATE_SEND_RECEIVE:					\
	case FLICK_STATE_RECEIVE:					\
		complete = CORBA_COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_UNMARSHAL:					\
		complete = CORBA_COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_EPILOGUE:					\
		complete = CORBA_COMPLETED_YES;				\
		break;							\
	}								\
}

#define FLICK_SERVER_STATE_TO_CORBA_COMPLETE(fstate, complete) {	\
	switch(fstate.state) {						\
	case FLICK_STATE_PROLOGUE:					\
	case FLICK_STATE_RECEIVE:					\
	case FLICK_STATE_UNMARSHAL:					\
		complete = CORBA_COMPLETED_NO;				\
		break;							\
	case FLICK_STATE_FUNCTION_CALL:					\
		complete = CORBA_COMPLETED_MAYBE;			\
		break;							\
	case FLICK_STATE_FUNCTION_RETURN:				\
	case FLICK_STATE_MARSHAL:					\
	case FLICK_STATE_SEND:						\
	case FLICK_STATE_EPILOGUE:					\
		complete = CORBA_COMPLETED_YES;				\
		break;							\
	}								\
}

#define flick_corba_server_error(ENCNAME, LINKNAME, _onerror, _finish) {    \
        int _temp_len, complete = CORBA_COMPLETED_MAYBE;		    \
	char *corba_error = flick_error_to_CORBA_exception_id(		    \
		_stub_state.error_number);				    \
	FLICK_STATE_TO_SERVER_START(_stub_state,			    \
				    LINKNAME);				    \
	FLICK_SERVER_STATE_TO_CORBA_COMPLETE(_stub_state,		    \
						complete);		    \
	flick_##LINKNAME##_encode_new_glob(				    \
		flick_##ENCNAME##_unsigned32_size, _onerror);		    \
	flick_##LINKNAME##_encode_new_chunk(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
	flick_##ENCNAME##_encode_unsigned32(0, CORBA_SYSTEM_EXCEPTION,	    \
					    unsigned int);		    \
	flick_##LINKNAME##_encode_end_chunk(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
	flick_##LINKNAME##_encode_end_glob(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
        _temp_len = strlen(corba_error);				    \
        flick_##LINKNAME##_encode_new_glob(				    \
		flick_##ENCNAME##_unsigned32_size, _onerror);		    \
        flick_##LINKNAME##_encode_new_chunk(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
        flick_##ENCNAME##_encode_unsigned32(0, _temp_len, unsigned int);    \
        flick_##LINKNAME##_encode_end_chunk(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
        flick_##LINKNAME##_encode_end_glob(				    \
		flick_##ENCNAME##_unsigned32_size);			    \
	/* XXX --- We should pass in the system exception type! */	    \
	flick_##ENCNAME##_encode_longstring(				    \
                corba_error,						    \
		null_flick_free,					    \
		LINKNAME,						    \
                _temp_len,						    \
		_onerror);						    \
	flick_##LINKNAME##_encode_new_glob(				    \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3, _onerror);	    \
	flick_##LINKNAME##_encode_new_chunk_align(8, 2, 0, 0);		    \
	/* minor code */						    \
	flick_##ENCNAME##_encode_unsigned32(0, 0, unsigned int);	    \
	flick_##ENCNAME##_encode_unsigned32(				    \
		flick_##ENCNAME##_unsigned32_size, complete, unsigned int); \
	flick_##LINKNAME##_encode_end_chunk(				    \
		(flick_##ENCNAME##_unsigned32_size) * 2);		    \
	flick_##LINKNAME##_encode_end_glob(				    \
		(flick_##ENCNAME##_unsigned32_size) * 2 + 3);		    \
	flick_##LINKNAME##_server_end_encode();				    \
	goto _finish;							    \
}

#define flick_corba_client_error(ENCNAME,			\
				 LINKNAME, _onerror)		\
{								\
	int complete = CORBA_COMPLETED_MAYBE;			\
								\
	FLICK_CLIENT_STATE_TO_CORBA_COMPLETE(			\
		_stub_state, complete);				\
	flick_set_exception(0 /*boa*/,				\
			    _ev,				\
			    flick_error_to_CORBA_exception_id(	\
				    _stub_state.		\
					error_number),		\
			    0,					\
			    complete);				\
	goto _onerror;						\
}

#define flick_corba_mu_error(ENCNAME, LINKNAME, _onerror) {	\
	goto _onerror;						\
}

#define flick_corba_msg_error(ENCNAME, LINKNAME, _onerror)	\
	flick_corba_client_error(ENCNAME, LINKNAME, _onerror)

#define flick_corba_send_error(ENCNAME, LINKNAME, _onerror)	\
	flick_corba_client_error(ENCNAME, LINKNAME, _onerror)

/*****************************************************************************/

/*
 * Following are all the temporary variable macros.
 */

#define flick_corba_ptr_not_nil(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = ((o_expr) != 0)

#endif /* __flick_pres_corba_h */

/* End of file. */

