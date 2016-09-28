/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la.h"
#include "la_thread.h"
#include "la_memory.h"

#define JL_THREAD_MUTEX_UNLOCKED 255

const char* la_thread_new(la_thread_t* thread, la_thread_fn_t fn,
	const char* name, void* data)
{
	la_thread_t newthread;

	newthread.fn = fn;
	newthread.thread = SDL_CreateThread(fn, name, data);
	newthread.id = SDL_GetThreadID(newthread.thread);
	if(newthread.thread == NULL) {
		return SDL_GetError();
	}else{
		return NULL;
	}
	SDL_DetachThread(newthread.thread);
	// Set thread.
	if(thread) *thread = newthread;
}

/**
 * Wait for a thread to exit.
 * @returns: Value returned from the thread.
**/
int32_t la_thread_old(la_thread_t* thread) {
	int32_t threadReturnValue = 0;

	SDL_WaitThread(thread->thread, &threadReturnValue);
	return threadReturnValue;
}

uint64_t la_thread_current(void) {
	return SDL_ThreadID();
}
