/*
 * Copyright (c) 1997, 1998, 1999 The University of Utah and
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

/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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

%token	sySkip
%token	syRoutine
%token	sySimpleRoutine
%token	sySimpleProcedure
%token	syProcedure
%token	syFunction

%token	sySubsystem
%token	syKernelUser
%token	syKernelServer

%token	syMsgOption
%token	syMsgSeqno
%token	syWaitTime
%token	syNoWaitTime
%token	syErrorProc
%token	syServerPrefix
%token	syUserPrefix
%token	syServerDemux
%token	syRCSId

%token	syImport
%token	syUImport
%token	sySImport

%token	syIn
%token	syOut
%token	syInOut
%token	syRequestPort
%token	syReplyPort
%token	sySReplyPort
%token	syUReplyPort

%token	syType
%token	syArray
%token	syStruct
%token	syOf

%token	syInTran
%token	syOutTran
%token	syDestructor
%token	syCType
%token	syCUserType
%token	syCServerType

%token	syCString

%token	syColon
%token	sySemi
%token	syComma
%token	syPlus
%token	syMinus
%token	syStar
%token	syDiv
%token	syLParen
%token	syRParen
%token	syEqual
%token	syCaret
%token	syTilde
%token	syLAngle
%token	syRAngle
%token	syLBrack
%token	syRBrack
%token	syBar

%token	syError			/* lex error */

%token	<number>	syNumber
%token	<symtype>	sySymbolicType
%token	<identifier>	syIdentifier
%token	<string>	syString syQString
%token	<string>	syFileName
%token	<flag>		syIPCFlag

%left	syPlus,syMinus
%left	syStar,syDiv


%type	<statement_kind> ImportIndicant
%type	<number> VarArrayHead ArrayHead StructHead  IntExpOrError
%type	<type> NamedTypeSpec NamedTypeSpecOrError TransTypeSpec TypeSpec
%type	<type> CStringSpec
%type	<type> BasicTypeSpec PrevTypeSpec ArgumentType
%type	<symtype> PrimIPCType PrimIPCTypeOrError IPCType IPCTypeOrError
%type	<routine> RoutineDecl Routine SimpleRoutine
%type	<routine> Procedure SimpleProcedure Function
%type	<direction> Direction
%type	<argument> Argument Arguments ArgumentList
%type	<flag> IPCFlags IPCFlagOrError
%type	<identifier> IdentifierOrError IdentifierOrNULLError
%type   <identifier> IdentifierOrEmptyError
%type	<number> NumberOrError
%type	<string> StringOrNULLError QStringOrError FileNameOrError
%{

#include <stdio.h>

#include "error.h"
#include "lexxer.h"
#include "global.h"
#include "mig_string.h"
#include "type.h"
#include "routine.h"
#include "statement.h"
#include <mom/compiler.h>

int yylex();

static const char *import_name(statement_kind_t sk);

void
yyerror(const char *s)
{
#if 0
	error(s);
#endif
}

void
ParseError(const char *s)
{
	error(s);
}

void
ParseErrorWithSavedPos(const char *s)
{
	int realtokenpos = tokenpos;
	int reallineno = lineno;
	
	tokenpos = savedpos;
	lineno = savedlineno;
	error(s);
	tokenpos = realtokenpos;
	lineno = reallineno;
}

%}

%union
{
    u_int number;
    identifier_t identifier;
    const_string_t string;
    statement_kind_t statement_kind;
    ipc_type_t *type;
    struct
    {
	u_int innumber;		/* msgt_name value, when sending */
	const_string_t instr;
	u_int outnumber;	/* msgt_name value, when receiving */
	const_string_t outstr;
	u_int size;		/* 0 means there is no default size */
    } symtype;
    routine_t *routine;
    arg_kind_t direction;
    argument_t *argument;
    ipc_flags_t flag;
}

%%

Statements		:	/* empty */
			|	Statements Statement
			;

Statement		:	Subsystem SemiOrError
			|	WaitTime SemiOrError
			|	MsgOption SemiOrError
			|	Error SemiOrError
			|	ServerPrefix SemiOrError
			|	UserPrefix SemiOrError
			|	ServerDemux SemiOrError
			|	TypeDecl SemiOrError
			|	RoutineDecl SemiOrError
{
    register statement_t *st = stAlloc();

    st->stKind = skRoutine;
    st->stRoutine = $1;
    rtCheckRoutine($1);
    if (BeVerbose)
	rtPrintRoutine($1);
}
			|	sySkip sySemi
				{ rtSkip(1); }
			|	sySkip NumberOrError SemiOrError
				{ rtSkip($2); }
			|	Import SemiOrError
			|	RCSDecl SemiOrError
			|	sySemi
			|	error sySemi
				{ ParseError("invalid statement"); yyerrok; }
			;

Subsystem		:	SubsystemStart SubsystemMods
				SubsystemName SubsystemBase
{
    if (BeVerbose)
    {
	printf("Subsystem %s: base = %u%s%s\n\n",
	       SubsystemName, SubsystemBase,
	       IsKernelUser ? ", KernelUser" : "",
	       IsKernelServer ? ", KernelServer" : "");
    }
}
			;

SubsystemStart		:	sySubsystem
{
    if (SubsystemName != strNULL)
    {
	warn("previous Subsystem decl (of %s) will be ignored", SubsystemName);
	IsKernelUser = FALSE;
	IsKernelServer = FALSE;
	strfree((string_t) SubsystemName);
    }
}
			;

SubsystemMods		:	/* empty */
			|	SubsystemMods SubsystemMod
			;

SubsystemMod		:	syKernelUser
{
    if (IsKernelUser)
	warn("duplicate KernelUser keyword");
    IsKernelUser = TRUE;
}
			|	syKernelServer
{
    if (IsKernelServer)
	warn("duplicate KernelServer keyword");
    IsKernelServer = TRUE;
}
			;

SubsystemName		:	IdentifierOrNULLError { SubsystemName = $1; }
			;

SubsystemBase		:	NumberOrError	{ SubsystemBase = $1; }
			;

MsgOption		:	LookString syMsgOption StringOrNULLError
{
    if (streql($3, "MACH_MSG_OPTION_NONE"))
    {
	MsgOption = strNULL;
	if (BeVerbose)
	    printf("MsgOption: cancelled\n\n");
    }
    else
    {
	MsgOption = $3;
	if (BeVerbose)
	    printf("MsgOption %s\n\n",$3);
    }
}
			;

WaitTime		:	LookString syWaitTime StringOrNULLError
{
    WaitTime = $3;
    if (BeVerbose)
	printf("WaitTime %s\n\n", WaitTime);
}
			|	syNoWaitTime
{
    WaitTime = strNULL;
    if (BeVerbose)
	printf("NoWaitTime\n\n");
}
			;

Error			:	syErrorProc IdentifierOrEmptyError
{
    ErrorProc = $2;
    if (BeVerbose)
	printf("ErrorProc %s\n\n", ErrorProc);
}
			;

ServerPrefix		:	syServerPrefix IdentifierOrEmptyError
{
    ServerPrefix = $2;
    if (BeVerbose)
	printf("ServerPrefix %s\n\n", ServerPrefix);
}
			;

UserPrefix		:	syUserPrefix IdentifierOrEmptyError
{
    UserPrefix = $2;
    if (BeVerbose)
	printf("UserPrefix %s\n\n", UserPrefix);
}
			;

ServerDemux		:	syServerDemux IdentifierOrNULLError
{
    ServerDemux = $2;
    if (BeVerbose)
	printf("ServerDemux %s\n\n", ServerDemux);
}
			;

Import			:	LookFileName ImportIndicant FileNameOrError
{
    register statement_t *st = stAlloc();
    st->stKind = $2;
    st->stFileName = $3;

    if (BeVerbose)
	printf("%s %s\n\n", import_name($2), $3);
}
			;

ImportIndicant		:	syImport	{ $$ = skImport; }
			|	syUImport	{ $$ = skUImport; }
			|	sySImport	{ $$ = skSImport; }
			;

RCSDecl			:	LookQString syRCSId QStringOrError
{
    if (RCSId != strNULL)
	warn("previous RCS decl will be ignored");
    if (BeVerbose)
	printf("RCSId %s\n\n", $3);
    RCSId = $3;
}
			;

TypeDecl		:	syType NamedTypeSpecOrError
{
    register identifier_t name = $2->itName;

    if (itLookUp(name) != itNULL)
	warn("overriding previous definition of %s", name);
    itInsert(name, $2);
}
			;

NamedTypeSpec           :	syIdentifier syEqual TransTypeSpec
				{ itTypeDecl($1, $$ = $3); }
			;

NamedTypeSpecOrError    :	IdentifierOrError EqualOrError TransTypeSpec
				{ itTypeDecl($1, $$ = $3); }
			;

TransTypeSpec		:	TypeSpec
				{ $$ = itResetType($1); }
			|	TransTypeSpec syInTran ColonOrError
				IdentifierOrError IdentifierOrError
				LParenOrError IdentifierOrError RParenOrError
{
    $$ = $1;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $4))
	warn("conflicting translation types (%s, %s)",
	     $$->itTransType, $4);
    $$->itTransType = $4;

    if (($$->itInTrans != strNULL) && !streql($$->itInTrans, $5))
	warn("conflicting in-translation functions (%s, %s)",
	     $$->itInTrans, $5);
    $$->itInTrans = $5;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $7))
	warn("conflicting server types (%s, %s)",
	     $$->itServerType, $7);
    $$->itServerType = $7;
}
			|	TransTypeSpec syOutTran ColonOrError
				IdentifierOrError IdentifierOrError
				LParenOrError IdentifierOrError RParenOrError
{
    $$ = $1;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
	warn("conflicting server types (%s, %s)",
	     $$->itServerType, $4);
    $$->itServerType = $4;

    if (($$->itOutTrans != strNULL) && !streql($$->itOutTrans, $5))
	warn("conflicting out-translation functions (%s, %s)",
	     $$->itOutTrans, $5);
    $$->itOutTrans = $5;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $7))
	warn("conflicting translation types (%s, %s)",
	     $$->itTransType, $7);
    $$->itTransType = $7;
}
			|	TransTypeSpec syDestructor ColonOrError
				IdentifierOrError LParenOrError
				IdentifierOrError RParenOrError
{
    $$ = $1;

    if (($$->itDestructor != strNULL) && !streql($$->itDestructor, $4))
	warn("conflicting destructor functions (%s, %s)",
	     $$->itDestructor, $4);
    $$->itDestructor = $4;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $6))
	warn("conflicting translation types (%s, %s)",
	     $$->itTransType, $6);
    $$->itTransType = $6;
}
			|	TransTypeSpec syCType ColonOrError
				IdentifierOrError
{
    $$ = $1;

    if (($$->itUserType != strNULL) && !streql($$->itUserType, $4))
	warn("conflicting user types (%s, %s)",
	     $$->itUserType, $4);
    $$->itUserType = $4;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
	warn("conflicting server types (%s, %s)",
	     $$->itServerType, $4);
    $$->itServerType = $4;
}
			|	TransTypeSpec syCUserType ColonOrError
				IdentifierOrError
{
    $$ = $1;

    if (($$->itUserType != strNULL) && !streql($$->itUserType, $4))
	warn("conflicting user types (%s, %s)",
	     $$->itUserType, $4);
    $$->itUserType = $4;
}
			|	TransTypeSpec syCServerType
				ColonOrError IdentifierOrError
{
    $$ = $1;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
	warn("conflicting server types (%s, %s)",
	     $$->itServerType, $4);
    $$->itServerType = $4;
}
			;

TypeSpec		:	BasicTypeSpec
				{ $$ = $1; }
			|	PrevTypeSpec
				{ $$ = $1; }
			|	VarArrayHead TypeSpec
				{ $$ = itVarArrayDecl($1, $2); }
			|	ArrayHead TypeSpec
				{ $$ = itArrayDecl($1, $2); }
			|	syCaret TypeSpec
				{ $$ = itPtrDecl($2); }
			|	StructHead TypeSpec
				{ $$ = itStructDecl($1, $2); }
			|	CStringSpec
				{ $$ = $1; }
			| 	error {
					ParseError("expecting a type");
					$$ = itShortDecl(0, "error",
		     				0, "error",
		 				1);
				}
			;

BasicTypeSpec		:	IPCType
{
    $$ = itShortDecl($1.innumber, $1.instr,
		     $1.outnumber, $1.outstr,
		     $1.size);
}
			|	syLParen IPCTypeOrError CommaOrError
				IntExpOrError IPCFlags RParenOrError
{
    $$ = itLongDecl($2.innumber, $2.instr,
		    $2.outnumber, $2.outstr,
		    $2.size, $4, $5);
}
			;

IPCFlags		:	/* empty */
				{ $$ = flNone; }
			|	IPCFlags syComma /* Comma always required */
				IPCFlagOrError
{
    if ($1 & $3)
	warn("redundant IPC flag ignored");
    else
	$$ = $1 | $3;
}
			|	IPCFlags syComma IPCFlagOrError
				syLBrack RBrackOrError
{
    if ($3 != flDealloc)
	warn("only Dealloc is variable");
    else
	$$ = $1 | flMaybeDealloc;
}
			;

PrimIPCType		:	syNumber
{
    $$.innumber = $$.outnumber = $1;
    $$.instr = $$.outstr = strNULL;
    $$.size = 0;
}
			|	sySymbolicType
				{ $$ = $1; }
			;

PrimIPCTypeOrError	:	PrimIPCType
				{ $$ = $1; }
			|	error
{
	ParseError("number or IPC Type expected");
	$$.innumber = $$.outnumber = 0;
	$$.instr = $$.outstr = "<error>";
	$$.size = 0;
}
			;

IPCTypeOrError		:	IPCType
				{ $$ = $1; }
			|	error
{
	ParseError("number or IPC Type expected");
	$$.innumber = $$.outnumber = 0;
	$$.instr = $$.outstr = "<error>";
	$$.size = 0;
}
			;

IPCType			:	PrimIPCType
				{ $$ = $1; }
			|	PrimIPCType syBar PrimIPCTypeOrError
{
    if ($1.size != $3.size)
    {
	if ($1.size == 0)
	    $$.size = $3.size;
	else if ($3.size == 0)
	    $$.size = $1.size;
	else
	{
	    error("unequal sizes in IPCTypes (%d, %d) not supported",
		  $1.size, $3.size);
	    $$.size = 0;
	}
    }
    else
	$$.size = $1.size;
    $$.innumber = $1.innumber;
    $$.instr = $1.instr;
    $$.outnumber = $3.outnumber;
    $$.outstr = $3.outstr;
}
			;

PrevTypeSpec		:	syIdentifier
				{ $$ = itPrevDecl($1); }
			|	syIdentifier syBar
				{ $$ = itPrevDecl($1);
				  ParseError(("`|' not allowed (legal only "
					      "for standard MIG IPC types)"));
                                }
			|	syIdentifier syBar syIdentifier
				{ $$ = itPrevDecl($1);
				  ParseError(("`|' not allowed (legal only "
					      "for standard MIG IPC types)"));
                                }
			;

VarArrayHead		:	syArray LBrackOrError syRBrack OfOrError
				{ $$ = 0; }
			|	syArray LBrackOrError syStar RBrackOrError
				OfOrError
				{ $$ = 0; }
			|	syArray LBrackOrError syStar syColon
				IntExpOrError RBrackOrError OfOrError
				{ $$ = $5; }
			;

ArrayHead		:	syArray LBrackOrError IntExpOrError
				RBrackOrError OfOrError
				{ $$ = $3; }
			;

StructHead		:	syStruct LBrackOrError IntExpOrError
				RBrackOrError OfOrError
				{ $$ = $3; }
			;

CStringSpec		:	syCString LBrackOrError IntExpOrError
				RBrackOrError
				{ $$ = itCStringDecl($3, FALSE); }
			|	syCString LBrackOrError syStar ColonOrError
				IntExpOrError RBrackOrError
				{ $$ = itCStringDecl($5, TRUE); }
			;

IntExpOrError		: 	IntExpOrError	syPlus	IntExpOrError
				{ $$ = $1 + $3;	}
			| 	IntExpOrError	syMinus	IntExpOrError
				{ $$ = $1 - $3;	}
			|	IntExpOrError	syStar	IntExpOrError
				{ $$ = $1 * $3;	}
			| 	IntExpOrError	syDiv	IntExpOrError
				{ $$ = $1 / $3;	}
			|	NumberOrError
				{ $$ = $1;	}
			|	syLParen IntExpOrError RParenOrError
				{ $$ = $2;	}
			;

RoutineDecl		:	Routine			{ $$ = $1; }
			|	SimpleRoutine		{ $$ = $1; }
			|	Procedure		{ $$ = $1; }
			|	SimpleProcedure		{ $$ = $1; }
			|	Function		{ $$ = $1; }
			;

Routine			:	syRoutine IdentifierOrError Arguments
				{ $$ = rtMakeRoutine($2, $3); }
			;

SimpleRoutine		:	sySimpleRoutine IdentifierOrError Arguments
				{ $$ = rtMakeSimpleRoutine($2, $3); }
			;

Procedure		:	syProcedure IdentifierOrError Arguments
				{ $$ = rtMakeProcedure($2, $3); }
			;

SimpleProcedure		:	sySimpleProcedure IdentifierOrError Arguments
				{ $$ = rtMakeSimpleProcedure($2, $3); }
			;

Function		:	syFunction IdentifierOrError Arguments
				ArgumentType
				{ $$ = rtMakeFunction($2, $3, $4); }
			;

Arguments		:	LParenOrError syRParen
				{ $$ = argNULL; }
			|	LParenOrError ArgumentList RParenOrError
				{ $$ = $2; }
			;

ArgumentList		:	Argument
				{ $$ = $1; }
			|	Argument saveTokenPos sySemi ArgumentList
{
    $$ = $1;
    $$->argNext = $4;
}
			|	Argument saveTokenPos SemiError
				{ $$ = $1; }
			;

Argument		:	Direction syIdentifier ArgumentType
				IPCFlags
{
    $$ = argAlloc();
    $$->argKind = $1;
    $$->argName = $2;
    $$->argType = $3;
    $$->argFlags = $4;
}
			;

Direction		:	/* empty */	{ $$ = akNone; }
			|	syIn		{ $$ = akIn; }
			|	syOut		{ $$ = akOut; }
			|	syInOut		{ $$ = akInOut; }
			|	syRequestPort	{ $$ = akRequestPort; }
			|	syReplyPort	{ $$ = akReplyPort; }
			|	sySReplyPort	{ $$ = akSReplyPort; }
			|	syUReplyPort	{ $$ = akUReplyPort; }
			|	syWaitTime	{ $$ = akWaitTime; }
			|	syMsgOption	{ $$ = akMsgOption; }
			|	syMsgSeqno	{ $$ = akMsgSeqno; }
			;

ArgumentType		:	ColonOrError IdentifierOrError
{
	if ($2[0])
    	{
    		$$ = itLookUp($2);
    		if ($$ == itNULL)
			error("type `%s' not defined", $2);
	}
}
			|	ColonOrError NamedTypeSpec
				{ $$ = $2; }
			;

SemiError 		:	sySemi {
					ParseErrorWithSavedPos("extra `;'");
				}
			;

SemiOrError	 	:	sySemi
			|	error {
					ParseError("`;' expected");
				}

EqualOrError	 	:	syEqual
			|	error {
					ParseError("`=' expected");
				}

CommaOrError	 	:	syComma
			|	error {
					ParseError("`,' expected");
				}

LParenOrError	 	:	syLParen
			|	error {
					ParseError("`(' expected");
				}

RParenOrError	 	:	syRParen
			|	error {
					ParseError("`)' expected");
				}

LBrackOrError	 	:	syLBrack
			|	error {
					ParseError("`[' expected");
				}

RBrackOrError	 	:	syRBrack
			|	error {
					ParseError("`]' expected");
				}

OfOrError	 	:	syOf
			|	error {
					ParseError("`of' expected");
				}

ColonOrError	 	:	syColon
			|	error {
					ParseError("`:' expected");
				}

IPCFlagOrError          :	syIPCFlag { $$ = $1; }
			|	error {
					ParseError(("IPC flag expected "
						    "because of extra comma "
						    "found"));
					$$ = flNone;
				}

IdentifierOrError       :	syIdentifier { $$ = $1; }
			|	error {
					ParseError("identifier expected");
					$$ = flick_asprintf("_error %d_%d_",
							    lineno, tokenpos);
				}

IdentifierOrEmptyError  :	syIdentifier { $$ = $1; }
			|	error {
					ParseError("identifier expected");
					$$ = "";
				}

IdentifierOrNULLError   :	syIdentifier { $$ = $1; }
			|	error {
					ParseError("identifier expected");
					$$ = strNULL;
				}

NumberOrError   	:	syNumber { $$ = $1; }
			|	error {
					ParseError("number expected");
					$$ = 0;
				}

StringOrNULLError       	:	syString { $$ = $1; }
			|	error {
					ParseError("string expected");
					$$ = strNULL;
				}
QStringOrError       	:	syQString { $$ = $1; }
			|	error {
					ParseError("quoted string expected");
					$$ = "\"<error>\"";
				}

FileNameOrError       	:	syFileName { $$ = $1; }
			|	error {
					ParseError("filename expected");
					$$ = "error.error";
				}

LookString		:	/* empty */
				{ LookString(); }
			;

LookFileName		:	/* empty */
				{ LookFileName(); }
			;

LookQString		:	/* empty */
				{ LookQString(); }
			;
saveTokenPos		:	/* empty */
				{ saveTokenPos(); }
			;


%%

static const char *
import_name(statement_kind_t sk)
{
    switch (sk)
    {
      case skImport:
	return "Import";
      case skSImport:
	return "SImport";
      case skUImport:
	return "UImport";
      default:
	fatal("import_name(%d): not import statement", (int) sk);
	/*NOTREACHED*/
    }
}

/* End of file. */

