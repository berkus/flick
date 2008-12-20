/* 
 * Mach Operating System
 * Copyright (c) 1991,1990, 1999 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <mom/libmeta.h>
#include <mom/pres_c.h>

#include "error.h"
#include "global.h"

mig_boolean_t BeQuiet = FALSE;
mig_boolean_t BeVerbose = FALSE;
mig_boolean_t UseMsgRPC = TRUE;
mig_boolean_t GenSymTab = FALSE;

mig_boolean_t IsKernelUser = FALSE;
mig_boolean_t IsKernelServer = FALSE;

const_string_t RCSId = strNULL;

const_string_t SubsystemName = strNULL;
u_int SubsystemBase = 0;

const_string_t MsgOption = strNULL;
const_string_t WaitTime = strNULL;
const_string_t ErrorProc = "MsgError";
const_string_t ServerPrefix = "";
const_string_t UserPrefix = "";
const_string_t ServerDemux = strNULL;
const_string_t SubrPrefix = "";
const_string_t RoutinePrefix = "";

string_t yyinname;

const char LintLib[] = "defined(LINTLIBRARY)";

void
init_global(void)
{
    yyinname = strmake("<no name yet>");
}

string_t UserFilePrefix = strNULL;
string_t UserHeaderFileName = strNULL;
string_t ServerHeaderFileName = strNULL;
string_t InternalHeaderFileName = strNULL;
string_t UserFileName = strNULL;
string_t ServerFileName = strNULL;

void
more_global(void)
{
    if (SubsystemName == strNULL)
	fatal("no SubSystem declaration");

    if (UserHeaderFileName == strNULL)
	UserHeaderFileName = strconcat(SubsystemName, ".h");
    else if (streql(UserHeaderFileName, "/dev/null"))
	UserHeaderFileName = strNULL;

    if (UserFileName == strNULL)
	UserFileName = strconcat(SubsystemName, "User.c");
    else if (streql(UserFileName, "/dev/null"))
	UserFileName = strNULL;

    if (ServerFileName == strNULL)
	ServerFileName = strconcat(SubsystemName, "Server.c");
    else if (streql(ServerFileName, "/dev/null"))
	ServerFileName = strNULL;

    if (ServerDemux == strNULL)
       ServerDemux = strconcat(SubsystemName, "_server");
    if (RoutinePrefix != NULL)
      ServerDemux = strconcat (RoutinePrefix, ServerDemux);
}

io_file_index builtin_file;
io_file_index root_file;
const char *root_filename = 0;

const char *channel_names[MIG_CHANNEL_MAX] = {
	"client",
	"client",
	"server",
	"server"
};

data_channel_index *channel_maps[MIG_CHANNEL_MAX];

void init_channel_maps(pres_c_1 *out_pres)
{
	int lpc;
	
	for( lpc = 0; lpc < MIG_CHANNEL_MAX; lpc++ ) {
		channel_maps[lpc] = (data_channel_index *)
			mustmalloc(sizeof(data_channel_index) *
				   out_pres->meta_data.files.files_len);
	}
}

void map_file_channels(pres_c_1 *out_pres, io_file_index file)
{
	data_channel_index dc;
	int lpc;
	
	for( lpc = 0; lpc < MIG_CHANNEL_MAX; lpc++ ) {
		dc = meta_add_channel(&out_pres->meta_data, file,
				      channel_names[lpc]);
		if( !(lpc % 2) )
			out_pres->meta_data.channels.channels_val[dc].flags |=
				DATA_CHANNEL_DECL;
		else
			out_pres->meta_data.channels.channels_val[dc].flags |=
				DATA_CHANNEL_IMPL;
		channel_maps[lpc][file] = dc;
	}
}
