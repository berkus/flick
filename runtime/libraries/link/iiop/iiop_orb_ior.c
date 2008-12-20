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

#include "iiop-link.h"
#include <flick/encode/cdr.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

/*
 * This function converts an object into an IOR.  It is used by
 * `CORBA_ORB_object_to_string()' and `flick_cdr_encode_IOR_internal()'.
 * `buf' is a destination buffer which may be reallocated if necessary.
 * `buffer_len' is an inout parameter which is how much memory is
 * allocated for buf.
 * `start_pos' is an index into buf, which is where the ior will be placed.
 * The function returns the actual size of the ior in bytes or 0 for Error.
 */

const unsigned int null_obj_ior[] = {1, 0, 0};
const unsigned int null_obj_ior_len = 12; /* in bytes */

unsigned int flick_cdr_make_ior(/*  in   */ FLICK_TARGET obj,
				/* inout */ char **buf,
				/* inout */ unsigned int *buffer_len,
				/*  in   */ unsigned int start_pos)
{
	unsigned int obj_type_id_len = 0;
	unsigned int obj_key_len = 0;
	unsigned int host_name_len = 0;
	unsigned int profile_len = 0;
	unsigned int ior_len, pos, len;
	
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
	char *buffer;
	
	/* buffer_len and buf must be valid pointers */
	assert(buffer_len);
	assert(buf);
	
	buffer = *buf;
	
	if (!obj) {
		/* ensure there's enough space in the buffer */
		if (*buffer_len < (len = start_pos + null_obj_ior_len)) {
			buffer = t_realloc(buffer, char, len);
			if (!buffer)
				return 0; /* ERROR */
			*buf = buffer;
			*buffer_len = len;
		}
		
		/* place the null_obj_ior into the buffer */
		memcpy(buffer, (char *)null_obj_ior, null_obj_ior_len);

		/* done */
		return null_obj_ior_len;
	}
	
	/*
	 * Compute the lengths of the various strings that go into the
	 * IOR.
	 */
	obj_type_id_len = obj->type_id_len;
	obj_key_len = obj->key._length;
	host_name_len = strlen(obj->boa->hostname);
	
	/*
	 * Compute the number of bytes in the `IIOP::ProfileBody'.
	 * (Refer to Section 12.7.2 of the CORBA 2.0 specification.
	 * Also note that this same calculation is made in the runtime
	 * function `CORBA_ORB_object_to_string'.)
	 */
	profile_len = (  1 /* encapsulation (1 octet) */
			 + 2 /* `IIOP::Version' (2 chars) */
			 + 1 /* pad byte before host name length */
			 + 4 /* host name length */
			 + (host_name_len + 1) /* host name + NUL */
			 + ((host_name_len % 2) ? 0 : 1)
			 /* pad to get to 2-byte alignment */
			 + 2 /* port number */
			 + (((host_name_len % 4) >= 2) ? 2 : 0)
			 /* pad to get to 4-byte alignment */
			 + 4 /* object key length */
			 + obj_key_len /* object key len (incl. NUL) */
		);
	
	/*
	 * Compute the number of bytes in the entire IOR, including the
	 * preceding pad.
	 */
	ior_len = (4 /* `IOP::IOR.type_id' string length */
		   + (((obj_type_id_len + 1) + 3) & ~3)
		   /* `IOR.type_id' data + pad to 4-byte alignment */
		   + 4 /* `IOP::IOR.profiles' sequence length (1) */
		   + 4 /* `IOP::TaggedProfile.tag' */
		   + 4 /* `IOP::TaggedProfile.profile_data' seq len */
		   + profile_len /* `profile_data' sequence data*/
		);
	
	if (*buffer_len < (len = start_pos + ior_len)) {
		buffer = t_realloc(buffer, char, len);
		if (!buffer)
			return 0; /* ERROR */
		*buf = buffer;
		*buffer_len = len;
	}
	
	/* go to the correct location for storing the IOR */
	buffer = (char *)buffer + start_pos;
	
	/* Clear the buffer so we don't have to zero out pad bytes. */
	memset(buffer, 0, (ior_len + 3) & ~3);
	pos = 0;
	
	/* Encode the object `type_id'. */
	len = obj_type_id_len + 1;
	*(unsigned int *) &buffer[pos] = len;
	pos += 4;
	strncpy((&buffer[pos]), obj->type_id, len);
	pos += (len + 3) & ~3;
	
	/*
	 * Encode the number of TaggedProfiles the IOR TaggedProfile sequence
	 * (1 or 0).
	 */
	*(unsigned int *) &buffer[pos] = (!!obj);
	pos += 4;
	
	/* Encode the ProfileID and the length of the profile body. */
	*(unsigned int *) &buffer[pos] = IOP_TAG_INTERNET_IOP;
	pos += 4;
	*(unsigned int *) &buffer[pos] = profile_len;
	pos += 4;
	
	/* Enocde the encapsulation flag. */
	buffer[pos] = native_endianness;
	pos += 1;
	/* Encode the `IIOP::Version'. */
	buffer[pos] = 1;
	pos += 1;
	buffer[pos] = 0;
	pos += 1 + 1 /* pad byte */;
	
	/* Encode the host name. */
	*(unsigned int *) &buffer[pos] = host_name_len + 1;
	pos += 4;
	strncpy(&(buffer[pos]), obj->boa->hostname,
		(host_name_len + 1));
	pos += (host_name_len + 1 /* NUL */ + 1 /* opt pad byte */)
	       & ~1;
	
	/* Encode the port number. */
	*(unsigned short *) &buffer[pos] = obj->boa->hostport;
	pos = (pos + 2 /* port */ + 3 /* opt pad */) & ~3;
	
	/* Encode the object key. */
	*(unsigned int *) &buffer[pos] = obj_key_len;
	pos += 4;
	memcpy(&(buffer[pos]), obj->key._buffer, obj_key_len);
	pos += obj_key_len;
	
	assert(pos == ior_len);		
	
	return ior_len;
}


/*
 * `flick_cdr_encode_IOR_internal' is the runtime function that encodes an
 * object reference as a CORBA-style Interoperable Object Reference (IOR).
 * It takes a significant amount of computes to produce an IOR, so it's not
 * worthwhile to inline this code.
 */
int flick_cdr_encode_IOR_internal(
	flick_marshal_stream_t _stream,
	FLICK_TARGET obj,
	const char *link,
	int ref_adjust)
{
	unsigned int ior_prefix_pad, ior_len;
	
	/*
	 * We only know how to do this for IIOP runtime buffers; our stream
	 * code is adapted from `flick_iiop_encode_new_glob'.
	 */
	if (strcmp(link, "iiop"))
		assert(!"Can't marshal a CORBA IOR to a non-IIOP buffer.");
		
	/*
	 * Compute the number of pad bytes that must precede the IOR.
	 */
	if ((unsigned long) _stream->buf_current % 4)
		ior_prefix_pad = 4 - ((unsigned long)_stream->buf_current % 4);
	else
		ior_prefix_pad = 0;
	
	/* Encode the precomputed IOR */
	if (obj) {
		ior_len = ior_prefix_pad + obj->ior_len;
		flick_iiop_encode_new_glob(ior_len, error);
		flick_iiop_encode_new_chunk(ior_len);
		flick_cdr_encode_bcopy(ior_prefix_pad, obj->ior, obj->ior_len);
		flick_iiop_encode_end_chunk(ior_len);
		flick_iiop_encode_end_glob(ior_len);
	} else {
		flick_iiop_encode_new_glob(null_obj_ior_len, error);
		flick_iiop_encode_new_chunk(null_obj_ior_len);
		flick_cdr_encode_bcopy(ior_prefix_pad,
				       null_obj_ior, null_obj_ior_len);
		flick_iiop_encode_end_chunk(null_obj_ior_len);
		flick_iiop_encode_end_glob(null_obj_ior_len);
	}
	return 0;
  error:
	return 1;
}


/*****************************************************************************/

/* extracts the data from an IOR.  Returns the length of the data */
int flick_cdr_parse_IOR(char *buf, int cdr_swap /* 1 if swapping is active */,
	      char **out_OAaddr,
	      unsigned short *out_boa_port,
	      char **out_obj_type_id,
	      unsigned int *out_obj_type_id_len,
	      CORBA_octet **out_obj_key,
	      unsigned int *out_obj_key_len,
	      CORBA_Environment *ev) 
{
	/* buf does *not* include any `IOR:' prefix */
	
	/* We're using the CORBA 2.0 p10.17 stringified object. */
	
	unsigned int pos, len, res, iiop_profile_pos;
	int bail;
		
	unsigned int i;
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
	char encapsulation_endianness;
	unsigned int profile_count;
	
	/* No object --> no BOA --> no `CORBA_BOA_set_exception'. */
	ev->_major = CORBA_NO_EXCEPTION;
	
	/*
	 * `buf' now holds a CORBA encapsulation of an IOR as described
	 * in Section 10.6.2 of the CORBA 2.0 specification.
	 *
	 * `pos' is our index into `buf'.
	 */
	pos = 0;
	bail = 0;	
	
	/*
	 * First, get the object `type_id', a string.
	 */
	if (!cdr_swap)
		len = (*(unsigned int *) &buf[pos]);
	else
		len = swap_long(*(unsigned int *) &buf[pos]);
	pos += 4;
	
	if (len) {
		*out_obj_type_id_len = len - 1;
		*out_obj_type_id = t_malloc(char, len);
		if (!*out_obj_type_id) {
			flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
					    0, CORBA_COMPLETED_NO);
			fprintf(stderr,
				"Error: can't malloc memory for object "
				"type_id.\n");
			bail = 1;
		} else {		
			strncpy(*out_obj_type_id, &(buf[pos]), len);
			if ((*out_obj_type_id)[len - 1] != 0) {
				/* XXX --- This should never happen. */
				fprintf(stderr,
					"Warning: IOR object type_id was not "
					"NUL terminated.  Terminating it.\n");
				(*out_obj_type_id)[len - 1] = 0;
			}
		}	
		pos += (len + 3) & ~3;
	} else {		
		*out_obj_type_id_len = 0;
		*out_obj_type_id = 0;
	}
	
	/*
	 * Now, find the IOP Profile within our IOR's sequence of
	 * TaggedProfiles.
	 */
	if (!cdr_swap)
		profile_count = (*(unsigned int *) &buf[pos]);
	else
		profile_count = swap_long(*(unsigned int *) &buf[pos]);
	pos += 4;
	if (profile_count == 0) {
		/*
		 * No profiles?  Then this is a null object reference
		 * (Section 10.6.2).
		 */
		if (*out_obj_type_id) free(*out_obj_type_id);
		
		/* nullify the data pointers */
		*out_OAaddr = *out_obj_type_id = 0;
		*out_obj_key = 0;
		return pos; /* return the total size of the ior */
	}
	
	iiop_profile_pos = 0;
	for (i = 0; i < profile_count; ++i) {
		IOP_ProfileId tag;
		
		/* Align to read next 4-byte tag. */
		pos = (pos + 3) & ~3;
		
		if (!cdr_swap) {
			tag = (*(unsigned int *) &buf[pos]);
			pos += 4;
			len = (*(unsigned int *) &buf[pos]);
			pos += 4;
		} else {
			tag = swap_long(*(unsigned int *) &buf[pos]);
			pos += 4;
			len = swap_long(*(unsigned int *) &buf[pos]);
			pos += 4;
		}
		
		if (tag == IOP_TAG_INTERNET_IOP) {
			/*
			 * Is this an IIOP 1.0 Profile?  Note that `buf[pos]'
			 * currently points to an encapsulation header
			 * (a char).
			 */
			if ((buf[pos + 1] == 1) && buf[pos + 2] == 0)
				/*
				 * Don't break yet --- just remember this `pos'
				 * and keep scanning until we reach the end of
				 * the IOR.
				 */
				iiop_profile_pos = pos;
		}
		/* Skip over this profile. */
		pos += len;
	}
	/*
	 * At this point, `pos' points to the byte after the IOR.  We will
	 * return this value if we bail out so that if we are decoding from
	 * a stream, we can adjust the stream pointer as if we decoded
	 * the complete IOR.)
	 */
	res = pos;
		
	if (bail)
		/*
		 * We're bailing because we couldn't allocate memory for the
		 * `obj_type_id' string.  We waited until here to bail out so
		 * that the stream pointer would point past the IOR.
		 */
		return res;
	
	if (!iiop_profile_pos) {
		/* XXX --- Is `BAD_PARAM' the right exception? */
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_BAD_PARAM,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Warning: no IIOP Profile in IOR.  Returning a null "
			"object reference.\n");
		if (*out_obj_type_id) free(*out_obj_type_id);
		return res;
	}
	pos = iiop_profile_pos;
	
	/*
	 * At this point, `pos' is the index of the IIOP Profile
	 * encapsulation in `buf'.  `pos' is also 4-byte aligned, since
	 * we just read past the (unsigned int) ProfileId.
	 *
	 * By adding 4 to `pos' here, we skip over the endianness
	 * indicator (1 octet) for the IIOP Profile encapsulation, the
	 * `IIOP::Version' struct (2 chars), and the pad byte preceding
	 * the `host' string length.
	 */
	encapsulation_endianness = buf[pos];
	pos += 4;
	
	/* Get the host name. */
	if (encapsulation_endianness == native_endianness)
		len = (*(unsigned int *) &buf[pos]);
	else
		len = swap_long(*(unsigned int *) &buf[pos]);
	pos += 4;
	
	*out_OAaddr = t_malloc(char, len);
	if (!*out_OAaddr) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for host name.\n");
		if (*out_obj_type_id) free(*out_obj_type_id);
		return res;
	}
	strncpy(*out_OAaddr, &(buf[pos]), len);
		
	if ((*out_OAaddr)[len - 1] != 0) {
		/* XXX --- This should never happen. */
		fprintf(stderr,
			"Warning: IIOP host name was not NUL "
			"terminated.  Terminating it.\n");
		(*out_OAaddr)[len - 1] = 0;
	}
	pos += (len + 1) & ~1;
	
	/* Get the BOA port. */
	if (encapsulation_endianness == native_endianness)
		*out_boa_port = (*(unsigned short *) &buf[pos]);
	else
		*out_boa_port = swap_short(*(unsigned short *) &buf[pos]);
	pos += 2;
	
	/* Get the object key. */
	/* Align for object key length. */
	pos = (pos + 3) & ~3;
	if (encapsulation_endianness == native_endianness)
		*out_obj_key_len = (*(unsigned int *) &buf[pos]);
	else
		*out_obj_key_len = swap_long(*(unsigned int *) &buf[pos]);
	pos += 4;
	
	*out_obj_key = t_malloc(CORBA_octet, *out_obj_key_len);
	if (!*out_obj_key) {
		flick_set_exception(0 /*boa*/, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for object "
			"key.\n");
		if (*out_obj_type_id) free(*out_obj_type_id);
		if (*out_OAaddr) free(*out_OAaddr);
		return res;
	}
	memcpy(*out_obj_key, &(buf[pos]), *out_obj_key_len);
	/*
	 * Note that the object key is a sequence of octets, *not* a
	 * NUL-terminated string.  So do not check for termination.
	 */
	
	return res;
}

/*
 * `flick_cdr_decode_IOR_internal' is the runtime function that decodes an
 * Interoperable Object Reference (IOR) from a stream.  It takes a significant
 * amount of work to decode an IOR, so it's not wothwhile to inline this code.
 */
FLICK_TARGET flick_cdr_decode_IOR_internal(
	flick_marshal_stream_t _stream,
	int cdr_swap,
	const char *link,
	int ref_adjust)
{
	char *OAaddr;
	unsigned short boa_port;
	char *obj_type_id;
	CORBA_octet *obj_key;
	unsigned int obj_type_id_len, obj_key_len;
	
	CORBA_Environment _ev;
	FLICK_TARGET res;
	
	unsigned int ior_prefix_pad, ior_size;
	
	/*
	 * Make sure that we're unmarshaling from an IIOP runtime stream, since
	 * that's the only kind we know how to manipulate after decoding the
	 * IOR.
	 */
	if (strcmp(link, "iiop"))
		assert(!"Can't unmarshal a CORBA IOR from a non-IIOP buffer.");
	
	/*
	 * Compute the number of pad bytes that must precede the IOR.
	 */
	if ((unsigned long) _stream->buf_current % 4)
		ior_prefix_pad = 4 - ((unsigned long)_stream->buf_current % 4);
	else
		ior_prefix_pad = 0;
	
	/*
	 * We are ready to decode the IOR from `stream'.
	 */
	ior_size = flick_cdr_parse_IOR(((char *) _stream->buf_current)
				       + ior_prefix_pad,  
				       cdr_swap, &OAaddr,
				       &boa_port, &obj_type_id,
				       &obj_type_id_len,
				       &obj_key, &obj_key_len, &_ev);
	/*
	 * Update our stream pointer before we try to parse the IIOP 1.0
	 * Profile.  (This way, if we bail out, our stream still shows that
	 * we decoded the complete IOR.)
	 */
	ior_size += ior_prefix_pad;
	flick_iiop_check_span(ior_size, error);
	flick_iiop_decode_new_glob(ior_size);
	flick_iiop_decode_new_chunk(ior_size);
	flick_iiop_decode_end_chunk(ior_size);
	flick_iiop_decode_end_glob(ior_size);
	
	if (_ev._major != CORBA_NO_EXCEPTION) 
		return 0;
	
	res = flick_create_object(0 /*use default ORBid*/,
				  0 /*use default OAid*/,
				  OAaddr, boa_port, obj_type_id,
				  obj_type_id_len,
				  obj_key, obj_key_len, &_ev);
        /* was copied by CORBA_ORB_BOA_init_internal */
	if (OAaddr) free(OAaddr); 
	if (_ev._major != CORBA_NO_EXCEPTION) {
		if (obj_type_id) free(obj_type_id);
		if (obj_key) free(obj_key);
		return 0;
	}
		
        /* return the object */
	return res;

  error:
	return 0;
}


/*****************************************************************************/

/* End of file. */

