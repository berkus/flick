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

%{
  /* .c file output stuff */

#include "helpers.hh"
#include "Errors.hh"
#include <mom/compiler.h>
#include <stdlib.h>
#include <string.h>

  void yyerror(const char *); 
  int yylex();
  int scope;
  aoi cur_aoi;
  int aoi_length;
  int saved_aoi_len;
  char *curr_struct_id;
  aoi_interface *cur_interface;
  char DISCRM[3] = {'_', 'd', 0};
  char UNION_LABEL[3] = {'_', 'u', 0};
  aoi_field *default_union;
  int enum_val;
  int last_pos;
  int in_oneway; /* 1 if parsing a oneway operation, 0 otherwise */
  io_file_index builtin_file;
%}

%start mystart

%union {
  char			*id;	// for identifiers (not strings) 
  unsigned int		integer;// for any integer token 
  double		real;	// for any real value token 
  char			*str;	// for string tokens 
  char			chr;	// for character literals tokens 
  struct aoi_def	*defn;	// this is the definition 
  unsigned int		kind;	// this is for a const type specifier 
  struct aoi_operation	*op;	// this is for interface operations 
  unsigned int		ref;	// for references 
  struct aoi_type_u	*type;	// for type descriptors 
  struct aoi_field	*field; // for passing names fields 
  struct VoidArray	*array; // for lists of anything... 
  struct Declaration	*decl;	// for complex declarations 
  struct aoi_const_u	*cnst;	// for constants... 
  struct aoi_parameter	*parm;	// for parameters... 
  void			*tmp;	// for misc pointers
}

%token	<id>		ID
%token	<integer>	LIT_INT
%token	<real>		LIT_REAL
%token	<str>		LIT_STRING
%token	<chr>		LIT_CHR

%token			SEMI
%token			MODULE
%token			LBRACE
%token			RBRACE
%token 			INTERFACE
%token			COLON
%token			EQUAL
%token			OR
%token			XOR
%token			AND
%token			LSHIFT
%token			RSHIFT
%token			ADD
%token			SUB
%token			MUL
%token			DIV
%token			MOD
%token			NEG
%token			SCOPE
%token			LPAREN
%token			RPAREN
%token			BTRUE
%token			BFALSE
%token			TYPEDEF
%token			COMMA
%token			FLOAT
%token			DOUBLE
%token			LONG
%token			SHORT
%token			UNSIGNED
%token			INT     /* invalid */
%token			CHAR
%token			BOOLEAN
%token			OCTET
%token			ANY
%token			CONST
%token			STRUCT
%token			UNION
%token			SWITCH
%token			CASE
%token			DEFAULT
%token			ENUM
%token			SEQUENCE
%token			LT
%token			GT
%token			STRING
%token			LBRACK
%token			RBRACK
%token			EXCEPTION
%token			ONEWAY
%token			VOID
%token			IN
%token			INOUT
%token			OUT
%token			RAISES
%token			CONTEXT
%token			ATTRIBUTE
%token			READONLY
%token			OBJECT

%type <array>		attr_dcl;
%type <array>		declarators;
%type <array>		switch_body;
%type <array>		case;
%type <array>		_case_labels;
%type <array>		member_list;
%type <array>		_members;
%type <array>		member;
%type <array>		_simple_declarators;
%type <array>		_scoped_names;
%type <array>		_enumerators;
%type <array>		_fixed_arrays;
%type <array>		parameter_dcls;
%type <array>		_raises_expr;
%type <array>		raises_expr;
%type <array>		_param_dcls;
%type <cnst>		case_label;
%type <cnst>		const_expr
%type <cnst>		or_expr
%type <cnst>		xor_expr
%type <cnst>		and_expr
%type <cnst>		shift_expr
%type <cnst>		add_expr
%type <cnst>		mult_expr
%type <cnst>		unary_expr
%type <cnst>		primary_expr
%type <cnst>		literal
%type <decl>		declarator;
%type <decl>		complex_declarator;
%type <decl>		array_declarator;
%type <defn>		type_declarator;
%type <ref> 		_constr_type;
%type <ref>		struct_type;
%type <ref>		union_type;
%type <ref>		enum_type;
%type <field>		element_spec;
%type <integer>		pos_int_const
%type <integer>		boolean_literal
%type <integer>		fixed_array_size
%type <integer>		_op_attribute;
%type <integer>		param_attribute;
%type <integer>		_readonly;
%type <kind>		const_type;
%type <kind>		real_type;
%type <kind>		integer_type;
%type <kind>		char_type;
%type <kind>		boolean_type;
%type <kind>		octet_type;
%type <kind>		any_type;
%type <op>		op_dcl;
%type <parm>		param_dcl;
%type <ref>		scoped_name;
%type <ref>		scoped_name_or_error;
%type <str>		interface_header;
%type <str>		simple_declarator;
%type <str>		enumerator;
%type <type>		type_spec_or_error;
%type <type>		type_spec;
%type <type>		simple_type_spec_or_error;
%type <type>		simple_type_spec;
%type <type>		constr_type_spec;
%type <type>		base_type_spec;
%type <type>		template_type_spec;
%type <type>		sequence_type;
%type <type>		string_type;
%type <type>		switch_type_spec;
%type <type>		op_type_spec;
%type <type>		param_type_spec_or_error;
%type <type>		param_type_spec;
%type <id>		id;

%%

mystart		: {Start();} specification {Finish();}
		;

specification	: 
		| specification definition
		;

definition	: type_dcl semi 
		| const_dcl semi 
		| except_dcl semi
		| interface semi
		| module semi
		| error {ParseError("invalid definition");} SEMI
		;

module		: MODULE id lbrace {
		  aoi_def *def_tmp = new_aoi_def($2,scope);
		  def_tmp->binding = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  def_tmp->binding->kind = AOI_NAMESPACE;
		  AddDef(def_tmp, AOI_NAMESPACE);
		  AddScope($2);
		} specification RBRACE {
		  DelScope();
		}
		;

interface	: interface_dcl
		| forward_dcl
		;

interface_dcl	: interface_header LBRACE {
		  aoi_def *tmp = new_aoi_def($1, scope);
		  last_pos = cur_aoi.defs.defs_len;
		  AddDef(tmp, AOI_INTERFACE);
		  cur_aoi.defs.defs_val[last_pos].binding =
		    (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  cur_aoi.defs.defs_val[last_pos].binding->kind =
		    AOI_INTERFACE;
		  cur_aoi.defs.defs_val[last_pos].binding->aoi_type_u_u.
		    interface_def = *cur_interface;
		  cur_interface = &cur_aoi.defs.defs_val[last_pos].binding->
				  aoi_type_u_u.interface_def;
		  AddScope($1);
		} interface_body RBRACE {
		  DelScope();
		  cur_interface->code = GetInterfaceCode($1);
		  cur_interface = GetNewInterface();
		}
		;

forward_dcl	: INTERFACE id {
		  if (FindLocalName($2, 0) == -1) {
		    aoi_def *def_tmp = new_aoi_def($2,scope);
		    def_tmp->binding =
		      (aoi_type)mustcalloc(sizeof(aoi_type_u));
		    def_tmp->binding->kind = AOI_FWD_INTRFC;
		    def_tmp->binding->aoi_type_u_u.fwd_intrfc_def = -1;
		    AddDef(def_tmp, AOI_FWD_INTRFC);
		  }
		}
		;

interface_header: INTERFACE id inheritance_spec {
		  $$ = $2;
		}
		;

interface_body	:
		| interface_body export
		;

export		: type_dcl semi 
		| const_dcl semi
		| except_dcl semi
		| attr_dcl semi {AddAttrs($1);}
		| op_dcl semi {AddOp($1);}
		| error {ParseError("invalid interface export"); } SEMI
		;

inheritance_spec: COLON _scoped_names {
		  AddParents($2);
		}
		|
		;

_scoped_names	: scoped_name_or_error {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, int, $1);
		}
		| _scoped_names COMMA scoped_name_or_error {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, int, $3);
		}
		;

scoped_name_or_error: scoped_name {$$ = $1;}
		| error {
		  ParseError("expecting identifier");
		  $$ = 0;
		}
scoped_name	: ID {
		  $$ = FindLocalName($1);
		}
		| SCOPE id {
		  $$ = FindGlobalName($2);
		}
		| scoped_name SCOPE id {
		  $$ = FindScopedName($3, $1);
		}
		;

const_dcl	: CONST const_type id equal const_expr {
		  AddDef(AoiConst($2, $3, $5), AOI_CONST);
		}
		| CONST error SEMI {
		  ParseError("invalid constant type");
		}
		;

const_type	: integer_type {$$ = $1;}
		| char_type {$$ = $1;}
		| boolean_type {$$ = $1;}
		| real_type {$$ = $1;}
		| string_type {$$ = kSTRING;}
		| scoped_name {$$ = GetConstType($1);}
		;

const_expr	: or_expr {$$ = $1;}
		;

or_expr		: xor_expr {$$ = $1;}
		| or_expr OR xor_expr {
		  $$ = const_or($1, $3);
		}
		;

xor_expr	: and_expr {$$ = $1;}
		| xor_expr XOR and_expr {
		  $$ = const_xor($1, $3);
		}
		;

and_expr	: shift_expr {$$ = $1;}
		| and_expr AND shift_expr {
		  $$ = const_and($1, $3);
		}
		;

shift_expr	: add_expr {$$ = $1;}
		| shift_expr LSHIFT add_expr {
		  $$ = const_lshft($1, $3);
		}
		| shift_expr RSHIFT add_expr {
		  $$ = const_rshft($1, $3);
		}
		;

add_expr	: mult_expr {$$ = $1;}
		| add_expr ADD mult_expr {
		  $$ = const_add($1,$3);
		}
		| add_expr SUB mult_expr {
		  $$ = const_sub($1,$3);
		}
		;

mult_expr	: unary_expr {$$ = $1;}
		| mult_expr MUL unary_expr {
		  $$ = const_mul($1,$3);
		}
		| mult_expr DIV unary_expr {
		  $$ = const_div($1,$3);
		}
		| mult_expr MOD unary_expr {
		  $$ = const_mod($1,$3);
		}
		;

unary_expr	: primary_expr {$$ = $1;}
		| NEG primary_expr {
		  $$ = const_bit($2);
		}
		| ADD primary_expr {
		  $$ = const_pos($2);
		}
		| SUB primary_expr {
		  $$ = const_neg($2);
		}
		;

primary_expr	: scoped_name {
		  $$ = GetConstVal($1);
		}
		| literal {$$ = $1;}
		| LPAREN const_expr rparen {$$ = $2;}
		| error {
		  ParseError("invalid constant expression");
		  $$ = MakeConstInt(1);
		}
		;

literal		: LIT_INT {$$ = MakeConstInt($1);}
		| LIT_STRING {$$ = MakeConstString($1);}
		| LIT_CHR {$$ = MakeConstChar($1);}
		| LIT_REAL {$$ = MakeConstReal($1);}
		| boolean_literal {$$ = MakeConstInt($1);}
		;

boolean_literal	: BTRUE {$$ = 1;}
		| BFALSE {$$ = 0;}
		;

pos_int_const	: const_expr {$$ = GetPosInt($1);}
		;

type_dcl	: TYPEDEF type_declarator {AddDef($2, AOI_STRUCT);}
		| struct_type {}
		| union_type {}
		| enum_type {}
		| TYPEDEF error {ParseError("invalid type");} SEMI
		;

type_declarator	: type_spec_or_error declarators {
		  int zz;
		  for (zz = 0; zz < $2->cur_num - 1; zz++)  {
		    AddDef(GetAoiDefFromDecl($1, ARRAY_REF(*$2, Declaration, zz)), AOI_STRUCT);
		  }
		  $$ = GetAoiDefFromDecl($1, ARRAY_REF(*$2, Declaration, zz));
		}
		;
type_spec_or_error  : type_spec {$$ = $1;}
		| error {
		  ParseError("invalid type"); 
   		  $$ = MakeAoiType(kSLONG);
		}
		;
type_spec	: simple_type_spec {$$ = $1;}
		| constr_type_spec {$$ = $1;}
		;

simple_type_spec_or_error	: simple_type_spec {$$ = $1;}
			| error {
			  ParseError("invalid type"); 
			  $$ = MakeAoiType(kSLONG);
			}
			;
simple_type_spec: base_type_spec {$$ = $1;}
		| template_type_spec {$$ = $1;}
		| scoped_name {
		  $$ = GetAoiType($1);
		}
		;

base_type_spec	: real_type {$$ = MakeAoiType($1);}
		| integer_type {$$ = MakeAoiType($1);}
		| char_type {$$ = MakeAoiType($1);}
		| boolean_type {$$ = MakeAoiType($1);}
		| octet_type {$$ = MakeAoiType($1);}
		| any_type {$$ = MakeAoiType($1);}
		| OBJECT {$$ = MakeAoiType(kOBJECT);}
		;

template_type_spec	: sequence_type {$$ = $1;}
			| string_type {$$ = $1;}
			;

constr_type_spec: _constr_type {
		  $$ = GetAoiType($1);
		}
		;

_constr_type    : struct_type { $$ = $1;}
		| union_type { $$ = $1;}
		| enum_type { $$ = $1;}
		;

declarators	: declarator {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, Declaration, *$1);
		}
		| declarators COMMA declarator {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, Declaration, *$3);
		}
		;

declarator	: simple_declarator {
		  $$ = (Declaration *)mustcalloc(sizeof(Declaration));
		  $$->sizes.cur_num = 0;
		  $$->name = $1;
		}
		| complex_declarator {$$ = $1;}
		;

simple_declarator: id {$$ = $1;}
		;

complex_declarator: array_declarator {$$ = $1;}
		;

real_type	: FLOAT {$$ = kFLOAT;}
		| DOUBLE {$$ = kDOUBLE;}
		;

integer_type	: LONG {$$ = kSLONG;}
		| SHORT {$$ = kSSHORT;}
		| LONG LONG {$$ = kSLLONG;}
		| UNSIGNED LONG {$$ = kULONG;}
		| UNSIGNED SHORT {$$ = kUSHORT;}
		| UNSIGNED LONG LONG {$$ = kULLONG;}
		| UNSIGNED INT {
		  GrammarError("use `long' or `short' instead of `int'");
		  $$ = kULONG; }
		| UNSIGNED error {
		  ParseError("`unsigned' must be followed by `long' or `short'");
		  $$ = kULONG;
		}
		| INT { 
		  GrammarError("use `long' or `short' instead of `int'");
		  $$ = kSLONG; }
		;

char_type	: CHAR {$$ = kCHAR;}
		;

boolean_type	: BOOLEAN {$$ = kBOOL;}
		;

octet_type	: OCTET {$$ = kOCTET;}
		;
		
any_type	: ANY {$$ = kANY;}
		;

struct_type	: STRUCT id { $<str>$=curr_struct_id; curr_struct_id = $2; } 
		  LBRACE {
		  $<ref>$ = cur_aoi.defs.defs_len;  
		  aoi_def *tmp = new_aoi_def($2,scope);
		  tmp->binding = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  tmp->binding->kind = AOI_STRUCT;
		  AddDef(tmp, AOI_STRUCT);
		  AddScope($2);
		} member_list rbrace {
		  $$ = $<ref>5;
		  DelScope();
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.struct_def.
		    slots.slots_len = $6->cur_num;
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.struct_def.
		    slots.slots_val = &(ARRAY_REF(*$6,aoi_field,0));
		  curr_struct_id = $<str>3;
		}
		| STRUCT id {
		  $$ = FindLocalName($2,FALSE);
		  if ((signed)$$ == -1) {
 		    GrammarError("missing `{' in struct definition");
		    $$ = new_error_ref($2);
		  }
		  else {
		    GrammarError(("missing `{'.  (Hint: omit `struct' when "
				  "using a defined structure type)"));
		  }
		}
		;

member_list	: member {
		  $$ = $1;
		}
		| member_list member {
		  aoi_def fake_def;
		  
		  fake_def.binding = 0;
		  fake_def.name = 0;
		  fake_def.scope = scope;
		  $$ = $1;
		  for (int zz = 0; zz < $2->cur_num; zz++) {
		    for (int xx = 0; xx < $$->cur_num; xx++) {
		      if( (ARRAY_REF(*$$, aoi_field, xx).name) &&
		          !strcmp(ARRAY_REF(*$$, aoi_field, xx).name,
				  ARRAY_REF(*$2,aoi_field,zz).name)) {
			SemanticError("'%s' already used",
				      ARRAY_REF(*$2,aoi_field,zz).name);
		      }
		      fake_def.name = ARRAY_REF(*$2,aoi_field,zz).name;
		      DupeError(&fake_def, AOI_STRUCT);
		    }
		    ADD_TO_ARRAY(*$$, aoi_field, ARRAY_REF(*$2,aoi_field,zz));
		  }
		}
		| error {
	 	  ParseError("invalid member of structure `%s'",
			     curr_struct_id);
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = 0;
		  $$->act_num = 0;
 		}  
		;

member		: type_spec declarators semi {
		  /*
		   * By doing `yyerrok' here, we often improve error reporting
		   * for the next member.
		   */
		  yyerrok;
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = $2->cur_num;
		  $$->act_num = $2->act_num;
		  $$->data = (void *)mustcalloc(sizeof(aoi_field)*$$->act_num);
		  for (int zz = 0; zz < $2->cur_num; zz++) {
		    aoi_field the_field;
		    the_field.name = ARRAY_REF(*$2, Declaration, zz).name;
		    the_field.type = GetAoiTypeFromDecl($1, ARRAY_REF(*$2, Declaration, zz));
		    ARRAY_REF(*$$, aoi_field, zz) = the_field;
		  }
		}
		;

union_type	: UNION id SWITCH lparen {
		  aoi_def *tmp = new_aoi_def($2,scope);
		  tmp->binding = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  tmp->binding->kind = AOI_UNION;
		  $<integer>$ = cur_aoi.defs.defs_len; 
		  AddDef(tmp, AOI_UNION);
		  AddScope($2);
		} switch_type_spec rparen lbrace {  
		  $<field>$ = default_union;
		  default_union = 0;
		} switch_body RBRACE {
		  $$ = $<integer>5;
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			discriminator.name = &(DISCRM[0]);
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			discriminator.type = $6;
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			union_label = &(UNION_LABEL[0]);
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			cases.cases_len = $10->cur_num;
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			cases.cases_val = &(ARRAY_REF(*$10, aoi_union_case,0));
		  if (!default_union) {
		    /*
		     * CORBA IDL-defined unions have an implicit void default
		     * branch, if no explicit default branch is specified.
		     * This is the semantics of the IDL, not an artifact of any
		     * particular presentation.
		     */
		    default_union = (aoi_field *)
				    mustcalloc(sizeof(aoi_field));
		    /* `default_union->name' is unused, since type is void. */
		    default_union->name = ir_strlit("");
		    default_union->type = (aoi_type)
					  mustcalloc(sizeof(aoi_type_u));
		    default_union->type->kind = AOI_VOID;
		  }
		  cur_aoi.defs.defs_val[$$].binding->aoi_type_u_u.union_def.
			dfault = default_union;
		  default_union = $<field>9;
		  DelScope();
		}
		| UNION id {
		  $$ = FindLocalName($2,FALSE);
		  if ((signed)$$ == -1) {
 		    GrammarError("missing `switch' in union definition");
		    $$ = new_error_ref($2);
		  }
		  else {
		    GrammarError(("missing `switch'.  (Hint: omit `union' "
				  "when using a defined union type)"));
		  }
		}
		;

switch_type_spec: integer_type {$$ = MakeAoiType($1);}
		| char_type {$$ = MakeAoiType($1);}
		| boolean_type {$$ = MakeAoiType($1);}
		| enum_type {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_INTEGER;
		  $$->aoi_type_u_u.integer_def.min = 0;
		  $$->aoi_type_u_u.integer_def.range = ~0;
		}
		| scoped_name {$$ = GetAoiType($1);}
		| error {
		  ParseError("invalid switch type"); 
		  $$ = MakeAoiType(kSLONG);
		}
		;

switch_body	: case {$$ = $1;}
		| switch_body case {
		  $$ = $1;
		  for (int zz = 0; zz < $2->cur_num; zz++) {
		    if (ARRAY_REF(*$2,aoi_union_case,zz).val) {
		      CheckUnionCaseDuplicate(
			$$,
			&ARRAY_REF(*$2, aoi_union_case, zz));
		      ADD_TO_ARRAY(*$$,
				   aoi_union_case,
				   ARRAY_REF(*$2, aoi_union_case, zz));
		    } else {
		      if (default_union)
			SemanticError("more than one `default' case in union");
		      default_union = &(ARRAY_REF(*$2,aoi_union_case,zz).var);
		    }
		  }
		}
		;
	    
case		: _case_labels element_spec semi {
		  /*
		   * By doing `yyerrok' here, we often improve error reporting
		   * for the next case.
		   */
		  yyerrok;
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  aoi_union_case tmp;
		  tmp.var = *$2;
		  tmp.val = ARRAY_REF(*$1, aoi_const, 0);
		  NEW_ARRAY(*$$, aoi_union_case, tmp);
		  for (int itmp = 1; itmp < $1->cur_num; itmp++) {
		    tmp.val = ARRAY_REF(*$1, aoi_const, itmp);
		    ADD_TO_ARRAY(*$$, aoi_union_case, tmp);
		  }
		}
		| error SEMI {
		  ParseError("invalid case");
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, int, 1);
		  (*$$).cur_num = 0; /* empty */
		}
		;

_case_labels	: case_label {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, aoi_const, $1);
		}
		| _case_labels case_label {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, aoi_const, $2);
		}
		;

case_label	: CASE const_expr colon {$$ = $2;}
		| DEFAULT colon {$$ = 0;}
		;

element_spec	: type_spec_or_error declarator {
		  $$ = (aoi_field *)mustcalloc(sizeof(aoi_field));
		  $$->name = $2->name;
		  $$->type = GetAoiTypeFromDecl($1, *$2);
		}
		;

enum_type	: ENUM id lbrace _enumerators rbrace {
		  aoi_def *tmp = new_aoi_def($2,scope);
		  tmp->binding = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  tmp->binding->kind = AOI_ENUM;
		  tmp->binding->aoi_type_u_u.enum_def.enum_label = $2;
		  /*
		   * XXX --- HACK!
		   *
		   * Here we have a problem: The names of the enumeration
		   * members must have AOI definitions (`aoi_def's) in order
		   * for the `scoped_name' production to find them, as it must
		   * (for example) when the values are used as case labels in
		   * a discriminated union.  But AOI enumeration members don't
		   * generally have AOI definitions (`aoi_def's); they are not
		   * indexable by any `aoi_ref' but are instead contained in an
		   * AOI_ENUM.
		   *
		   * Our ``solution'' is to make the `enum' be empty and to
		   * create AOI_CONSTs for the members.  This solution works
		   * with the standard CORBA C presentation of enumerations.
		   * However, it presumably isn't equivalent under all present-
		   * ation styles, so we really shouldn't be doing things this
		   * way.
		   *
		   * (An alternate solution would be to create two definitions
		   * for each member: one within the AOI_ENUM, and one special
		   * `aoi_def' that carries the member's value.  The front end
		   * would use the latter in order to parse IDL input; the
		   * presentation generator would uses the former to generate
		   * code and would ignore the latter entirely.  This solution
		   * would require adding a new kind of AOI type, however, and
		   * I wanted to contain this hack as much as possible.)
		   * - Addendum - Well not really...  Its been changed to
		   * output the AOI_CONSTs as IMPLIED so that the consts can
		   * still be referenced without a problem.
		   *
		   * When we get around to redoing AOI, this hack should be
		   * eliminated.
		   */
		  /* We SHOULD do this: */
		  tmp->binding->aoi_type_u_u.enum_def.defs.defs_len
			 = $4->cur_num;
		  tmp->binding->aoi_type_u_u.enum_def.defs.defs_val
			 = &(ARRAY_REF(*$4, aoi_field, 0));
		  AddDef(tmp, AOI_CONST);
		  $$ = cur_aoi.defs.defs_len - 1;
		  /*
		   * And we are then forced to make `aoi_def's for the members
		   * of the enumeration.
		   */
		  for (int zztmp = 0; zztmp < $4->cur_num; zztmp++) {
		    aoi_def temp;
		    temp.name = ARRAY_REF(*$4, aoi_field, zztmp).name;
		    temp.scope = scope;
		    temp.binding = ARRAY_REF(*$4, aoi_field, zztmp).type;
		    AddDef(&temp, AOI_CONST)->idl_file = builtin_file;
		  }
		}
		;

_enumerators	: enumerator {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  enum_val = 0;
		  aoi_field tmp;
		  tmp.name = $1;
		  tmp.type = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  tmp.type->kind = AOI_CONST;
		  tmp.type->aoi_type_u_u.const_def.type = MakeAoiType(kSLONG);
		  tmp.type->aoi_type_u_u.const_def.value =
		    (aoi_const)mustcalloc(sizeof(aoi_const_u));
		  tmp.type->aoi_type_u_u.const_def.value->kind = AOI_CONST_INT;
		  tmp.type->aoi_type_u_u.const_def.value->aoi_const_u_u.
		    const_int = enum_val;
		  NEW_ARRAY(*$$, aoi_field, tmp);
		}
		| _enumerators COMMA enumerator {
		  $$ = $1;
		  aoi_field tmp;
		  tmp.name = $3;
		  tmp.type = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  tmp.type->kind = AOI_CONST;
		  tmp.type->aoi_type_u_u.const_def.type = MakeAoiType(kSLONG);
		  tmp.type->aoi_type_u_u.const_def.value =
		    (aoi_const)mustcalloc(sizeof(aoi_const_u));
		  tmp.type->aoi_type_u_u.const_def.value->kind = AOI_CONST_INT;
		  tmp.type->aoi_type_u_u.const_def.value->aoi_const_u_u.
		    const_int = ++enum_val;
		  ADD_TO_ARRAY(*$$, aoi_field, tmp);
		}
		;

enumerator	: id {$$ = $1;}
		;

sequence_type	: SEQUENCE lt simple_type_spec_or_error COMMA pos_int_const gt {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_ARRAY;
		  $$->aoi_type_u_u.array_def.element_type = $3;
		  $$->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NONE;
		  aoi_type length = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  length->kind = AOI_INTEGER;
		  length->aoi_type_u_u.integer_def.min = 0;
		  length->aoi_type_u_u.integer_def.range = $5;
		  $$->aoi_type_u_u.array_def.length_type = length;
		}
		| SEQUENCE lt simple_type_spec_or_error gt {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_ARRAY;
		  $$->aoi_type_u_u.array_def.element_type = $3;
		  $$->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NONE;
		  aoi_type length = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  length->kind = AOI_INTEGER;
		  length->aoi_type_u_u.integer_def.min = 0;
		  length->aoi_type_u_u.integer_def.range = ~0;
		  $$->aoi_type_u_u.array_def.length_type = length;
		}
		;

string_type	: STRING LT pos_int_const gt {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_ARRAY;
		  aoi_type element = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  element->kind = AOI_CHAR;
		  element->aoi_type_u_u.char_def.bits = 8;
		  element->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
		  $$->aoi_type_u_u.array_def.element_type = element;
		  $$->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NULL_TERMINATED_STRING;
		  aoi_type length = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  length->kind = AOI_INTEGER;
		  length->aoi_type_u_u.integer_def.min = 0;
		  length->aoi_type_u_u.integer_def.range = $3;
		  $$->aoi_type_u_u.array_def.length_type = length;
		}
		| STRING {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_ARRAY;
		  aoi_type element = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  element->kind = AOI_CHAR;
		  element->aoi_type_u_u.char_def.bits = 8;
		  element->aoi_type_u_u.char_def.flags = AOI_CHAR_FLAG_NONE;
		  $$->aoi_type_u_u.array_def.element_type = element;
		  $$->aoi_type_u_u.array_def.flgs = AOI_ARRAY_FLAG_NULL_TERMINATED_STRING;
		  aoi_type length = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  length->kind = AOI_INTEGER;
		  length->aoi_type_u_u.integer_def.min = 0;
		  length->aoi_type_u_u.integer_def.range = ~0;
		  $$->aoi_type_u_u.array_def.length_type = length;
		}
		;

array_declarator: id _fixed_arrays {
		  $$ = (Declaration *)mustcalloc(sizeof(Declaration));
		  $$->name = $1;
		  $$->sizes = *$2;
		}
		;

_fixed_arrays	: fixed_array_size {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, unsigned int, $1);
		}
		| _fixed_arrays fixed_array_size {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, unsigned int, $2);
		}
		;

fixed_array_size: LBRACK pos_int_const rbrack {
		  $$ = $2;
		}
		;

except_dcl	: EXCEPTION id lbrace {
		  aoi_def *tmp = new_aoi_def($2,scope);
		  $<integer>$ = cur_aoi.defs.defs_len;
		  AddDef(tmp, AOI_STRUCT);
		  AddScope($2);
		} _members RBRACE {
		  DelScope();
		  cur_aoi.defs.defs_val[$<integer>4].binding 
		    = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  cur_aoi.defs.defs_val[$<integer>4].binding->kind 
		    = AOI_EXCEPTION;
		  cur_aoi.defs.defs_val[$<integer>4].binding->aoi_type_u_u.exception_def.slots.slots_len 
		    = $5->cur_num;
		  cur_aoi.defs.defs_val[$<integer>4].binding->aoi_type_u_u.exception_def.slots.slots_val 
		    = &(ARRAY_REF(*$5,aoi_field,0));
		}
		;

_members	: {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = 0;
		  $$->act_num = 0;
		  $$->data = 0;
		}
		| _members member {
		  $$ = $1;
		  for (int zz = 0; zz < $2->cur_num; zz++) {
		    ADD_TO_ARRAY(*$$, aoi_field, ARRAY_REF(*$2,aoi_field,zz));
		  }
		}
 		| _members error {
	 	  ParseError("invalid member of exception declaration");
		  $$ = $1;
 		}  

		;

op_dcl		: _op_attribute op_type_spec {
			  if (($1) && ($2->kind != AOI_VOID)) {
			    SemanticError("oneway operations must return void");
			  }
		} id parameter_dcls  _raises_expr _context_expr {
			  // XXX - need to add support for context expressions
			  $$ = (aoi_operation *)mustcalloc(sizeof(aoi_operation));
			  $$->name = $4;
			  $$->return_type = $2;
			  $$->flags = (aoi_op_flags)
				      ($1 ? AOI_OP_FLAG_ONEWAY :
					    AOI_OP_FLAG_NONE);
			  $$->params.params_len = $5->cur_num;
			  $$->params.params_val = &(ARRAY_REF(*$5, aoi_parameter, 0));
			  $$->exceps.exceps_len = $6->cur_num;
			  $$->exceps.exceps_val = &(ARRAY_REF(*$6, aoi_type, 0));
			  $$->request_code = GetRequest($4);
			  $$->reply_code = GetReply($4);
		}
		;

_op_attribute	: {$$ = in_oneway = 0;}
		| ONEWAY {$$ = in_oneway = 1;}
		;

_raises_expr	: {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = 0;
		  $$->act_num = 0;
		  $$->data = (void *)mustcalloc(sizeof(int)*10);
		}
		| raises_expr {$$ = $1;}
		;

_context_expr	:
		| context_expr
		;

op_type_spec	: VOID {
		  $$ = (aoi_type)mustcalloc(sizeof(aoi_type_u));
		  $$->kind = AOI_VOID;
		}
		| param_type_spec {$$ = $1;}
		;

parameter_dcls	: lparen _param_dcls rparen {$$ = $2;}
		| lparen rparen {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = 0;
		  $$->data = (void *)mustcalloc(sizeof(int));
		}
		;

_param_dcls	: param_dcl {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, aoi_parameter, *$1);
		}
		| _param_dcls COMMA param_dcl {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, aoi_parameter, *$3);
		}
		| _param_dcls COMMA error {
		  if (in_oneway)
		    ParseError("expecting `in'");
		  else
  		    ParseError("expecting `in', `inout', or `out'");
		  $$ = $1;
		}
		;

param_dcl	: param_attribute {
		  if (in_oneway && ($1 != AOI_DIR_IN)) {
		    SemanticError("oneway operations allow `in' parameters only");
		  }
		} param_type_spec_or_error simple_declarator {
		  $$ = (aoi_parameter *)mustcalloc(sizeof(aoi_parameter));
		  $$->name = $4;
		  $$->type = $3;
		  $$->direction = (aoi_direction) $1;
		}
		;

param_attribute	: IN {$$ = AOI_DIR_IN;}
		| INOUT { $$ = AOI_DIR_INOUT;}
		| OUT {$$ = AOI_DIR_OUT;}
		;

param_type_spec_or_error	: param_type_spec {$$ = $1;}
			| error {
			  ParseError("invalid type"); 
			  $$ = MakeAoiType(kSLONG); 
			}

param_type_spec	: base_type_spec {$$ = $1;}
		| string_type {$$ = $1;}
		| scoped_name {
		  $$ = GetAoiType($1);
		}
		| sequence_type {
		  GrammarError(("`sequence<...>' cannot be used as a "
				"parameter type.  (Hint: use a `typedef'ed "
				"sequence type)"));
		  $$ = $1;
		}
		;

raises_expr	: RAISES lparen _scoped_names rparen {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  $$->cur_num = $3->cur_num;
		  $$->act_num = $3->cur_num;
		  $$->data = (VoidArray *)mustcalloc(sizeof(aoi_type) * $3->cur_num);
		  for (int zz = 0; zz < $3->cur_num; zz++) {
		    ARRAY_REF(*$$, aoi_type, zz)
		      = GetAoiType(ARRAY_REF(*$3, int, zz));
		  }
		}
		;

context_expr	: CONTEXT lparen _string_literals rparen
		| CONTEXT lparen rparen {
		  GrammarError("context expression missing string");
		}
		;

_string_literals: LIT_STRING {}
		| _string_literals COMMA LIT_STRING {}
		| _string_literals COMMA error {ParseError("expecting string");}
		;

attr_dcl	: _readonly ATTRIBUTE param_type_spec_or_error _simple_declarators {
		  aoi_def fake_def;
		  
		  fake_def.binding = 0;
		  fake_def.scope = scope;
		  fake_def.name = 0;
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  for (int zz = 0; zz < $4->cur_num; zz++) {
		    aoi_attribute a;
		    a.name = ARRAY_REF(*$4, char*, zz);
		    a.type = $3;
		    a.readonly = $1;
		    a.read_request_code = GetReadRequest(a.name);
		    a.read_reply_code = GetReadReply(a.name);
		    a.write_request_code = GetWriteRequest(a.name, a.readonly);
		    a.write_reply_code = GetWriteReply(a.name, a.readonly);
		    fake_def.name = a.name;
		    DupeError(&fake_def, AOI_STRUCT);
		    if (zz) {
		      for (int xx = 0; xx < $$->cur_num; xx++) {
		        if (ARRAY_REF(*$$, aoi_attribute, xx).name &&
			    !strcmp(ARRAY_REF(*$$, aoi_attribute, xx).name,
			            a.name)) {
			  SemanticError("`%s' already defined", a.name);
			}
		      }
		      ADD_TO_ARRAY(*$$, aoi_attribute, a);
		    } else {
		      NEW_ARRAY(*$$, aoi_attribute, a);
		    }
		  }
		}
		;

_readonly	: {$$ = 0;}
		| READONLY {$$ = 1;}
		;

_simple_declarators : simple_declarator {
		  $$ = (VoidArray *)mustcalloc(sizeof(VoidArray));
		  NEW_ARRAY(*$$, char*, $1);
		}
		| _simple_declarators COMMA simple_declarator {
		  $$ = $1;
		  ADD_TO_ARRAY(*$$, char* ,$3);
		}
		;

id 		: ID { $$ = $1; }
		| LIT_INT id {
		  GrammarError("number not expected"); 
		  $$ = $2; 
		}
		| MUL id { 
		  GrammarError("explicit pointers not allowed"); 
		  $$ = $2; 
		}
		| error {
		  ParseError("expecting identifier");
		  /*
		   * "" is `const char *' in modern C++; `muststrdup' lets us
		   * avoid a warning about assigning "" to a `char *'.
		   */
		  $$ = muststrdup("");
		}
		;

lt		: LT
		| error { ParseError("expecting `<'"); }
		; 
gt		: GT
		| error { ParseError("expecting `>'"); }
		; 
colon		: COLON
		| error { ParseError("expecting `:'"); }
		;
equal		: EQUAL
		| error { ParseError("expecting `='"); }
		;
semi		: SEMI
		| error { ParseError("expecting `;'"); }
		;
lparen		: LPAREN
		| error	{ ParseError("expecting `('"); }
		;
rparen		: RPAREN
		| error	{ ParseError("expecting `)'"); }
		;
lbrace		: LBRACE
		| error	{ ParseError("expecting `{'"); }
		;
rbrace		: RBRACE
		| error	{ ParseError("expecting `}'"); }
		; 
rbrack		: RBRACK
		| error	{ ParseError("expecting `]'"); }
		;

%%

void
yyerror(const char * /* s */)
{
  fail = 1;
  errorflag = lineno;
  strcpy(errorlinebuf,linebuf);
  errorPos = tokenPos;
}

/* End of file. */

