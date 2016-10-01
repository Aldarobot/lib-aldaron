/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_TIME
#define LA_TIME

void la_time_sleep(double seconds);
double la_time(void);
double la_time_pulse(void);
double la_time_timer(double* timer);
double la_time_regulatefps(double* timer, uint8_t* on_time);

#endif
