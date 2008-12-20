/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

#ifndef __flick_pres_all_h
#define __flick_pres_all_h

/*
 * This `#include' is only for `memset', which may be output by Flick's
 * `cast_new_expr_assign_to_zero' to initialize the `_return' value to zero in
 * the case of arrays/structs/unions/named types.
 */
#include <string.h>


/******************************************************************************
 ******************************************************************************
 ****
 **** Common, presented parameter allocation functions.  See <flick/link/all.h>
 **** for shared but unpresented allocators (e.g., the `auto' allocator).
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * The `null' parameter allocator is used when no allocation/deallocation is
 * necessary.
 */
#define null_flick_alloc(a)
#define null_flick_free(a)


/******************************************************************************
 ******************************************************************************
 ****
 **** Common error handling and reporting.
 ****
 ******************************************************************************
 *****************************************************************************/

/* Dough-head error reporting */
/* These 2 includes are only for the dough-headed error reporting... */
#include <stdio.h>
#include <unistd.h>

/* Used for internal Flick errors. */
enum {
	FLICK_STATE_NONE,
	FLICK_STATE_PROLOGUE,
	FLICK_STATE_MARSHAL,
	FLICK_STATE_SEND,
	FLICK_STATE_SEND_RECEIVE,
	FLICK_STATE_FUNCTION_CALL,
	FLICK_STATE_FUNCTION_RETURN,
	FLICK_STATE_RECEIVE,
	FLICK_STATE_UNMARSHAL,
	FLICK_STATE_EPILOGUE,
	FLICK_STATE_MAX
};

struct flick_stub_state {
	int state;
	int error_number;
};

#define FLICK_STATE_TO_SERVER_START(fstate, LINKNAME) {		\
	switch((fstate).state) {				\
	case FLICK_STATE_PROLOGUE:				\
	case FLICK_STATE_RECEIVE:				\
	case FLICK_STATE_UNMARSHAL:				\
		flick_##LINKNAME##_server_end_decode();		\
	case FLICK_STATE_FUNCTION_CALL:				\
	case FLICK_STATE_FUNCTION_RETURN:			\
	case FLICK_STATE_MARSHAL:				\
	case FLICK_STATE_SEND:					\
	case FLICK_STATE_EPILOGUE:				\
		break;						\
	}							\
	switch((fstate).state) {				\
	case FLICK_STATE_PROLOGUE:				\
	case FLICK_STATE_RECEIVE:				\
	case FLICK_STATE_UNMARSHAL:				\
	case FLICK_STATE_FUNCTION_CALL:				\
	case FLICK_STATE_FUNCTION_RETURN:			\
		flick_##LINKNAME##_server_start_encode();	\
		break;						\
	case FLICK_STATE_MARSHAL:				\
		flick_##LINKNAME##_server_restart_encode();	\
		break;						\
	case FLICK_STATE_SEND:					\
	case FLICK_STATE_EPILOGUE:				\
		break;						\
	}							\
}

#define flick_stub_error(errval, _handler) {			\
	_stub_state.error_number = (errval);			\
	goto _handler;						\
}

/*
 * These `#define's must correspond to the strings found in
 * `c/pbe/lib/mu_make_error.cc'.
 */
#define FLICK_ERROR_NONE		0
#define FLICK_ERROR_CONSTANT		1
#define FLICK_ERROR_VIRTUAL_UNION	2
#define FLICK_ERROR_STRUCT_UNION	3
#define FLICK_ERROR_DECODE_SWITCH	4
#define FLICK_ERROR_COLLAPSED_UNION	5
#define FLICK_ERROR_VOID_UNION		6
#define FLICK_ERROR_COMMUNICATION	7
#define FLICK_ERROR_OUT_OF_BOUNDS	8
#define FLICK_ERROR_INVALID_TARGET	9
#define FLICK_ERROR_NO_MEMORY           10
#define FLICK_ERROR_MAX			11

#endif /* __flick_pres_all_h */

/* End of file. */

