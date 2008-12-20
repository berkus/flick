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

#ifndef __flick_encode_cdr_h
#define __flick_encode_cdr_h

#include <flick/encode/all.h>

/* On some systems (e.g., FreeBSD), `bcopy' is in <string.h>.   */
/* On other systems (e.g., Solaris), `bcopy' is in <strings.h>. */
#include <string.h>
#include <strings.h>

/*****************************************************************************/

extern char flick_is_little_endian;

#define flick_cdr_swap() (((char *)_buf_start)[6]	\
			  != flick_is_little_endian)

#define flick_cdr_unsigned32_size 4
#define flick_cdr_signed32_size 4
#define flick_cdr_swap_unsigned32_size 4
#define flick_cdr_swap_signed32_size 4

/* Begin encoding macros */

/* If you're not using a 2's complement machine, write these yourself... */

#define flick_cdr_encode(_ofs, _data, type)				\
	*((type *)(((char *)_buf_current) + (_ofs))) = (type)(_data)

#define flick_cdr_encode_char8(_ofs, _data, dest_type)			\
	flick_cdr_encode(_ofs, _data, char)
#define flick_cdr_encode_signed8(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, signed char)
#define flick_cdr_encode_unsigned8(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, unsigned char)
#define flick_cdr_encode_signed16(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, short)
#define flick_cdr_encode_unsigned16(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, signed short)
#define flick_cdr_encode_signed32(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, (_data), signed int)
#define flick_cdr_encode_unsigned32(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, unsigned int)
#define flick_cdr_encode_signed64(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, (_data), signed long long)
#define flick_cdr_encode_unsigned64(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, unsigned long long)
#define flick_cdr_encode_float32(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, float)
#define flick_cdr_encode_float64(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, double)
#define flick_cdr_swap_encode_char8(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, char)
#define flick_cdr_swap_encode_signed8(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, signed char)
#define flick_cdr_swap_encode_unsigned8(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, unsigned char)
#define flick_cdr_swap_encode_signed16(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, short)
#define flick_cdr_swap_encode_unsigned16(_ofs, _data, dest_type)	\
	flick_cdr_encode(_ofs, _data, signed short)
#define flick_cdr_swap_encode_signed32(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, (_data), signed int)
#define flick_cdr_swap_encode_unsigned32(_ofs, _data, dest_type)	\
	flick_cdr_encode(_ofs, _data, unsigned int)
#define flick_cdr_swap_encode_signed64(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, (_data), signed long long)
#define flick_cdr_swap_encode_unsigned64(_ofs, _data, dest_type)	\
	flick_cdr_encode(_ofs, _data, unsigned long long)
#define flick_cdr_swap_encode_float32(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, float)
#define flick_cdr_swap_encode_float64(_ofs, _data, dest_type)		\
	flick_cdr_encode(_ofs, _data, double)

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */
#define flick_cdr_encode_alloced_string(_data, _link,			\
					_size, _onerror)	{	\
	int _temp_iter = 0;						\
									\
        flick_##_link##_encode_new_glob((_size) + 8,_onerror);		\
	_buf_current =							\
	         (void *) ((((unsigned int) _buf_current) + 3)		\
	     	  & ~3);						\
	while((_data)[_temp_iter]) {					\
		((char *) _buf_current)[_temp_iter + 4] =		\
			((char *) (_data))[_temp_iter];			\
		_temp_iter++;						\
	}								\
	((char *) _buf_current)[_temp_iter + 4] = 0;			\
	*((int *) _buf_current) = _temp_iter + 1;			\
	flick_##_link##_encode_new_chunk(_temp_iter + 5);		\
	flick_##_link##_encode_end_chunk(_temp_iter + 5);		\
	flick_##_link##_encode_end_glob((_size) + 8);			\
}

#define flick_cdr_encode_string(_data, _dealloc, _link,			\
					_size, _onerror) {		\
	flick_cdr_encode_alloced_string(_data, ##_link, _size,		\
					_onerror);			\
	_dealloc((_data));						\
}

#define flick_cdr_encode_longstring(_data, _dealloc, _link,		\
					_size, _onerror) {		\
	int _temp_iter;							\
	flick_##_link##_encode_new_glob(_size + 4, _onerror);		\
	flick_##_link##_encode_new_chunk(_size);			\
	for (_temp_iter = 0;						\
	     (unsigned int)_temp_iter < (unsigned int)_size;		\
	     ++_temp_iter)						\
		*(((char *) _buf_current) + _temp_iter)			\
			= *(((const char *) (_data)) + _temp_iter);	\
	flick_##_link##_encode_end_chunk(_size);			\
	_dealloc((_data));						\
	flick_##_link##_encode_end_glob(_size + 4);			\
}

#define flick_cdr_swap_encode_longstring(_data, _dealloc, _link,	\
					_size, _onerror)		\
        flick_cdr_encode_longstring(_data, _dealloc, _link,		\
					_size, _onerror)
/*
 * ***XXX*** - End of string macros (see comment above).
 */

#define flick_cdr_encode_bcopy(_ofs, _src, _qty)	\
	(void) bcopy((const void *) (_src),		\
		     (char *) _buf_current + (_ofs),	\
		     (_qty))

/* End encoding section */

/* Constant conversion section */

#if BYTE_ORDER == BIG_ENDIAN

#define flick_cdr_convert_big_endian_to_local_unsigned32(a) (a)

#define flick_cdr_convert_big_endian_to_local_unsigned16(a) (a)

#define flick_cdr_swap_convert_big_endian_to_local_unsigned32(a)	\
       swap_unsigned32(a)

#define flick_cdr_swap_convert_big_endian_to_local_unsigned16(a)	\
       swap_unsigned16(a)
	       
#define flick_mem_convert_big_endian_to_local_unsigned32(a) (a)

#define flick_mem_convert_big_endian_to_local_unsigned16(a) (a)

#else

#define flick_cdr_convert_big_endian_to_local_unsigned32(a)	\
       swap_unsigned32(a)

#define flick_cdr_convert_big_endian_to_local_unsigned16(a)	\
       swap_unsigned16(a)
	       
#define flick_cdr_swap_convert_big_endian_to_local_unsigned32(a) (a)

#define flick_cdr_swap_convert_big_endian_to_local_unsigned16(a) (a)
	       
#define flick_mem_convert_big_endian_to_local_unsigned32(a)	\
       swap_unsigned32(a)

#define flick_mem_convert_big_endian_to_local_unsigned16(a)	\
       swap_unsigned16(a)
	       
#endif

/* End constant conversion section */

/* Decoding macros */

#define flick_cdr_decode(_ofs, _data, dest_type, type)			\
	(_data) = (dest_type)(*(type *)(((char *)_buf_current) + (_ofs)))

#define flick_cdr_decode_char8(_ofs, _data, dest_type)			\
	flick_cdr_decode(_ofs, _data, dest_type, char)
#define flick_cdr_decode_signed8(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, signed char)
#define flick_cdr_decode_unsigned8(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, unsigned char)
#define flick_cdr_decode_signed16(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, short)
#define flick_cdr_decode_unsigned16(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, signed short)
#define flick_cdr_decode_signed32(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, signed int)
#define flick_cdr_decode_unsigned32(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, unsigned int)
#define flick_cdr_decode_signed64(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, signed long long)
#define flick_cdr_decode_unsigned64(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, unsigned long long)
#define flick_cdr_decode_float32(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, float)
#define flick_cdr_decode_float64(_ofs, _data, dest_type)		\
	flick_cdr_decode(_ofs, _data, dest_type, double)

#define flick_cdr_swap_decode8(_ofs, _data, dest_type, type)		\
	(_data) = (dest_type)(*(type *)(((char *)_buf_current) + (_ofs)))
#define flick_cdr_swap_decode16(_ofs, _data, type, dest_type) {		\
	unsigned short __temp = (*(unsigned short *)((char *)_buf_current +\
						     (_ofs)));		\
	(_data) = (dest_type)(type)((__temp >> 8) | (__temp << 8));	\
}
#define flick_cdr_swap_decode32(_ofs, _data, dest_type, type) {		\
	unsigned int __temp = (*(unsigned int *)((char *)_buf_current +	\
						   (_ofs)));		\
	(_data) = (dest_type)((type)(((__temp >> 24) & 0x000000FFU) |	\
		       ((__temp >>  8) & 0x0000FF00U) |			\
		       ((__temp <<  8) & 0x00FF0000U) |			\
		       ((__temp << 24) & 0xFF000000U)));		\
}
#define flick_cdr_swap_decode64(_ofs, _data, dest_type, type) {	\
	flick_cdr_swap_decode32(_ofs,				\
				((unsigned int *)&(_data))[1],	\
				dest_type,			\
				unsigned int);			\
	flick_cdr_swap_decode32(_ofs+4,				\
				((unsigned int *)&(_data))[0],	\
				dest_type,			\
				unsigned int);			\
}
#define flick_cdr_swap_mem(_ofs, _data, size, dest_type, type) {	\
	type __temp = *(type *)((char *)_buf_current + (_ofs));	  	\
	char *__vals = (char *)&__temp;					\
	int __i = 0;							\
	for (; __i < size; __i++)					\
		((char *)((char *)_buf_current + (_ofs)))[size - 1 - __i]	\
			= __vals[__i];					\
	flick_cdr_decode(_ofs, _data, dest_type, type);			\
}
#define flick_cdr_swap_decode_char8(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode8(_ofs, _data, dest_type, char)
#define flick_cdr_swap_decode_signed8(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode8(_ofs, _data, dest_type, signed char)
#define flick_cdr_swap_decode_unsigned8(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode8(_ofs, _data, dest_type, unsigned char)
#define flick_cdr_swap_decode_signed16(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode16(_ofs, _data, dest_type, short)
#define flick_cdr_swap_decode_unsigned16(_ofs, _data, dest_type)	\
	flick_cdr_swap_decode16(_ofs, _data, dest_type, signed short)
#define flick_cdr_swap_decode_signed32(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode32(_ofs, _data, dest_type, signed int)
#define flick_cdr_swap_decode_unsigned32(_ofs, _data, dest_type)	\
	flick_cdr_swap_decode32(_ofs, _data, dest_type, unsigned int)
#define flick_cdr_swap_decode_signed64(_ofs, _data, dest_type)		\
	flick_cdr_swap_decode64(_ofs, _data, dest_type, signed long long)
#define flick_cdr_swap_decode_unsigned64(_ofs, _data, dest_type)	\
	flick_cdr_swap_decode64(_ofs, _data, dest_type, unsigned long long)
#define flick_cdr_swap_decode_float32(_ofs, _data, dest_type)		\
	flick_cdr_swap_mem(_ofs, _data, 4, dest_type, float)
#define flick_cdr_swap_decode_float64(_ofs, _data, dest_type)		\
	flick_cdr_swap_mem(_ofs, _data, 8, dest_type, double)
		
#define flick_cdr_decode_msgptr(_ofs, _ptr, type)	\
	(_ptr) = (type)((char *)_buf_current + (_ofs))

#define flick_cdr_swap_decode_msgptr(_ofs, _ptr, type)	\
	flick_cdr_decode_msgptr(_ofs, _ptr, type)

#define flick_cdr_decode_bcopy(_ofs, _dest, _qty)	\
	(void) bcopy((char *) _buf_current + (_ofs),	\
		     (void *) (_dest),			\
		     (_qty))

#define flick_cdr_swap_decode_bcopy(_ofs, _dest, _qty)	\
	flick_cdr_decode_bcopy(_ofs, _dest, _qty)

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */
/*
 * The macros for decoding variable-length strings of characters.  These macros
 * assume that when they are invoked, there is no open glob and therefore no
 * current chunk.
 *
 * Note that the `_alloc' parameter cannot be enclosed in ()'s because we must
 * allow `_alloc' to name a macro!
 *
 * Note that the encoded string length *includes* the terminating NULL!
 */

#define flick_cdr_decode_stringlen(_data, _link, _onerror) {		\
	flick_##_link##_decode_new_glob(7);				\
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0);		\
	flick_##_link##_check_span(4, _onerror);			\
	flick_cdr_decode_unsigned32(0, _data, unsigned int);		\
	flick_##_link##_decode_end_chunk(4);				\
	flick_##_link##_decode_end_glob(7);				\
}

#define flick_cdr_decode_alloced_string(_data, _link, _size) {		\
	int _temp_iter;							\
									\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	for (_temp_iter = 0;						\
	     (unsigned int)_temp_iter < (unsigned int)_size;		\
	     ++_temp_iter)						\
		*(((char *) (_data)) + _temp_iter)			\
			= *(((char *) _buf_current) + _temp_iter);	\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

/* This is only used for 'in' strings.  Since CDR encodes the final zero,
 * we can return a pointer directly into the buffer.
 */
#define flick_cdr_decode_auto_string(_data, _alloc, _link, _size) {	\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	(_data) = (char *)(_buf_current);				\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_cdr_decode_longstring(_data, _alloc, _link,		\
					_size, _onerror) {		\
	int _temp_iter;							\
	char *new_buf;		       					\
									\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	flick_##_link##_check_span(_size, _onerror);			\
        if( (new_buf = (char *)_alloc(_size)) )				\
	        (_data) = new_buf;					\
	else								\
		flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);	\
	for (_temp_iter = 0;						\
	     (unsigned int)_temp_iter < (unsigned int)_size;		\
	     ++_temp_iter)						\
		*(((char *) (_data)) + _temp_iter)			\
			= *(((char *) _buf_current) + _temp_iter);	\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_cdr_decode_string(_data, _alloc, _link, _size, _onerror)	\
	flick_cdr_decode_longstring(_data, _alloc, _link, _size, _onerror)

#define flick_cdr_swap_decode_stringlen(_data, _link, _onerror) {	\
	flick_##_link##_decode_new_glob(7);				\
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0);		\
	flick_##_link##_check_span(4, _onerror);			\
	flick_cdr_swap_decode_unsigned32(0, _data, unsigned int);	\
	flick_##_link##_decode_end_chunk(4);				\
	flick_##_link##_decode_end_glob(7);				\
}

#define flick_cdr_swap_decode_auto_string(_data, _alloc, _link, _size) {\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	(_data) = (char *)(_buf_current);				\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_cdr_swap_decode_alloced_string(_data, _link, _size) {	\
	int _temp_iter;							\
									\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	for (_temp_iter = 0;						\
	     (unsigned int)_temp_iter < (unsigned int)_size;		\
	     ++_temp_iter)						\
		*(((char *) (_data)) + _temp_iter)			\
			= *(((char *) _buf_current) + _temp_iter);	\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_cdr_swap_decode_longstring(_data, _alloc, _link,		\
					_size, _onerror) {		\
	int _temp_iter;							\
	char *new_buf;	       						\
									\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	flick_##_link##_check_span(_size, _onerror);			\
	if( (new_buf = (char *)_alloc(_size)) )				\
	        (_data) = new_buf;					\
        else								\
		flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);	\
	for (_temp_iter = 0;						\
		     (unsigned int)_temp_iter < (unsigned int)_size;	\
	     ++_temp_iter)						\
		*((char *)(((char *) (_data)) + (_temp_iter)))		\
			= *(((char *) _buf_current) + _temp_iter);	\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_cdr_swap_decode_string(_data, _alloc, _link,		\
					_size, _onerror)		\
	flick_cdr_swap_decode_longstring((_data), _alloc, _link,	\
					_size, _onerror)		\
/*
 * ***XXX*** - End of string macros (see comment above).
 */

#ifndef TRAPEZE
#define flick_cdr_encode_IOR(_link, _data, _ref_adj, _onerror)		\
     if( flick_cdr_encode_IOR_internal(_stream,				\
				       (_data),				\
				       (#_link),			\
				       (_ref_adj)) )			\
	     flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);

#define flick_cdr_decode_IOR(_link, _data, _ref_adj)			\
	(_data) = flick_cdr_decode_IOR_internal(_stream,		\
						0 /* no swap */,	\
						(#_link),		\
						(_ref_adj))

#define flick_cdr_swap_decode_IOR(_link, _data, _ref_adj)	\
	(_data) = flick_cdr_decode_IOR_internal(_stream,	\
						1 /* swap */,	\
						(#_link),	\
						(_ref_adj))

#define flick_cdrxx_encode_IOR(_link, _data, _ref_adj, _obj_type, _onerror) { \
	CORBA::Object_ptr obj = (CORBA::Object_ptr)_data; \
	\
	if( CORBA::is_nil(obj) ) { \
		flick_##_link##_encode_new_glob(16, _onerror); \
		flick_##_link##_encode_new_chunk_align(12, 2, 0, 0); \
		flick_cdr_encode_unsigned32(0, 1, unsigned int); \
		flick_cdr_encode_char8(4, '\0', char); \
		flick_cdr_encode_unsigned32(8, 0, unsigned int); \
		flick_##_link##_encode_end_chunk(12); \
		flick_##_link##_encode_end_glob(16); \
	} else { \
		int type_id_len, host_len, encap_len, lpc; \
		TAO_Stub *iiopobj = obj->_stubobj(); \
		TAO_MProfile &mprofile = (TAO_MProfile &)iiopobj->get_base_profiles(); \
		CORBA::ULong profile_count = mprofile.profile_count(); \
		TAO_IIOP_Profile *profile; \
		\
		type_id_len = strlen(iiopobj->type_id); \
		flick_##_link##_encode_new_glob(type_id_len + 1 + 7, \
						_onerror); \
		flick_cdr_encode_string(iiopobj->type_id, \
					null_flick_free, ##_link, \
					type_id_len + 1, _onerror); \
		flick_##_link##_encode_new_chunk_align(4, 2, 0, 0); \
		flick_cdr_encode_unsigned32(0, profile_count, unsigned int); \
		flick_##_link##_encode_end_chunk(4); \
		flick_##_link##_encode_end_glob(type_id_len + 1 + 7); \
		for( CORBA::ULong i = 0; i < profile_count; i++ ) { \
			profile = ACE_dynamic_cast(TAO_IIOP_Profile *, \
						   mprofile.get_profile(i)); \
			host_len = strlen((char *)profile->host()); \
			encap_len = \
				1 + \
				1 + \
				1 + \
				1 + \
				4 + \
				host_len + 1 + \
				(~host_len & 01) + \
				2 + \
				( host_len & 02) + \
				4 + \
				profile->object_key().length(); \
			flick_##_link##_encode_new_glob(encap_len, _onerror); \
			flick_##_link##_encode_new_chunk_align(12, 2, 0, 0); \
			flick_cdr_encode_unsigned32(0, \
						    TAO_IOP_TAG_INTERNET_IOP, \
						    unsigned int); \
			flick_cdr_encode_unsigned32(4, encap_len, \
						    unsigned int); \
			flick_cdr_encode_char8(8, TAO_ENCAP_BYTE_ORDER, \
					       char); \
			flick_cdr_encode_char8(9, profile->version().major, \
					       char); \
			flick_cdr_encode_char8(10, profile->version().minor, \
					       char); \
			flick_##_link##_encode_end_chunk(12); \
			flick_cdr_encode_string(profile->host(), \
						null_flick_free, ##_link, \
						host_len + 1, _onerror); \
			flick_##_link##_encode_new_chunk_align(2, 1, 0, 0); \
			flick_cdr_encode_unsigned16(0, profile->port(), \
						    short); \
			flick_##_link##_encode_end_chunk(2); \
			flick_##_link##_encode_new_chunk_align(4 + \
							       profile-> \
							       object_key(). \
							       length(), 2, \
							       0, \
							       0); \
			flick_cdr_encode_unsigned32(0, profile->object_key(). \
						    length(), unsigned int); \
			for( lpc = 0; \
			     (unsigned int)lpc < \
				     profile->object_key().length(); \
			     lpc++ ) { \
				flick_cdr_encode_char8(lpc + 4, \
						       profile-> \
						       object_key()[lpc], \
						       char); \
			} \
			flick_##_link##_encode_end_chunk(4 + profile-> \
							 object_key(). \
							 length()); \
			flick_##_link##_encode_end_glob(encap_len); \
		} \
	} \
}

#define flick_cdrxx_decode_IOR(_link, _data, _ref_adj, _obj_type, _onerror) { \
	int type_id_len, profile_count; \
	TAO_Stub *objdata = 0; \
	CORBA::String_var type_hint; \
	int lpc; \
	\
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
	flick_##_link##_check_span(4, _onerror); \
	flick_cdr_decode_unsigned32(0, type_id_len, int); \
	flick_##_link##_decode_end_chunk(4); \
	flick_cdr_decode_string(type_hint.inout(), \
				CORBA::string_alloc, ##_link, \
				type_id_len, _onerror); \
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
	flick_##_link##_check_span(4, _onerror); \
	flick_cdr_decode_unsigned32(0, profile_count, int); \
	flick_##_link##_decode_end_chunk(4); \
	if( profile_count ) { \
		TAO_MProfile mp(profile_count); \
		for( lpc = 0; lpc < profile_count; lpc++ ) { \
			CORBA::ULong tag, encap_len, host_len, key_len; \
			char byte_order; \
			unsigned int lpc2; \
			char *host; \
			short port; \
			TAO_IIOP_Profile *profile = new TAO_IIOP_Profile(TAO_ORB_Core_instance()); \
			TAO_ObjectKey *_key; \
			TAO_GIOP_Version *_version; \
			\
			flick_##_link##_decode_new_chunk(16); \
			flick_##_link##_check_span(16, _onerror); \
			flick_cdr_decode_unsigned32(0, tag, CORBA::ULong); \
			flick_cdr_decode_unsigned32(4, encap_len, CORBA::ULong); \
			flick_cdr_decode_char8(8, byte_order, char); \
			_version = ACE_const_cast(TAO_GIOP_Version *, &profile->version()); \
			flick_cdr_decode_char8(9, _version->major, char); \
			flick_cdr_decode_char8(10, \
					       _version->minor, char); \
			flick_cdr_decode_unsigned32(12, host_len, CORBA::ULong); \
			flick_##_link##_decode_end_chunk(16); \
			flick_cdr_decode_string(host, \
						CORBA::string_alloc, ##_link, \
						host_len, _onerror); \
			flick_##_link##_decode_new_chunk_align(2, 1, 0, 0); \
  			flick_##_link##_check_span(2, _onerror); \
			flick_cdr_decode_unsigned16(0, port, short); \
			flick_##_link##_decode_end_chunk(2); \
			flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
			flick_##_link##_check_span(4, _onerror); \
			flick_cdr_decode_unsigned32(0, key_len, CORBA::ULong); \
			flick_##_link##_decode_end_chunk(4); \
			profile->port(port); \
			profile->host(host); \
			ACE_INET_Addr *_addr = ACE_dynamic_cast(ACE_INET_Addr *, \
								(class ACE_INET_Addr *)&profile->object_addr()); \
			_addr->set(port, host); \
			_key = ACE_const_cast(TAO_ObjectKey *, &profile->object_key()); \
			_key->length(key_len); \
			flick_##_link##_decode_new_chunk(key_len); \
			flick_##_link##_check_span(key_len, _onerror); \
			for( lpc2 = 0; lpc2 < key_len; lpc2++ ) { \
				flick_cdr_decode_char8(lpc2, \
						       (*_key)[lpc2], char); \
			} \
			flick_##_link##_decode_end_chunk(key_len); \
			CORBA::string_free(host); \
			mp.give_profile(profile); \
		} \
		objdata = new TAO_Stub(type_hint._retn(), mp, TAO_ORB_Core_instance()); \
		if( objdata == 0 ) \
  			printf( "no obj!\n" );  \
		CORBA_Object *corba_proxy = 0; \
		TAO_ServantBase *servant = 0; \
		TAO_SERVANT_LOCATION servant_location = \
			TAO_ORB_Core_instance()->orb()-> \
			_get_collocated_servant(objdata, servant); \
		int _colloc = 0; \
		if (servant_location != TAO_SERVANT_NOT_FOUND) \
			_colloc = 1; \
		corba_proxy = new CORBA_Object(objdata, servant, _colloc); \
		if( corba_proxy ) { \
			(_data) = _obj_type::_narrow(corba_proxy); \
			CORBA::release(corba_proxy); \
		} else { \
			(_data) = _obj_type::_nil(); \
		} \
	} else { \
		(_data) = _obj_type::_nil(); \
	} \
}

#define flick_cdr_swapxx_decode_IOR(_link, _data, _ref_adj, _obj_type, _onerror) { \
	int type_id_len, profile_count; \
	TAO_Stub *objdata = 0; \
	CORBA::String_var type_hint; \
	int lpc; \
	\
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
	flick_##_link##_check_span(4, _onerror); \
	flick_cdr_swap_decode_unsigned32(0, type_id_len, int); \
	flick_##_link##_decode_end_chunk(4); \
	flick_cdr_swap_decode_string(type_hint.inout(), \
				CORBA::string_alloc, ##_link, \
				type_id_len, _onerror); \
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
	flick_##_link##_check_span(4, _onerror); \
	flick_cdr_swap_decode_unsigned32(0, profile_count, int); \
	flick_##_link##_decode_end_chunk(4); \
	if( profile_count ) { \
		TAO_MProfile mp(profile_count); \
		for( lpc = 0; lpc < profile_count; lpc++ ) { \
			CORBA::ULong tag, encap_len, host_len, key_len; \
			char byte_order; \
			unsigned int lpc2; \
			char *host; \
			short port; \
			TAO_IIOP_Profile *profile = new TAO_IIOP_Profile(TAO_ORB_Core_instance()); \
			TAO_ObjectKey *_key; \
			TAO_GIOP_Version *_version; \
			\
			flick_##_link##_decode_new_chunk(16); \
			flick_##_link##_check_span(16, _onerror); \
			flick_cdr_swap_decode_unsigned32(0, tag, CORBA::ULong); \
			flick_cdr_swap_decode_unsigned32(4, encap_len, CORBA::ULong); \
			flick_cdr_swap_decode_char8(8, byte_order, char); \
			_version = ACE_const_cast(TAO_GIOP_Version *, &profile->version()); \
			flick_cdr_swap_decode_char8(9, _version->major, char); \
			flick_cdr_swap_decode_char8(10, \
					       _version->minor, char); \
			flick_cdr_swap_decode_unsigned32(12, host_len, CORBA::ULong); \
			flick_##_link##_decode_end_chunk(16); \
			flick_cdr_swap_decode_string(host, \
						CORBA::string_alloc, ##_link, \
						host_len, _onerror); \
			flick_##_link##_decode_new_chunk_align(2, 1, 0, 0); \
  			flick_##_link##_check_span(2, _onerror); \
			flick_cdr_swap_decode_unsigned16(0, port, short); \
			flick_##_link##_decode_end_chunk(2); \
			flick_##_link##_decode_new_chunk_align(4, 2, 0, 0); \
			flick_##_link##_check_span(4, _onerror); \
			flick_cdr_swap_decode_unsigned32(0, key_len, CORBA::ULong); \
			flick_##_link##_decode_end_chunk(4); \
			profile->port(port); \
			profile->host(host); \
			ACE_INET_Addr *_addr = ACE_dynamic_cast(ACE_INET_Addr *, \
								(class ACE_INET_Addr *)&profile->object_addr()); \
			_addr->set(port, host); \
			_key = ACE_const_cast(TAO_ObjectKey *, &profile->object_key()); \
			_key->length(key_len); \
			flick_##_link##_decode_new_chunk(key_len); \
			flick_##_link##_check_span(key_len, _onerror); \
			for( lpc2 = 0; lpc2 < key_len; lpc2++ ) { \
				flick_cdr_swap_decode_char8(lpc2, \
						       (*_key)[lpc2], char); \
			} \
			flick_##_link##_decode_end_chunk(key_len); \
			CORBA::string_free(host); \
			mp.give_profile(profile); \
		} \
		objdata = new TAO_Stub(type_hint._retn(), mp, TAO_ORB_Core_instance()); \
		if( objdata == 0 ) \
  			printf( "no obj!\n" );  \
		CORBA_Object *corba_proxy = 0; \
		TAO_ServantBase *servant = 0; \
		TAO_SERVANT_LOCATION servant_location = \
			TAO_ORB_Core_instance()->orb()-> \
			_get_collocated_servant(objdata, servant); \
		int _colloc = 0; \
		if (servant_location != TAO_SERVANT_NOT_FOUND) \
			_colloc = 1; \
		corba_proxy = new CORBA_Object(objdata, servant, _colloc); \
		if( corba_proxy ) { \
			(_data) = _obj_type::_narrow(corba_proxy); \
			CORBA::release(corba_proxy); \
		} else { \
			(_data) = _obj_type::_nil(); \
		} \
	} else { \
		(_data) = _obj_type::_nil(); \
	} \
}

	
#endif /* ifndef TRAPEZE */

/*
 * Encoding and decoding type-tagged values, i.e., CORBA `any's.
 *
 * These macros assume that when they are invoked, there is no open glob and
 * therefore no current chunk.
 *
 * `any's are best handled by runtime library functions, and it is the back end
 * that specifies the runtime, which is tailored for a specific link layer.
 * Therefore, these `cdr' macros simply invoke the corresponding link-layer
 * macros.  This is something of a hack, since the link layer is supposed to be
 * independent of any particular data encoding, but as things work now, we need
 * to violate that independence in order to write reasonable m/u'ing runtime
 * library functions (like those that handle `any's).
 */
#define flick_cdr_encode_type_tag_and_value(_link,			\
					    _data,			\
					    _try_label,			\
					    _on_error)			\
	flick_##_link##_cdr_encode_type_tag_and_value((_data),		\
						      _try_label,	\
						      _on_error)

#define flick_cdr_decode_type_tag_and_value(_link,			\
					    _data,			\
					    _try_label,			\
					    _on_error)			\
	flick_##_link##_cdr_decode_type_tag_and_value((_data),		\
						      _try_label,	\
						      _on_error)

#define flick_cdr_swap_encode_type_tag_and_value(_link,			\
						 _data,			\
						 _try_label,		\
						 _on_error)		\
	flick_##_link##_cdr_swap_encode_type_tag_and_value((_data),	\
							   _try_label,	\
							   _on_error)

#define flick_cdr_swap_decode_type_tag_and_value(_link,			\
						 _data,			\
						 _try_label,		\
						 _on_error)		\
	flick_##_link##_cdr_swap_decode_type_tag_and_value((_data),	\
							   _try_label,	\
							   _on_error)


/*
 * Encoding and decoding type tags, i.e., `CORBA::TypeCode's.
 *
 * These macros assume that when they are invoked, there is no open glob and
 * therefore no current chunk.
 *
 * `TypeCode's are handled in the same way as `any's --- see the macros and
 * comment above.
 */
#define flick_cdr_encode_type_tag(_link, _data, _try_label, _on_error)	\
	flick_##_link##_cdr_encode_type_tag((_data),			\
					    _try_label,			\
					    _on_error)

#define flick_cdr_decode_type_tag(_link, _data, _try_label, _on_error)	\
	flick_##_link##_cdr_decode_type_tag((_data),			\
					    _try_label,			\
					    _on_error)

#define flick_cdr_swap_encode_type_tag(_link, _data, _try_label, _on_error) \
	flick_##_link##_cdr_swap_encode_type_tag((_data),		    \
						 _try_label,		    \
						 _on_error)

#define flick_cdr_swap_decode_type_tag(_link, _data, _try_label, _on_error) \
	flick_##_link##_cdr_swap_decode_type_tag((_data),		    \
						 _try_label,		    \
						 _on_error)

/* End decoding section */

/*
 * Following are all the temporary variable macros.
 */

/*
 * Macro to determine the encoded length of a string.
 * For CDR, the encoded length *includes* the terminator.
 */
#define flick_cdr_stringlen(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = strlen(o_expr) + 1

#endif /* __flick_encode_cdr_h */

/* End of file. */

