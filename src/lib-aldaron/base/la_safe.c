/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */

#include "la_safe.h"
#include "la_memory.h"

void la_safe_set(void* var, const void* set, size_t size) {
	SDL_AtomicLock(var);
	la_memory_copy(set, var + sizeof(SDL_SpinLock), size);
	SDL_AtomicUnlock(var);
}

void la_safe_get(void* var, void* set, size_t size) {
	SDL_AtomicLock(var);
	la_memory_copy(var + sizeof(SDL_SpinLock), set, size);
	SDL_AtomicUnlock(var);
}

void la_safe_set_float(safe_float_t* var, float value) {
	la_safe_set(var, &value, sizeof(float));
}

float la_safe_get_float(safe_float_t* var) {
	float value;
	la_safe_get(var, &value, sizeof(float));
	return value;
}

void la_safe_set_uint8(safe_uint8_t* var, uint8_t value) {
	SDL_AtomicLock(&var->lock);
	var->value = value;
	SDL_AtomicUnlock(&var->lock);
}

uint8_t la_safe_get_uint8(safe_uint8_t* var) {
	uint8_t value;

	la_safe_get(var, &value, sizeof(uint8_t));
	return value;
}

void la_safe_set_uint16(safe_uint16_t* var, uint16_t value) {
	la_safe_set(var, &value, sizeof(uint16_t));
}

uint16_t la_safe_get_uint16(safe_uint16_t* var) {
	uint16_t value;
	la_safe_get(var, &value, sizeof(uint16_t));
	return value;
}

void la_safe_set_uint32(safe_uint32_t* var, uint32_t value) {
	la_safe_set(var, &value, sizeof(uint32_t));
}

uint32_t la_safe_get_uint32(safe_uint32_t* var) {
	uint32_t value;
	la_safe_get(var, &value, sizeof(uint32_t));
	return value;
}

void la_safe_set_string(safe_string_t* var, const char* value) {
	la_safe_set(var, value, 256);
}

const char* la_safe_get_string(safe_string_t* var) {
	void* value = la_memory_instant();
	la_safe_get(var, &value, 256);
	return value;
}
