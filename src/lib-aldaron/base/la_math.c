/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_math.h>
#include <la.h>

#include <math.h>
#include <time.h>

#if defined(LA_WINDOWS)
//	struct timespec { long tv_sec; long tv_nsec; };
	int clock_gettime(int, struct timespec *spec);
#endif

uint32_t la_math_random(uint32_t a) {
	struct timespec time;
#if defined(LA_WINDOWS)
	clock_gettime(0, &time);
#else
	clock_gettime(CLOCK_MONOTONIC, &time);
#endif
	// Convert Nanoseconds to Seconds.
	return time.tv_nsec % a;
}

double la_math_add01(double v1, double v2) {
	double rtn = v1 + v2;
	while(rtn < 0.) rtn += 1.;
	while(rtn > 1.) rtn -= 1.;
	return rtn;
}

double la_math_dif01(double v1, double v2) {
	double rtn1 = fabs(la_math_add01(v1, -v2)); // Find 1 distance
	double rtn2 = 1. - rtn1; // Find complimentary distance

	return rtn1 < rtn2 ? rtn1 : rtn2;
}

void la_math_v3_add(la_v3_t* v1, la_v3_t v2) {
	v1->x += v2.x;
	v1->y += v2.y;
	v1->z += v2.z;
}

void la_math_v3_sub(la_v3_t* v1, la_v3_t v2) {
	v1->x -= v2.x;
	v1->y -= v2.y;
	v1->z -= v2.z;
}
