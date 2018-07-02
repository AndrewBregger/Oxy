#include "common.h"
#include <math.h>


#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

Buffer init_buffer(u64 size) {
  Buffer buffer;

  buffer.size = size;
  buffer.buffer = (u8*) malloc(size * sizeof(u8));

  return buffer;
}

void destroy_buffer(Buffer* buffer) {
  free((void*) buffer->buffer);
  buffer->size = 0;
}

void set_buffer(Buffer* buffer, const u8* content, u64 size) {
  if(size < buffer->size)
    memcpy((void*) buffer->buffer, (void*) content, buffer->size);
  else {
    destroy_buffer(buffer);
    *buffer = init_buffer(size);
    memcpy((void*) buffer->buffer, (void*) content, buffer->size);
  }
}

u8 at_buffer(Buffer* buffer, u64 index) {
  assert(index < buffer->size);
  return buffer->buffer[index];
}

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = MAX(16, MAX(1 + 2*buf_cap(buf), new_len));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

char *buf__printf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

void buf_test(void) {
    int *buf = NULL;
    assert(buf_len(buf) == 0);
    int n = 1024;
    for (int i = 0; i < n; i++) {
        buf_push(buf, i);
    }
    assert(buf_len(buf) == n);
    for (size_t i = 0; i < buf_len(buf); i++) {
        assert(buf[i] == i);
    }
    buf_free(buf);
    assert(buf == NULL);
    assert(buf_len(buf) == 0);
    char *str = NULL;
    buf_printf(str, "One: %d\n", 1);
    assert(strcmp(str, "One: 1\n") == 0);
    buf_printf(str, "Hex: 0x%x\n", 0x12345678);
    assert(strcmp(str, "One: 1\nHex: 0x12345678\n") == 0);
}


void arena_grow(Arena *arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(ARENA_BLOCK_SIZE, min_size), ARENA_ALIGNMENT);
    arena->ptr = malloc(size);
    assert(arena->ptr == ALIGN_DOWN_PTR(arena->ptr, ARENA_ALIGNMENT));
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(Arena *arena, size_t size) {
    if (size > (size_t)(arena->end - arena->ptr)) {
        arena_grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }
    void *ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ALIGN_DOWN_PTR(ptr, ARENA_ALIGNMENT));
    return ptr;
}

void arena_free(Arena *arena) {
    for (char **it = arena->blocks; it != buf_end(arena->blocks); it++) {
        free(*it);
    }
    buf_free(arena->blocks);
}

// Hash map

uint64_t hash_uint64(uint64_t x) {
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

uint64_t hash_ptr(const void *ptr) {
    return hash_uint64((uintptr_t)ptr);
}

uint64_t hash_mix(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

uint64_t hash_bytes(const void *ptr, size_t len) {
    uint64_t x = 0xcbf29ce484222325;
    const char *buf = (const char *)ptr;
    for (size_t i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }
    return x;
}

void *map_get(Map *map, const void *key) {
    if (map->len == 0) {
        return NULL;
    }
    assert(IS_POW2(map->cap));
    size_t i = (size_t)hash_ptr(key);
    assert(map->len < map->cap);
    for (;;) {
        i &= map->cap - 1;
        if (map->keys[i] == key) {
            return map->vals[i];
        } else if (!map->keys[i]) {
            return NULL;
        }
        i++;
    }
    return NULL;
}


void map_grow(Map *map, size_t new_cap) {
    new_cap = MAX(16, new_cap);
    Map new_map = {
        .keys = calloc(new_cap, sizeof(void *)),
        .vals = malloc(new_cap * sizeof(void *)),
        .cap = new_cap,
    };
    for (size_t i = 0; i < map->cap; i++) {
        if (map->keys[i]) {
            map_put(&new_map, map->keys[i], map->vals[i]);
        }
    }
    free((void *)map->keys);
    free(map->vals);
    *map = new_map;
}

void map_put(Map *map, const void *key, void *val) {
    assert(key);
    assert(val);
    if (2*map->len >= map->cap) {
        map_grow(map, 2*map->cap);
    }
    assert(2*map->len < map->cap);
    assert(IS_POW2(map->cap));
    size_t i = (size_t)hash_ptr(key);
    for (;;) {
        i &= map->cap - 1;
        if (!map->keys[i]) {
            map->len++;
            map->keys[i] = key;
            map->vals[i] = val;
            return;
        } else if (map->keys[i] == key) {
            map->vals[i] = val;
            return;
        }
        i++;
    }
}

void map_test(void) {
    Map map = {0};
    enum { N = 1024 };
    for (size_t i = 1; i < N; i++) {
        map_put(&map, (void *)i, (void *)(i+1));
    }
    for (size_t i = 1; i < N; i++) {
        void *val = map_get(&map, (void *)i);
        assert(val == (void *)(i+1));
    }
}


Arena intern_arena;
Map interns;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    uint64_t hash = hash_bytes(start, len);
    void *key = (void *)(uintptr_t)(hash ? hash : 1);
    Intern *intern = map_get(&interns, key);
    for (Intern *it = intern; it; it = it->next) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    Intern *new_intern = arena_alloc(&intern_arena, offsetof(Intern, str) + len + 1);
    new_intern->len = len;
    new_intern->next = intern;
    memcpy(new_intern->str, start, len);
    new_intern->str[len] = 0;
    map_put(&interns, key, new_intern);
    return new_intern->str;
}

const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}

#define FNV_OFFSET 14695981039346656037
#define FNV_PRIME 1099511628211
u64 string_hash(const char* string, u64 len) {
    u64 hash = (u64) FNV_OFFSET;

    for(u64 i = 0; i < len; ++i) {
        hash *= (u64) FNV_PRIME;
        hash ^= string[i];
    }

    return hash;

}

void init_builtin(StringTable* table) {
    table_insert_string(table, "i8");
    table_insert_string(table, "i16");
    table_insert_string(table, "i32");
    table_insert_string(table, "i64");
    table_insert_string(table, "u8");
    table_insert_string(table, "u16");
    table_insert_string(table, "u32");
    table_insert_string(table, "u64");
    table_insert_string(table, "f32");
    table_insert_string(table, "f64");
    table_insert_string(table, "bool");
    table_insert_string(table, "char");
    table_insert_string(table, "byte");
    table_insert_string(table, "null");

    table_insert_string(table, "true");
    table_insert_string(table, "false");
}

StringTable create_table(u64 size) {
  StringTable table;
  table.strings = (char**) malloc(sizeof(char*) * size);

  // set every element to NULL
  memset(table.strings, 0, sizeof(char*) * size);

  table.cap = size;
  table.num = 0;

  init_builtin(&table);

  return table;
}

void table_rehash(StringTable* table) {

}

const char* table_insert_string(StringTable* table, const char* string) {
  u64 hash = string_hash(string, strlen(string));
  u64 index = hash % table->cap;
  if(table_contains(table, string)) {
    // printf("Using String in table: %s\n", table->strings[index]);
    return (const char*) table->strings[index];
  }
  else {
    u64 length = strlen(string);
    table->strings[index] = (char*) malloc(length + 1);
    strcpy(table->strings[index], string);
    // printf("Inserting new string in table: %s %p\n", table->strings[index], (void*) table->strings[index]);
    return (const char*) table->strings[index];
  }
}

// for now assume there isnt a collision
bool table_contains(StringTable* table, const char* string) {
  u64 hash = string_hash(string, strlen(string));
  u64 index = hash % table->cap;
  return table->strings[index] != NULL;
}
