/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_time.h>
#include <la.h>

#include <time.h>
#include <Windows.h>

#if defined(LA_WINDOWS)
//	struct timespec { long tv_sec; long tv_nsec; };   //header part
	#define exp7           10000000LL     //1E+7     //C-file part
	#define exp9         1000000000LL     //1E+9
	#define w2ux 116444736000000000LL     //1.jan1601 to 1.jan1970
	#define CLOCK_MONOTONIC 0
	void unix_time(struct timespec *spec)
	{  __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
	   wintime -=w2ux;  spec->tv_sec  =wintime / exp7;
	                    spec->tv_nsec =wintime % exp7 *100;
	}
	int clock_gettime(int unused, struct timespec *spec)
	{  static  struct timespec startspec; static double ticks2nano;
	   static __int64 startticks, tps =0;    __int64 tmp, curticks;
	   QueryPerformanceFrequency((LARGE_INTEGER*)&tmp); //some strange system can
	   if (tps !=tmp) { tps =tmp; //init ~~ONCE         //possibly change freq ?
	                    QueryPerformanceCounter((LARGE_INTEGER*)&startticks);
	                    unix_time(&startspec); ticks2nano =(double)exp9 / tps; }
	   QueryPerformanceCounter((LARGE_INTEGER*)&curticks); curticks -=startticks;
	   spec->tv_sec  =startspec.tv_sec   +         (curticks / tps);
	   spec->tv_nsec =startspec.tv_nsec  + (double)(curticks % tps) * ticks2nano;
	         if (!(spec->tv_nsec < exp9)) { spec->tv_sec++; spec->tv_nsec -=exp9; }
	   return 0;
	}
#endif

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
#if defined(LA_UNIX)
	// Reset time.
	struct timespec time;
	time.tv_sec = 0;
	time.tv_nsec = 0l;
	clock_settime(CLOCK_MONOTONIC, &time);
#endif
}
