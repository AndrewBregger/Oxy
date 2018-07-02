#ifndef VALUE_H_
#define VALUE_H_

#include "common.h"
#include "token.h"

typedef struct MapValue MapValue;
// Value
// For now this is only going to hold basic literals
//

typedef enum ValueKind {
	Value_Invalid,
	Value_Bool,
	Value_String,
	Value_Integer,
	Value_Float,
	Value_Char,
	Value_Pointer,
	Value_Array,
	Value_Map,
	Value_Null
} ValueKind;

typedef struct Value {
	ValueKind kind;
	union {
		bool bool_value;
		const char* string_value;
	 	i64 integer_value;
	 	f64 float_value;
	 	char char_value;
	 	u64 pointer_value;
	 	struct Value* array_value;
	 	MapValue* map_value;
	};
} Value;

typedef struct MapValue {
	Value key;
	Value value;
} MapValue;

// the default value is an invalid value.
static Value default_value = {0};

Value alloc_value(ValueKind kind);

Value new_bool_value(bool val);
Value new_string_value(const char* val);
Value new_integer_value(i64 val);
Value new_float_value(f64 val);
Value new_char_value(char val);
Value new_pointer_value(u64 val);
Value new_array_value(Value* val);
Value new_map_value(MapValue* val);
Value new_null_value();


MapValue new_map_element(Value key, Value value);

Value value_integer_to_float(Value val);
Value value_float_to_integer(Value val);
Value value_integer_to_char(Value val);


bool value_is_integral(Value val);
bool value_is_float(Value val);
bool value_is_integer(Value val);

Value value_eval_binary(Token op, Value lhs, Value rhs);
Value value_eval_unary(Token op, Value lhs, Value rhs);

#endif