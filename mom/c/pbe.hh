/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 The University of Utah and
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

#ifndef _mom_pbe_c_hh_
#define _mom_pbe_c_hh_

#include <mom/c/be/be_state.hh>
#include <mom/c/be/be_file.hh>
#include <mom/c/be/be_flags.hh>
#include <mom/c/be/mu_state.hh>
#include <mom/pres_c.h>

/* Get the primary be state for a back end */
struct be_state *get_be_state();

/* Install various handlers for the mu_state/SCML/etc... */
void install_handlers(pres_c_1 *pres);
void register_state_handlers(struct be_state *state);
void register_arg_handlers(struct be_state *state);

/* Basic stub generation functions */
void w_header_includes(pres_c_1 *pres);
void w_stub(pres_c_1 *pres, int idx);
void w_marshal_stub(pres_c_1 *pres, mu_stub_info_node);
void w_unmarshal_stub(pres_c_1 *pres, mu_stub_info_node);
void w_client_stub(pres_c_1 *pres, int stub_idx);
void w_skel(pres_c_1 *pres, int stub_idx);
void w_send_stub(pres_c_1 *pres, int stub_idx);
void w_recv_stub(pres_c_1 *pres, int stub_idx);
void w_msg_marshal_stub(pres_c_1 *pres, int idx);
void w_msg_unmarshal_stub(pres_c_1 *pres, int idx);
void w_continue_stub(pres_c_1 *pres, int idx);

void do_main_output(pres_c_1 *pres);
/* make_interface_graph generates a static structure in the output
   file which encodes any type information needed for the runtimes.
   It currently defaults to creating OMG style Repository IDs */
void make_interface_graph( pres_c_1 *pres );

/* Convert a flags array to a tag_list for export to SCML */
tag_list *flags_to_tag_list(struct flags_in *in, struct flags_out *out,
			    int flag_count);

/* The current state */
extern struct be_state *the_state;
extern cast_expr emergency_return_value;

#endif /* _mom_pbe_c_hh_ */

/* End of file. */
