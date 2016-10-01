/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_time.h>

#include <time.h>

void la_time_sleep(double seconds) {
	struct timespec time;
	time.tv_sec = (int)seconds;
	time.tv_nsec = (int)((seconds - ((double)time.tv_sec)) * 1000000000.);
	nanosleep(&time, NULL);
}

double la_time(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	// Convert Nanoseconds to Seconds.
	return ((double)time.tv_sec) + (((double)time.tv_nsec) * 0.000000001); 
}

double la_time_pulse(void) {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	// Convert Nanoseconds to Seconds.
	return ((double)time.tv_nsec) * 0.000000001;
}

double la_time_timer(double* timer) {
	double prev_tick = *timer; // Temporarily Save Old Value

	*timer = la_time();
	return *timer - prev_tick; // Find difference In Seconds
}

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
