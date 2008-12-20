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

#define FLICK_PROTOCOL_TRAPEZE
#include "trapeze-link.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

/*
 * `flick_cdr_encode_IOR_internal' is the runtime function that encodes an
 * object reference as a CORBA-style Interoperable Object Reference (IOR).
 * It takes a significant amount of computes to produce an IOR, so it's not
 * worthwhile to inline this code.
 *
 * XXX --- The IIOP runtime should cache objects' IORs so that we can just
 * blast them into the message buffer.
 *
 * XXX --- If there is a bug in this code, you should also check `CORBA_ORB_
 * object_to_string' to see if has the same bug.  Someday we should combine the
 * marshaling guts of these two functions.
 */
#if 0
void flick_cdr_encode_IOR_internal(
	flick_marshal_stream_t stream,
	FLICK_TARGET obj,
	const char *link,
	int ref_adjust)
{
	unsigned int ior_prefix_pad;
	unsigned int obj_type_id_len;
	unsigned int obj_key_len;
	unsigned int host_name_len;
	unsigned int profile_len;
	unsigned int ior_len;
	
	char *buffer;
	unsigned int pos, len;
	
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
	/*
	 * Compute the number of pad bytes that must precede the IOR.
	 */
	if (((unsigned int) stream->buf_current) % 4)
		ior_prefix_pad = 4 -
				 (((unsigned int) stream->buf_current) % 4);
	else
		ior_prefix_pad = 0;
	
	if (obj) {
		/*
		 * Compute the lengths of the various strings that go into the
		 * IOR.
		 */
		obj_type_id_len = obj->type_id_len;
		obj_key_len = obj->key._length;
		host_name_len = strlen(obj->boa->orb->hostname);
		
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
		ior_len = (  ior_prefix_pad
			   + 4 /* `IOP::IOR.type_id' string length */
			   + (((obj_type_id_len + 1) + 3) & ~3)
			     /* `IOR.type_id' data + pad to 4-byte alignment */
			   + 4 /* `IOP::IOR.profiles' sequence length (1) */
			   + 4 /* `IOP::TaggedProfile.tag' */
			   + 4 /* `IOP::TaggedProfile.profile_data' seq len */
			   + profile_len /* `profile_data' sequence data*/
			  );
		
	} else {
		/*
		 * A null `FLICK_TARGET' has a simple profile.
		 */
		ior_len = (  ior_prefix_pad
			   + 4 /* `IOP::IOR.type_id' string length (0) */
			   + 4 /* `IOP::IOR.type_id' string data + pad */
			   + 4 /* `IOP::IOR.profiles' sequence length (0) */
			  );
	}
	
	/*
	 * Make sure that we have enough space in `stream' to contain the IOR.
	 * We only know how to do this for IIOP runtime buffers; our allocation
	 * code is adapted from `flick_iiop_encode_new_glob'.
	 */
	if (strcmp(link, "iiop"))
		assert(!"Can't marshal a CORBA IOR to a non-IIOP buffer.");
	
	if ((((char *) stream->buf_end) - ((char *) stream->buf_current))
	    < (int) ior_len) {
		int buf_len = ((char *) stream->buf_current)
			      - ((char *) stream->buf_start);
		int new_size = buf_len + ior_len;
		
		stream->buf_start = (void *) realloc(stream->buf_start,
						     new_size);
		if (!stream->buf_start)
			assert(!"Failed to realloc IIOP stream buffer.");
		
		stream->buf_current = ((char *) stream->buf_start) + buf_len;
		stream->buf_end     = ((char *) stream->buf_start) + new_size;
	}
	
	/*
	 * Finally, we are ready to encode the IOR for the given object.  Most
	 * of this code was ``stolen'' from `CORBA_ORB_object_to_string'.
	 *
	 * XXX --- Someday we need to combine the guts of this function with
	 * the guts of `CORBA_ORB_object_to_string'.
	 *
	 * `buffer' must be 4-byte aligned so that we can easily determine what
	 * values of `pos' represent 4-byte aligned positions.
	 */
	buffer = ((char *) stream->buf_current) + ior_prefix_pad;
	pos = 0;
	
	/* Encode the object `type_id'. */
	if (obj) {
		len = obj_type_id_len + 1;
		*(unsigned int *) &buffer[pos] = len;
		pos += 4;
		strncpy((&buffer[pos]), obj->type_id, len);
		pos += (len + 3) & ~3;
	} else {
		/* Null object references have an empty `type_id' string. */
		*(unsigned int *) &buffer[pos] = 1;
		pos += 4;
		*(char *) &buffer[pos] = 0;
		pos += 4;
	}
	
	/*
	 * Encode the number of TaggedProfiles the IOR TaggedProfile sequence
	 * (1 or 0).
	 */
	*(unsigned int *) &buffer[pos] = (!!obj);
	pos += 4;
	
	if (obj) {
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
		strncpy(&(buffer[pos]), obj->boa->orb->hostname,
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
	}
	assert((ior_prefix_pad + pos) == ior_len);
	
	/*
	 * Increment the stream's buffer pointer to point to the byte beyond
	 * the end of the IOR.
	 */
	stream->buf_current = buffer + pos;
}


/*****************************************************************************/

/*
 * `flick_cdr_decode_IOR_internal' is the runtime function that decodes an
 * Interoperable Object Reference (IOR) from a stream.  It takes a significant
 * amount of work to decode an IOR, so it's not wothwhile to inline this code.
 *
 * XXX --- The IIOP runtime should cache objects' IORs.
 *
 * XXX --- Most of this code was stolen from `CORBA_ORB_string_to_object'.  If
 * you find a bug in the code, check `CORBA_ORB_string_to_object' to see if it
 * has the same bug.  Someday we should combine the unmarshling guts of these
 * two functions.
 */
FLICK_TARGET flick_cdr_decode_IOR_internal(
	flick_marshal_stream_t stream,
	int cdr_swap,
	const char *link,
	int ref_adjust)
{
	char *host_name;
	unsigned short boa_port;
	char *obj_type_id, *obj_key;
	unsigned int obj_type_id_len, obj_key_len;
	
	struct hostent *he;
	CORBA_ORB orb;
	
	FLICK_TARGET res;
	
	unsigned int ior_prefix_pad;
	
	char *buf;
	unsigned int pos, len;
	unsigned int iiop_profile_pos;
	
	char encapsulation_endianness;
	unsigned int profile_count;
	unsigned int i;
	
	int bail;
	
	unsigned int endian_test = 1;
	char native_endianness = ((char *) &endian_test)[0];
	
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
	if (((unsigned int) stream->buf_current) % 4)
		ior_prefix_pad = 4 -
				 (((unsigned int) stream->buf_current) % 4);
	else
		ior_prefix_pad = 0;
	
	/*
	 * We are ready to decode the IOR from `stream'.  Most of this code was
	 * ``stolen'' from `CORBA_ORB_string_to_object'.
	 *
	 * XXX --- Someday we need to combine the guts of this function with
	 * the guts of `CORBA_ORB_string_to_object'.
	 *
	 * `buf' must be 4-byte aligned so that we can easily determine what
	 * values of `pos' represent 4-byte aligned positions.
	 */
	buf = ((char *) stream->buf_current) + ior_prefix_pad;
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
	
	obj_type_id_len = len - 1;
	obj_type_id = t_malloc(char, len);
	if (!obj_type_id) {
		fprintf(stderr,
			"Error: can't malloc memory for object type_id.\n");
		/*
		 * Don't bail yet; just set a flag, and bail after we've set
		 * our stream pointer to point after the IOR.
		 */
		bail = 1;
	} else {
		strncpy(obj_type_id, &(buf[pos]), len);
		if (obj_type_id[len - 1] != 0) {
			/* XXX --- This should never happen. */
			fprintf(stderr,
				"Warning: IOR object type_id was not NUL "
				"terminated.  Terminating it.\n");
			obj_type_id[len - 1] = 0;
		}
	}
	pos += (len + 3) & ~3;
	
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
		stream->buf_current = buf + pos;
		
		if (obj_type_id)
			free(obj_type_id);
		return 0;
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
	 * At this point, `pos' points to the byte after the IOR.  Update our
	 * stream pointer before we try to parse the IIOP 1.0 Profile.  (This
	 * way, if we bail out, our stream still shows that we decoded the
	 * complete IOR.)
	 */
	stream->buf_current = buf + pos;
	
	if (bail)
		/*
		 * We're bailing because we couldn't allocate memory for the
		 * `obj_type_id' string.  We waited until here to bail out so
		 * that the stream pointer would point past the IOR.
		 */
		return 0;
	
	if (!iiop_profile_pos) {
		fprintf(stderr,
			"Warning: no IIOP Profile in IOR.  Returning a null "
			"object reference.\n");
		free(obj_type_id);
		return 0;
	}
	pos = iiop_profile_pos;
	
	/*
	 * At this point, `pos' is the index of the IIOP Profile encapsulation
	 * in `buf'.  `pos' is also 4-byte aligned, since we just read past the
	 * (unsigned long) ProfileId.
	 *
	 * By adding 4 to `pos' here, we skip over the endianness indicator
	 * (1 octet) for the IIOP Profile encapsulation, the `IIOP::Version'
	 * struct (2 chars), and the pad byte preceding the `host' string
	 * length.
	 */
	encapsulation_endianness = buf[pos];
	pos += 4;
	
	/* Get the host name. */
	if (encapsulation_endianness == native_endianness)
		len = (*(unsigned int *) &buf[pos]);
	else
		len = swap_long(*(unsigned int *) &buf[pos]);
	pos += 4;
	
	host_name = t_malloc(char, len);
	if (!host_name) {
		fprintf(stderr,
			"Error: can't malloc memory for host name.\n");
		free(obj_type_id);
		return 0;
	}
	strncpy(host_name, &(buf[pos]), len);
	if (host_name[len - 1] != 0) {
		/* XXX --- This should never happen. */
		fprintf(stderr,
			"Warning: IIOP host name was not NUL terminated.  "
			"Terminating it.\n");
		host_name[len - 1] = 0;
	}
	pos += (len + 1) & ~1;
	
	/* Get the BOA port. */
	if (encapsulation_endianness == native_endianness)
		boa_port = (*(unsigned short *) &buf[pos]);
	else
		boa_port = swap_short(*(unsigned short *) &buf[pos]);
	pos += 2;
	
	/* Get the object key. */
	/* Align for object key length. */
	pos = (pos + 3) & ~3;
	if (encapsulation_endianness == native_endianness)
		obj_key_len = (*(unsigned long *) &buf[pos]);
	else
		obj_key_len = swap_long(*(unsigned long *) &buf[pos]);
	pos += 4;
	
	obj_key = t_malloc(char, obj_key_len);
	if (!obj_key) {
		fprintf(stderr,
			"Error: can't malloc memory for object key.\n");
		free(host_name);
		free(obj_type_id);
		return 0;
	}
	memcpy(obj_key, &(buf[pos]), obj_key_len);
	/*
	 * Note that the object key is a sequence of octets, *not* a
	 * NUL-terminated string.  So do not check for termination.
	 */
	pos += obj_key_len; /* Leave `pos' at single-byte alignment. */
	
	/*
	 * Convert the `host_name' into a FQDN, which we use to key our ORBs.
	 */
	if (!(he = gethostbyname(host_name))) {
		fprintf(stderr,
			"Warning: unable to identify host `%s'.\n",
			host_name);
	} else {
		free(host_name);
		host_name = t_malloc(char, strlen(he->h_name) + 1);
		if (!host_name) {
			fprintf(stderr,
				"Error: can't malloc memory for host "
				"name.\n");
			free(obj_type_id);
			free(obj_key);
			return 0;
		}
		strcpy(host_name, he->h_name);
	}
	
	/*
	 * At this point we have `host_name', `boa_port', `obj_type_id', and
	 * `obj_key' set.  We need to build the object --- but don't try to
	 * connect to it just yet.
	 */
	for (i = 0; i < (unsigned int) orb_count; ++i)
		if (!strcmp(orbs[i]->hostname, host_name))
			break;
	
	if (((int) i) == orb_count) {
		CORBA_ORB *new_orbs;
		
		new_orbs = t_realloc(orbs, CORBA_ORB, (orb_count + 1));
		if (!new_orbs) {
			fprintf(stderr,
				"Error: can't malloc memory for new ORB.\n");
			free(host_name);
			free(obj_type_id);
			free(obj_key);
			return 0;
		}
		orbs = new_orbs;
		++orb_count;
		
		orbs[i] = t_calloc(struct CORBA_ORB_type, 1);
		if (!orbs[i]) {
			fprintf(stderr,
				"Error: can't malloc memory for new ORB.\n");
			free(host_name);
			free(obj_type_id);
			free(obj_key);
			return 0;
		}
		
		orbs[i]->hostname = host_name;
		
	} else
		/*
		 * Don't need `host_name' anymore; the located ORB already has
		 * a copy of its name.
		 */
		free(host_name);
	
	orb = orbs[i];
	
	for (i = 0; (int)i < orb->OA_count; ++i)
		if (orb->boas[i]->hostport == boa_port)
			break;
	
	if (((int) i) == orb->OA_count) {
		CORBA_BOA *new_boas;
		
		new_boas = t_realloc(orb->boas,
				     CORBA_BOA, (orb->OA_count + 1));
		if (!new_boas) {
			fprintf(stderr,
				"Error: can't malloc memory for new BOA.\n");
			free(obj_type_id);
			free(obj_key);
			return 0;
		}
		
		new_boas[i] = t_calloc(struct CORBA_BOA_type, 1);
		if (!new_boas[i]) {
			fprintf(stderr,
				"Error: can't malloc memory for new BOA.\n");
			free(obj_type_id);
			free(obj_key);
			return 0;
		}
		
		orb->boas = new_boas;
		orb->OA_count += 1;
		
		orb->boas[i]->orb = orb;
		orb->boas[i]->hostport = boa_port;
	}
	
	/*
	 * Finally, we can create the FLICK_TARGET!  Yippee!
	 */
	res = t_malloc(FLICK_TARGET_STRUCT, 1);
	if (!res) {
		fprintf(stderr,
			"Error: can't malloc memory for Object.\n");
		free(obj_type_id);
		free(obj_key);
		return 0;
	}
	
	res->boa = orb->boas[i];
	res->key._buffer = obj_key;
	res->key._length = obj_key_len;
	res->key._maximum = 1024;
	res->type_id = obj_type_id;
	res->type_id_len = obj_type_id_len;
	
	return res;
}
#endif

/*****************************************************************************/

/* End of file. */

