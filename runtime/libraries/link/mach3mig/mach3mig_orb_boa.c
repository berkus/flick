/*
 * Copyright (c) 1998, 1999 The University of Utah and
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

#include "mach3mig-link.h"
#include "flick/encode/mach3mig.h"

CORBA_ReferenceData *CORBA_BOA_get_id(CORBA_BOA ths, CORBA_Object obj,
				      CORBA_Environment *ev)
{
	CORBA_ReferenceData *res;
	int i;
	char *name = 0;

	for (i = 0; i < ths->obj_count; i++) {
		if (ths->objs[i] == obj) {
			name = ths->names[i];
			break;
		}
	}
	
	res = t_malloc(CORBA_ReferenceData, 1);
	if (!res) {
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return 0;
	}
	
	if (name) {
		res->_length = strlen(name);
		res->_maximum = res->_length;
		res->_buffer = t_malloc(CORBA_octet, res->_maximum);
		if (!res->_buffer) {
			free(res);
			flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
					    0, CORBA_COMPLETED_NO);
			return 0;
		}
		memcpy(res->_buffer, name, res->_length);
	} else {
		res->_length = res->_maximum = 0;
		res->_buffer = 0;
	}
	
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
	
	return res;
}

/* used to be called CORBA_BOA_obj_is_ready, which was entirely wrong.
   see Section 8.2 of the CORBA 2.0 Spec.   
   This function will create an object of the given type and implementation. */
CORBA_Object CORBA_BOA_create(CORBA_BOA ths, CORBA_ReferenceData *obj_key, 
			      const char *type_id, FLICK_SERVER impl,
			      CORBA_Environment *ev)
{
	CORBA_Object *new_objs;
	char **new_names;
	char **new_types;
	FLICK_SERVER *new_impls;
	
	char *ref_key_buffer;
	char *ref_type_id;
	
	int type_id_len;
	
	mach_port_t new_right;
	
	/* Allocate the memory we need for the new object reference. */
	type_id_len = strlen(type_id);

	ref_key_buffer = t_malloc(char, obj_key->_length + 1);
	ref_type_id = t_malloc(char, (type_id_len + 1));
	
	if (!ref_key_buffer || !ref_type_id) {
		if (ref_key_buffer) free(ref_key_buffer);
		if (ref_type_id) free(ref_type_id);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't malloc memory for new object.\n");
		return 0;
	}
	
	/* Reallocate our BOA's vector of references. */
	new_objs = t_realloc(ths->objs,
			     CORBA_Object,
			     (ths->obj_count + 1));
	new_names = t_realloc(ths->names,
			      char *,
			      (ths->obj_count + 1));
	new_types = t_realloc(ths->types,
			      char *,
			      (ths->obj_count + 1));
	new_impls = t_realloc(ths->impls,
			      FLICK_SERVER,
			      (ths->obj_count + 1));
	
	if (!new_objs || !new_names || !new_types || !new_impls) {
		free(ref_key_buffer);
		free(ref_type_id);
		if (new_objs) free(new_objs);
		if (new_names) free(new_names);
		if (new_types) free(new_types);
		if (new_impls) free(new_impls);
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't realloc memory for CORBA_BOA object "
			"references.\n");
		return 0;
	}
	
	/* allocate a port */
	if (mach_port_allocate(mach_task_self(),
			       MACH_PORT_RIGHT_RECEIVE,
			       &new_right)
	    != KERN_SUCCESS) {
		free(ref_key_buffer);
		free(ref_type_id);
		free(new_objs);
		free(new_names);
		free(new_types);
		flick_set_exception(ths, ev, ex_CORBA_INTERNAL,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't allocate a receive port for object "
			"reference.\n");
		return 0;
	}
	
	memcpy(ref_key_buffer, obj_key->_buffer, obj_key->_length);
	ref_key_buffer[obj_key->_length] = 0; /* null-terminate */
	strncpy(ref_type_id, type_id, (type_id_len + 1));

	/* check this port into the nameserver */
	if (netname_check_in(ths->name_server_port,
			     ref_key_buffer,
			     mach_task_self(),
			     new_right)
	    != KERN_SUCCESS) {
		free(ref_key_buffer);
		free(ref_type_id);
		free(new_objs);
		free(new_names);
		free(new_types);
		flick_set_exception(ths, ev, ex_CORBA_INTERNAL,
				    0, CORBA_COMPLETED_NO);
		fprintf(stderr,
			"Error: can't check in object to nameserver.\n");
		return 0;
	}
	
	ths->objs = new_objs;
	ths->names = new_names;
	ths->types = new_types;
	ths->impls = new_impls;

	ths->impls[ths->obj_count] = impl;
	ths->types[ths->obj_count] = ref_type_id;
	ths->names[ths->obj_count] = ref_key_buffer;
	ths->objs[ths->obj_count] = new_right;
	ths->obj_count++;
	
	/* Finally, print the URL--style stringified object ref. */
	{
		char *url;
		
		url = CORBA_ORB_object_to_string(ths->orb, new_right, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return 0;
		
		printf("Object `%s' is ready.\n", ref_key_buffer);
		printf("  URL:\t%s\n", url);
		
		free(url);
	}
	
	CORBA_BOA_set_exception(ths, ev, CORBA_NO_EXCEPTION, 0, 0);
	
	return new_right;
}

void CORBA_BOA_impl_is_ready(CORBA_BOA ths, CORBA_Environment *ev)
{
	mach_port_t task_self = mach_task_self();
	mach_port_t portset;
	int i;
	
	kern_return_t res;
	char *buffer;
	mach_msg_header_t *request;
	mig_reply_header_t *reply;
	/*
	 * XXX - Here we specify the largest size message the server
	 * can receive.  If necessary, change the value of
	 * MAX_REPLY_BUFFER_SIZE in runtime/headers/flick/link/mach3mig.h.
	 */
	int bufsize = MAX_REPLY_BUFFER_SIZE;
	
	/* Allocate the send/receive buffer */
	buffer = t_malloc(char, bufsize);
	if (!buffer) {
		flick_set_exception(ths, ev, ex_CORBA_NO_MEMORY,
				    0, CORBA_COMPLETED_NO);
		return;
	}
	request = (mach_msg_header_t *) buffer;
	reply = (mig_reply_header_t *) buffer;
	
	/*
	 * Under normal conditions, we will never return.  If we ever
	 * do, it is because of an unrecoverable error.  Set the
	 * exception now, so handling the error is easy.
	 */
	flick_set_exception(ths, ev, ex_CORBA_INTERNAL,
			    0, CORBA_COMPLETED_NO);
	
	/* Create a port set for all the objects' ports. */
	if (mach_port_allocate(task_self,
			       MACH_PORT_RIGHT_PORT_SET,
			       &portset)
	    != KERN_SUCCESS)
		return;
	
	/* Advertise our server name so clients can connect. */
	for (i = 0; i < ths->obj_count; i++) {
		/* place the object's port into our port set */
		if (mach_port_move_member(task_self,
					  ths->objs[i],
					  portset)
		    != KERN_SUCCESS)
			return;
	}
	
	while (1) {
		res = mach_msg(request, MACH_RCV_MSG, 0, bufsize,
			       portset, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
		
		if (res != MACH_MSG_SUCCESS) {
			fprintf(stderr, "Unsuccessful msg receive: %s",
				mach_error_string(res));
			goto bail;
		}
		while (res == MACH_MSG_SUCCESS) {
			boolean_t ok;
			
			/* XXX - linear search through objects to find the
			   correct server function to call. */
			for (i = 0; i < ths->obj_count; i++) {
				if (ths->objs[i] == request->msgh_local_port) {
					ok = ths->impls[i](
						request,
						(mach_msg_header_t *) reply);
					break;
				}
			}
			
			/* check to make sure we found the object */
			if (i == ths->obj_count || !ok) {
				/* build an appropriate error */
				struct flick_stub_state _stub_state = { 0, 0 };
				mach_msg_header_t *OutHeadP
					= (mach_msg_header_t *) reply;
				mach_msg_header_t *InHeadP = request;
				void *_buf_current, *_buf_end;
				mig_reply_header_t *_buf_start;
				unsigned int exstrlen
					= sizeof(ex_CORBA_NO_IMPLEMENT);
				unsigned int len = * (unsigned int *)
						   &InHeadP[1];
				unsigned int msgsize = 24 + (exstrlen + 3) % 4;
				unsigned int i;
				
				flick_mach3mig_server_start_encode();
				flick_mach3mig_encode_new_glob(msgsize
							       + (len + 2) * 8,
							       _onerror1);
				/*
				 * Copy the operation ID from the request.
				 * We have to prepend '$' for the reply name.
				 */
				flick_mach3mig_encode_new_chunk((len + 2) * 8);
				flick_mach3mig_encode_unsigned32(
					0, len + 1, unsigned int);
				flick_mach3mig_encode_char8(
					8, '$', char);
				for (i = 0; i < len; i++)
					flick_mach3mig_encode_char8(
						i * 8 + 16,
						*((char *)&InHeadP[1] + 12),
						char);
				flick_mach3mig_encode_end_chunk((len + 2) * 8);
				flick_mach3mig_encode_new_chunk(msgsize);
				flick_mach3mig_encode_unsigned32(
					0,
					CORBA_SYSTEM_EXCEPTION,
					unsigned int);
				flick_mach3mig_array_encode_string_c_type(
					8, exstrlen, 1, 0, _type);
				flick_mach3mig_array_encode_bcopy(
					12,
					ex_CORBA_NO_IMPLEMENT,
					exstrlen);
				flick_mach3mig_encode_unsigned32(
					msgsize - 16, 0 /* minor code */,
					unsigned int); 
				flick_mach3mig_encode_unsigned32(
					msgsize - 8, CORBA_COMPLETED_NO,
					unsigned int);
				flick_mach3mig_encode_end_chunk(msgsize);
				flick_mach3mig_encode_end_glob(msgsize +
							       (len + 2) * 8);
			  _onerror1:
				flick_mach3mig_server_end_encode();
			}
			
			if (reply->Head.msgh_remote_port == MACH_PORT_NULL) {
				/* no reply port, just get another request */
				break;
			}
			
			if (ok && reply->RetCode == MIG_NO_REPLY) {
				/* deallocate reply port right */
				(void) mach_port_deallocate(
					task_self,
					reply->Head.msgh_remote_port);
				break;
			}
			
			/* Send reply to prev request and receive another: */
			
			res = mach_msg(&reply->Head,
				       MACH_SEND_MSG | MACH_RCV_MSG,
				       reply->Head.msgh_size,
				       bufsize,
				       portset,
				       MACH_MSG_TIMEOUT_NONE,
				       MACH_PORT_NULL);
			if (res != MACH_MSG_SUCCESS) {
				if (res == MACH_SEND_INVALID_DEST) {
				/* deallocate reply port right */
					(void) mach_port_deallocate(
						task_self,
						reply->Head.msgh_remote_port);
				} else {
					fprintf(stderr,
						"Unsuccessful msg "
						"send/receive: %s",
						mach_error_string(res));
					goto bail;
				}
			}

			/*
			 * XXX - check for reallocated buffer?
			 * The dispatch funcions should never realloc
			 * the buffer, so we shouldn't have to check.
			 */
		}
	}
	
  bail:
	/* Should never be reached. */
	/* but just in case, clean up everything... */
	mach_port_destroy(task_self,
			  portset);
}

/* End of file. */

