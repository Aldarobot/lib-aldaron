/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <stdint.h>

const char* la_string_ffloat(float value);
const char* la_string_fint(int32_t value);
void la_string_append(char* string, const char* appender);
uint32_t la_string_upto(const char* string, char chr);
uint8_t la_string_next(const char* string, const char* match);
