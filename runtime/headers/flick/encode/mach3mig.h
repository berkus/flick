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

#ifndef __flick_encode_mach3mig_h
#define __flick_encode_mach3mig_h

#include <flick/encode/all.h>

/* On some systems (e.g., FreeBSD), `bcopy' is in <string.h>.   */
/* On other systems (e.g., Solaris), `bcopy' is in <strings.h>. */
#include <string.h>
#include <strings.h>

/*****************************************************************************/

#define flick_mach3mig_unsigned32_size 8
#define flick_mach3mig_signed32_size 8

/******************************************************************************
 ******************************************************************************
 ****
 **** Encoding
 ****
 ******************************************************************************
 *****************************************************************************/

#define flick_mach3mig_encode_prim(_ofs, __data, _name, _bits, _ctype) { \
	struct st {							 \
		mach_msg_type_t _t;					 \
		_ctype _v;						 \
	} *_p								 \
		= (struct st *) (((char *) _buf_current) + (_ofs));	 \
	mach_msg_type_t _tmpl						 \
		= { (_name), (_bits), 1, 1, 0, 0 };			 \
									 \
	_p->_t = _tmpl;							 \
	_p->_v = (_ctype) (__data);					 \
}

#define flick_mach3mig_encode_prim_no_type_tag(_ofs, __data, _ctype)	     \
	*(_ctype *) (((char *) _buf_current) + (_ofs)) = ((_ctype) (__data))

#define flick_mach3mig_encode_boolean(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_BOOLEAN, 32,	\
				   flick_signed32_t)

#define flick_mach3mig_encode_char8(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_CHAR, 8,	\
				   flick_signed8_t)

#define flick_mach3mig_encode_char16(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_CHAR, 16,	\
				   flick_signed16_t)

#define flick_mach3mig_encode_signed8(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_INTEGER_8, 8,	\
				   flick_signed8_t)

#define flick_mach3mig_encode_unsigned8(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_8, 8,		\
				   flick_unsigned8_t)

#define flick_mach3mig_encode_signed16(_ofs, __data, dest_type)		\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_16, 16,	\
				   flick_signed16_t)

#define flick_mach3mig_encode_unsigned16(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_16, 16,	\
				   flick_unsigned16_t)

#define flick_mach3mig_encode_signed32(_ofs, __data, dest_type)		\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 32,	\
				   flick_signed32_t)

#define flick_mach3mig_encode_unsigned32(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 32,	\
				   flick_unsigned32_t)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_encode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_64, 64,	\
				   flick_signed64_t)
#  define flick_mach3mig_encode_unsigned64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_64, 64,	\
				   flick_unsigned64_t)
#else
#  define flick_mach3mig_encode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 64,	\
				   flick_signed64_t)
#  define flick_mach3mig_encode_unsigned64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 64,	\
				   flick_unsigned64_t)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_encode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL_32, 32,		\
				   flick_float32_t)
#else
#  define flick_mach3mig_encode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL, 32,		\
				   flick_float32_t)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_encode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL_64, 64,		\
				   flick_float64_t)
#else
#  define flick_mach3mig_encode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_encode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL, 64,		\
				   flick_float64_t)
#endif /* MACH_MSG_TYPE_REAL_64 */

/*
 * XXX --- Not currently used, since a port type_tag becomes the _port_right
 * parameter to flick_mach3mig_encode_port.
 */
#define flick_mach3mig_encode_type_tag(_ofs, _type_tag, _bits)	\
	*(mach_msg_type_t *) (((char *) _buf_current) + (_ofs))	\
		= { (_type_tag), (_bits), 1, 1, 0, 0 }

/*
 * XXX --- Not currently used, since a port type_tag becomes the _port_right
 * parameter to flick_mach3mig_encode_port.
 */
#define flick_mach3mig_encode_port_no_type_tag(_ofs, __data)		  \
	flick_mach3mig_encode_prim_no_type_tag(_ofs, __data, mach_port_t)

#define flick_mach3mig_encode_port32(_ofs, __data, _adjust, _port_right,    \
				     _onerror) {			    \
	int flick_ref_adjust = (_adjust);				    \
        /*								    \
	 * This switch is not necessary for mach, but it is an example of   \
	 * how to handle polymorphic ports in a back end that must maintain \
	 * a ref_count.							    \
	 */								    \
	if (flick_ref_adjust < 0) /* polymorphic */			    \
		switch (_port_right) {					    \
		case MACH_MSG_TYPE_PORT_NAME:				    \
		case MACH_MSG_TYPE_COPY_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:			    \
			/* a copy is always 0 on encode */		    \
			flick_ref_adjust = 0;				    \
			break;						    \
		case MACH_MSG_TYPE_MOVE_RECEIVE:			    \
		case MACH_MSG_TYPE_MOVE_SEND:				    \
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:			    \
			/* a move is 1 or more on encode */		    \
			flick_ref_adjust = -flick_ref_adjust;		    \
			break;						    \
		case MACH_MSG_TYPE_POLYMORPHIC:				    \
		default:						    \
			flick_stub_error(FLICK_ERROR_INVALID_TARGET,	    \
					 _onerror);			    \
			break;						    \
		}							    \
	if (flick_ref_adjust > 1) {					    \
		if (mach_port_mod_refs(mach_task_self(), (__data),	    \
				       _port_right, -(flick_ref_adjust-1))) \
			flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);  \
	}								    \
        flick_mach3mig_encode_prim(_ofs, __data,			    \
				   _port_right,	32,			    \
				   mach_port_t);			    \
}

/*
 * Array type descriptors.
 */

#define flick_mach3mig_array_encode_scopy(_ofs, _ptr, _size) {		\
	typedef struct {						\
		char _data[_size];					\
	} *_ts;								\
									\
	* (_ts) (((char *) _buf_current) + (_ofs)) = * (_ts) (_ptr);	\
}

#define flick_mach3mig_array_encode_bcopy(_ofs, _src, _qty)	\
	(void) bcopy((void *) (_src),				\
		     (char *) _buf_current + (_ofs),		\
		     (_qty))

#define flick_mach3mig_array_encode_type(_ofs, _name, _bits, _ctype,	  \
					 _num, _inl, _dealloc) {	  \
	mach_msg_type_t *_p						  \
		= (mach_msg_type_t *) (((char *) _buf_current) + (_ofs)); \
	mach_msg_type_t _tmpl = { (_name), (_bits), (_num),		  \
				  (_inl), 0, (_dealloc), 0 };		  \
	*_p = _tmpl;							  \
}

#define flick_mach3mig_array_encode_long_type(_ofs, _name, _bits,	      \
					      _ctype, _num, _inl,	      \
					      _dealloc) {		      \
	mach_msg_type_long_t *_p					      \
		= (mach_msg_type_long_t *) (((char *) _buf_current)	      \
					    + (_ofs));			      \
	mach_msg_type_long_t _tmpl = { { 0, 0, 0, (_inl), 1, (_dealloc), 0 }, \
				       (_name), (_bits), (_num) };	      \
	*_p = _tmpl;							      \
}

#define flick_mach3mig_array_encode_boolean_type(_ofs,			\
						 _num, _inl, _dealloc,	\
						 _long)			\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   MACH_MSG_TYPE_BOOLEAN, 32,	\
					   flick_signed32_t,		\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_char8_type(_ofs,			\
					       _num, _inl, _dealloc,	\
					       _long)			\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   MACH_MSG_TYPE_CHAR, 8,	\
					   flick_signed8_t,		\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_char16_type(_ofs,			\
						_num, _inl, _dealloc,	\
						_long)			\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   MACH_MSG_TYPE_CHAR, 16,	\
					   flick_signed16_t,		\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_signed8_type(_ofs,			\
						 _num, _inl, _dealloc,	\
						 _long)			\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   MACH_MSG_TYPE_INTEGER_8, 8,	\
					   flick_signed8_t,		\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_unsigned8_type(_ofs,		 \
						   _num, _inl, _dealloc, \
						   _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_8, 8,	 \
					   flick_unsigned8_t,		 \
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_signed16_type(_ofs,			 \
						  _num, _inl, _dealloc,	 \
						  _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_16, 16, \
					   flick_signed16_t,		 \
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_unsigned16_type(_ofs,		  \
						    _num, _inl, _dealloc, \
						    _long)		  \
	flick_mach3mig_array_encode##_long(_ofs,			  \
					   MACH_MSG_TYPE_INTEGER_16, 16,  \
					   flick_unsigned16_t,		  \
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_signed32_type(_ofs,			 \
						  _num, _inl, _dealloc,	 \
						  _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_32, 32, \
					   flick_signed32_t,		 \
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_unsigned32_type(_ofs,		  \
						    _num, _inl, _dealloc, \
						    _long)		  \
	flick_mach3mig_array_encode##_long(_ofs,			  \
					   MACH_MSG_TYPE_INTEGER_32, 32,  \
					   flick_unsigned32_t,		  \
					   _num, _inl, _dealloc)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_array_encode_signed64_type(_ofs,		  \
						    _num, _inl, _dealloc, \
						    _long)		  \
	flick_mach3mig_array_encode##_long(_ofs,			  \
					   MACH_MSG_TYPE_INTEGER_64, 64,  \
					   flick_signed64_t,		  \
					   _num, _inl, _dealloc)
#  define flick_mach3mig_array_encode_unsigned64_type(_ofs,		    \
						      _num, _inl, _dealloc, \
						      _long)		    \
	flick_mach3mig_array_encode##_long(_ofs,			    \
					   MACH_MSG_TYPE_INTEGER_64, 64,    \
					   flick_unsigned64_t,		    \
					   _num, _inl, _dealloc)
#else
#  define flick_mach3mig_array_encode_signed64_type(_ofs,		  \
						    _num, _inl, _dealloc, \
						    _long)		  \
	flick_mach3mig_array_encode##_long(_ofs,			  \
					   MACH_MSG_TYPE_INTEGER_32, 64,  \
					   flick_signed64_t,		  \
					   _num, _inl, _dealloc)
#define flick_mach3mig_array_encode_unsigned64_type(_ofs,		  \
						    _num, _inl, _dealloc, \
						    _long)		  \
	flick_mach3mig_array_encode##_long(_ofs,			  \
					   MACH_MSG_TYPE_INTEGER_32, 64,  \
					   flick_unsigned64_t,		  \
					   _num, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_array_encode_float32_type(_ofs,		 \
						   _num, _inl, _dealloc, \
						   _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_REAL_32, 32,	 \
					   flick_float32_t,		 \
					   _num, _inl, _dealloc)
#else
#  define flick_mach3mig_array_encode_float32_type(_ofs,		 \
						   _num, _inl, _dealloc, \
						   _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_REAL, 32,	 \
					   flick_float32_t,		 \
					   _num, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_array_encode_float64_type(_ofs,		 \
						   _num, _inl, _dealloc, \
						   _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_REAL_64, 64,	 \
					   flick_float64_t,		 \
					   _num, _inl, _dealloc)
#else
#  define flick_mach3mig_array_encode_float64_type(_ofs,		 \
						   _num, _inl, _dealloc, \
						   _long)		 \
	flick_mach3mig_array_encode##_long(_ofs,			 \
					   MACH_MSG_TYPE_REAL, 64,	 \
					   flick_float64_t,		 \
					   _num, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_REAL_64 */

#define flick_mach3mig_array_encode_string_c_type(_ofs,			\
						  _num, _inl, _dealloc,	\
						  _long)		\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   MACH_MSG_TYPE_STRING_C, 8,	\
					   flick_signed8_t,		\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_port32_type(_ofs,			\
						_right,			\
						_num, _inl, _dealloc,	\
						_long)			\
	flick_mach3mig_array_encode##_long(_ofs,			\
					   _right, 32,			\
					   mach_port_t,			\
					   _num, _inl, _dealloc)

#define flick_mach3mig_array_encode_prim(_ofs, __data, _name, _bits,	\
					 _ctype) {			\
	_ctype *_p = (_ctype *) (((char *) _buf_current) + (_ofs));	\
	*_p = (_ctype) (__data);					\
}

#define flick_mach3mig_array_encode_boolean(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_BOOLEAN, 32,	\
					 flick_signed32_t)

#define flick_mach3mig_array_encode_char8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_CHAR, 8,		\
					 flick_signed8_t)

#define flick_mach3mig_array_encode_char16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_CHAR, 16,	\
					 flick_signed16_t)

#define flick_mach3mig_array_encode_signed8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_8, 8,	\
					 flick_signed8_t)

#define flick_mach3mig_array_encode_unsigned8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_8, 8,	\
					 flick_unsigned8_t)

#define flick_mach3mig_array_encode_signed16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_16, 16,	\
					 flick_signed16_t)

#define flick_mach3mig_array_encode_unsigned16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_16, 16,	\
					 flick_unsigned16_t)

#define flick_mach3mig_array_encode_signed32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 32,	\
					 flick_signed32_t)

#define flick_mach3mig_array_encode_unsigned32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 32,	\
					 flick_unsigned32_t)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_array_encode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_64, 64,	\
					 flick_signed64_t)
#  define flick_mach3mig_array_encode_unsigned64(_ofs, __data, dest_type) \
	flick_mach3mig_array_encode_prim(_ofs, __data,			  \
					 MACH_MSG_TYPE_INTEGER_64, 64,	  \
					 flick_unsigned64_t)
#else
#  define flick_mach3mig_array_encode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 64,	\
					 flick_signed64_t)
#  define flick_mach3mig_array_encode_unsigned64(_ofs, __data, dest_type) \
	flick_mach3mig_array_encode_prim(_ofs, __data,			  \
					 MACH_MSG_TYPE_INTEGER_32, 64,	  \
					 flick_unsigned64_t)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_array_encode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL_32, 32,	\
					 flick_float32_t)
#else
#  define flick_mach3mig_array_encode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL, 32,	\
					 flick_float32_t)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_array_encode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL_64, 64,	\
					 flick_float64_t)
#else
#  define flick_mach3mig_array_encode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_encode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL, 64,	\
					 flick_float64_t)
#endif /* MACH_MSG_TYPE_REAL_64 */

#define flick_mach3mig_array_encode_port32(_ofs, __data, _adjust,	    \
					   _port_right, _onerror) {	    \
	int flick_ref_adjust = (_adjust);				    \
        /*								    \
	 * This switch is not necessary for mach, but it is an example of   \
	 * how to handle polymorphic ports in a back end that must maintain \
	 * a ref_count.							    \
	 */								    \
	if (flick_ref_adjust < 0) /* polymorphic */			    \
		switch (_port_right) {					    \
		case MACH_MSG_TYPE_PORT_NAME:				    \
		case MACH_MSG_TYPE_COPY_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:			    \
			/* a copy is always 0 on encode */		    \
			flick_ref_adjust = 0;				    \
			break;						    \
		case MACH_MSG_TYPE_MOVE_RECEIVE:			    \
		case MACH_MSG_TYPE_MOVE_SEND:				    \
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:			    \
			/* a move is 1 or more on encode */		    \
			flick_ref_adjust = -flick_ref_adjust;		    \
			break;						    \
		case MACH_MSG_TYPE_POLYMORPHIC:				    \
		default:						    \
			flick_stub_error(FLICK_ERROR_INVALID_TARGET,	    \
					 _onerror);			    \
			break;						    \
		}							    \
	if (flick_ref_adjust > 1) {					    \
		if (mach_port_mod_refs(mach_task_self(), (__data),	    \
				       _port_right, -(flick_ref_adjust-1))) \
			flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);  \
	}								    \
        flick_mach3mig_array_encode_prim(_ofs, __data,			    \
					 _port_right, 32,		    \
					 mach_port_t);			    \
}

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */
#define flick_mach3mig_encode_longstring(__data, _dealloc, _link,	\
					 _size, _onerror)		\
	flick_mach3mig_encode_string(__data, _dealloc, _link,		\
				     _size, _onerror)

#define flick_mach3mig_encode_alloced_string(__data, _link,		\
					     _size, _onerror)		\
	flick_mach3mig_encode_string(__data, null_flick_alloc, _link,	\
				     _size, _onerror)

#define flick_mach3mig_encode_string(__data, _dealloc, _link, _size,	      \
				     _onerror) {			      \
	if (_size < 4096) {						      \
		struct { mach_msg_type_t _t; char _v[_size]; } *_p;	      \
		int _temp_iter = 0;					      \
		flick_##_link##_encode_new_glob(sizeof(*_p), _onerror);	      \
		_p = (void *) _buf_current;				      \
		while(((char *) (__data))[_temp_iter]) {		      \
			_p->_v[_temp_iter] = ((char *) (__data))[_temp_iter]; \
			_temp_iter++;					      \
		}							      \
		_p->_v[_temp_iter] = 0;					      \
		{							      \
			mach_msg_type_t _tmpl = { MACH_MSG_TYPE_STRING_C, 8,  \
						  _temp_iter + 1, 1, 0, 0 };  \
			_p->_t = _tmpl;					      \
		}							      \
		flick_##_link##_encode_new_chunk(_temp_iter + 5);	      \
		flick_##_link##_encode_end_chunk(_temp_iter + 5);	      \
		flick_##_link##_encode_end_glob(sizeof(*_p));		      \
	} else {							      \
		struct { mach_msg_type_long_t _t; char _v[_size]; } *_p;      \
		int _temp_iter = 0;					      \
		flick_##_link##_encode_new_glob(sizeof(*_p), _onerror);	      \
		_p = (void *) _buf_current;				      \
		while(((char *) (__data))[_temp_iter]) {		      \
			_p->_v[_temp_iter] = ((char *) (__data))[_temp_iter]; \
			_temp_iter++;					      \
		}							      \
		_p->_v[_temp_iter] = 0;					      \
		{							      \
			mach_msg_type_long_t _tmpl			      \
				= { { 0, 0, 0, 1, 1, 0, 0 },		      \
				    MACH_MSG_TYPE_STRING_C,		      \
				    8, _temp_iter + 1 };		      \
			_p->_t = _tmpl;					      \
		}							      \
		flick_##_link##_encode_new_chunk(_temp_iter + 13);	      \
		flick_##_link##_encode_end_chunk(_temp_iter + 13);	      \
		flick_##_link##_encode_end_glob(sizeof(*_p));		      \
	}								      \
	_dealloc(__data);						      \
}

/*
 * ***XXX*** - End of string macros (see comment above).
 */

/******************************************************************************
 ******************************************************************************
 ****
 **** Decoding
 ****
 ******************************************************************************
 *****************************************************************************/

#if TypeCheck
#define flick_iftypecheck(code) code
#else
#define flick_iftypecheck(code)
#endif

#define flick_mach3mig_decode_prim(_ofs, __data, _name, _bits, _ctype) { \
	struct st {							 \
		mach_msg_type_t _t;					 \
		_ctype _v;						 \
	} *_p								 \
		= (struct st *) (((char *) _buf_current) + (_ofs));	 \
									 \
	flick_iftypecheck( ({						 \
		mach_msg_type_t _tmpl					 \
			= { (_name), (_bits), 1, 1, 0, 0 };		 \
		if (*((flick_signed32_t *) &_tmpl)			 \
		    != *((flick_signed32_t *) &_p->_t))			 \
			return MIG_TYPE_ERROR;				 \
	}) )								 \
	(_ctype) (__data) = _p->_v;					 \
}

#define flick_mach3mig_decode_prim_no_type_tag(_ofs, __data, _ctype)	\
	(__data) = *(_ctype*) (((char *) _buf_current) + (_ofs))

#define flick_mach3mig_decode_boolean(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_BOOLEAN, 32,	\
				   flick_signed32_t)

#define flick_mach3mig_decode_char8(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_CHAR, 8,	\
				   flick_signed8_t)

#define flick_mach3mig_decode_char16(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_CHAR, 16,	\
				   flick_signed16_t)

#define flick_mach3mig_decode_signed8(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,		\
				   MACH_MSG_TYPE_INTEGER_8, 8,	\
				   flick_signed8_t)

#define flick_mach3mig_decode_unsigned8(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_8, 8,		\
				   flick_unsigned8_t)

#define flick_mach3mig_decode_signed16(_ofs, __data, dest_type)		\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_16, 16,	\
				   flick_signed16_t)

#define flick_mach3mig_decode_unsigned16(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_16, 16,	\
				   flick_unsigned16_t)

#define flick_mach3mig_decode_signed32(_ofs, __data, dest_type)		\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 32,	\
				   flick_signed32_t)

#define flick_mach3mig_decode_unsigned32(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 32,	\
				   flick_unsigned32_t)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_decode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_64, 64,	\
				   flick_signed64_t)
#  define flick_mach3mig_decode_unsigned64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_64, 64,	\
				   flick_unsigned64_t)
#else
#  define flick_mach3mig_decode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 64,	\
				   flick_signed64_t)
#  define flick_mach3mig_decode_unsigned64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_INTEGER_32, 64,	\
				   flick_unsigned64_t)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_decode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL_32, 32,		\
				   flick_float32_t)
#else
#  define flick_mach3mig_decode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL, 32,		\
				   flick_float32_t)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_decode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL_64, 64,		\
				   flick_float64_t)
#else
#  define flick_mach3mig_decode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_decode_prim(_ofs, __data,			\
				   MACH_MSG_TYPE_REAL, 64,		\
				   flick_float64_t)
#endif /* MACH_MSG_TYPE_REAL_64 */

/*
 * NOTE: when type-checking, doesn't check the _type_tag for equality.
 */
#define flick_mach3mig_decode_type_tag(_ofs, _type_tag, _bits) {	  \
	flick_iftypecheck( ({						  \
		mach_msg_type_t *_p = (((char *) _buf_current) + (_ofs)); \
		mach_msg_type_t _mask = { 0, 0xFF,			  \
					  1, 1, 1, 1 };			  \
		mach_msg_type_t _tmpl = { (_type_tag), (_bits),		  \
					  1, 1, 0, 0 };			  \
									  \
		if ((  *((flick_signed32_t *) &_tmpl)			  \
		     & *((flick_signed32_t *) &_mask))			  \
		    != (  *((flick_signed32_t *)			  \
			    (((char *) _buf_current) + (_ofs)))		  \
			& *((flick_signed32_t *) &_mask)))		  \
			return MIG_TYPE_ERROR;				  \
	}) )								  \
	(_type_tag) = *((mach_msg_type_t *)				  \
			(((char *) _buf_current) + (_ofs)));		  \
}

/*
 * XXX --- Not currently used, since a port type_tag becomes the _port_right
 * parameter to flic_mach3mig_decode_port.
 */
#define flick_mach3mig_decode_port_no_type_tag(_ofs, __data)		  \
	flick_mach3mig_decode_prim_no_type_tag(_ofs, __data, mach_port_t)

#define flick_mach3mig_decode_port32(_ofs, __data, _adjust, _port_right,    \
				     _onerror) {			    \
	int flick_ref_adjust = (_adjust);				    \
									    \
	flick_mach3mig_decode_prim(_ofs, __data,			    \
				   _port_right, 32,			    \
				   mach_port_t);			    \
        /*								    \
	 * This switch is not necessary for mach, but it is an example of   \
	 * how to handle polymorphic ports in a back end that must maintain \
	 * a ref_count.							    \
	 */								    \
        if (flick_ref_adjust < 0) /* polymorphic */			    \
		switch (_port_right) {					    \
		case MACH_MSG_TYPE_PORT_NAME:				    \
		case MACH_MSG_TYPE_COPY_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:			    \
		case MACH_MSG_TYPE_MOVE_RECEIVE:			    \
		case MACH_MSG_TYPE_MOVE_SEND:				    \
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:			    \
		case MACH_MSG_TYPE_POLYMORPHIC:				    \
			/* always 1 or more on decode */		    \
			flick_ref_adjust = -flick_ref_adjust;		    \
			break;						    \
		default:						    \
			flick_stub_error(FLICK_ERROR_INVALID_TARGET,	    \
					 _onerror);			    \
			break;						    \
		}							    \
	if (flick_ref_adjust > 1)					    \
		if (mach_port_mod_refs(mach_task_self(), (__data),	    \
				       _port_right, flick_ref_adjust-1))    \
			flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);  \
}

/*
 * Array type descriptors.
 */

#define flick_mach3mig_array_decode_scopy(_ofs, _ptr, _size) {		\
	typedef struct {						\
		char _data[_size];					\
	} *_ts;								\
									\
	* (_ts) (_ptr) = * (_ts) (((char *) _buf_current) + (_ofs));	\
}

#define flick_mach3mig_array_decode_msgptr(_ofs, _ptr, type)	\
	(_ptr) = (type) (((char *) _buf_current) + (_ofs))

#define flick_mach3mig_array_decode_bcopy(_ofs, _dest, _qty)	\
	(void) bcopy((char *) _buf_current + (_ofs),		\
		     (void *) (_dest),				\
		     (_qty))

#define flick_mach3mig_array_decode_type(_ofs, _name, _bits, _num, _inl, \
					 _dealloc) {			 \
	mach_msg_type_t _tmpl						 \
		= *((mach_msg_type_t *)					 \
		    (((char *) _buf_current) + (_ofs)));		 \
									 \
	(_num) = _tmpl.msgt_number;					 \
	flick_iftypecheck( ({						 \
		mach_msg_type_t _p = { (_name), (_bits), (0), (_inl),	 \
				       0, (_dealloc), 0 };		 \
									 \
		_p.msgt_number = (_num);				 \
		if (*((flick_signed32_t *) &_tmpl)			 \
		    != *((flick_signed32_t *) &_p))			 \
			return MIG_TYPE_ERROR;				 \
	}) )								 \
}

#define flick_mach3mig_array_decode_long_type(_ofs, _name, _bits, _num,	\
					      _inl, _dealloc) {		\
	mach_msg_type_long_t _tmpl					\
		= *((mach_msg_type_long_t *)				\
		    (((char *) _buf_current) + (_ofs)));		\
									\
	(_num) = _tmpl.msgtl_number;					\
	flick_iftypecheck( ({						\
		mach_msg_type_long_t _p					\
			= { { 0, 0, 0, (_inl), 1, (_dealloc), 0 },	\
			    (_name), (_bits), (0) };			\
									\
		_p.msgtl_number = (_num);				\
		if (memcmp(&_tmpl, &_p, sizeof(_tmpl)))			\
			return MIG_TYPE_ERROR;				\
	}) )								\
}

#define flick_mach3mig_array_decode_boolean_type(_ofs, __data,		\
						 _inl, _dealloc,	\
						 _long)			\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_BOOLEAN, 32,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_char8_type(_ofs, __data,		\
					       _inl, _dealloc,		\
					       _long)			\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_CHAR, 8,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_char16_type(_ofs, __data,		\
						_inl, _dealloc,		\
						_long)			\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_CHAR, 16,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_signed8_type(_ofs, __data,		\
						 _inl, _dealloc,	\
						 _long)			\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_INTEGER_8, 8,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_unsigned8_type(_ofs, __data,	\
						   _inl, _dealloc,	\
						   _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_INTEGER_8, 8,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_signed16_type(_ofs, __data,		 \
						  _inl, _dealloc,	 \
						  _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_16, 16, \
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_unsigned16_type(_ofs, __data,	 \
						    _inl, _dealloc,	 \
						    _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_16, 16, \
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_signed32_type(_ofs, __data,		 \
						  _inl, _dealloc,	 \
						  _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_32, 32, \
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_unsigned32_type(_ofs, __data,	 \
						    _inl, _dealloc,	 \
						    _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_32, 32, \
					   __data, _inl, _dealloc)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_array_decode_signed64_type(_ofs, __data,	 \
						    _inl, _dealloc,	 \
						    _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_64, 64, \
					   __data, _inl, _dealloc)
#  define flick_mach3mig_array_decode_unsigned64_type(_ofs, __data,	 \
						      _inl, _dealloc,	 \
						      _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_64, 64, \
					   __data, _inl, _dealloc)
#else
#  define flick_mach3mig_array_decode_signed64_type(_ofs, __data,	 \
						    _inl, _dealloc,	 \
						    _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_32, 64, \
					   __data, _inl, _dealloc)
#  define flick_mach3mig_array_decode_unsigned64_type(_ofs, __data,	 \
						      _inl, _dealloc,	 \
						      _long)		 \
	flick_mach3mig_array_decode##_long(_ofs,			 \
					   MACH_MSG_TYPE_INTEGER_32, 64, \
					   __data, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_array_decode_float32_type(_ofs, __data,	\
						   _inl, _dealloc,	\
						   _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_REAL_32, 32,	\
					   __data, _inl, _dealloc)
#else
#  define flick_mach3mig_array_decode_float32_type(_ofs, __data,	\
						   _inl, _dealloc,	\
						   _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_REAL, 32,	\
					   __data, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_array_decode_float64_type(_ofs, __data,	\
						   _inl, _dealloc,	\
						   _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_REAL_64, 64,	\
					   __data, _inl, _dealloc)
#else
#  define flick_mach3mig_array_decode_float64_type(_ofs, __data,	\
						   _inl, _dealloc,	\
						   _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_REAL, 64,	\
					   __data, _inl, _dealloc)
#endif /* MACH_MSG_TYPE_REAL_64 */

#define flick_mach3mig_array_decode_string_c_type(_ofs, __data,		\
						  _inl, _dealloc,	\
						  _long)		\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   MACH_MSG_TYPE_STRING_C, 8,	\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_port32_type(_ofs,			\
						_right, __data,		\
						_inl, _dealloc,		\
						_long)			\
	flick_mach3mig_array_decode##_long(_ofs,			\
					   _right, 32,			\
					   __data, _inl, _dealloc)

#define flick_mach3mig_array_decode_prim(_ofs, __data, _name, _bits,	\
					 _ctype) {			\
	_ctype *_p = (_ctype *) (((char *) _buf_current) + (_ofs));	\
									\
	(_ctype) (__data) = *_p;					\
}

#define flick_mach3mig_array_decode_boolean(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_BOOLEAN, 32,	\
					 flick_signed32_t)

#define flick_mach3mig_array_decode_char8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_CHAR, 8,		\
					 flick_signed8_t)

#define flick_mach3mig_array_decode_char16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_CHAR, 16,	\
					 flick_signed16_t)

#define flick_mach3mig_array_decode_signed8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_8, 8,	\
					 flick_signed8_t)

#define flick_mach3mig_array_decode_unsigned8(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_8, 8,	\
					 flick_unsigned8_t)

#define flick_mach3mig_array_decode_signed16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_16, 16,	\
					 flick_signed16_t)

#define flick_mach3mig_array_decode_unsigned16(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_16, 16,	\
					 flick_unsigned16_t)

#define flick_mach3mig_array_decode_signed32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 32,	\
					 flick_signed32_t)

#define flick_mach3mig_array_decode_unsigned32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 32,	\
					 flick_unsigned32_t)

#ifdef MACH_MSG_TYPE_INTEGER_64
#  define flick_mach3mig_array_decode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_64, 64,	\
					 flick_signed64_t)
#  define flick_mach3mig_array_decode_unsigned64(_ofs, __data, dest_type) \
	flick_mach3mig_array_decode_prim(_ofs, __data,			  \
					 MACH_MSG_TYPE_INTEGER_64, 64,	  \
					 flick_unsigned64_t)
#else
#  define flick_mach3mig_array_decode_signed64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_INTEGER_32, 64,	\
					 flick_signed64_t)
#  define flick_mach3mig_array_decode_unsigned64(_ofs, __data, dest_type) \
	flick_mach3mig_array_decode_prim(_ofs, __data,			  \
					 MACH_MSG_TYPE_INTEGER_32, 64,	  \
					 flick_unsigned64_t)
#endif /* MACH_MSG_TYPE_INTEGER_64 */

#ifdef MACH_MSG_TYPE_REAL_32
#  define flick_mach3mig_array_decode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL_32, 32,	\
					 flick_float32_t)
#else
#  define flick_mach3mig_array_decode_float32(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL, 32,	\
					 flick_float32_t)
#endif /* MACH_MSG_TYPE_REAL_32 */

#ifdef MACH_MSG_TYPE_REAL_64
#  define flick_mach3mig_array_decode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL_64, 64,	\
					 flick_float64_t)
#else
#  define flick_mach3mig_array_decode_float64(_ofs, __data, dest_type)	\
	flick_mach3mig_array_decode_prim(_ofs, __data,			\
					 MACH_MSG_TYPE_REAL, 64,	\
					 flick_float64_t)
#endif /* MACH_MSG_TYPE_REAL_64 */

#define flick_mach3mig_array_decode_port32(_ofs, __data, _adjust,	    \
					   _port_right, _onerror) {	    \
	int flick_ref_adjust = (_adjust);				    \
									    \
	flick_mach3mig_array_decode_prim(_ofs, __data,			    \
					 _port_right, 32,		    \
					 mach_port_t);			    \
        /*								    \
	 * This switch is not necessary for mach, but it is an example of   \
	 * how to handle polymorphic ports in a back end that must maintain \
	 * a ref_count.							    \
	 */								    \
        if (flick_ref_adjust < 0) /* polymorphic */			    \
		switch (_port_right) {					    \
		case MACH_MSG_TYPE_PORT_NAME:				    \
		case MACH_MSG_TYPE_COPY_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND:				    \
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:			    \
		case MACH_MSG_TYPE_MOVE_RECEIVE:			    \
		case MACH_MSG_TYPE_MOVE_SEND:				    \
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:			    \
		case MACH_MSG_TYPE_POLYMORPHIC:				    \
			/* always 1 or more on decode */		    \
			flick_ref_adjust = -flick_ref_adjust;		    \
			break;						    \
		default:						    \
			flick_stub_error(FLICK_ERROR_INVALID_TARGET,	    \
					 _onerror);			    \
			break;						    \
		}							    \
	if (flick_ref_adjust > 1)					    \
		if (mach_port_mod_refs(mach_task_self(), (__data),	    \
				       _port_right, flick_ref_adjust-1))    \
			flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);  \
}

/*
 * ***XXX***
 *
 * These string macros should no longer be used.  Unfortunately, some
 * of the other macros and runtime code *DO* use them, so they're left
 * in for now, but they will be removed ASAP.
 */
#define flick_mach3mig_decode_stringlen(_data, _link, _onerror) {	\
	flick_##_link##_decode_new_glob(7);				\
	flick_##_link##_decode_new_chunk_align(4, 2, 0, 0);		\
        flick_##_link##_check_span(4, _onerror);			\
	flick_mach3mig_array_decode_string_c_type(0, _data, 1, 0,	\
						  _long_type);		\
	flick_##_link##_decode_end_chunk(4);				\
	flick_##_link##_decode_end_glob(7);				\
}

#define flick_mach3mig_decode_longstring(__data, _alloc, _link,		\
					 _size, _onerror) {		\
	char *_v, *new_buf;						\
									\
	flick_##_link##_decode_new_glob(_size);				\
	flick_##_link##_decode_new_chunk(_size);			\
	_v = (char *) _buf_current;					\
	if ((new_buf = _alloc(_size + 1)) != 0) {			\
		(__data) = new_buf;					\
	else								\
		flick_stub_error(FLICK_ERROR_NO_MEMORY, _onerror);	\
	while (_size >= 0) {						\
		((char *) (__data))[_size] = _v[_size];			\
		_size--;						\
	}								\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob(_size);				\
}

#define flick_mach3mig_decode_alloced_string(__data, _link, _size) {	\
	char *_v;							\
									\
	flick_##_link##_decode_new_glob((_size));			\
	flick_##_link##_decode_new_chunk(_size);			\
	_v = (char *) _buf_current;					\
	while (_size >= 0) {						\
		((char *) (__data))[_size] = _v[_size];			\
		_size--;						\
	}								\
	flick_##_link##_decode_end_chunk(_size);			\
	flick_##_link##_decode_end_glob((_size));			\
}

#define flick_mach3mig_decode_string(__data, _alloc, _link, _size,	\
				     _onerror)				\
	flick_mach3mig_decode_longstring(__data, _alloc, _link, _size,	\
					 _onerror)

#define flick_mach3mig_decode_auto_string(__data, _alloc, _link, _size) { \
	flick_##_link##_decode_new_glob(_size);				  \
	flick_##_link##_decode_new_chunk(_size);			  \
	(__data) = (void *) _buf_current;				  \
	flick_##_link##_decode_end_chunk(_size);			  \
	flick_##_link##_decode_end_glob(_size);				  \
}

/*
 * ***XXX*** - End of string macros (see comment above).
 */

/*
 * Following are all the temporary variable macros.
 */

/*
 * Macro to determine the encoded length of a string.
 * For Mach3MIG, the encoded length *includes* the terminator.
 */
#define flick_mach3mig_stringlen(o_type, o_expr, t_type, t_expr)	\
	(t_expr) = strlen(o_expr) + 1

#endif /* __flick_encode_mach3mig_h */

/* End of file. */

