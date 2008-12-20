/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _MINT_H_RPCGEN
#define	_MINT_H_RPCGEN

#include <rpc/rpc.h>
#ifndef _flick_mint_h
#define _flick_mint_h

#include <rpc/types.h>
#include <rpc/xdr.h>

typedef int mint_ref;
#define	mint_ref_null -1
#define	mint_slot_index_null -1

typedef struct mint_const_u *mint_const;

enum mint_const_kind {
	MINT_CONST_INT = 1,
	MINT_CONST_CHAR = 2,
	MINT_CONST_FLOAT = 3,
	MINT_CONST_STRUCT = 4,
	MINT_CONST_ARRAY = 5
};
typedef enum mint_const_kind mint_const_kind;

enum mint_const_category {
	MINT_CONST_LITERAL = 1,
	MINT_CONST_SYMBOLIC = 2
};
typedef enum mint_const_category mint_const_category;

struct mint_const_int_u {
	mint_const_category kind;
	union {
		long value;
		char *name;
	} mint_const_int_u_u;
};
typedef struct mint_const_int_u mint_const_int_u;

struct mint_const_char_u {
	mint_const_category kind;
	union {
		char value;
		char *name;
	} mint_const_char_u_u;
};
typedef struct mint_const_char_u mint_const_char_u;

struct mint_const_float_u {
	mint_const_category kind;
	union {
		double value;
		char *name;
	} mint_const_float_u_u;
};
typedef struct mint_const_float_u mint_const_float_u;

typedef struct {
	u_int mint_const_struct_len;
	mint_const *mint_const_struct_val;
} mint_const_struct;

typedef struct {
	u_int mint_const_array_len;
	mint_const *mint_const_array_val;
} mint_const_array;

struct mint_const_u {
	mint_const_kind kind;
	union {
		mint_const_int_u const_int;
		mint_const_char_u const_char;
		mint_const_float_u const_float;
		mint_const_struct const_struct;
		mint_const_array const_array;
	} mint_const_u_u;
};
typedef struct mint_const_u mint_const_u;

enum mint_def_kind {
	MINT_VOID = 0,
	MINT_BOOLEAN = 1,
	MINT_INTEGER = 2,
	MINT_SCALAR = 3,
	MINT_FLOAT = 4,
	MINT_CHAR = 5,
	MINT_ARRAY = 6,
	MINT_STRUCT = 7,
	MINT_UNION = 8,
	MINT_INTERFACE = 9,
	MINT_SYSTEM_EXCEPTION = 10,
	MINT_ANY = 11,
	MINT_TYPE_TAG = 12,
	MINT_TYPED = 13
};
typedef enum mint_def_kind mint_def_kind;

struct mint_integer_def {
	int min;
	u_int range;
};
typedef struct mint_integer_def mint_integer_def;

typedef u_int mint_scalar_flags;
#define	MINT_SCALAR_FLAG_NONE 0
#define	MINT_SCALAR_FLAG_SIGNED 1
#define	MINT_SCALAR_FLAG_UNSIGNED 2

struct mint_scalar_def {
	int bits;
	mint_scalar_flags flags;
};
typedef struct mint_scalar_def mint_scalar_def;

typedef u_int mint_char_flags;
#define	MINT_CHAR_FLAG_NONE 0
#define	MINT_CHAR_FLAG_SIGNED 1
#define	MINT_CHAR_FLAG_UNSIGNED 2

struct mint_char_def {
	int bits;
	mint_char_flags flags;
};
typedef struct mint_char_def mint_char_def;

struct mint_float_def {
	int bits;
};
typedef struct mint_float_def mint_float_def;

struct mint_array_def {
	mint_ref element_type;
	mint_ref length_type;
};
typedef struct mint_array_def mint_array_def;

struct mint_struct_def {
	struct {
		u_int slots_len;
		mint_ref *slots_val;
	} slots;
};
typedef struct mint_struct_def mint_struct_def;

struct mint_union_case {
	mint_const val;
	mint_ref var;
};
typedef struct mint_union_case mint_union_case;

struct mint_union_def {
	mint_ref discrim;
	struct {
		u_int cases_len;
		mint_union_case *cases_val;
	} cases;
	mint_ref dfault;
};
typedef struct mint_union_def mint_union_def;

struct mint_typed_def {
	mint_ref tag;
	mint_ref ref;
};
typedef struct mint_typed_def mint_typed_def;

enum mint_interface_right {
	MINT_INTERFACE_NAME = 0,
	MINT_INTERFACE_INVOKE = 1,
	MINT_INTERFACE_INVOKE_ONCE = 2,
	MINT_INTERFACE_SERVICE = 3
};
typedef enum mint_interface_right mint_interface_right;

struct mint_interface_def {
	mint_interface_right right;
};
typedef struct mint_interface_def mint_interface_def;

struct mint_def {
	mint_def_kind kind;
	union {
		mint_integer_def integer_def;
		mint_scalar_def scalar_def;
		mint_float_def float_def;
		mint_char_def char_def;
		mint_array_def array_def;
		mint_struct_def struct_def;
		mint_union_def union_def;
		mint_interface_def interface_def;
		mint_typed_def typed_def;
	} mint_def_u;
};
typedef struct mint_def mint_def;

struct mint_standard_refs {
	mint_ref void_ref;
	mint_ref bool_ref;
	mint_ref signed8_ref;
	mint_ref signed16_ref;
	mint_ref signed32_ref;
	mint_ref signed64_ref;
	mint_ref unsigned8_ref;
	mint_ref unsigned16_ref;
	mint_ref unsigned32_ref;
	mint_ref unsigned64_ref;
	mint_ref char8_ref;
	mint_ref float32_ref;
	mint_ref float64_ref;
	mint_ref interface_name_ref;
	mint_ref interface_invoke_ref;
	mint_ref interface_invoke_once_ref;
	mint_ref interface_service_ref;
	mint_ref system_exception_ref;
};
typedef struct mint_standard_refs mint_standard_refs;

struct mint_1 {
	struct {
		u_int defs_len;
		mint_def *defs_val;
	} defs;
	mint_standard_refs standard_refs;
};
typedef struct mint_1 mint_1;
#endif /* _flick_mint_h */

/* the xdr functions */
extern bool_t xdr_mint_ref();
extern bool_t xdr_mint_const();
extern bool_t xdr_mint_const_kind();
extern bool_t xdr_mint_const_category();
extern bool_t xdr_mint_const_int_u();
extern bool_t xdr_mint_const_char_u();
extern bool_t xdr_mint_const_float_u();
extern bool_t xdr_mint_const_struct();
extern bool_t xdr_mint_const_array();
extern bool_t xdr_mint_const_u();
extern bool_t xdr_mint_def_kind();
extern bool_t xdr_mint_integer_def();
extern bool_t xdr_mint_scalar_flags();
extern bool_t xdr_mint_scalar_def();
extern bool_t xdr_mint_char_flags();
extern bool_t xdr_mint_char_def();
extern bool_t xdr_mint_float_def();
extern bool_t xdr_mint_array_def();
extern bool_t xdr_mint_struct_def();
extern bool_t xdr_mint_union_case();
extern bool_t xdr_mint_union_def();
extern bool_t xdr_mint_typed_def();
extern bool_t xdr_mint_interface_right();
extern bool_t xdr_mint_interface_def();
extern bool_t xdr_mint_def();
extern bool_t xdr_mint_standard_refs();
extern bool_t xdr_mint_1();

#endif /* !_MINT_H_RPCGEN */