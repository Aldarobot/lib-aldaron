/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_buffer.h>
#include <la_memory.h>
#include <la_string.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void la_buffer_init(la_buffer_t* buffer) {
	buffer->curs = 0;
	buffer->size = 8;
	buffer->data = la_memory_allocate(8);
}

void la_buffer_free(la_buffer_t* buffer) {
	free(buffer->data);
}

void la_buffer_fromdata(la_buffer_t* buffer, const void *data, uint32_t size) {
	buffer->curs = 0;
	buffer->size = size;
	buffer->data = la_memory_makecopy(data, size);
}

void la_buffer_fromstring(la_buffer_t* buffer, const char* string) {
	return la_buffer_fromdata(buffer, string, strlen(string));
}

void la_buffer_format(la_buffer_t* buffer, const char* format, ...) {
	va_list arglist;

	va_start(arglist, format);
	buffer->curs = vsnprintf(NULL, 0, format, arglist);
	va_end(arglist);
	la_buffer_resize(buffer);
	va_start(arglist, format);
	vsnprintf((void*)buffer->data, buffer->size, format, arglist);
	va_end(arglist);
}

uint8_t la_buffer_byte(la_buffer_t* buffer) {
	// Avoid memory errors.
	if(buffer->curs >= buffer->size)
		buffer->curs = buffer->size - 1;
	// Return byte at cursor.
	return buffer->data[buffer->curs];
}

void la_buffer_read(la_buffer_t* buffer, void* var, uint32_t varsize) {
	void* area = buffer->data + buffer->curs;

	la_memory_copy(area, var, varsize);
	buffer->curs += varsize;
}

void la_buffer_write(la_buffer_t* buffer, const void* var, uint32_t varsize) {
	void* dest = &(buffer->data[buffer->curs]);

	buffer->curs = buffer->curs + varsize;
	la_buffer_resize(buffer);
	la_memory_copy(var, dest, varsize);
}

void la_buffer_resize(la_buffer_t* buffer) {
	while(1) {
		if(buffer->curs < buffer->size) break;

		uint64_t oldsize = buffer->size;

		buffer->size *= 2;
		buffer->data = la_memory_resize(buffer->data, buffer->size);
		la_memory_clear(buffer->data + oldsize, oldsize);
	}
}

void la_buffer_del(la_buffer_t* buffer) {
	int i;

	for(i = buffer->curs; i < buffer->size - 1; i++)
		buffer->data[i] = buffer->data[i+1];
	buffer->data[strlen((void*)buffer->data)] = '\0';
	if(buffer->curs > buffer->size)
		buffer->curs = buffer->size - 1;
}

void la_buffer_ins(la_buffer_t* buffer, uint8_t pvalue) {
	la_buffer_resize(buffer);
	
	if(la_buffer_byte(buffer) == '\0') {
		// Append at end.
		buffer->data[buffer->curs] = pvalue;
		buffer->curs++;
		la_buffer_resize(buffer);
		buffer->data[buffer->curs] = '\0';
	}else{
		// Insert in the middle.
		size_t curs = buffer->curs;
		size_t string_len = strlen((void*)buffer->data);

		// Resize if not enough space
		buffer->curs = string_len + 1;
		la_buffer_resize(buffer);
		buffer->data[buffer->curs] = '\0';
		// Move data
		memmove(buffer->data + curs + 1, buffer->data + curs,
			string_len - curs);
		// Set byte
		buffer->data[curs] = pvalue;
		// Set cursor
		buffer->curs = curs + 1;
	}
}

char* la_buffer_tostring(la_buffer_t* buffer) {
	return (void*)(buffer->data);
}

uint8_t la_buffer_next(la_buffer_t* buffer, const char* particle) {
	return la_string_next((void*)buffer->data + buffer->curs, particle);
}
