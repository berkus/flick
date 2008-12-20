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

#define FLICK_PROTOCOL_TRAPEZE_ONC
#include "trapeze-link.h"

/*
 * XXX --- Blech!  Flick-generated code currently encodes a -1 to indicate
 * all ONC ``system excpetions.''  When that is fixed (see PR flick/262), we
 * can be smarter.
 */
#define FLICK_TRAPEZE_ONC_SERVER_ERROR (-1)

/* This stuff is used by Flick for transport independent
 * buffer management & message transmission.
 * This should NOT be used by user code - Flick's code is the only stuff
 * that should be using it.
 */

static FLICK_TARGET refs;
static unsigned int count_servers = 0;

/* This one _can_ be used to register servers
   (if they want to build their own main function)
   */
int
flick_server_register(FLICK_SERVER_DESCRIPTOR server, FLICK_SERVER func)
{
	FLICK_TARGET new_refs, this_ref;
	
	/* Reallocate our BOA's vector of references. */
	new_refs = t_realloc(refs,
			     FLICK_TARGET_STRUCT,
			     count_servers + 1);
	if (!new_refs) {
		fprintf(stderr,
			"Error: can't realloc memory for server "
			"references.\n");		
		return 0;
	}
	refs = new_refs;
	count_servers += 1;
	
	this_ref = &(refs[count_servers - 1]);
	
	this_ref->dest        = 2;  /* XXX - should not be hard-coded */
	this_ref->host        = 0;  /* XXX - not actually used */
	this_ref->server_func = func;
	this_ref->u.header.xid = 0;
#if 0	/* These parameters are built into the generated stub code. */
	this_ref->u.header.dir = 0;
	this_ref->u.header.rpcvers = 2;
#endif
	this_ref->u.header.prog = server.prog_num;
	this_ref->u.header.vers = server.vers_num;
	
	return 1;
}

FLICK_TARGET find_server(FLICK_BUFFER msg, void *_buf_start)
{
	int prog, vers, i;
	flick_trapeze_get_prog_and_vers(prog, vers);
	for (i = 0; i < count_servers; i++)
		if ((prog == refs[i].u.header.prog) && 
		    (vers == refs[i].u.header.vers))
			break;
	if (i < count_servers)
		return &(refs[i]);
	else
		return 0;
}


/* This one _can_ be used to begin grabbing messages
   (if they want to build their own main function)
   */
void
flick_server_run()
{
	trapeze_mcp_init(1 /* yes, I'm the server */);
	
	/* Accept new clients, read client requests & send replies forever. */
	while (1) {
		FLICK_BUFFER msg;
		void *buffer;
		FLICK_TARGET obj;
						       
		flick_trapeze_server_get_request(&msg);
		buffer = tpz_mtod(msg);
		
		obj = find_server(msg, buffer);
		if (!obj)
			flick_trapeze_server_send_sun_exception(
				msg,
				FLICK_TRAPEZE_ONC_SERVER_ERROR);
		else {
			switch ((obj->server_func)(msg, buffer, obj)) {
			case FLICK_OPERATION_SUCCESS:
				/* end_encode already sent reply */
				break;
				
			case FLICK_OPERATION_SUCCESS_NOREPLY:
				break;
				
			default:
				/* FLICK_OPERATION_FAILURE */
				/*
				 * XXX --- The server dispatch function should
				 * have already encoded and sent an exception.
				 */
				break;
			}
		}
	}
}

/* End of file. */

