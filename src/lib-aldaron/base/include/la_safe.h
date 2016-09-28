/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_SAFE
#define LA_SAFE

#include "SDL_thread.h"
#include <stdint.h>

typedef struct {
	SDL_SpinLock lock;
	uint8_t value;
} safe_uint8_t;

typedef struct {
	SDL_SpinLock lock;
	uint16_t value;
} safe_uint16_t;

typedef struct {
	SDL_SpinLock lock;
	uint32_t value;
} safe_uint32_t;

typedef struct {
	SDL_SpinLock lock;
	float value;
} safe_float_t;

typedef struct {
	SDL_SpinLock lock;
	char value[256];
} safe_string_t;

typedef struct {
	SDL_SpinLock lock;
	void* value;
} safe_pointer_t;

void la_safe_set(void* var, const void* set, size_t size);
void la_safe_get(void* var, void* set, size_t size);
void la_safe_set_float(safe_float_t* var, float value);
float la_safe_get_float(safe_float_t* var);
void la_safe_set_uint8(safe_uint8_t* var, uint8_t value);
uint8_t la_safe_get_uint8(safe_uint8_t* var);
void la_safe_set_uint16(safe_uint16_t* var, uint16_t value);
uint16_t la_safe_get_uint16(safe_uint16_t* var);
void la_safe_set_uint32(safe_uint32_t* var, uint32_t value);
uint32_t la_safe_get_uint32(safe_uint32_t* var);
void la_safe_set_string(safe_string_t* var, const char* value);
const char* la_safe_get_string(safe_string_t* var);
void la_safe_set_pointer(safe_pointer_t* var, const void* value);
const void* la_safe_get_pointer(safe_pointer_t* pointer);

#endif
