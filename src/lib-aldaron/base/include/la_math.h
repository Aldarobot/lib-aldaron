/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_MATH
#define LA_MATH

#include <stdint.h>

typedef struct{
	float x, y, z;
}la_v3_t;

typedef struct{
	float x, y, z, w;
}la_v4_t;

// Collision Box / Line / Etc.
typedef struct{
	la_v3_t pos; // Position ( X/Y/Z )
	la_v3_t ofs; // Position Offset ( W/H/D )
}jl_area_t;

uint32_t la_math_random(uint32_t a);
double la_math_add01(double v1, double v2);
double la_math_dif01(double v1, double v2);
void la_math_v3_add(la_v3_t* v1, la_v3_t v2);
void la_math_v3_sub(la_v3_t* v1, la_v3_t v2);

#endif
