/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_SIGNAL
#define LA_SIGNAL

#include <SDL_thread.h>

typedef struct{
	SDL_atomic_t wait;
}la_signal_t;

void la_signal_init(la_signal_t* wait);
void la_signal_wait(la_signal_t* wait);
void la_signal_send(la_signal_t* wait);

#endif
