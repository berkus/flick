/*
 * Copyright (c) 1999 The University of Utah and
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

#include <sys/param.h>

#include "trapeze-link.h"

/*
 * Our Trapeze message format has a fixed representation for system exception
 * data: two 4-byte, network-byte-order unsigned integers.  The first integer
 * describes the type of the exception, and the second describes whether or not
 * the operation was completed.
 *
 * This file contains the functions to translate between this format and the
 * various presentations of system exceptions.
 */

/*****************************************************************************/

/*
 * The `trapeze_corba_exception_table' is used to translate between Trapeze
 * on-the-wire system exception codes and CORBA system exception IDs, which are
 * strings.
 *
 * Given a Trapeze exception ID (an `ex_TRAPEZE_*' value as defined in
 * <flick/link/trapeze.h>, one indexes into this array using that ID to find
 * the corresponding CORBA ID.  The final element of this array must be a null
 * pointer.
 */
static CORBA_char *trapeze_corba_exception_table[] =
{
	ex_CORBA_UNKNOWN,
	ex_CORBA_BAD_PARAM,
	ex_CORBA_NO_MEMORY,
	ex_CORBA_IMP_LIMIT,
	ex_CORBA_COMM_FAILURE,
	ex_CORBA_INV_OBJREF,
	ex_CORBA_NO_PERMISSION,
	ex_CORBA_INTERNAL,
	ex_CORBA_MARSHAL,
	ex_CORBA_INITIALIZE,
	ex_CORBA_NO_IMPLEMENT,
	ex_CORBA_BAD_TYPECODE,
	ex_CORBA_BAD_OPERATION,
	ex_CORBA_NO_RESOURCES,
	ex_CORBA_NO_RESPONSE,
	ex_CORBA_PERSIST_STORE,
	ex_CORBA_BAD_INV_ORDER,
	ex_CORBA_TRANSIENT,
	ex_CORBA_FREE_MEM,
	ex_CORBA_INV_IDENT,
	ex_CORBA_INV_FLAG,
	ex_CORBA_INTF_REPOS,
	ex_CORBA_BAD_CONTEXT,
	ex_CORBA_OBJ_ADAPTER,
	ex_CORBA_DATA_CONVERSION,
	ex_CORBA_OBJECT_NOT_EXIST,
	ex_CORBA_TRANSACTION_REQUIRED,
	ex_CORBA_TRANSACTION_ROLLEDBACK,
	ex_CORBA_INVALID_TRANSACTION,
	/* Required terminating null string. */
	0
};

/*
 * Encode a CORBA-presentation system exception according to our Trapeze
 * message format.  Note that the reply message kind (`CORBA_SYSTEM_EXCEPTION'
 * in our case) has already been encoded, so we need only encode the system
 * exception data.
 */
void
flick_trapeze_encode_corba_system_exception(
	CORBA_Environment *ev,
	void *buf_current)
{
	unsigned int *exc_id		= ((unsigned int *) buf_current);
	unsigned int *exc_completed	= exc_id + 1;
	unsigned int i;
	
	/*
	 * Determine the Trapeze exception ID that corresponds to the given
	 * CORBA exception ID.
	 */
	for (i = 0; trapeze_corba_exception_table[i]; ++i) {
		if (!strcmp(CORBA_exception_id(ev),
			    trapeze_corba_exception_table[i]))
			break;
	}
	
	/* Encode the exception ID. */
	if (trapeze_corba_exception_table[i])
		*exc_id = htonl(i);
	else
		*exc_id = htonl(ex_TRAPEZE_UNKNOWN);
	
	/* Encode the completion flag. */
	*exc_completed = htonl(((flick_system_exception *)
				CORBA_exception_value(ev))->completed);
}

/*
 * Decode a Trapeze message format system exception and translate it into a
 * CORBA system exception.  The reply message kind (`CORBA_SYSTEM_EXCEPTION')
 * has already been decoded (into `ev->major', in fact), so we need only decode
 * the system exception data.
 */
void
flick_trapeze_decode_corba_system_exception(
	CORBA_Environment *ev,
	void *buf_current)
{
	unsigned int exc_id		= ntohl(*((int *) buf_current));
	unsigned int exc_completed	= ntohl(*(((int *) buf_current) + 1));
	flick_system_exception *exc	= flick_system_exception_alloc();
	
	char *exc_id_string;
	
	/*****/
	
	if (exc_id >= (sizeof(trapeze_corba_exception_table)
		       / sizeof(*trapeze_corba_exception_table)))
		exc_id = ex_TRAPEZE_UNKNOWN;
	exc_id_string = trapeze_corba_exception_table[exc_id];
	
	exc->minor = 0;
	switch (exc_completed) {
	case TRAPEZE_COMPLETED_YES:
		exc->completed = CORBA_COMPLETED_YES;
		break;
	case TRAPEZE_COMPLETED_NO:
		exc->completed = CORBA_COMPLETED_NO;
		break;
	case TRAPEZE_COMPLETED_MAYBE:
	default:
		exc->completed = CORBA_COMPLETED_MAYBE;
		break;
	}
	
	/*
	 * Fill out the CORBA exception.  Note that `exc' becomes ``owned'' by
	 * `ev'; we must not free `exc' ourselves.
	 */
	CORBA_BOA_set_exception(0 /* XXX */, ev,
				CORBA_SYSTEM_EXCEPTION,
				find_system_exception_id(
					exc_id_string,
					strlen(exc_id_string) + 1
					),
				exc);
}


/*****************************************************************************/

/*
 * Encode a Sun-presentation system exception according to our Trapeze message
 * format.  The reply message kind has already been encoded, so we need only
 * encode the system exception data.
 */
void
flick_trapeze_encode_sun_system_exception(
	int ev,
	void *buf_current)
{
	unsigned int *exc_id		= ((unsigned int *) buf_current);
	unsigned int *exc_completed	= exc_id + 1;
	
	/*
	 * A Sun system exception is simply an error code --- it has no
	 * substructure for us to encode.  So, we make it up!
	 *
	 * XXX --- Flick's Sun PG prevents us from doing a better job with
	 * this.  If the generated code were right, `ev' would be set to a real
	 * RPC `accept_stat' value.  But as of now it's not --- it's always -1!
	 */
	*exc_id = htonl(ex_TRAPEZE_UNKNOWN);
	*exc_completed = htonl(TRAPEZE_COMPLETED_MAYBE);
}

/*
 * Decode a Trapeze message format system exception and translate it into a Sun
 * system exception.  The reply message kind has already been decoded, so all
 * we need only decode the system exception data.
 */
void
flick_trapeze_decode_sun_system_exception(
	int ev,
	void *buf_current)
{
	/*
	 * A Sun system exception is simply an error code --- it has no place
	 * in which we can store the on-the-wire data.  So, we throw it away!
	 */
	/* Nothing to do. */
}


/*****************************************************************************/

/* End of file. */

