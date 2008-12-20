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

#ifndef __flick_pres_sun_h
#define __flick_pres_sun_h

#include <flick/pres/all.h>

/* <errno.h> for `errno', used in the Sun presentation. */
#include <errno.h>
/* <stdlib.h> for `malloc' and `free', used in the Sun presentation. */
#include <stdlib.h>
/* <assert.h> for `assert', used in create_client macro. */
#include <assert.h>

/* if not working over Sun/TCP, we still need the definition of svc_req */
#ifndef __flick_link_suntcp_h
#define CLIENT __OLD_CLIENT
#include <rpc/rpc.h>
#undef CLIENT
#endif

/* may already be defined from <flick/link/suntcp.h> */
#ifndef _typedef___FLICK_SERVER_LOCATION
#define _typedef___FLICK_SERVER_LOCATION
typedef struct FLICK_SERVER_LOCATION {
	char *server_name;
	unsigned int prog_num;
	unsigned int vers_num;
} FLICK_SERVER_LOCATION;
#endif

/* FLICK_TARGET, not sun_flick_target, should be used in the link layer */
typedef struct flick_target_struct *sun_flick_target;
typedef struct flick_target_struct CLIENT;

/* This one must be used by the client to open a connection to the server */
int	flick_client_create(sun_flick_target, FLICK_SERVER_LOCATION);

#define create_client(clnt, srvr) {			\
	int success = flick_client_create(clnt, srvr);	\
	assert(success);				\
}

/* This one must be used by the client to close a connection to the server */
void	flick_client_destroy(sun_flick_target);


/* The sun allocator - BFD */
#define sun_flick_alloc(a) malloc(a)
#define sun_flick_free(a) free(a)

/* Stuff for the sun presentation */
typedef int flick_env_t;
#define flick_errno errno

#define flick_sun_init_environment()		\
	flick_errno = 0;

/* The sun exceptions don't contain any data - XXX */
#define flick_sun_encode_system_exception(errval, _enc_name,		\
					  _link_name, _onerror) {	\
	/* Avoid `gcc' warnings about _onerror label not being used. */	\
	if (0) goto _onerror;						\
}

#define flick_sun_decode_system_exception(errval, _enc_name,		\
					  _link_name, _onerror) {	\
	/* Avoid `gcc' warnings about _onerror label not being used. */	\
	if (0) goto _onerror;						\
}

#define flick_sun_server_error(_enc_name, _link_name, _onerror, _finish)\
{									\
	FLICK_STATE_TO_SERVER_START(_stub_state,			\
			    _link_name);				\
	flick_##_link_name##_encode_new_glob_plain(4, _onerror);	\
	flick_##_link_name##_encode_new_chunk_plain(4);			\
	flick_##_enc_name##_encode_signed32(0, -1, unsigned int);	\
	flick_##_link_name##_encode_end_chunk_plain(4);			\
	flick_##_link_name##_encode_end_glob_plain(4);			\
	flick_##_link_name##_server_end_encode();			\
	goto _finish;							\
}

#define FLICK_ERROR_TO_SUN_CLIENT_ERROR(fstate)	\
{						\
	switch(fstate.error_number) {		\
	case FLICK_ERROR_CONSTANT:		\
	case FLICK_ERROR_VIRTUAL_UNION:		\
	case FLICK_ERROR_STRUCT_UNION:		\
	case FLICK_ERROR_DECODE_SWITCH:		\
	case FLICK_ERROR_COLLAPSED_UNION:	\
	case FLICK_ERROR_VOID_UNION:		\
	case FLICK_ERROR_OUT_OF_BOUNDS:		\
	case FLICK_ERROR_INVALID_TARGET:	\
		errno = EINVAL;			\
		break;				\
	case FLICK_ERROR_COMMUNICATION:		\
		errno = EIO;			\
		break;				\
	case FLICK_ERROR_NO_MEMORY:		\
		errno = ENOMEM;			\
		break;				\
	}					\
}

#define flick_sun_client_error(_enc_name, _link_name, _onerror) {	\
	FLICK_ERROR_TO_SUN_CLIENT_ERROR(_stub_state);			\
	goto _onerror;							\
}

/* These things are pretty brain-dead right now... */
#define flick_sun_mu_error(_enc_name, _link_name, _onerror) {	\
	FLICK_ERROR_TO_SUN_CLIENT_ERROR(_stub_state);		\
	goto _onerror;						\
}

/*****************************************************************************/

/*
 * Following are all the temporary variable macros.
 */

#define flick_sun_ptr_not_nil(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = ((o_expr) != 0)

#endif /* __flick_pres_sun_h */

/* End of file. */

