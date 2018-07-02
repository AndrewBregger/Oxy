#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef struct Buffer {
  const u8* buffer;
  u64 size;
} Buffer;

// if buffer is null then initialize the buffer with size but initilize it to 0
Buffer init_buffer(u64 size);
void destroy_buffer(Buffer* buffer);

void set_buffer(Buffer* buffer, const u8* content, u64 size);

u8 at_buffer(Buffer* buffer, u64 index);

#define and &&
#define or ||


// https://github.com/pervognsen/bitwise/blob/master/ion/common.c
// the following are general data structure needed in a compiler.
// I do not use C in general so I am borrow it.

// Buffer, Map, Arean Allocation, and String thing (i do not know the point)

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size);
char *buf__printf(char *buf, const char *fmt, ...);
void buf_test(void);

typedef struct Arena {
    char *ptr;
    char *end;
    char **blocks;
} Arena;

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE (1024 * 1024)
// #define ARENA_BLOCK_SIZE 1024

void arena_grow(Arena *arena, size_t min_size);
void *arena_alloc(Arena *arena, size_t size);
void arena_free(Arena *arena);
// Hash map

uint64_t hash_uint64(uint64_t x);
uint64_t hash_ptr(const void *ptr);
uint64_t hash_mix(uint64_t x, uint64_t y);
uint64_t hash_bytes(const void *ptr, size_t len);

typedef struct Map {
    const void **keys;
    void **vals;
    size_t len;
    size_t cap;
} Map;

void *map_get(Map *map, const void *key);
void map_put(Map *map, const void *key, void *val);

void map_grow(Map *map, size_t new_cap);
void map_put(Map *map, const void *key, void *val);
void map_test(void);
// String interning

typedef struct Intern {
    size_t len;
    struct Intern *next;
    char str[];
} Intern;

Arena intern_arena;
Map interns;

const char *str_intern_range(const char *start, const char *end);
const char *str_intern(const char *str);

u64 string_hash(const char* string, u64 len);

#define TABLE_START 1024
// a table to store strings only once.
typedef struct StringTable {
  char** strings;
  u64 cap;
  u64 num;
} StringTable;


StringTable create_table(u64 size);
void table_rehash(StringTable* table);

// if the string is already in the table then it returns that pointer
// if it is not then, the string is inserted into the table and returned
//
// NOTE: Creates a new copy of the string.
const char* table_insert_string(StringTable* table, const char* string);

bool table_contains(StringTable* table, const char* string);

#endif
