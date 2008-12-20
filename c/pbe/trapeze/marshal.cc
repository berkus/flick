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

#include <assert.h>
#include <mom/compiler.h>
#include <mom/c/libcast.h>

#include "trapeze.h"

#if 0
void w_marshal_stub(pres_c_1 *pres, mu_stub_info_node msi)
{
	struct mu_abort_block *mab;
	cast_stmt reaper_label;
	
	trapeze_mu_state must(pres,
			   MUST_ENCODE | MUST_DEALLOCATE,
			   RPCM_TRUSTED, "mu");
	
	must.add_stmt(must.change_stub_state(FLICK_STATE_MARSHAL));
	emergency_return_value = cast_new_expr_name("return 1");
	reaper_label = cast_new_label(
		"_flick_reaper",
		cast_new_stmt_expr(emergency_return_value));
	
	mab = new mu_abort_block();
	mab->set_kind(MABK_THREAD);
	mab->begin();
	must.abort_block = mab;
	
	pres_c_marshal_stub *stub;
	cast_def *stub_cast_decl;
	cast_stmt stub_cast_body;
	
	/* Find the `pres_c_marshal_stub' that describes this stub. */
	assert(pres);
	assert(pres->stubs.stubs_val[msi.stub_idx].kind
	       == PRES_C_MARSHAL_STUB);
	stub = &(pres->stubs.stubs_val[msi.stub_idx].pres_c_stub_u.mstub);
	
	/* Find the CAST declaration of this stub. */
	assert(stub->c_func >= 0);
	assert(stub->c_func < (signed int)pres->stubs_cast.cast_scope_len);
	stub_cast_decl = &(pres->stubs_cast.cast_scope_val[stub->c_func]);
	assert(stub_cast_decl->u.kind == CAST_FUNC_DECL);
	
	must.current_param_dir = msi.stub_dir;
	
	/* Specialize the cast_decl and add to the regular CAST defs. */
	cast_func_type *stub_func_type
		= &stub_cast_decl->u.cast_def_u_u.func_type;
	cast_ref newdef = cast_add_def(
		&pres->cast,
		must.mu_mapping_stub_call_name(msi.stub_idx),
		CAST_SC_NONE,
		CAST_FUNC_DECL,
		PASSTHRU_DATA_CHANNEL,
		CAST_PROT_NONE);
	assert(newdef >= 0);
	assert(newdef < (cast_ref) pres->cast.cast_scope_len);
	stub_cast_decl = &pres->cast.cast_scope_val[newdef];
	stub_cast_decl->u.cast_def_u_u.func_type = *stub_func_type;
	
	/* We generate the body of the stub in a rather circuitous way.
	   We call `mu_func_params' to marshal the arguments to the current
	   stub.  The `stub->i' inline is an inline_atom representing the datum
	   we need to process.  The index within the atom is an index into the
	   current stub's argument list, telling us which argument to marshal.
	   The mapping within that atom is a PRES_C_MAPPING_STUB that points
	   back to the current marshal stub.
	   
	   When we attempt to marshal the argument named by `stub->i', we'll
	   reach `mu_mapping_stub'.  That function will descend into the
	   current marshal stub, get the current stub's `seethru_map', and
	   start inlining code.  Clever, eh?  */
	
	/* Make sure that we have a license to inline the body of the current
	   stub exactly once.  Without this "bonus," `mu_mapping_stub' might
	   simply decide to emit a call to the current mapping stub --- hardly
	   useful when our purpose is to *generate* the body! */
	must.stub_inline_depth[msi.stub_idx]
		= MAX_STUB_RECURSIVE_INLINE_DEPTH - 1;
	
	must.mu_func_params(stub->c_func, stub->itype, stub->i);
	must.break_glob();
	
	/* Create the CAST body of the stub. */
	if (must.c_block
	    && (must.c_block->kind == CAST_STMT_BLOCK))
		stub_cast_body = must.c_block;
	else {
		stub_cast_body = cast_new_block(0, 0);
		if (must.c_block)
			cast_block_add_stmt(&(stub_cast_body->
					      cast_stmt_u_u.block),
					    must.c_block);
	}
	
	/* If we get to here it executed normally so we return 0 */
	cast_block_add_stmt( &stub_cast_body->cast_stmt_u_u.block,
			     cast_new_return(
				     cast_new_expr_lit_int(0,0)) );
	/* Put our abort block in */
	must.abort_ctxt->current_node->add_block(stub_cast_body);
	must.abort_ctxt->current_node->rollback_block();
	
	/* Finally, output the stub code. */
	cast_w_func_type(stub_cast_decl->name,
			 &(stub_cast_decl->u.cast_def_u_u.func_type),
			 0);
	w_printf("\n");
	cast_w_stmt(stub_cast_body, 0);
	w_printf("\n");
}
#else
void w_marshal_stub(pres_c_1 */*pres*/, mu_stub_info_node /*msi*/)
{
}
#endif

#if 0
static void w_unmarshal_stubs(pres_c_1 *pres, mu_stub_info_node msi, int swap)
{
	trapeze_mu_state must(pres,
			   MUST_DECODE | MUST_ALLOCATE,
			   RPCM_TRUSTED, "mu", swap);
	
	must.add_stmt(change_stub_state(FLICK_STATE_UNMARSHAL));
	must.abort_ctxt = new mu_abort_context();
	must.abort_ctxt->current_node->add_abort_stmt( cast_new_return(
		cast_new_expr_lit_int(1,0)) );
	must.current_span = new mu_msg_span;
	must.current_span->set_kind(MSK_SEQUENTIAL);
	must.current_span->begin();
	
	pres_c_marshal_stub *stub;
	cast_def *stub_cast_decl;
	cast_stmt stub_cast_body;
	
	/* Find the `pres_c_marshal_stub' that describes this stub. */
	assert(pres);
	assert(pres->stubs.stubs_val[msi.stub_idx].kind
	       == PRES_C_UNMARSHAL_STUB);
	stub = &(pres->stubs.stubs_val[msi.stub_idx].pres_c_stub_u.ustub);
	
	/* Find the CAST declaration of this stub. */
	assert(stub->c_func >= 0);
	assert(stub->c_func < (signed int)pres->stubs_cast.cast_scope_len);
	stub_cast_decl = &(pres->stubs_cast.cast_scope_val[stub->c_func]);
	assert(stub_cast_decl->u.kind == CAST_FUNC_DECL);
	
	must.current_param_dir = msi.stub_dir;
	
	/* Specialize the cast_decl and add to the regular CAST defs. */
	cast_func_type *stub_func_type
		= &stub_cast_decl->u.cast_def_u_u.func_type;
	cast_ref newdef = cast_add_def(
		&pres->cast,
		must.mu_mapping_stub_call_name(msi.stub_idx),
		CAST_SC_NONE,
		CAST_FUNC_DECL,
		PASSTHRU_DATA_CHANNEL,
		CAST_PROT_NONE);
	assert(newdef >= 0);
	assert(newdef < (cast_ref) pres->cast.cast_scope_len);
	stub_cast_decl = &pres->cast.cast_scope_val[newdef];
	stub_cast_decl->u.cast_def_u_u.func_type = *stub_func_type;
	
	/* Generate the body of this stub using the same technique described
	   in `w_marshal_stub', above. */
	must.stub_inline_depth[msi.stub_idx]
		= MAX_STUB_RECURSIVE_INLINE_DEPTH - 1;
	must.mu_func_params(stub->c_func, stub->itype, stub->i);
	must.break_glob();
	
	/* Create the CAST body of the stub. */
	if (must.c_block
	    && (must.c_block->kind == CAST_STMT_BLOCK))
		stub_cast_body = must.c_block;
	else {
		stub_cast_body = cast_new_block(0, 0);
		if (must.c_block)
			cast_block_add_stmt(&(stub_cast_body->
					      cast_stmt_u_u.block),
					    must.c_block);
	}
	/* Collapse the span tree and commit the values */
	must.current_span->end();
	must.current_span->collapse();
	must.current_span->commit();
	
	cast_block_add_stmt( &stub_cast_body->cast_stmt_u_u.block,
			     cast_new_return(
				     cast_new_expr_lit_int(0,0)) );
	must.abort_ctxt->current_node->add_block(stub_cast_body);
	must.abort_ctxt->current_node->rollback_block();
	
	/* Finally, output the stub code. */
	cast_w_func_type(stub_cast_decl->name,
			 &(stub_cast_decl->u.cast_def_u_u.func_type),
			 0);
	w_printf("\n");
	cast_w_stmt(stub_cast_body, 0);
	w_printf("\n");
}

void w_unmarshal_stub(pres_c_1 *pres, mu_stub_info_node msi) 
{
	w_unmarshal_stubs(pres, msi, TRAPEZE_NO_SWAP);
//	w_unmarshal_stubs(pres, msi, TRAPEZE_SWAP);
}
#else
void w_unmarshal_stub(pres_c_1 */*pres*/, mu_stub_info_node /*msi*/) 
{
}
#endif

/* End of file. */

