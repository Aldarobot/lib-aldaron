/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_BUFFER
#define LA_BUFFER

typedef data_t la_buffer_t;

void la_buffer_init(la_buffer_t* buffer);
void la_buffer_fdata(la_buffer_t* a, const void *data, uint32_t size);
uint8_t la_buffer_byte(la_buffer_t* buffer);
void la_buffer_resize(la_buffer_t* buffer);
void la_buffer_format(la_buffer_t* buffer, const char* format, ...);
void la_buffer_read(void* var, uint32_t varsize, la_buffer_t* buffer);
char* la_buffer_tostring(la_buffer_t* a);
void la_buffer_add(la_buffer_t* buffer, const void* var, uint32_t varsize);
void la_buffer_del(la_buffer_t* buffer);
void la_buffer_ins(la_buffer_t* buffer, uint8_t pvalue);

#define jl_data_byte la_buffer_byte

#endif
