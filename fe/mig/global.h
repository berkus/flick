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

#ifndef	_GLOBAL_H
#define	_GLOBAL_H

#include <sys/types.h>
#include <mom/libmeta.h>
#include <mom/pres_c.h>
#include <mom/compiler.h>

#include "boolean.h"
#include "mig_string.h"

extern mig_boolean_t BeQuiet;	/* no warning messages */
extern mig_boolean_t BeVerbose;	/* summarize types, routines */
extern mig_boolean_t UseMsgRPC;
extern mig_boolean_t GenSymTab;

extern mig_boolean_t IsKernelUser;
extern mig_boolean_t IsKernelServer;

extern const_string_t RCSId;

extern const_string_t SubsystemName;
extern u_int SubsystemBase;

extern const_string_t MsgOption;
extern const_string_t WaitTime;
extern const_string_t ErrorProc;
extern const_string_t ServerPrefix;
extern const_string_t UserPrefix;
extern const_string_t ServerDemux;
extern const_string_t SubrPrefix;
extern const_string_t RoutinePrefix;

extern int yylineno;
extern string_t yyinname;

extern void init_global(void);

extern string_t UserFilePrefix;
extern string_t UserHeaderFileName;
extern string_t ServerHeaderFileName;
extern string_t InternalHeaderFileName;
extern string_t UserFileName;
extern string_t ServerFileName;

extern void more_global(void);

extern const char LintLib[];

#ifndef NULL
#define NULL 0
#endif

enum {
	MIG_CHANNEL_CLIENT_DECL,
	MIG_CHANNEL_CLIENT_IMPL,
	MIG_CHANNEL_SERVER_DECL,
	MIG_CHANNEL_SERVER_IMPL,
	MIG_CHANNEL_MAX
};

extern io_file_index builtin_file;
extern io_file_index root_file;
extern const char *root_filename;
extern const char *channel_names[MIG_CHANNEL_MAX];
extern data_channel_index *channel_maps[MIG_CHANNEL_MAX];
void init_channel_maps(pres_c_1 *out_pres);
void map_file_channels(pres_c_1 *out_pres, io_file_index file);

#endif	/* _GLOBAL_H */
