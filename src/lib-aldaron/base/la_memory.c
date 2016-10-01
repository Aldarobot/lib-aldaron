/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_memory.h>
#include <la_safe.h>
#include <la.h>

#include <malloc.h>

typedef struct{
	char temp[256];
}la_memory_collector_t;

static la_memory_collector_t la_memory_collection[64];
static safe_uint8_t la_memory_collection_index;

uint64_t la_memory_used(void) {
	struct mallinfo mi;

	mi = mallinfo();
	return mi.uordblks;
}

void* la_memory_clear(void* data, uint64_t size) {
	memset(data, 0, size);
	return data;
}

void* la_memory_copy(const void* src, void* dst, uint64_t size) {
	memcpy(dst, src, size);
	return dst;
}

void* la_memory_stringcopy(const char* src, char* dst, uint64_t size) {
	dst[size] = '\0';
	return la_memory_copy(src, dst, size);
}

void* la_memory_makecopy(const void* data, uint64_t size) {
	void* dest = malloc(size);
	return la_memory_copy(data, dest, size);
}

void* la_memory_allocate(uint64_t size) {
	void* data = malloc(size);
	return la_memory_clear(data, size);
}

void* la_memory_resize(void* data, uint64_t size) {
	if(!size) la_panic("la_memory_resize: size is zero");
	return realloc(data, size);
}

void* la_memory_free(void* data) {
	free(data);
	return NULL;
}

void* la_memory_instant(void) {
	uint8_t index = la_safe_get_uint8(&la_memory_collection_index);

	la_safe_set_uint8(&la_memory_collection_index, index >= 63? 0: index+1);
	return la_memory_collection[index].temp;
}
