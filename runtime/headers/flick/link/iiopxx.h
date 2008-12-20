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

#ifndef __flick_link_iiopxx_h
#define __flick_link_iiopxx_h

#include <sys/types.h>
#include <sys/socket.h>
#include <flick/link/all.h>
#include <tao/GIOP_Server_Request.h>
#include <tao/corba.h>
#include <tao/Reply_Dispatcher.h>
#include <tao/Connector_Registry.h>

extern int flick_seqnum;

typedef struct FLICK_BUFFER {
	/*
	 * A pointer to the underlying TAO stream.  This is here primarily for
	 * the benefit of the `any'-processing macros, which need access to the
	 * TAO stream so that they can invoke TAO's runtime for handling `any'
	 * values.
	 */
	union {
		TAO_InputCDR * 		in_cdr_ptr;
		TAO_OutputCDR *		out_cdr_ptr;
	} tao_stream;
	
	/*
	 * A pointer to the `ACE_Message_Block' that is contained within the
	 * TAO stream.
	 */
	ACE_Message_Block *		msg_block;
	
	/*
	 * frag_start is the pointer to the start of the memory in the current
	 * message block.  This is needed because we're using TAO's anys and
	 * the message may become fragmented requiring us to be able to handle
	 * multiple message blocks.  So we keep track of the start of the
	 * fragment separate from the start of the message.
	 */
	void *				frag_start;
	
	/*
	 * Finally, the usual Flick stream pointers and `stub_state' struct.
	 */
	void *				buf_start;
	void *				buf_current;
	void *				buf_end;
	struct flick_stub_state		stub_state;
	
} *flick_marshal_stream_t, FLICK_BUFFER;

#define _buf_start (_stream->buf_start)
#define _buf_current (_stream->buf_current)
#define _buf_end (_stream->buf_end)
#define _stub_state (_stream->stub_state)

typedef struct flick_msg_struct_t * flick_msg_t;

#define flick_iiopxx_check_span(_size,_onerror)				\
	if( (((char *)_buf_current) + _size) > (char *)_buf_end)	\
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _onerror);

/* stream <==> msg conversion macros */
#define MSG_HDR_PAD 64

/* Used for both client request & server reply... */
#define flick_iiopxx_build_message_header(loc, msg_type) {	\
	((char *)(loc))[0] = 'G';				\
	((char *)(loc))[1] = 'I';				\
	((char *)(loc))[2] = 'O';				\
	((char *)(loc))[3] = 'P';				\
	((char *)(loc))[4] = 1;					\
	((char *)(loc))[5] = 0;					\
	/* little endian flag... */				\
	((short *)(loc))[3] = 1;				\
	((char *)(loc))[7] = msg_type;				\
	/* ((int *)(loc))[2] = size of rest of message */	\
}

#ifdef VERIFY
/* Used for both client reply & server request... */
#define flick_iiopxx_verify_message_header(loc, msg_type) {	\
	if (((char *)(loc))[0] != 'G' ||			\
	    ((char *)(loc))[1] != 'I' ||			\
	    ((char *)(loc))[2] != 'O' ||			\
	    ((char *)(loc))[3] != 'P' ||			\
	    ((char *)(loc))[4] != 1 ||				\
	    ((char *)(loc))[5] != 0 ||				\
	    ((char *)(loc))[7] != msg_type)			\
		fprintf(stderr, "Malformed Message Header!\n");	\
}
#else
#define flick_iiopxx_verify_message_header(loc, msg_type) {}
#endif

#define flick_iiopxx_discard_service_context_list() {			      \
	char *_buf_cursor = (char *)_buf_current;			      \
									      \
	unsigned int _service_context_seq_len;				      \
	unsigned int _service_context_len;				      \
	unsigned int _i;						      \
									      \
	_service_context_seq_len = *(unsigned int *) _buf_cursor;	      \
	_buf_cursor += 4;						      \
									      \
	if (_service_context_seq_len == 0) {				      \
		/* Zero is zero even when it's byteswapped. */		      \
	} else {							      \
		int _swap = ((char *)_buf_start)[6] == TAO_ENCAP_BYTE_ORDER;  \
									      \
		if (_swap)						      \
			_service_context_seq_len			      \
				= swap_unsigned32(_service_context_seq_len);  \
									      \
		for (_i = 0; _i < _service_context_seq_len; ++_i) {	      \
			/* Skip the `context_id', an unsigned long. */	      \
			_buf_cursor += 4;				      \
			_service_context_len				      \
				= *(unsigned int *) _buf_cursor;	      \
			if (_swap)					      \
				_service_context_len			      \
					= swap_unsigned32(		      \
						_service_context_len);	      \
			_buf_cursor += 4;				      \
			/*						      \
			 * Note: It is always correct to 4-byte align here,   \
			 * because every service context is followed by an    \
			 * unsigned long.  Even the last is followed by an    \
			 * unsigned long in the GIOP request or reply header. \
			 */						      \
			_buf_cursor += (_service_context_len + 3) & ~3;	      \
		}							      \
	}								      \
									      \
	_buf_current = _buf_cursor;					      \
}

#define flick_iiopxx_discard_request_id() {				\
	_buf_current = (void *)(((char *)_buf_current) + 4);		\
}

/* Initialization / Exit code */
#define flick_iiopxx_client_start_encode() {				      \
	_stream->tao_stream.out_cdr_ptr = &_out_cdr;			      \
	_stream->msg_block = (ACE_Message_Block *)_out_cdr.begin();	      \
	_stream->frag_start = (void *)((unsigned int)			      \
				       (_stream->msg_block->base() + 7)	      \
				       & (unsigned int) ~7);		      \
	_stream->buf_start = _stream->frag_start;			      \
	_stream->buf_current = _stream->buf_start;			      \
	_stream->buf_end = (char *) _stream->msg_block->base()		      \
			   + _stream->msg_block->size();		      \
	/* Build the `GIOP::MessageHeader' (note Request == 0). */	      \
	flick_iiopxx_build_message_header(_buf_start, 0);		      \
	/* The `GIOP::RequestHeader.service_context', a sequence. */	      \
	/* For now, we always encode an empty sequence. */		      \
	((int *) _buf_start)[3] = 0;					      \
	((int *) _buf_start)[4] = _my_request_id;			      \
	/* The `GIOP::RequestHeader.response_expected' flag is set later. */  \
	/*								      \
	 * From here, we need to put the object key, operation string,	      \
	 * plus the 'Principal' opaque interface.			      \
	 */								      \
	_buf_current = &(((int *) _buf_start)[6]);			      \
	_stream->msg_block->rd_ptr((char *) _buf_start);		      \
}

#define flick_iiopxx_client_end_encode() {				\
	_stream->msg_block->wr_ptr((char *)_stream->buf_current);	\
}

#define flick_iiopxx_client_set_response_expected(val) {		\
	/*								\
	 * 20 ==  12 `GIOP::MessageHeader' bytes			\
	 *       + 4 `GIOP::RequestHeader.service_context' bytes	\
	 *       + 4 `GIOP::RequestHeader.requist_id' bytes		\
	 *								\
	 * XXX --- This assumes that the service context list is a	\
	 * zero-element sequence.					\
	 */								\
	((char *) _buf_start)[20] = (val);				\
}

#define flick_iiopxx_client_start_decode() {				\
	_stream->tao_stream.in_cdr_ptr = _in_cdr;			\
	_stream->msg_block = (ACE_Message_Block *) _in_cdr->start();	\
	_stream->frag_start = (void *)					\
		     (((unsigned int) (_stream->msg_block->base() + 7))	\
		      & (unsigned int) ~7);				\
	_buf_start = _stream->frag_start;				\
	_buf_current = _stream->msg_block->rd_ptr() - 4;		\
	_buf_end = _stream->msg_block->wr_ptr();			\
}

#define flick_iiopxx_client_end_decode() /* Nothing to do. */

#define flick_iiopxx_server_start_decode() {				 \
	_tao_context = 0;						 \
	_stream->tao_stream.in_cdr_ptr = &_in_cdr;			 \
	_stream->msg_block = (ACE_Message_Block *)_in_cdr.start();	 \
	_stream->frag_start = (void *)					 \
		     ((unsigned int) (_stream->msg_block->base() + 7)	 \
		      & (unsigned int) ~7);				 \
	_buf_start = _stream->frag_start;				 \
	_buf_current = (void *) (_tao_req.operation() - 4);		 \
	_buf_end = _stream->msg_block->wr_ptr();			 \
	/* Verify the request GIOP MessageHeader (note Request == 0). */ \
	/* flick_iiopxx_verify_message_header(_buf_start, 0); */	 \
	/*								 \
	 * NOTE: The server dispatch loop sets the stream pointer to	 \
	 * point to the start of the operation name --- i.e., we've	 \
	 * already parsed up to the `operation' field of the GIOP	 \
	 * RequestHeader.  See the function `find_implementation'.	 \
	 */								 \
}

#define flick_iiopxx_server_end_decode() /* Nothing to do. */

#define flick_iiopxx_server_start_encode() {				  \
	_stream->tao_stream.out_cdr_ptr = &_out_cdr;			  \
	_stream->msg_block = (ACE_Message_Block *)_out_cdr.begin();	  \
	_stream->frag_start = (void *)((unsigned int)			  \
				       (_stream->msg_block->base() + 7)	  \
				       & (unsigned int) ~7);		  \
	_stream->msg_block->rd_ptr((char *) _stream->frag_start);	  \
	_stream->buf_start = _stream->frag_start;			  \
	_buf_current = _buf_start;					  \
	_buf_end = (char *) _stream->msg_block->base()			  \
		   + _stream->msg_block->size();			  \
	flick_iiopxx_build_message_header(_buf_start, 1); /* Reply = 1 */ \
	((unsigned int *) _buf_start)[3] = 0;	/* ServiceContext */	  \
	((unsigned int *) _buf_start)[4]				  \
		= ((TAO_GIOP_ServerRequest *) &_tao_req)->request_id();	  \
	_buf_current = &(((int *) _buf_start)[5]);			  \
}

#define flick_iiopxx_server_restart_encode()		\
	_buf_current = &(((int *) _buf_start)[5])

#define flick_iiopxx_server_end_encode() {				\
	_stream->msg_block->wr_ptr((char *) _stream->buf_current);	\
}

/* Globbing & Chunking code */
/*
 * XXX --- The IIOPXX runtime function `flick_cdr_encode_IOR_internal' knows how
 * globbing is implemented.  If you change this function you must also change
 * the globbing in that runtime function.
 */
#define flick_iiopxx_encode_new_glob(max_size,_onerror) {		      \
	if( (((char *)_buf_current) + max_size) > _buf_end ) {		      \
		int _old_size = (char *)_buf_current -			      \
			(char *)_stream->frag_start;			      \
		_stream->msg_block->rd_ptr((char *)_stream->frag_start);      \
 		_stream->msg_block->wr_ptr((char *)_buf_current);	      \
		if( ACE_CDR::grow(_stream->msg_block, _old_size + max_size) ) \
			goto _onerror;					      \
		if(_stream->frag_start == _buf_start) {			      \
			_stream->frag_start = (void *)			      \
				((unsigned int)				      \
				 (_stream->msg_block->base() + 7) &	      \
				 (unsigned int)~7);			      \
			_buf_start = _stream->frag_start; 		      \
		} else 							      \
			_stream->frag_start = (void *)			      \
				((unsigned int)				      \
				 (_stream->msg_block->base() + 7) &	      \
				 (unsigned int)~7);			      \
		_buf_current = (char *)_stream->frag_start + _old_size;	      \
		_buf_end = (char *)_stream->msg_block->base() +		      \
			_stream->msg_block->size();			      \
	}								      \
}

#define flick_iiopxx_encode_end_glob(max_size)	/* We don't do squat */
#define flick_iiopxx_encode_new_chunk(size)	/* Don't do anything */
#define flick_iiopxx_encode_end_chunk(size) \
	_buf_current = (void *)(((char *)_buf_current) + size);
#define flick_iiopxx_encode_new_chunk_align(size, final_bits, init_bits, init_ofs) {	\
	unsigned int _align = (1 << (final_bits)) - 1;					\
	_buf_current =								\
		(void *)(((unsigned int)(((char *)_buf_current) + _align)) & ~_align);	\
}

#define flick_iiopxx_encode_new_glob_plain(max_size,_onerror) {		\
	flick_iiopxx_encode_new_glob(max_size, _onerror);		\
}
#define flick_iiopxx_encode_end_glob_plain(max_size)	/* We don't do squat */
#define flick_iiopxx_encode_new_chunk_plain(size)	/* Don't do anything */
#define flick_iiopxx_encode_end_chunk_plain(size)

#define flick_iiopxx_decode_new_glob(max_size)	/* We don't do squat */
#define flick_iiopxx_decode_end_glob(max_size)	/* Again - do nothing */
#define flick_iiopxx_decode_new_chunk(size)	/* Fait rien */
#define flick_iiopxx_decode_end_chunk(size) \
	_buf_current = (void *)(((char *)_buf_current) + size);
#define flick_iiopxx_decode_new_chunk_align(size, final_bits, init_bits, init_ofs) {	\
	unsigned int _align = (1 << (final_bits)) - 1;					\
	_buf_current =								\
		(void *)(((unsigned int)(((char *)_buf_current) + _align)) & ~_align);	\
}


/*****************************************************************************/

/* The following stuff probably belongs in corba_on_iiopxx.h */

#define flick_iiopxx_client_encode_target(ref, _ofs, ENCNAME, _onerror) { \
	unsigned int lpc; \
	unsigned int __len = (_tp)->object_key().length(); \
	flick_iiopxx_encode_new_chunk(4); \
	flick_iiopxx_encode_new_glob(7 + __len, _onerror); \
	flick_##ENCNAME##_encode_unsigned32(0, __len, unsigned int); \
	flick_iiopxx_encode_end_chunk(4); \
	for( lpc = 0; lpc < __len; lpc++ ) { \
		((char *)_buf_current)[lpc] = ((_tp)->object_key())[lpc]; \
	} \
	_buf_current = (void *)(((unsigned int)((char *)_buf_current + \
		__len + 3)) & ~3); \
	flick_iiopxx_encode_end_glob(7 + __len); \
}



#define flick_iiopxx_client_decode_target(ref, _ofs, ENCNAME, _onerror) \
	if (0) goto _onerror
#define flick_iiopxx_server_encode_target(ref, _ofs, ENCNAME, _onerror) \
	if (0) goto _onerror
#define flick_iiopxx_server_decode_target(ref, _ofs, ENCNAME, _onerror) { \
	(ref) = this;							  \
	if (0) goto _onerror;						  \
}

#define flick_iiopxx_mark_port_for_cleanup(a, b)

typedef struct {
	int empty;
} FLICK_TARGET;


/*****************************************************************************/

/*
 * Encoding and decoding type-tagged values, i.e., CORBA `any's.
 *
 * These macros assume that when they are invoked, there is no open glob and
 * therefore no current chunk.
 *
 * `any's are best handled by runtime library functions, and it is the back end
 * that specifies the runtime, which is tailored for a specific link layer.
 * That is why these macros are here at all, instead of at the `encode' layer.
 *
 * The macros are defined for each supported encoding format (e.g., CDR).
 */
#define flick_iiopxx_cdr_encode_type_tag_and_value(_data,		 \
						   _try_label,		 \
						   _on_error) {		 \
       CORBA::Environment ACE_TRY_ENV;					 \
	/*								 \
	 * Resync TAO's stream with that of the Flick-generated stub.	 \
	 */								 \
	_stream->msg_block->wr_ptr((char *) _buf_current);		 \
	/*								 \
	 * Use TAO to encode the `any' value.  See TAO's `CDR.cpp' file, \
	 * `operator<<(TAO_OutputCDR& cdr, CORBA::Any &x)'.		 \
	 */								 \
	ACE_TRY_EX(_try_label) {					 \
		CORBA::TypeCode::traverse_status status =		 \
			TAO_MARSHAL_ANY::instance()->encode(		 \
				0,					 \
				&(_data),				 \
				0,					 \
				_stream->tao_stream.out_cdr_ptr,	 \
				ACE_TRY_ENV);				 \
		ACE_TRY_CHECK_EX(_try_label);				 \
		if (status != CORBA::TypeCode::TRAVERSE_CONTINUE)	 \
			flick_stub_error(FLICK_ERROR_COMMUNICATION,	 \
					 _on_error);			 \
	}								 \
	ACE_CATCH (CORBA_Exception, ex) {				 \
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _on_error);	 \
	}								 \
	ACE_ENDTRY;							 \
	/*								 \
	 * Finally, resync the stub's stream with TAO's.		 \
	 */								 \
	_stream->msg_block = (ACE_Message_Block *)_stream->tao_stream.	 \
		out_cdr_ptr->current();					 \
	_stream->frag_start = _stream->msg_block->rd_ptr();		 \
	_buf_current = _stream->msg_block->wr_ptr();			 \
	_buf_end = (_stream->msg_block->base()				 \
		    + _stream->msg_block->size());			 \
}

#define flick_iiopxx_cdr_decode_type_tag_and_value(_data,		 \
						   _try_label,		 \
						   _on_error) {		 \
       CORBA::Environment ACE_TRY_ENV;					 \
	/*								 \
	 * Resync TAO's stream with that of the Flick-generated stub.	 \
	 */								 \
	_stream->msg_block->rd_ptr((char *) _buf_current);		 \
	/*								 \
	 * Use TAO to decode the `any' value.  See TAO's `CDR.cpp' file, \
	 * `operator>>(TAO_InputCDR& cdr, CORBA::Any &x)'.		 \
	 */								 \
	ACE_TRY_EX(_try_label) {					 \
		CORBA::TypeCode::traverse_status status =		 \
			TAO_MARSHAL_ANY::instance()->decode(		 \
				0,					 \
				&(_data),				 \
				0,					 \
				_stream->tao_stream.in_cdr_ptr,		 \
				ACE_TRY_ENV);				 \
		ACE_TRY_CHECK_EX(_try_label);				 \
		if (status != CORBA::TypeCode::TRAVERSE_CONTINUE)	 \
			flick_stub_error(FLICK_ERROR_COMMUNICATION,	 \
					 _on_error);			 \
	}								 \
	ACE_CATCH (CORBA_Exception, ex) {				 \
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _on_error);	 \
	}								 \
	ACE_ENDTRY;							 \
	/*								 \
	 * Finally, resync the stub's stream with TAO's.		 \
	 */								 \
	_buf_current = _stream->msg_block->rd_ptr();			 \
}

/*
 * We can just call the regular `cdr' versions; TAO's runtime handles swapping
 * as necessary.
 */
#define flick_iiopxx_cdr_swap_encode_type_tag_and_value(_data,		\
							_try_label,	\
							_on_error)	\
	flick_iiopxx_cdr_encode_type_tag_and_value((_data),		\
						   _try_label,		\
						   _on_error)

#define flick_iiopxx_cdr_swap_decode_type_tag_and_value(_data,		\
							_try_label,	\
							_on_error)	\
	flick_iiopxx_cdr_decode_type_tag_and_value((_data),		\
						   _try_label,		\
						   _on_error)


/*
 * Encoding and decoding type tags, i.e., `CORBA::TypeCode's.
 *
 * These macros assume that when they are invoked, there is no open glob and
 * therefore no current chunk.
 *
 * `TypeCode's are handled in the same way as `any's --- see the macros and
 * comment above.
 *
 * The macros are defined for each supported encoding format (e.g., CDR).
 */
#define flick_iiopxx_cdr_encode_type_tag(_data,				 \
					 _try_label,			 \
					 _on_error) {			 \
       CORBA::Environment ACE_TRY_ENV;					 \
	/*								 \
	 * Resync TAO's stream with that of the Flick-generated stub.	 \
	 */								 \
	_stream->msg_block->wr_ptr((char *) _buf_current);		 \
	/*								 \
	 * Use TAO to encode the `any' value.  See TAO's `CDR.cpp' file, \
	 * `operator<<(TAO_OutputCDR& cdr, CORBA::Any &x)'.		 \
	 */								 \
	ACE_TRY_EX(_try_label) {					 \
		CORBA::TypeCode::traverse_status status =		 \
			TAO_MARSHAL_TYPECODE::instance()->encode(	 \
				0,					 \
				&(_data),				 \
				0,					 \
				_stream->tao_stream.out_cdr_ptr,	 \
				ACE_TRY_ENV);				 \
		ACE_TRY_CHECK_EX(_try_label);				 \
		if (status != CORBA::TypeCode::TRAVERSE_CONTINUE)	 \
			flick_stub_error(FLICK_ERROR_COMMUNICATION,	 \
					 _on_error);			 \
	}								 \
	ACE_CATCH (CORBA_Exception, ex) {				 \
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _on_error);	 \
	}								 \
	ACE_ENDTRY;							 \
	/*								 \
	 * Finally, resync the stub's stream with TAO's.		 \
	 */								 \
	_stream->msg_block = (ACE_Message_Block *)_stream->tao_stream.	 \
		out_cdr_ptr->current();					 \
	_stream->frag_start = _stream->msg_block->rd_ptr();		 \
	_buf_current = _stream->msg_block->wr_ptr();			 \
	_buf_end = (_stream->msg_block->base()				 \
		    + _stream->msg_block->size());			 \
}

#define flick_iiopxx_cdr_decode_type_tag(_data, _try_label, _on_error) { \
       CORBA::Environment ACE_TRY_ENV;					 \
	/*								 \
	 * Resync TAO's stream with that of the Flick-generated stub.	 \
	 */								 \
	_stream->msg_block->rd_ptr((char *) _buf_current);		 \
	/*								 \
	 * Use TAO to decode the `any' value.  See TAO's `CDR.cpp' file, \
	 * `operator>>(TAO_InputCDR& cdr, CORBA::Any &x)'.		 \
	 */								 \
	ACE_TRY_EX(_try_label) {					 \
		CORBA::TypeCode::traverse_status status =		 \
			TAO_MARSHAL_TYPECODE::instance()->decode(	 \
				0,					 \
				&((CORBA::TypeCode_ptr &)(_data)),	 \
				0,					 \
				_stream->tao_stream.in_cdr_ptr,		 \
				ACE_TRY_ENV);				 \
		ACE_TRY_CHECK_EX(_try_label);				 \
		if (status != CORBA::TypeCode::TRAVERSE_CONTINUE)	 \
			flick_stub_error(FLICK_ERROR_COMMUNICATION,	 \
					 _on_error);			 \
	}								 \
	ACE_CATCH (CORBA_Exception, ex) {				 \
		flick_stub_error(FLICK_ERROR_COMMUNICATION, _on_error);	 \
	}								 \
	ACE_ENDTRY;							 \
	/*								 \
	 * Finally, resync the stub's stream with TAO's.		 \
	 */								 \
	_buf_current = _stream->msg_block->rd_ptr();			 \
}

/*
 * We can just call the regular `cdr' versions; TAO's runtime handles swapping
 * as necessary.
 */
#define flick_iiopxx_cdr_swap_encode_type_tag(_data, _try_label, _on_error) \
	flick_iiopxx_cdr_encode_type_tag((_data), _try_label, _on_error)

#define flick_iiopxx_cdr_swap_decode_type_tag(_data, _try_label, _on_error) \
	flick_iiopxx_cdr_decode_type_tag((_data), _try_label, _on_error)


/*****************************************************************************/

#endif /* __flick_link_iiopxx_h */

/* End of file. */

