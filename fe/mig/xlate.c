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

#include <stdio.h>
#include <assert.h>
#include <rpc/types.h>

#include <mom/compiler.h>
#include <mom/idl_id.h>
#include <mom/libmint.h>
#include <mom/c/libcast.h>
#include <mom/c/libpres_c.h>
#include <mom/pres_c.h>

/* #include "cpu.h" */
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "mom_routines.h"
#include "global.h"
#include "boolean.h"
#include "xlate.h"
#include "xlate_util.h"

/* Control flags for the n_args routine. */

#define NONE 0x0 /* No special control */
#define COUNT_ARR 0x1 /* Count arrays as two arguments. */
#define NOIN_OUT 0x2 /* Don't count inout arguments */

/* Quickie lookup macro into the MINT part of the PRES_C file*/
#define m(n) (out_pres_c.mint.defs.defs_val[n])

/* Macro to define cast scope */
#define c_scope out_pres_c.cast

/* Macro to define cast scope */
#define c_stubscope out_pres_c.stubs_cast

/* Macro to define stub's location in the PRES_C */
#define c_stubs out_pres_c.stubs

/* Lookup to get at func def inside CAST */
#define c(n) (c_scope.cast_scope_val[n].u.cast_def_u_u.func_type)

/* Lookup to get at include def inside CAST */
#define i(n) (c_scope.cast_scope_val[n].u.cast_def_u_u.include)

/* Lookup into the stubs array */
#define s(n) (c_stubs.stubs_val[n])

/* Variable to store presentation stuff. Declared in main.c */
extern pres_c_1 out_pres_c;

/* Generate a client or server? */

extern int gen_client;
extern int gen_server;

extern char *ihead_name;

/* 
 * Walks through statement list, translating MIG routines.
 */		    
static void translate_routines()
{
    statement_t *CurState; 
    
    int c_include;

    int routine_count = 0;
    int simple_count = 0;

    int stub_count = 0;

    /* 
     * Three MINT references which form the top level of the MINT interface. 
     * A MINT interface is a union of interface types.(MIG, DCE, Sun, 
     * etc.) Each interface value in the top level union contains the
     * stubs for the interface.
     */

    mint_ref procs_union, intf_union, top_union; 
    
    pres_c_inline error_i;
    
    error_i = pres_c_new_inline_atom(-1, pres_c_new_mapping(
	    PRES_C_MAPPING_DIRECT));

    /* 
     * Inital count of the routines
     */
    for (CurState = StatementList; CurState != NULL;
				   CurState = CurState->stNext)
	{
	    if (CurState->stKind == skRoutine)
		{
		    if (CurState->stRoutine->rtKind == rkRoutine)
			{
			    routine_count++;
			}
		    else if (CurState->stRoutine->rtKind == rkSimpleRoutine)
			{
			    simple_count++;
			}
		}			    
	}

    /* 
     * Make the procedure union.
     * Its cases will be created on demand.
     */
    procs_union = get_union_def(0);

    /* 
     * Make the interface union. Only has one slot, corresponding to the
     * subsystem we are implementing.
     */
    intf_union = get_union_def(1);
    
    m(intf_union).mint_def_u.union_def.cases.cases_val[0].val = 
	mint_new_const_int(SubsystemBase);
    m(intf_union).mint_def_u.union_def.cases.cases_val[0].var = 
	procs_union;

    /* 
     * Make the top union. Only has one slot for MIG routines.
     */
    top_union = get_union_def(1);
    
    m(top_union).mint_def_u.union_def.cases.cases_val[0].val = 
	mint_new_const_int(IDL_MIG); /* MIG's Magic Number */
    m(top_union).mint_def_u.union_def.cases.cases_val[0].var = 
	intf_union;

    /* 
     * Make the array of C stubs, 1 client stub per routine, 
     * and one server skeleton.
     */
    assert(c_stubs.stubs_len == 0);
    if (gen_client)
	c_stubs.stubs_len = routine_count + simple_count;
    if (gen_server)
	c_stubs.stubs_len++;
    c_stubs.stubs_val = 
	    mustcalloc((c_stubs.stubs_len)*sizeof(pres_c_stub));

    assert(stub_count == 0);

    if (gen_server)
	{
	    int new_def;
	    char *new_name;
	    
	    /* 
	     * Make the server stub 
	     */
	    assert(stub_count < ((int) c_stubs.stubs_len));
	    
	    /*
	     * XXX --- The parser doesn't tell us which were included and which
	     * were not.
	     */
	    /* default case --
	    s(stub_count).included = PASSTHRU_DATA_CHANNEL; */
	    s(stub_count).kind = PRES_C_SERVER_SKEL;
	    
	    if (!ServerDemux)
	    {
		new_name = flick_asprintf("%s_server",
					  (SubsystemName == strNULL ?
					   "" : SubsystemName));
	    }
	    else
		new_name = strmake(ServerDemux);
	    
	    /* Add to the CAST scope. */
	    new_def = cast_add_def(&(c_scope),
				   cast_new_scoped_name(new_name, NULL),
				   CAST_SC_EXTERN,
				   CAST_VAR_DECL,
				   PASSTHRU_DATA_CHANNEL,
				   CAST_PROT_NONE);
	    
	    c_scope.cast_scope_val[new_def].u.cast_def_u_u.var_type = 
		cast_new_type_name("flick_server_t");
	    
	    /*
	     * Add to the stubs-only CAST scope.  This is the one that really
	     * counts, and what `sskel.c_def' will refer to in the back ends.
	     */
	    new_def = cast_add_def(&(c_stubscope),
				   cast_new_scoped_name(new_name, NULL),
				   CAST_SC_EXTERN,
				   CAST_VAR_DECL,
				   PASSTHRU_DATA_CHANNEL,
				   CAST_PROT_NONE);
	    
	    c_stubscope.cast_scope_val[new_def].u.cast_def_u_u.var_type = 
		cast_new_type_name("flick_server_t");
	    
	    s(stub_count).pres_c_stub_u.sskel.c_def = new_def;
	    
	    s(stub_count).pres_c_stub_u.sskel.request_itype = top_union;
	    s(stub_count).pres_c_stub_u.sskel.reply_itype = top_union;
	    
	    s(stub_count).pres_c_stub_u.sskel.funcs.funcs_len = 
		routine_count + simple_count;
	    
	    s(stub_count).pres_c_stub_u.sskel.funcs.funcs_val = 
		mustcalloc(s(stub_count).pres_c_stub_u.sskel.funcs.funcs_len
			   * sizeof(pres_c_func));
	    
	    stub_count++;
	}
    
    /* 
     * Generate MINT, CAST and client stubs(presentation structures) for each
     * routine.
     */

    /* Add in iheader include, if needed */
    if (gen_client && ihead_name != NULL)
	emit_include_stmt(ihead_name, 0);
    
    for (CurState = StatementList; CurState != NULL;
				   CurState = CurState->stNext)
	{
	    switch (CurState->stKind)
	    {
	    	case skImport:
		case skUImport:
		case skSImport:

		    if (CurState->stKind == skUImport && !gen_client)
		    	break;
		    if (CurState->stKind == skSImport && !gen_server)
		    	break;

		    /* XXX --- Should use `emit_include_stmt' here. */
		    
		    c_include = cast_add_def(&(c_scope),
					     null_scope_name,
					     CAST_SC_NONE,
					     CAST_INCLUDE,
					     PASSTHRU_DATA_CHANNEL,
					     CAST_PROT_NONE);
		    
		    if (CurState->stFileName[0] == '<' &&
			CurState->stFileName[strlen(CurState->stFileName)-1] ==
			'>')
			{
			    i(c_include).system_only = 1;
			}
		    else if (CurState->stFileName[0] == '"' &&
			     CurState->stFileName[strlen(
				     CurState->stFileName)-1] == '"')
			{
			    i(c_include).system_only = 0;
			}
		    else
			panic("Malformed import statement seen.");
		    
		    i(c_include).filename = mustmalloc(
			    strlen(CurState->stFileName));

		    strcpy(i(c_include).filename, CurState->stFileName + 1);
		    
		    i(c_include).filename[strlen(i(c_include).filename)-1] = 0;

		    break;

		case skRoutine:
		    if (gen_client)
			{
			    if ((CurState->stRoutine->rtKind == rkRoutine)
				|| (CurState->stRoutine->rtKind == 
				     rkSimpleRoutine))
				{
				    pres_c_stub *stub = &(s(stub_count));
				    
				    pres_c_client_stub *cstub =
					&(stub->pres_c_stub_u.cstub);
				    
				    assert(stub_count
					   < ((int) c_stubs.stubs_len));
				    
				    /*
				     * XXX --- The parser doesn't tell us
				     * which were included and which were not.
				     */
				    /* default case --
				    stub->included = PASSTHRU_DATA_CHANNEL; */
				    
				    stub->kind = PRES_C_CLIENT_STUB;
				    
				    cstub->op_flags = PRES_C_STUB_OP_FLAG_NONE;
				    
				    cstub->request_itype = top_union;
				    cstub->reply_itype = top_union;
				    
				    make_routine(CurState->stRoutine,
						 PRES_C_CLIENT_STUB,
						 procs_union,
						 &cstub->c_func,
						 &cstub->target_itype,
						 &cstub->target_i,
						 &cstub->client_itype,
						 &cstub->client_i,
						 &cstub->request_i,
						 &cstub->reply_i);
				    
				    cstub->error_itype = mint_ref_null;
				    cstub->error_i = error_i;
				    
				    if (CurState->stRoutine->rtKind == 
					rkSimpleRoutine) {
					    cstub->op_flags |=
						    PRES_C_STUB_OP_FLAG_ONEWAY;
				    }
				}
		            else
			        panic("Unsupported routine kind %d.", 
				      CurState->stRoutine->rtKind);
			}

		    if (gen_server)
		    {
			pres_c_server_func *sfunc;

			assert(s(0).kind == PRES_C_SERVER_SKEL);
			assert(stub_count - 1 >= 0);
			assert(stub_count - 1
			       < ((int) (s(0).pres_c_stub_u.sskel.
					 funcs.funcs_len)));

			/* Set the type of function */
			s(0).pres_c_stub_u.sskel.funcs.
				funcs_val[stub_count - 1].kind
				= PRES_C_SERVER_FUNC;

			sfunc = &s(0).pres_c_stub_u.sskel.funcs.
				funcs_val[stub_count - 1].pres_c_func_u.sfunc;

		        sfunc->op_flags = PRES_C_STUB_OP_FLAG_NONE;
			if (CurState->stRoutine->rtKind == 
			    rkSimpleRoutine) {
				sfunc->op_flags |=
					PRES_C_STUB_OP_FLAG_ONEWAY;
			}
			
			make_routine(CurState->stRoutine,
				     PRES_C_SERVER_SKEL,
				     procs_union,
				     &sfunc->c_func,
				     &sfunc->target_itype,
				     &sfunc->target_i,
				     &sfunc->client_itype,
				     &sfunc->client_i,
				     &sfunc->request_i,
				     &sfunc->reply_i);
			
			sfunc->error_itype = mint_ref_null;
			sfunc->error_i = error_i;
		    }

		    stub_count++;		    		    
		    break;
	   
	        case skRCSDecl:
		{
		    panic("RCS Declaration not supported.");
		}

	        default:
	    	{
		    panic("Unknown statement kind %d\n", CurState->stKind);
		}
	    }
	}

    if (gen_client)
	assert(stub_count == ((int) c_stubs.stubs_len));
    if (gen_server)
	assert(stub_count
	       == ((int) s(0).pres_c_stub_u.sskel.funcs.funcs_len) + 1);
}
	    
/* 
 * This adds in some useful standard types, and then calls translate_routines
 * which does most the work.
 */
void translate()
{
	mint_add_standard_defs(&out_pres_c.mint);
	translate_routines();
}

/* End of file. */

