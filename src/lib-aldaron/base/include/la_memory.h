/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <stdint.h>

uint64_t la_memory_used(void);
void* la_memory_clear(void* data, uint64_t size);
void* la_memory_copy(const void* src, void* dst, uint64_t size);
void* la_memory_stringcopy(const char* src, char* dst, uint64_t size);
void* la_memory_makecopy(const void* data, uint64_t size);
void* la_memory_allocate(uint64_t size);
void* la_memory_resize(void* data, uint64_t size);
void* la_memory_free(void* data);
void* la_memory_instant(void);
const char* la_memory_dtostr(float value);
