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

#ifndef _flick_link_all_h_
#define _flick_link_all_h_

#include <flick/link/alloca.h>

/*
 * Include <flick/pres/all.h> because the individual link-layer runtimes need
 * the definitions for error handling, e.g., the `flick_stub_state' typedef.
 *
 * XXX --- We should put the error-handling stuff somewhere else!
 */
#include <flick/pres/all.h>

/******************************************************************************
 ******************************************************************************
 ****
 **** Typedef's for internal types which Flick-generated code may use.  (See
 **** `c/libpres_c/mint_to_ctype_name.cc'.)
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * On BSD, <sys/types.h> defines `{u_,}int{8,16,32}_t' through inclusion of
 * <machine/machtypes.h>.  But there's no corresponding set of types for SysV?
 *
 * #include <sys/types.h>
 */

typedef		char		flick_signed8_t;
typedef		unsigned char	flick_unsigned8_t;
typedef		short		flick_signed16_t;
typedef		unsigned short	flick_unsigned16_t;
typedef		int		flick_signed32_t;
typedef		unsigned int	flick_unsigned32_t;

typedef		char		flick_char8_t;
/* typedef	w_char		flick_char16_t; */

typedef		float		flick_float32_t;
typedef		double		flick_float64_t;

/*
 * A server skeleton returns a `flick_operation_success_t' to indicate to the
 * runtime how a request was handled.
 */
typedef enum
{
	/* The operation did not execute properly or completely. */
	FLICK_OPERATION_FAILURE         = 1,
	
	/* The operation executed completely, and a reply must be sent. */
	FLICK_OPERATION_SUCCESS         = 2,
	
	/*
	 * The operation executed completely, but no reply should be sent
	 * (e.g., the request was a `oneway' operation).
	 */
	FLICK_OPERATION_SUCCESS_NOREPLY = 3
	
} flick_operation_success_t;


/******************************************************************************
 ******************************************************************************
 ****
 **** Macros and functions used internally by Flick-generated code.
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * The `auto' allocator is used to allocate parameter storage on the run-time
 * stack.  This allocator is used only within a server skeleton (obviously),
 * and only when the allocator for a particular bit of storage is unspecified
 * by the presentation generator --- meaning that user-written code is never
 * allowed to free or realloc or retain or otherwise manipulate the storage.
 */
#define auto_flick_alloc(a) alloca((a))
#define auto_flick_free(a)

/*
 * This is used to allocate a handle for an `in' parameter.  It is used by the
 * decomposed presentation style to keep the actual pointer to the data within
 * the message structure.
 *
 * XXX --- This uses the GNU C statement-as-expression language extension.
 */
#define msg_handle_flick_alloc(a) ({			\
	int _i = 0;					\
	void **_p = &(_msg->pvect[0]);			\
	while (*_p) {					\
		_p++;					\
		if (++_i == 7 && *_p) {			\
			_p = &(((void **) *_p)[0]);	\
			_i = 0;				\
		}					\
	}						\
	if (_i == 7) {					\
		*_p = calloc(sizeof(void *), 8);	\
		_p = &(((void **) *_p)[0]);		\
	}						\
							\
	(void *) _p;					\
})
#define msg_handle_flick_free(a)


/******************************************************************************
 ******************************************************************************
 ****
 **** Common link-layer functions and macros.
 ****
 ******************************************************************************
 *****************************************************************************/

/*
 * I hate non-POSIX `realloc's!
 */
#define nonposix_realloc(pointer, sz) \
	((pointer) ? realloc((pointer), (sz)) : malloc((sz)))

/*
 * ``Typed'' malloc, calloc, and realloc.
 */
#define t_malloc(type, count) \
	((type *) malloc((count) * sizeof(type)))

#define t_calloc(type, count) \
	((type *) calloc((count), sizeof(type)))

#define t_realloc(ptr, type, count) \
	((type *) nonposix_realloc((ptr), (count) * sizeof(type)))


/*****************************************************************************/

#endif /* _flick_link_all_h_ */

/* End of file. */

