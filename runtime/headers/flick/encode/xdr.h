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

#ifndef __flick_encode_xdr_h
#define __flick_encode_xdr_h

#include <flick/encode/all.h>

/* On some systems (e.g., FreeBSD), `bcopy' is in <string.h>.   */
/* On other systems (e.g., Solaris), `bcopy' is in <strings.h>. */
#include <string.h>
#include <strings.h>

/*****************************************************************************/

#define flick_xdr_unsigned32_size 4
#define flick_xdr_signed32_size 4

/* Begin encoding macros */

/* If you're not using a 2's complement machine, write these yourself. */
#define flick_xdr_get_signed8(a) a
#define flick_xdr_get_signed16(a) a
#define flick_xdr_get_signed32(a) a

#ifdef HYPER
#define flick_xdr_get_signed64_hi(a) ((a) >> 32)
#define flick_xdr_get_signed64_lo(a) ((a) & 0xFFFFFFFF)
# define flick_xdr_get_int_hi(a) ((a) >> 32)
# define flick_xdr_get_int_lo(a) ((a) & 0xFFFFFFFFLL)
#endif

#if BYTE_ORDER == BIG_ENDIAN
# define __BYTESWAPLONG(a) a
# define __BYTESWAPSHORT(a) a

# ifdef HYPER
#  define __BYTESWAPHYPER(a) a
# endif

#else /* Little endian - we need to byte swap */

# define __BYTESWAPLONG(a) ((a >> 24) |			\
			  ((a & 0x00FF0000) >> 8 ) |	\
			  ((a & 0x0000FF00) << 8 ) |	\
			  (a << 24))
# define __BYTESWAPSHORT(a) ((a >> 8) | (a << 8))

# ifdef HYPER

#  define __BYTESWAPHYPER(a) ((a >> 56) |				\
			    ((a & 0x00FF000000000000LL) >> 40) |	\
			    ((a & 0x0000FF0000000000LL) >> 24) |	\
			    ((a & 0x000000FF00000000LL) >> 8) |		\
			    ((a & 0x00000000FF000000LL) << 8) |		\
			    ((a & 0x0000000000FF0000LL) << 24) |	\
			    ((a & 0x000000000000FF00LL) << 40) |	\
			    (a << 56))
# endif

#endif /* BYTE_ORDER == BIG_ENDIAN */

#define flick_xdr_encode_long(_ofs, _data, dest_type)		\
	*(unsigned int *) (((char *) _buf_current) + (_ofs))	\
		= __BYTESWAPLONG((unsigned int) (_data))

#define flick_xdr_encode_short(_ofs, _data, dest_type)		\
	*(unsigned int *) (((char *) _buf_current) + (_ofs))	\
		= __BYTESWAPLONG((unsigned int) (_data))

#define flick_xdr_encode_byte(_ofs, _data, dest_type)		\
	*(unsigned int *) (((char *) _buf_current) + (_ofs))	\
		= __BYTESWAPLONG((unsigned int) (_data))

#define flick_xdr_encode_boolean(_ofs, _data, dest_type)	\
	flick_xdr_encode_long(_ofs, _data, dest_type)

#define flick_xdr_encode_char8(_ofs, _data, dest_type)	\
	flick_xdr_encode_byte(_ofs, _data, dest_type)

#define flick_xdr_encode_char8_packed(_ofs, _data, dest_type)	\
	*(((char *) _buf_current) + (_ofs)) = (_data)

#define flick_xdr_encode_signed8(_ofs, _data, dest_type)		\
	flick_xdr_encode_byte(_ofs, flick_xdr_get_signed8(_data))

#define flick_xdr_encode_unsigned8(_ofs, _data, dest_type)	\
	flick_xdr_encode_byte(_ofs, _data, dest_type)

#define flick_xdr_encode_signed16(_ofs, _data, dest_type)	\
	flick_xdr_encode_short(_ofs,				\
			       flick_xdr_get_signed16(_data),	\
			       dest_type)

#define flick_xdr_encode_unsigned16(_ofs, _data, dest_type)	\
	flick_xdr_encode_short(_ofs, _data, dest_type)

#define flick_xdr_encode_signed32(_ofs, _data, dest_type)		      \
	flick_xdr_encode_long(_ofs, flick_xdr_get_signed32(_data), dest_type)

#define flick_xdr_encode_unsigned32(_ofs, _data, dest_type)	\
	flick_xdr_encode_long(_ofs, _data, dest_type)

#define flick_xdr_encode_float32(_ofs, _data, dest_type) {	\
	union {							\
		float f;					\
		unsigned int i;					\
	} __z;							\
								\
	__z.f = (_data);					\
	flick_xdr_encode_long(_ofs, __z.i, dest_type);		\
}

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */

#define flick_xdr_encode_alloced_string(_data, _link, _size, _onerror) { \
	int _temp_iter = 0;						 \
									 \
	flick_##_link##_encode_new_glob(((_size) + 4), _onerror);	 \
	while ((_data)[_temp_iter]) {					 \
		((char *) _buf_current)[_temp_iter + 4] =		 \
			((char *) (_data))[_temp_iter];			 \
		_temp_iter++;						 \
	}								 \
	*((int *) _buf_current) = __BYTESWAPLONG(_temp_iter);		 \
	while (_temp_iter & 3)						 \
		((char *) (_buf_current))[_temp_iter++] = 0;		 \
	flick_##_link##_encode_new_chunk(_temp_iter + 4);		 \
	flick_##_link##_encode_end_chunk(_temp_iter + 4);		 \
	flick_##_link##_encode_end_glob((_size) + 4);			 \
}

#define flick_xdr_encode_longstring(_data, _dealloc, _link_name,	\
				    _size, _onerror) {			\
	int _temp_iter = 0;						\
									\
        flick_##_link_name##_encode_new_glob((_size), _onerror);	\
	flick_##_link_name##_encode_new_chunk((_size));			\
	while ((_data)[_temp_iter]) {					\
		*(((char *) _buf_current) + _temp_iter)			\
			= ((char *) (_data))[_temp_iter];		\
	_temp_iter++;							\
	}								\
	while (_temp_iter & 3)						\
		((char *) _buf_current)[_temp_iter++] = 0;		\
	flick_##_link_name##_encode_end_chunk((_size));			\
	_dealloc(_data);						\
	flick_##_link_name##_encode_end_glob((_size));			\
}

#define flick_xdr_encode_string(_data, _dealloc, _link_name,		\
				_size, _onerror) {			\
	int _temp_len;							\
									\
	for (_temp_len = 0; (_data)[_temp_len]; _temp_len++)		\
		;							\
	flick_##_link_name##_encode_new_glob(4, _onerror);		\
	flick_##_link_name##_encode_new_chunk(4);			\
	flick_xdr_encode_unsigned32(0, _temp_len, unsigned int);	\
	flick_##_link_name##_encode_end_chunk(4);			\
	flick_##_link_name##_encode_end_glob(4);			\
        _temp_len = (_temp_len + 3) & ~3;				\
	flick_xdr_encode_longstring(_data, _dealloc, _link_name,	\
				    _temp_len, _onerror);		\
}

/*
 * ***XXX*** - End of string macros (see comment above).
 */

#if BYTE_ORDER == BIG_ENDIAN

# ifdef HYPER

#  define flick_xdr_encode_signed64(_ofs, _data, dest_type)	\
 	*(hyper_t *) (((char *) _buf_current) + (_ofs))		\
		= flick_xdr_get_signed64(_data)

#  define flick_xdr_encode_unsigned64(_ofs, _data, dest_type)		\
 	*(hyper_t *) (((char *) _buf_current) + (_ofs)) = (_data)

#  define flick_xdr_encode_float64(_ofs, _data, dest_type)		\
 	*(double *) (((char *) _buf_current) + (_ofs)) = (_data)

# else

#  define flick_xdr_encode_float64(_ofs, _data, dest_type) {	\
 	union swap {						\
 		double _d;					\
 		int i[2];					\
 	} __z;							\
								\
 	__z._d = (_data);					\
 	flick_xdr_encode_long(_ofs, __z.i[0], dest_type);	\
 	flick_xdr_encode_long((_ofs) + 4, __z.i[1], dest_type);	\
}

# endif /* Hyper */

#else /* Little endian byte ordering */

# ifdef HYPER

#  define flick_xdr_encode_signed64(_ofs, _data, dest_type) {	\
	flick_xdr_encode_long(_ofs,				\
			      flick_xdr_get_signed64_hi(_data),	\
			      dest_type);			\
  	flick_xdr_encode_long((_ofs) + 4,			\
			      flick_xdr_get_signed64_lo(_data),	\
			      dest_type);			\
}

#  define flick_xdr_encode_unsigned64(_ofs, _data, dest_type) {	\
  	flick_xdr_encode_long(_ofs,				\
			      flick_xdr_get_int_hi(_data),	\
			      dest_type);			\
  	flick_xdr_encode_long((_ofs) + 4,			\
			      flick_xdr_get_int_lo(_data),	\
			      dest_type);			\
}

# endif  /* Hyper */

# define flick_xdr_encode_float64(_ofs, _data, dest_type) {	\
  	union swap {						\
  		double _d;					\
  		unsigned int i[2];				\
  	} __z;							\
  	__z._d = (_data);					\
  	flick_xdr_encode_long(_ofs, __z.i[1], dest_type);	\
  	flick_xdr_encode_long((_ofs) + 4, __z.i[0], dest_type);	\
  }

#endif /* Byte Ordering */

#define flick_xdr_encode_target(ref, _ofs) {				\
	memcpy(_buf_current, ref.header.buf_hdr,			\
	       sizeof(call_header));					\
	_buf_current							\
	       = &(((char *) _buf_current)[sizeof(call_header)]);	\
}

#define flick_xdr_encode_bcopy(_ofs, _src, _qty)	\
	(void) bcopy((void *) (_src),			\
		     (char *) _buf_current + (_ofs),	\
		     (_qty));

/* XXX */
#define flick_xdr_encode_port32(a, b, c) {}

/* End encoding section */

/* Constant conversion crap... */

#if BYTE_ORDER == BIG_ENDIAN

#define flick_xdr_make_big_endian_const_unsigned32(a) (a)

#define flick_xdr_make_big_endian_const_unsigned16(a) (a)

#else

#define flick_xdr_make_big_endian_const_unsigned32(a) swap_unsigned32(a)

#define flick_xdr_make_big_endian_const_unsigned16(a) swap_unsigned16(a)

#endif

/* End conversion crap... */

/* Decoding macros */

#define flick_xdr_decode_long(_ofs)				\
	(__BYTESWAPLONG(*(unsigned int *)			\
			(((char *) _buf_current) + (_ofs))))

#define flick_xdr_decode_short(_ofs)					\
	(__BYTESWAPSHORT(*(unsigned short *)				\
			 (((char *) _buf_current) + (_ofs) + 2)))

#define flick_xdr_decode_byte(_ofs)					\
	(*(unsigned char *) (((char *) _buf_current) + (_ofs) + 3))

#define flick_xdr_decode_boolean(_ofs, _data, dest_type)	\
	(_data) = (bool_t) flick_xdr_decode_byte(_ofs)

#define flick_xdr_decode_char8(_ofs, _data, dest_type)		\
	(_data) = (char) flick_xdr_decode_byte(_ofs)

#define flick_xdr_decode_char8_packed(_ofs, _data, dest_type)		  \
	(_data) = *((unsigned char *) (((char *) _buf_current) + (_ofs)))

#define flick_xdr_decode_signed8(_ofs, _data, dest_type)		\
	(_data) = (signed char)						\
		  flick_xdr_set_signed8(flick_xdr_decode_byte(_ofs))

#define flick_xdr_decode_unsigned8(_ofs, _data, dest_type)	\
	(_data) = (unsigned char) flick_xdr_decode_byte(_ofs)

#define flick_xdr_decode_signed16(_ofs, _data, dest_type)	\
	(_data) = (signed short) flick_xdr_decode_short(_ofs)

#define flick_xdr_decode_unsigned16(_ofs, _data, dest_type)	\
	(_data) = (unsigned short) flick_xdr_decode_short(_ofs)

#define flick_xdr_decode_signed32(_ofs, _data, dest_type)	\
	(_data) = (signed int) flick_xdr_decode_long(_ofs)

#define flick_xdr_decode_unsigned32(_ofs, _data, dest_type)	\
	(_data) = (unsigned int) flick_xdr_decode_long(_ofs)

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */
#define flick_xdr_decode_stringlen(_data, _link, _onerror) {	\
	flick_##_link##_decode_new_glob(4);			\
        flick_##_link##_check_span(4, _onerror);		\
	flick_##_link##_decode_new_chunk(4);			\
	flick_xdr_decode_unsigned32(0, _data, unsigned int);	\
	flick_##_link##_decode_end_chunk(4);			\
	flick_##_link##_decode_end_glob(4);			\
}

#define flick_xdr_decode_string_exact(_data, _link, _len) {		\
	int _temp_iter = 0;						\
									\
	flick_##_link##_decode_new_glob((_len));			\
	flick_##_link##_decode_new_chunk((_len));			\
									\
	for (; _temp_iter < (_len); ++_temp_iter)			\
		*(((char *) (_data)) + _temp_iter)			\
			= *(((char *) _buf_current) + _temp_iter);	\
	(_data)[_temp_iter] = 0;					\
									\
	flick_##_link##_decode_end_chunk((_len));			\
	flick_##_link##_decode_end_glob((_len));			\
}

#define flick_xdr_decode_alloced_string(_data, _link, _size)	\
	flick_xdr_decode_string_exact(_data, _link, _size)

#define flick_xdr_decode_longstring(_data, _alloc, _link_name, _size,	\
				    _onerror) {				\
	char *new_buf;							\
									\
	if ((new_buf = (char *) _alloc((_size) + 1)) != 0)		\
		(_data) = new_buf;					\
	else								\
		flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);	\
									\
	flick_xdr_decode_string_exact(_data, _link_name, _size);	\
}

#define flick_xdr_decode_string(_data, _alloc, _link_name, _size,	\
				_onerror)				\
        flick_xdr_decode_longstring(_data, _alloc, _link_name, _size,	\
				    _onerror)

/*
 * This is only used for `in' strings.  Since XDR doesn't encode the
 * final zero, we may or may not be able to return a pointer.  We used
 * to check for that here, but now Flick does it for us.  This macro
 * is only called when we have at least one 0-padded byte after the
 * string.
 */
#define flick_xdr_decode_auto_string(_data, _alloc, _link, _size) {	\
	flick_##_link##_decode_new_glob((_size));			\
	flick_##_link##_decode_new_chunk((_size));			\
	(_data) = (char *) (_buf_current);				\
	flick_##_link##_decode_end_chunk((_size));			\
	flick_##_link##_decode_end_glob((_size));			\
}

/*
 * ***XXX*** - End of string macros (see comment above).
 */

#ifdef HYPER

# define flick_xdr_decode_signed64(_ofs, _data, dest_type)		\
	(_data) = __BYTESWAPHYPER(*(hyper_t *)				\
				  (((char *) _buf_current) + (_ofs)))

# define flick_xdr_decode_unsigned64(_ofs, _data, dest_type)		\
	(_data) = __BYTESWAPHYPER(*(uhyper_t *)				\
				  (((char *) _buf_current) + (_ofs)))
#endif /* Hyper */

#if BYTE_ORDER == BIG_ENDIAN

# define flick_xdr_decode_float32(_ofs, _data, dest_type)		\
	(_data) = *(float *) (((char *) _buf_current) + (_ofs))

# define flick_xdr_decode_float64(_ofs, _data, dest_type) {		\
	/* (_data) = *(double *) (((char *) _buf_current) + _ofs) */	\
  	union swap {							\
  		double _d;						\
  		unsigned int i[2];					\
  	} *__z = (union swap *)&(_data);				\
	__z->i[0] = (unsigned int) flick_xdr_decode_long(_ofs);		\
  	__z->i[1] = (unsigned int) flick_xdr_decode_long((_ofs) + 4);	\
}

#else /* Byte order */

# define flick_xdr_decode_float32(_ofs, _data, dest_type) {	\
	union swap {						\
		float f;					\
		unsigned int i;					\
	} *__z = (union swap *) &(_data);			\
	__z->i = (unsigned int) flick_xdr_decode_long(_ofs);	\
}

# define flick_xdr_decode_float64(_ofs, _data, dest_type) {		\
  	union swap {							\
  		double _d;						\
  		unsigned int i[2];					\
  	} *__z = (union swap *) &(_data);				\
	__z->i[1] = (unsigned int) flick_xdr_decode_long(_ofs);		\
  	__z->i[0] = (unsigned int) flick_xdr_decode_long((_ofs) + 4);	\
}

#endif /* byte order */

#define flick_xdr_decode_msgptr(_ofs, _ptr, type)		\
	(_ptr) = (type) (((char *) _buf_current) + (_ofs))

#define flick_xdr_swap_decode_msgptr(_ofs, _ptr, type)	\
	flick_xdr_decode_msgptr((_ofs), (_ptr), type)

#define flick_xdr_decode_bcopy(_ofs, _dest, _qty)	\
	(void) bcopy(((char *) _buf_current) + (_ofs),	\
		     (void *) (_dest),			\
		     (_qty))

#define flick_xdr_swap_decode_bcopy(_ofs, _dest, _qty)	\
	flick_xdr_decode_bcopy((_ofs), (_dest), (_qty))

/* XXX */
#define flick_xdr_decode_port32(a,b,c) {}
/* End decoding section */


/*
 * Following are all the temporary variable macros.
 */

/*
 * Macro to determine the encoded length of a string.
 * For XDR, the encoded length *excludes* the terminator.
 */
#define flick_xdr_stringlen(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = strlen(o_expr)

#endif /* __flick_encode_xdr_h */

/* End of file. */

