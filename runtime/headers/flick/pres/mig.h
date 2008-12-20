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

#ifndef __flick_pres_mig_h
#define __flick_pres_mig_h

#include <flick/pres/all.h>
#include <mach.h>
#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <mach/message.h>

typedef boolean_t flick_server_t(mach_msg_header_t *InHeadP,
				 mach_msg_header_t *OutHeadP);


/*
 * XXX --- The function `rtSetArgDefaults' in `fe/mig/routine.c' uses these.
 * Should we just use the `local' and `remote' stuff directly?
 */
#define msgh_request_port		msgh_local_port
#define MACH_MSGH_BITS_REQUEST(bits)	MACH_MSGH_BITS_LOCAL(bits)
#define msgh_reply_port			msgh_remote_port
#define MACH_MSGH_BITS_REPLY(bits)	MACH_MSGH_BITS_REMOTE(bits)


/******************************************************************************
 ******************************************************************************
 ****
 **** Memory allocation/deallocation
 ****
 ******************************************************************************
 *****************************************************************************/

#define out_of_line_flick_free(_addr, _size) \
	mach_vm_flick_free(_addr, _size);

#define mach_vm_flick_alloc(_size)					\
(									\
	{vm_address_t __addr;						\
	if (vm_allocate(mach_task_self(), (vm_address_t *) &__addr,	\
			(_size), 1))					\
		return _return;						\
	(void*)__addr;}							\
)

#define mach_vm_flick_free(_addr, _size)				\
{									\
	if (vm_deallocate(mach_task_self(), (vm_address_t)(_addr),	\
			  (_size)))					\
		return _return;						\
}

#define auto_port_allocate(a)						\
{									\
	struct flick_port_list *_top =					\
		alloca(sizeof(struct flick_port_list));			\
	_top->next = _global_ports_to_free;				\
	_top->p = (a);							\
	_global_ports_to_free = _top;					\
}


/******************************************************************************
 ******************************************************************************
 ****
 **** Environment and error handling.
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * Map Flick internal error codes to appropriate CORBA exception IDs.  This is
 * a macro because `errval' is always a constant, so the C compiler can reduce
 * the expression to the right CORBA exception ID.
 */
#define FLICK_ERROR_TO_MIG_RETURN_CODE(_errval)				\
  (									\
   ((_errval) == FLICK_ERROR_CONSTANT)        ? MIG_BAD_ARGUMENTS :	\
   ((_errval) == FLICK_ERROR_VIRTUAL_UNION)   ? KERN_FAILURE :		\
   ((_errval) == FLICK_ERROR_STRUCT_UNION)    ? MIG_BAD_ARGUMENTS :	\
   ((_errval) == FLICK_ERROR_DECODE_SWITCH)   ? MIG_BAD_ID :		\
   ((_errval) == FLICK_ERROR_COLLAPSED_UNION) ? MIG_REPLY_MISMATCH :	\
   ((_errval) == FLICK_ERROR_VOID_UNION)      ? KERN_FAILURE :		\
   ((_errval) == FLICK_ERROR_COMMUNICATION)   ? MACH_SEND_INTERRUPTED :	\
   ((_errval) == FLICK_ERROR_OUT_OF_BOUNDS)   ? MIG_ARRAY_TOO_LARGE :	\
   ((_errval) == FLICK_ERROR_INVALID_TARGET)  ? KERN_FAILURE :          \
   ((_errval) == FLICK_ERROR_NO_MEMORY)       ? KERN_FAILURE :          \
   KERN_FAILURE								\
  )

#define flick_mig_init_environment()

#define flick_mig_encode_system_exception(loc, ENCNAME, LINKNAME,	\
						_onerror)		\
{									\
	flick_##LINKNAME##_encode_new_glob(0,_onerror);			\
	/*DO NOTHING*/							\
	flick_##LINKNAME##_encode_end_glob(0);				\
}

#define flick_mig_decode_system_exception(loc, ENCNAME, LINKNAME,	\
						_onerror)		\
{									\
	flick_##LINKNAME##_check_span(0,_onerror);			\
	/*DO NOTHING*/							\
}

/* Only works with mach3mig back-end */		       
#define flick_mig_client_error(ENCNAME, LINKNAME, _onerror) {	\
	goto _onerror;						\
}

/* Only works with mach3mig back-end */
#define flick_mig_server_error(ENCNAME, LINKNAME, _onerror, _finish) {	\
	switch( _stub_state.state ) {					\
	case FLICK_STATE_PROLOGUE:					\
	case FLICK_STATE_RECEIVE:					\
	case FLICK_STATE_UNMARSHAL: {					\
		mach_port_t _save_local_target, _save_reply_port;	\
		signed int _save_msgh_id = _buf_start->Head.msgh_id;	\
									\
		flick_##LINKNAME##_server_decode_target(		\
			_save_local_target,				\
			0, 0, LINKNAME);				\
		flick_##LINKNAME##_server_decode_client(		\
			_save_reply_port,				\
			0, 0, LINKNAME);				\
		flick_##LINKNAME##_server_start_encode();		\
		flick_##LINKNAME##_server_encode_target(		\
			_save_local_target,				\
			0, 0, LINKNAME);				\
		flick_##LINKNAME##_server_encode_client(		\
			_save_reply_port,				\
			0, 0, LINKNAME);				\
		/* A reply's ID is 100 greater than the request's */	\
		_buf_start->Head.msgh_id = _save_msgh_id + 100;		\
		_buf_start->RetCode = FLICK_ERROR_TO_MIG_RETURN_CODE(	\
			_stub_state.error_number);			\
		flick_##LINKNAME##_server_end_encode();			\
		break;							\
	}								\
	default: {							\
		flick_##LINKNAME##_server_restart_encode();		\
		_buf_start->RetCode = FLICK_ERROR_TO_MIG_RETURN_CODE(	\
			_stub_state.error_number);			\
		flick_##LINKNAME##_server_end_encode();			\
		break;							\
	}								\
	}								\
	goto _finish;							\
}

/*****************************************************************************/

/*
 * Following are all the temporary variable macros.
 */

#define flick_mig_ptr_not_nil(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = ((o_expr) != 0)

#endif /* __flick_pres_mig_h */

/* End of file. */

