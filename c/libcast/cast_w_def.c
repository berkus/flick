/*
 * Copyright (c) 1995, 1996, 1998, 1999 The University of Utah and
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

#include <stdlib.h>

#include <mom/compiler.h>
#include <mom/c/libcast.h>

void cast_w_storage_class(cast_storage_class sc) 
{
	switch (sc) {
	case CAST_SC_STATIC:
		w_printf("static ");
		break;
	case CAST_SC_EXTERN:
		w_printf("extern ");
		break;
	case CAST_SC_REGISTER:    
		w_printf("register ");
		break;
	case CAST_SC_MUTABLE:
		w_printf("mutable ");
		break;
	case CAST_SC_NONE:
	case CAST_SC_AUTO:
	default:
		break;
	}
}

void cast_w_def(cast_def_t def, int indent)
{
	if( cast_meta->channels.channels_val[def->channel].flags &
	    DATA_CHANNEL_SQUELCHED )
		return;
	switch (def->u.kind)
	{
		case CAST_TYPEDEF:
			if( cast_language == CAST_C ) {
				w_printf("#");
				w_i_printf(indent,
					   "ifndef _typedef___");
				cast_w_scoped_name(&def->name);
				w_printf("\n");
				w_printf("#");
				w_i_printf(indent,
					   "define _typedef___");
				cast_w_scoped_name(&def->name);
				w_printf("\n");
			}
			w_i_printf(indent, "typedef ");
			cast_w_type(def->name,
				    def->u.cast_def_u_u.
				    typedef_type, indent);
			w_printf(";\n");
			if( cast_language == CAST_C ) {
				w_printf("#");
				w_i_printf(indent,
					   "endif /* _typedef___");
				cast_w_scoped_name(&def->name);
				w_printf(" */\n");
			}
			break;
		case CAST_TYPE: {
			cast_scoped_name scname;
			
			w_indent(indent);
			switch( def->u.cast_def_u_u.type->kind ) {
			case CAST_TYPE_CLASS_NAME:
			case CAST_TYPE_AGGREGATE:
			case CAST_TYPE_ENUM:
				scname = null_scope_name;
				break;
			default:
				scname = def->name;
				break;
			}
			cast_w_type(scname,
				    def->u.cast_def_u_u.type,
				    indent);
			w_printf(";\n\n");
			break;
		}
		case CAST_DEFINE:
			w_i_printf(indent, "#define ");
			cast_w_scoped_name(&def->name);
			w_printf(" (");
			cast_w_expr(def->u.cast_def_u_u.define_as,0);
			w_printf(")\n");
			break;
			
		case CAST_INCLUDE:
			w_printf("#include ");
			if (def->u.cast_def_u_u.include.system_only)
				w_printf("<%s>\n",
					 def->u.cast_def_u_u.include.filename);
			else
				w_printf("\"%s\"\n",
					 def->u.cast_def_u_u.include.filename);

			break;

		case CAST_DIRECT_CODE:
			w_printf(def->u.cast_def_u_u.direct.code_string);
			break;
			
		case CAST_FUNC_DECL:
			w_indent(indent);
			cast_w_storage_class(def->sc);
			cast_w_func_type(def->name,
					 &def->u.cast_def_u_u.
					 func_type,
					 indent);
			w_printf(";\n");
			break;
		case CAST_FUNC_DEF:
			w_indent(indent);
			cast_w_storage_class(def->sc);
			cast_w_func_type(def->name,
					 &def->u.cast_def_u_u.func_def.type,
					 indent);
			w_printf("\n");
			cast_w_block(&def->u.cast_def_u_u.func_def.block,
				     indent);
			w_printf("\n");
			break;
		case CAST_VAR_DECL:
			w_indent(indent);
			cast_w_storage_class(def->sc);
			cast_w_type(def->name, def->u.cast_def_u_u.var_type,
				    indent);
			w_printf(";\n");
			break;
		case CAST_VAR_DEF:
			w_indent(indent);
			cast_w_storage_class(def->sc);
			cast_w_type(def->name, def->u.cast_def_u_u.var_def.
				    type, indent);
			if (def->u.cast_def_u_u.var_def.init)
			{
				cast_w_init(def->u.cast_def_u_u.var_def.init,
					    indent);
			}
			w_printf(";\n");
			break;
		case CAST_USING:
			w_i_printf(indent,
				   "using %s",
				   def->u.cast_def_u_u.using_scope ==
				   CAST_USING_NAME ? "" :
				   def->u.cast_def_u_u.using_scope ==
				   CAST_USING_TYPENAME ? "typename " :
				   def->u.cast_def_u_u.using_scope ==
				   CAST_USING_NAMESPACE ? "namespace " :
				   "");
			cast_w_scoped_name(&def->name);
			w_printf(";\n");
			break;
		case CAST_NAMESPACE:
			w_i_printf(indent, "%s ", cast_namespace_str);
			cast_w_scoped_name(&def->name);
			w_printf(" {\n");
			cast_w_scope(def->u.cast_def_u_u.new_namespace,
				     indent+1);
			w_i_printf(indent, "}\n\n");
			break;
		case CAST_LINKAGE:
			w_i_printf(indent, "extern \"");
			cast_w_scoped_name(&def->name);
			w_printf("\" {\n");
			cast_w_scope(def->u.cast_def_u_u.linkage, indent+1);
			w_i_printf(indent, "}\n\n");
			break;
		case CAST_FRIEND:
			w_i_printf(indent, "friend ");
			cast_w_type(null_scope_name,
				    def->u.cast_def_u_u.friend_decl,
				    0);
			break;
		default:
			panic("unknown cast_def_kind %d", def->u.kind);
	}
}

/* End of file. */

