/*
 * Copyright (c) 1995, 1996, 1998 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 *
 * This file is part of Flick, the Flexible IDL Compiler Kit.
 *
 * Flick is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Flick is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Flick; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place #330, Boston, MA 02111, USA.
 */

/* Pretty print a type declaration. Declared in type.c */
extern void itPrintDecl(identifier_t name, const ipc_type_t *it);

/* Make the MINT arguments for a routine. */
extern mint_ref make_mint_args(routine_t *CurRoutine,int check_flags);

/* Make the CAST arguments for a routine. */
extern void make_cast_args(routine_t *CurRoutine,int c_index,
			   int is_server_skel);

/* Make the presentation linkages between the MINT and CAST structures */
extern void make_routine(routine_t *CurRoutine,
			 int which_stub,
			 mint_ref procs_union,
			 cast_ref *out_c_func,
			 mint_ref *out_target_m,
			 pres_c_inline *out_target_inline,
			 mint_ref *out_client_m,
			 pres_c_inline *out_client_inline,
			 pres_c_inline *out_request_inline,
			 pres_c_inline *out_reply_inline);

