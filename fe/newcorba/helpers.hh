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

#ifndef _helpers_hh
#define _helpers_hh

#include "Errors.hh"
extern "C" {
#include <mom/aoi.h>
}

extern const char *infilename;
extern const char *root_filename;


/*****************************************************************************/

/*
 * Macros for manipulating `VoidArray's, which the parser uses to manage arrays
 * of various types.
 */

#define NEW_ARRAY(array, type, value) {				\
	(array).cur_num = 1;					\
	(array).act_num = 8;					\
	(array).data = (void *) mustcalloc(sizeof(type) * 8);	\
	((type *) ((array).data))[0] = value;			\
}

#define ADD_TO_ARRAY(array, type, value) {				  \
	if ((array).cur_num == (array).act_num) {			  \
		(array).act_num += 8;					  \
		type * QQtemp = (type *) ((array).data);		  \
		(array).data = (void *) mustcalloc(sizeof(type)		  \
						   * (array).act_num);	  \
		for (int QQtmp = 0; QQtmp < (array).cur_num; ++QQtmp)	  \
			((type *) ((array).data))[QQtmp] = QQtemp[QQtmp]; \
		free(QQtemp);						  \
	}								  \
	((type *) ((array).data))[(array).cur_num++] = value;		  \
}

#define ARRAY_REF(array, type, pos) ((type *)((array).data))[pos]


/*****************************************************************************/

enum {
	kBOOL,
	kCHAR,
	kDOUBLE,
	kFLOAT,
	kSTRING,
	kOCTET,
	kANY,
	kSSHORT,
	kSLONG,
	kUSHORT,
	kULONG,
	kERROR,
	kOBJECT,
	kSLLONG, /* long longs (64 bits) */
	kULLONG  /* unsigned long longs (64 bits) */
};

typedef unsigned int types;

struct VoidArray {
	int cur_num, act_num;
	void *data;
};

struct Declaration {
	VoidArray sizes;
	char *name;
};

struct ref_list {
	ref_list(char *n, ref_list *r)
	{
		name = n;
		next = r;
	}
	
	char *name;
	ref_list *next;
};

extern int saved_aoi_len;
extern int scope;
extern aoi cur_aoi;
extern int aoi_length; 
/* This needs to be allocated to begin with (otherwise we get S/R errors). */
extern aoi_interface *cur_interface;


/*****************************************************************************/

void Start();
void Finish();

void AddScope(char *);
void DelScope();
void DupeError(aoi_def *, aoi_kind);
aoi_interface *GetNewInterface(void);
aoi_def *AoiConst(types, char *, aoi_const);
aoi_def *new_aoi_def(const char *, int); 
int new_error_ref(const char *);
aoi_def *AddDef(aoi_def *, aoi_kind); /* `kind' only used for dup. checking. */
void AddAttrs(VoidArray *);
void AddOp(aoi_operation *);
void AddParents(VoidArray *);
int FindLocalName(char *, int err = 1);
int FindGlobalName(char *);
int FindScopedName(char *, aoi_ref);
int GetScopedNameList(ref_list *);
int GetInsideScope(ref_list *, int);
types GetConstType(aoi_ref);
aoi_const GetConstVal(aoi_ref);
aoi_type GetAoiType(aoi_ref);
aoi_type MakeAoiType(types);
aoi_type GetAoiTypeFromDecl(aoi_type, Declaration);
aoi_def *GetAoiDefFromDecl(aoi_type, Declaration);
aoi_const GetReadRequest(char *);
aoi_const GetReadReply(char *);
aoi_const GetWriteRequest(char *, int);
aoi_const GetWriteReply(char *, int);
aoi_const GetRequest(char *);
aoi_const GetReply(char *);
aoi_const GetInterfaceCode(char*);
unsigned int GetPosInt(aoi_const);
int isInt(aoi_const_u);
int isFloat(aoi_const_u);

aoi_const const_or(aoi_const, aoi_const);
aoi_const const_xor(aoi_const, aoi_const);
aoi_const const_and(aoi_const, aoi_const);
aoi_const const_lshft(aoi_const, aoi_const);
aoi_const const_rshft(aoi_const, aoi_const);
aoi_const const_mul(aoi_const, aoi_const);
aoi_const const_div(aoi_const, aoi_const);
aoi_const const_add(aoi_const, aoi_const);
aoi_const const_sub(aoi_const, aoi_const);
aoi_const const_mod(aoi_const, aoi_const);
aoi_const const_bit(aoi_const);
aoi_const const_neg(aoi_const);
aoi_const const_pos(aoi_const);
aoi_const MakeConstInt(int);
aoi_const MakeConstString(char*);
aoi_const MakeConstPackedString(char*);
aoi_const MakeConstChar(char);
aoi_const MakeConstReal(double);

void CheckUnionCaseDuplicate(VoidArray *, aoi_union_case *);

void ResolveForwardInterfaces();

#endif /* _helpers_hh */

/* End of file. */

