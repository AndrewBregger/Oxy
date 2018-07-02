#include "checker.h"
#include "scope.h"
#include "entity.h"
#include "type.h"
#include "oxy.h"
#include "report.h"
#include "value.h"

extern bool debug;

void debug_checker(const char* funct, u32 line) {
	if(debug) printf("%s|%u\n", funct, line);
}

Type* bool_t = NULL;
Type* i8_t = NULL;
Type* i16_t = NULL;
Type* i32_t = NULL;
Type* i64_t = NULL;
Type* u8_t = NULL;
Type* u16_t = NULL;
Type* u32_t = NULL;
Type* u64_t = NULL;
Type* null_t = NULL;
// Type* string_t = NULL;
Type* char_t = NULL;
Type* f32_t = NULL;
Type* f64_t = NULL;

void init_globals(Checker* checker) {

}

Checker new_checker(StringTable* table) {
	Checker checker = {0};
	checker.types = NULL;
	checker.files = NULL;
	checker.global_scope = NULL;
	checker.current_scope = NULL;
	checker.queue = new_queue();

	init_globals(&checker);
}