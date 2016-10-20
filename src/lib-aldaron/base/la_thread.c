/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_thread.h>

#include <SDL_thread.h>

const char* la_thread_new(la_thread_t* thread_out, la_thread_fn_t fn,
	const char* name, void* data)
{
	la_thread_t newthread;

	newthread.fn = fn;
	newthread.thread = SDL_CreateThread(fn, name, data);
	newthread.id = SDL_GetThreadID(newthread.thread);
	// Set thread.
	if(thread_out != NULL)
		*thread_out = newthread;
	else
		SDL_DetachThread(newthread.thread);
	// Return error, if there is one.
	return (newthread.thread == NULL) ? SDL_GetError() : NULL;
}

int32_t la_thread_old(la_thread_t* thread) {
	int32_t threadReturnValue = 0;

	SDL_WaitThread(thread->thread, &threadReturnValue);
	return threadReturnValue;
}

uint64_t la_thread_current(void) {
	return SDL_ThreadID();
}
