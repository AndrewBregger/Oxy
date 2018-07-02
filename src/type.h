#ifndef TYPE_H_
#define TYPE_H_
#include "common.h"
#include "value.h"

typedef struct Entity Entity;
typedef struct Scope Scope;

typedef enum TypeKind {
	Type_None,
	Type_U8,
	Type_U16,
	Type_U32,
	Type_U64,
	Type_I8,
	Type_I16,
	Type_I32,
	Type_I64,
	Type_F32,
	Type_F64,
	Type_Bool,
	Type_Char,
	Type_Function,
	// Type_Struct,
	// Type_Enum,
	Type_Aggragate,
	// etc
	Type_Array,
	// DynamicArray,
	Type_Ptr,
#ifdef USE_REF_TYPE
	Type_Ref,
#endif
	Type_Null,
	Type_Byte = Type_U8,

	Num_Types
} TypeKind;

typedef struct Type Type;
typedef struct Expr Expr;
typedef struct Item Item;

typedef struct FunctionType {
	Type** parameters;
	u32 num_params;

	Type* ret;
	Item* item;
} FunctionType;

// since structs and enums are equivalent by name,
// each struct and enum's types will be unique.
typedef struct AggregateType {
	Ident* name;
	Entity** members;
	u32 num_members;
	Item* item;
	Scope* scope;
} AggregateType;

typedef struct ArrayType {
	Type* elem;
	Value size;
	// Expr* size; // or value?
} ArrayType;

typedef struct Type {
	TypeKind kind; 	//< what is this type.
	u32 size;	    	//< the size of this type.
	const char* name; 		//< the name of the type? can be null

	u64 uid; 				//< unique identifier for the type

	union {
		FunctionType funct;
		AggragateType agg;
		// EnumType entype;
		ArrayType arrtype;
		Type* ptr;
#ifdef USE_REF_TYPE
		Type* ref;
#endif
	};

} Type;

// this doesnt need to be in the header.
Type* new_type(TypeKind kind, u32 size);

Type* new_array(Type* base, Value size);

#ifdef USE_REF_TYPE
Type* new_ref(Type* base);
#endif

Type* new_ptr(Type* base);

Type* new_aggragate_type(Entity** members, Scope* scope, Item* item);

Type* new_function_type(Type** members, Type* ret, Item* item);

Type* new_null_type();

// rank meaning which has precedence.
// IE. if an operation is done with
// i32 and f32 then f32 will have precedence
// forcing the i32 to casted/promoted to f32.
i32 type_rank(Type* type);

bool is_arithmetic_type(Type* type);

bool is_integer_type(Type* type);

bool is_ptr_type(Type* type);

bool is_null_type(Type* type);

bool is_bool_type(Type* type);

bool is_type_struct(Type* type);

bool is_type_enum(Type* type);

const char* type_string(Type* type);

bool is_primative_type(Type* type);

bool equivalent_types(Type* type1, Type* type2);


#endif
