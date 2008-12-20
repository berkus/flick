/*
 * Copyright (c) 1999 The University of Utah and
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
#include <stdlib.h>
#include <string.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>
#include <mom/c/pbe.hh>

struct scml_pres_handler : public scml_handler {
	pres_c_1 *pres;
};

int c_print_cast_func_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		struct scml_pres_handler *sh;
		cast_func_type *ftype = 0;
		unsigned int lpc;
		cast_def *cfunc;
		cast_type type;
		tag_item *ti;
		int def;
		
		sh = (struct scml_pres_handler *)
			sc->get_cmd_def()->get_handler();
		def = find_tag(sc->get_rvalues(), "def")->data.tag_data_u.i;
		ti = find_tag(sc->get_rvalues(), "unused");
		cfunc = &sh->pres->stubs_cast.cast_scope_val[def];
		/* Write the function out */
		switch( cfunc->u.kind ) {
		case CAST_FUNC_DECL:
			ftype = &cfunc->u.cast_def_u_u.func_type;
			break;
		case CAST_TYPEDEF:
		case CAST_TYPE:
			type = cfunc->u.cast_def_u_u.type;
			while( type->kind != CAST_TYPE_FUNCTION ) {
				switch( type->kind ) {
				case CAST_TYPE_REFERENCE:
				case CAST_TYPE_POINTER:
					type = type->cast_type_u_u.
						pointer_type.target;
					break;
				case CAST_TYPE_QUALIFIED:
					type = type->cast_type_u_u.
						qualified.actual;
					break;
				case CAST_TYPE_TEMPLATE:
					type = type->cast_type_u_u.
						template_type.def;
					break;
				default:
					panic("Can't find a "
					      "function type");
					break;
				}
			}
			ftype = &type->cast_type_u_u.func_type;
			break;
		default:
			panic("Can't find a function type");
			break;
		}
		for( lpc = 0; lpc < tag_data_length(&ti->data); lpc++ ) {
			ftype->params.params_val[get_tag_data(&ti->data,
							      lpc).i].
				spec |= CAST_PARAM_UNUSED;
		}
		switch( cfunc->u.kind ) {
		case CAST_FUNC_DECL:
			cast_w_func_type(cfunc->name, ftype, 0);
			break;
		case CAST_TYPEDEF:
		case CAST_TYPE:
			cast_w_type(cfunc->name,
				    cfunc->u.cast_def_u_u.type,
				    0);
			break;
		default:
			panic("Can't find a function type");
			break;
		}
		retval = 1;
	}
	return( retval );
}

int c_cast_init_expr_handler(struct scml_token *st, struct scml_context *sc)
{
	int retval = 0;
	
	if( sc->handle_params(st) ) {
		char *name_str, *expr_str, *type_str;
		struct scml_pres_handler *sh;
		tag_item *ti;
		
		sc->set_flags(sc->get_parent()->get_flags());
		sh = (struct scml_pres_handler *)
			sc->get_cmd_def()->get_handler();
		ti = find_tag(sc->get_rvalues(), "name");
		name_str = scml_string::tag_string(ti);
		ti = find_tag(sc->get_rvalues(), "expr");
		expr_str = scml_string::tag_string(ti);
		ti = find_tag(sc->get_rvalues(), "type");
		type_str = scml_string::tag_string(ti);
		if( type_str ) {
			ti = find_tag(sc->get_parent()->get_rvalues(),
				      type_str);
		}
		if( name_str && expr_str &&
		    (ti->data.kind == TAG_CAST_TYPE) ) {
			cast_expr init_expr;
			
			init_expr = cast_new_expr_assign_to_zero(
				cast_new_expr_name(expr_str),
				ti->data.tag_data_u.ctype,
				&(sh->pres->cast)
				);
			add_tag(sc->get_parent()->get_rvalues(),
				name_str,
				TAG_CAST_EXPR,
				init_expr);
			retval = 1;
		}
	}
	return( retval );
}

void install_handlers(pres_c_1 *pres)
{
	struct translation_handler_entry *the;
	struct scml_handler_table *sht;
	struct scml_pres_handler *sh;
	
	sht = scml_context::get_handler_table();
	sh = new scml_pres_handler;
	sh->name = "c-print-cast-func-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_print_cast_func_handler;
	sht->add_handler(sh);
	sh->pres = pres;
	
	sht = scml_context::get_handler_table();
	sh = new scml_pres_handler;
	sh->name = "c-cast-init-expr-handler";
	sh->kind = SHK_C_FUNCTION;
	sh->function.c_func = c_cast_init_expr_handler;
	sht->add_handler(sh);
	sh->pres = pres;
	
	mu_state::translation_handlers = create_hash_table(15);
	the = new translation_handler_entry;
	the->entry.name = "type_cast";
	the->handler = mu_type_cast_mapping_handler;
	add_entry(mu_state::translation_handlers, &the->entry);
	
	the = new translation_handler_entry;
	the->entry.name = "&";
	the->handler = mu_ampersand_mapping_handler;
	add_entry(mu_state::translation_handlers, &the->entry);
}
