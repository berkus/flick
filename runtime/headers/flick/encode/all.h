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

#ifndef __flick_encode_all_h
#define __flick_encode_all_h

#include <flick/encode/endian.h>

/*
 * `flick_check_byte_order' is used by the runtime in order to verify that the
 * value of `BYTE_ORDER' matches reality on the current host.
 */
extern char flick_is_little_endian;

#if BYTE_ORDER == BIG_ENDIAN
#  define flick_check_byte_order() {					     \
	if (flick_is_little_endian) {					     \
		printf(("\nBYTE_ORDER should be defined as LITTLE_ENDIAN!  " \
			"%d\n"),					     \
		       flick_is_little_endian);				     \
		exit(1);						     \
	}								     \
}
#else
#  define flick_check_byte_order() {					  \
	if (!flick_is_little_endian) {					  \
		printf(("\nBYTE_ORDER should be defined as BIG_ENDIAN!  " \
			"%d\n"),					  \
		       flick_is_little_endian);				  \
		exit(1);						  \
	}								  \
}
#endif

/******************************************************************************
 ******************************************************************************
 ****
 **** Macros used internally by the marshaling and unmarshaling macros.
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_plain_decode_unsigned32(_ofs, _data)			\
	((_data) = *(unsigned int *)(_stream->buf_current + _ofs)))

/*
 * If you're not using a 2's complement machine, write these yourself.
 */

#define swap_unsigned32(a)			\
	((((a) & 0xFF000000U) >> 24) |		\
	 (((a) & 0x00FF0000U) >> 8 ) |		\
	 (((a) & 0x0000FF00U) << 8 ) |		\
	 (((a) & 0x000000FFU) << 24))

#define swap_unsigned16(a)			\
	((((a) & 0xFF00) >> 8) |		\
	 (((a) & 0x00FF) << 8))

#endif /* __flick_encode_all_h */

/* End of file. */
	
