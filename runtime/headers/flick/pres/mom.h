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

#ifndef __flick_pres_mom_h
#define __flick_pres_mom_h

#define PRESNAME mom

#include <flick/pres/all.h>

#include <oskit/c/errno.h>
#include <oskit/c/sys/types.h>

typedef struct mom_exc_t {
	int _type;
	void *_except;
} mom_exc_t;

/*
 * The Fluke allocator, invoked by the Fluke presentation generator.
 */
#define mom_flick_alloc(a) (malloc(a))
#define mom_flick_free(a) (free(a))


/********
 * Other miscellaneous presented functions....
 ********/

void mom_sequence_set_release(void *seq, oskit_bool_t rel);

oskit_bool_t mom_sequence_get_release(void *seq);


/* These are all the error macros */
#define exc_mom_no_exception (0)
#define exc_mom_system_exception (-1)

#define FLICK_ERROR_TO_MOM_ERROR(_errval)			\
  (								\
   ((_errval) == FLICK_ERROR_CONSTANT)        ? EINVAL :	\
   ((_errval) == FLICK_ERROR_VIRTUAL_UNION)   ? EINVAL :	\
   ((_errval) == FLICK_ERROR_STRUCT_UNION)    ? EINVAL :	\
   ((_errval) == FLICK_ERROR_DECODE_SWITCH)   ? ENOSYS :	\
   ((_errval) == FLICK_ERROR_COLLAPSED_UNION) ? EINVAL :	\
   ((_errval) == FLICK_ERROR_VOID_UNION)      ? EINVAL :	\
   ((_errval) == FLICK_ERROR_COMMUNICATION)   ? EIO :		\
   ((_errval) == FLICK_ERROR_OUT_OF_BOUNDS)   ? EINVAL :	\
   ((_errval) == FLICK_ERROR_NO_MEMORY)       ? ENOMEM :	\
   EIO								\
  )
#define flick_mom_init_environment()		\
{						\
	_ev->_type = exc_mom_no_exception;	\
}

#define flick_mom_encode_system_exception(loc, ENCNAME, LINKNAME, _onerror)	\
{										\
	flick_##LINKNAME##_encode_new_glob(4, _onerror);			\
	flick_##LINKNAME##_encode_new_chunk(4);					\
	flick_##ENCNAME##_encode_signed32(0, (int) ((loc)->_except), signed int);\
	flick_##LINKNAME##_encode_end_chunk(4);					\
	flick_##LINKNAME##_encode_end_glob(4);					\
}

#define flick_mom_decode_system_exception(loc, ENCNAME, LINKNAME, _onerror)	\
{										\
	flick_##LINKNAME##_decode_new_glob(4);					\
	flick_##LINKNAME##_check_span(4,_onerror);				\
	flick_##LINKNAME##_decode_new_chunk(4);					\
	flick_##ENCNAME##_decode_signed32(0, (int) ((loc)->_except), signed int);	\
	flick_##LINKNAME##_decode_end_chunk(4);					\
	flick_##LINKNAME##_decode_end_glob(4);					\
}

#define flick_mom_server_error(ENCNAME, LINKNAME, _onerror, _finish) {	\
	FLICK_STATE_TO_SERVER_START(_stub_state, LINKNAME);		\
	flick_##LINKNAME##_encode_new_glob_plain(8,_onerror);		\
	flick_##LINKNAME##_encode_new_chunk_plain(8);			\
	flick_##ENCNAME##_encode_signed32(0, exc_mom_system_exception, signed int);	\
	flick_##ENCNAME##_encode_signed32(4, (int)			\
		(FLICK_ERROR_TO_MOM_ERROR(_stub_state.			\
					error_number)), signed int);	\
	flick_##LINKNAME##_encode_end_chunk_plain(8);			\
	flick_##LINKNAME##_encode_end_glob_plain(8);			\
	flick_##LINKNAME##_server_end_encode();				\
	goto _finish;							\
}

/* The client errors are easy - just set the value, and return */
#define flick_mom_client_error(ENCNAME, LINKNAME, _onerror) {		\
	_ev->_type = exc_mom_system_exception;				\
	_ev->_except = (void *)FLICK_ERROR_TO_MOM_ERROR(		\
		_stub_state.error_number);				\
	goto _onerror;							\
}

#define flick_mom_mu_error(ENCNAME, LINKNAME, _onerror) {		\
	goto _onerror;							\
}

/*****************************************************************************/

/*
 * Following are all the temporary variable macros.
 */

#define flick_mig_ptr_not_nil(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = ((o_expr) != 0)

#endif /* __flick_pres_mom_h */

/* End of file. */
	
