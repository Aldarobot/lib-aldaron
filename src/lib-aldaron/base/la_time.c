/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la.h"
#include <time.h>

/**
 * Sleep for specified number of seconds.
 * @param jl: The library context.
 * @param seconds: The amount of seconds to sleep.
**/
void la_time_sleep(double seconds) {
	struct timespec time;
	time.tv_sec = (int)seconds;
	time.tv_nsec = (int)((seconds - ((double)time.tv_sec)) * 1000000000.);
	nanosleep(&time, NULL);
}

/**
 * Get time in seconds since the start of the program.
 * @param jl: The library context.
**/
double la_time(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	// Convert Nanoseconds to Seconds.
	return ((double)time.tv_sec) + (((double)time.tv_nsec) * 0.000000001); 
}

/**
 * Time things up to a second.
 * @param jl: The library context.
 * @param timer: Pointer to timer variable.
 * @returns: Seconds passed since last call.
**/
double la_time_timer(double* timer) {
	double prev_tick = *timer; // Temporarily Save Old Value

	*timer = la_time();
	return *timer - prev_tick; // Find difference In Seconds
}

/**
 * 
**/
double la_time_regulatefps(double* timer, uint8_t* on_time) {
	double prev_tick = *timer; // Temporarily Save Old Value
	double this_tick = la_time();

	*on_time = this_tick < prev_tick + (1./60.);
	la_time_sleep((1./60.6) - (this_tick - prev_tick));
	*timer = la_time();
	return *timer - prev_tick; // Find difference In Seconds
}

// internal functions:

void la_time_init__(void) {
	// Reset time.
	struct timespec time;
	time.tv_sec = 0;
	time.tv_nsec = 0l;
	clock_settime(CLOCK_MONOTONIC, &time);
}
