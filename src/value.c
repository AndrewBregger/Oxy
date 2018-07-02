#include "value.h"

#include <assert.h>

Value alloc_value(ValueKind kind) {
	Value val = default_value;
	val.kind = kind;
	return val;
}

Value new_bool_value(bool val) {
	Value v = alloc_value(Value_Bool);
	v.bool_value = val;
	return v;
}

Value new_string_value(const char* val) {
	Value v = alloc_value(Value_String);
	v.string_value = val;
	return v;
}

Value new_integer_value(i64 val) {
	Value v = alloc_value(Value_Integer);
	v.integer_value = val;
	return v;
}

Value new_float_value(f64 val) {
	Value v = alloc_value(Value_Float);
	v.float_value = val;
	return v;
}

Value new_char_value(char val) {
	Value v = alloc_value(Value_Char);
	v.char_value = val;
	return v;
}

Value new_pointer_value(u64 val) {
	Value v = alloc_value(Value_Pointer);
	v.pointer_value = val;
	return v;
}

Value new_array_value(Value* val) {
	Value v = alloc_value(Value_Array);
	v.array_value = val;
	return v;
}

Value new_map_value(MapValue* val) {
	Value v = alloc_value(Value_Map);
	v.map_value = val;
	return v;
}

MapValue new_map_element(Value key, Value value) {
	MapValue val;
	val.key = key;
	val.value = value;
	return val;
}

void check_size(Value val, ValueKind kind) {

}

Value value_integer_to_float(Value val) {
	assert(val.kind == Value_Integer);
	check_size(val, Value_Float);
	return new_float_value((f64) val.float_value);
}

Value value_float_to_integer(Value val) {
	assert(val.kind == Value_Float);
	check_size(val, Value_Integer);
	return new_integer_value((i64) val.float_value);
}

Value value_integer_to_char(Value val) {
	assert(val.kind == Value_Integer);
	check_size(val, Value_Char);
	return new_char_value((char) val.integer_value);
}

// is represented as an integer
bool value_is_integer_root(Value val) {
	return val.kind == Value_Integer || val.kind == Value_Char ||
				 val.kind == Value_Bool;
}

// #define CASE(field, op) \
// 	switch(rhs.kind) {
// 		Value_Bool:

// 		Value_Integer:
// 		Value_Char:

// 		Value_Float:

// 		Value_Pointer:

// 		Value_String:
// 		Value_Array:
// 		Value_Map:
// 		Value_Invalid:
// 	}

Value value_eval_binary(Token op, Value lhs, Value rhs) {
	if(!value_is_integral(lhs) and !value_is_integral(rhs))
		return default_value;

	switch(op.kind) {
		case Tkn_Plus: {
			// switch(lhs.kind) {
			// 	case Tkn_
			// }
		} break;
		case Tkn_Minus:
		case Tkn_Slash:
		case Tkn_Astrick:
		case Tkn_AstrickAstrick:
		case Tkn_LessLess:
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;
			return new_integer_value(lhs.integer_value >> rhs.integer_value);
		case Tkn_GreaterGreater:
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;
			return new_integer_value(lhs.integer_value << rhs.integer_value);
		case Tkn_Percent:
			// I would like to have support for floating point modulos
			// without using fmod.

			// for now, both operands must be integers.
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;

			return new_integer_value(lhs.integer_value % rhs.integer_value);
		case Tkn_Pipe:
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;

			return new_integer_value(lhs.integer_value | rhs.integer_value);
		case Tkn_Ampersand:
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;

			return new_integer_value(lhs.integer_value & rhs.integer_value);
		case Tkn_Carrot:
			if(!value_is_integer_root(lhs) or !value_is_integer_root(rhs))
				return default_value;

			return new_integer_value(lhs.integer_value ^ rhs.integer_value);
		default:
			return default_value;
	}
}

Value new_null_value() {
	return alloc_value(Value_Null);
}

Value value_eval_unary(Token op, Value lhs, Value rhs) {

}

bool value_is_integral(Value val) {
	return value_is_float(val) ||
				 value_is_integer(val) ||
				 val.kind == Value_Char ||
				 val.kind == Value_Bool;
}

bool value_is_float(Value val) {
	return val.kind == Value_Float;
}

bool value_is_integer(Value val) {
	return val.kind == Value_Integer;
}
