/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la_memory.h"
#include "la.h"
#include "la_safe.h"

#include <malloc.h>
#include <string.h>

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

/**
 * Format a string.
 * @param rtn: A variable to put the formated string.  It is assumed the size is
 *	80 bytes ( char rtn[80] )
 * @param format: The format string, can include %s, %f, %d, etc.
**/
void jl_mem_format(char* rtn, const char* format, ... ) {
	rtn[0] = '\0';
	if(format) {
		va_list arglist;

		va_start( arglist, format );
		vsnprintf( rtn, 80, format, arglist );
		va_end( arglist );
		//printf("done %s\n", (char*)rtn);
	}
}

/**
 * Format a string.
 * @param rtn: A variable to put the formated string.  The size must be the size
 *	of format plus an additional 128 bytes for expansion.
 * @param format: The string to format.
**/
void jl_mem_format2(char* rtn, const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	vsnprintf( rtn, strlen(format) + 128, format, arglist );
	va_end( arglist );
}

/**
 * Generate a random integer from 0 to "a"
 * @param a: 1 more than the maximum # to return
 * @returns: a random integer from 0 to "a"
*/
uint32_t jl_mem_random_int(uint32_t a) {
	return rand()%a;
}

/**
 * Add 2 Numbers, Keeping within a range of 0-1. ( Overflow )
**/
double jl_mem_addwrange(double v1, double v2) {
	double rtn = v1 + v2;
	while(rtn < 0.) rtn += 1.;
	while(rtn > 1.) rtn -= 1.;
	return rtn;
}

/**
 * Find the smallest difference within a range of 0-1. ( Overflow )
**/
double jl_mem_difwrange(double v1, double v2) {
	double rtn1 = fabs(jl_mem_addwrange(v1, -v2)); // Find 1 distance
	double rtn2 = 1. - rtn1; // Find complimentary distance
	if(rtn1 < rtn2)
		return rtn1;
	else
		return rtn2;
}

/**
 * Add a vector onto another vector.
 * @param v1: Vector to change.
 * @param v2: Vector to add.
**/
void jl_mem_vec_add(jl_vec3_t* v1, const jl_vec3_t* v2) {
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

/**
 * Subtract a vector from another vector.
 * @param v1: Vector to change.
 * @param v2: Vector to subtract.
**/
void jl_mem_vec_sub(jl_vec3_t* v1, const jl_vec3_t v2) {
	v1->x -= v2.x;
	v1->y -= v2.y;
	v1->z -= v2.z;
}

/**
 * Count the number of characters in as string until 'chr' is a character.
 * @param string: The string to check
 * @param chr: The character to look for.
**/
uint32_t jl_mem_string_upto(const char* string, char chr) {
	int i;

	for(i = 0; i < strlen(string); i++) {
		if(string[i] == chr) return i;
	}
	return strlen(string);
}

typedef struct{
	char temp[256];
}la_memory_collector_t;

static la_memory_collector_t la_memory_collection[64];
static safe_uint8_t la_memory_collection_index;

void* la_memory_instant(void) {
	uint8_t index = la_safe_get_uint8(&la_memory_collection_index);

	la_safe_set_uint8(&la_memory_collection_index, index >= 63? 0: index+1);
	return la_memory_collection[index].temp;
}

const char* la_memory_dtostr(float value) {
	void* instant = la_memory_instant();
	sprintf(instant, "%f", value);
	return instant;
}
