/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_math.h>

#include <math.h>
#include <time.h>

/**
 * Generate a random integer from 0 to "a"
 * @param a: 1 more than the maximum # to return
 * @returns: a random integer from 0 to "a"
*/
uint32_t la_math_random(uint32_t a) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	// Convert Nanoseconds to Seconds.
	return time.tv_nsec % a;
}

/**
 * Add 2 Numbers, Keeping within a range of 0-1. ( Overflow )
**/
double la_math_add01(double v1, double v2) {
	double rtn = v1 + v2;
	while(rtn < 0.) rtn += 1.;
	while(rtn > 1.) rtn -= 1.;
	return rtn;
}

/**
 * Find the smallest difference within a range of 0-1. ( Overflow )
**/
double la_math_dif01(double v1, double v2) {
	double rtn1 = fabs(la_math_add01(v1, -v2)); // Find 1 distance
	double rtn2 = 1. - rtn1; // Find complimentary distance

	return rtn1 < rtn2 ? rtn1 : rtn2;
}

/**
 * Add a vector onto another vector.
 * @param v1: Vector to change.
 * @param v2: Vector to add.
**/
void la_math_v3_add(la_v3_t* v1, la_v3_t v2) {
	v1->x += v2.x;
	v1->y += v2.y;
	v1->z += v2.z;
}

/**
 * Subtract a vector from another vector.
 * @param v1: Vector to change.
 * @param v2: Vector to subtract.
**/
void la_math_v3_sub(la_v3_t* v1, la_v3_t v2) {
	v1->x -= v2.x;
	v1->y -= v2.y;
	v1->z -= v2.z;
}
