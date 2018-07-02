#include "type.h"
#include "ast.h"
#include <assert.h>

#include "entity.h"

static u64 id = 1;
Type* new_type(TypeKind kind, u32 size) {
	Type* type = (Type*) malloc(sizeof(Type));

	type->kind = kind;
	type->size = size;
	type->uid = id++;

	return type;
}

Type* new_array(Type* base, Value size) {
	assert(size.kind == Value_Integer);
	Type* array = new_type(Type_Array, base->size * size.integer_value);

	array->arrtype.elem = base;
	array->arrtype.size = size;

	return array;
}

#ifdef USE_REF_TYPE
Type* new_ref(Type* base) {
	// for 64 bit arch it will be 8 bytes
	Type* ref = new_type(Type_Ref, 8); // get the system size of ref.
	ref->ref = base;
	return ref;
}
#endif

Type* new_ptr(Type* base) {
	// for 64 bit arch it will be 8 bytes
	Type* ptr = new_type(Type_Ptr, 8); // get the system size of ptr.
	ptr->ptr = base;
	return ptr;
}

Type* new_aggregate_type(Entity** members, Scope* scope, Item* item) {
	u32 size = 0;
	u32 num_members = buf_len(members);

	for(u32 i = 0; i < num_members; ++i)
		size += members[i]->type->size;

	Type* type 							= new_type(Type_Aggragate, size);
	type->agg.members 			= members;
	type->agg.num_members 	= num_members;
	type->agg.item  				= item;
	type->agg.scope 				= scope;

	if(is_type_struct(type))
		type->agg.name = item->structure.name;
	else
		type->agg.name = item->enumeration.name;

	return type;
}

Type* new_function_type(Type** members, Type* ret, Item* item) {
	Type* funct = new_type(Type_Function, 8);

	funct->funct.parameters = members;
	funct->funct.num_params = buf_len(members);
	funct->funct.ret = ret;
	funct->funct.item = item;

	return funct;
}

Type* new_null_type() {
	return new_type(Type_Null, 0);
}

bool is_arithmetic_type(Type* type) {
	switch(type->kind) {
		case Type_U8:
		case Type_U16:
		case Type_U32:
		case Type_U64:
		case Type_I8:
		case Type_I16:
		case Type_I32:
		case Type_I64:
		case Type_F32:
		case Type_F64:
		case Type_Bool:
		case Type_Char:
			return true;
		default:
			return false;
	}
}

bool is_integer_type(Type* type) {
	switch(type->kind) {
		case Type_U8:
		case Type_U16:
		case Type_U32:
		case Type_U64:
		case Type_I8:
		case Type_I16:
		case Type_I32:
		case Type_Char:
		case Type_I64:
			return true;
		default:
			return false;
	}
}

bool is_ptr_type(Type* type) {
	return type->kind == Type_Ptr;
}

bool is_null_type(Type* type) {
	return type->kind == Type_Null;
}

bool is_bool_type(Type* type) {
	return type->kind = Type_Bool;
}

const char* type_string(Type* type) {
	if(!type)
		return "null type";

	if(type->name)
		return type->name;

	// local buffer
	char buffer[512];

	switch(type->kind) {
		case Type_Function: {
			sprintf(buffer, "fn(");
			// add generics here
			for(u32 i = 0; i < type->funct.num_params; ++i) {

				if(i != 0)
					strcat(buffer, ", ");

				strcat(buffer, type_string(type->funct.parameters[i]));
			}
			strcat(buffer, ")");

			if(type->funct.ret)
				strcat(buffer, type_string(type->funct.ret));

		} break;
		case Type_Ptr: {
			sprintf(buffer, "*");
			strcat(buffer, type_string(type->ptr));
		} break;
		case Type_Array: {
			sprintf(buffer, "[%llu]", type->arrtype.size.integer_value);
			strcat(buffer, type_string(type->arrtype.elem));
		} break;
		case Type_Aggragate: {
			sprintf(buffer, "%s", type->agg.name->value);
			// add generics here
		}
		default:
			return NULL;
	}

	u32 size = strlen(buffer);
	char* str = (char*) malloc(size);
	strcpy(str, buffer);
	type->name = str;

	return type->name;
}

bool is_primative_type(Type* type) {
	switch(type->kind) {
		case Type_U8:
		case Type_U16:
		case Type_U32:
		case Type_U64:
		case Type_I8:
		case Type_I16:
		case Type_I32:
		case Type_I64:
		case Type_F32:
		case Type_F64:
		case Type_Bool:
		case Type_Char:
		case Type_Null:
			return true;
		default:
			return false;
	}
}

bool is_type_struct(Type* type) {
	if(type->kind == Type_Aggragate)
		return type->agg.item->kind == ItemStruct;
	return false;
}

bool is_type_enum(Type* type) {
	if(type->kind == Type_Aggragate)
		return type->agg.item->kind == ItemEnum;
	return false;
}

bool equivalent_types(Type* type1, Type* type2) {

	assert(type1 && type2);

	if(type1 == type2)
		return true;

	// if they are primatives then their kinds will be equal.
	if(is_primative_type(type1))
		return type1->kind == type2->kind;

	// if it gets here then it must be an compound type.

	if(type1->kind != type2->kind) return false;

	switch(type1->kind) {
		case Type_Function:
			break;
		case Type_Aggragate: {
			// no structural type equivalency
			// if the names are the same then they are the smae type.
			// but the members will be checked for completeness
			return type1->agg.name->value == type2->agg.name->value;
		} break;
		default:
			return false;
	}
}

i32 type_rank(Type* type) {
	switch(type->kind) {
		case Type_U8:
			return 1;
		case Type_I8:
			return 1;
		case Type_U16:
			return 1;
		case Type_I16:
			return 1;
		case Type_U32:
			return 2;
		case Type_I32:
			return 2;
		case Type_U64:
			return 2;
		case Type_I64:
			return 2;
		case Type_F32:
			return 3;
		case Type_F64:
			return 4;
		default:
			return -1;
	}
}