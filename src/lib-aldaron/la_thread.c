/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLthread.c
 *	This file handles a separate thread for drawing graphics.
**/
#include "JLprivate.h"
#include "la_safe.h"

#define JL_THREAD_MUTEX_UNLOCKED 255

//
// Static Functions
//

// Initialize a new thread.
static void jl_thread_init_new(jl_t* jl, uint8_t thread_id) {
	jl_print_init_thread__(jl, thread_id);
}

//
// Exported Functions
//

/**
 * Create a thread.  User can program up to 16 threads.
 * @param jl: The library context.
 * @param name: The name of the thread.
 * @param fn: The main function of the thread.
 * @returns: The thread ID number.
**/
uint8_t jl_thread_new(jl_t *jl, const char* name, SDL_ThreadFunction fn) {
	int8_t i, rtn = -1;

	jl_print_function(jl, "jl-thread-new");
	// Skip main thread ( i = 1 )
	for(i = 1; i < 16; i++) {
		jl_print_function(jl, "i=");
		// Look for not init'd thread.
		if(jl->jl_ctx[i].thread == NULL) {
			jl_print_function(jl, "thread-init-new");

			jl_thread_wait_init(jl, &jl->wait);
			// Run thread-specific initalizations
			jl_thread_init_new(jl, i);
			jl_print_return(jl, "thread-init-new");
			// Create a thread
			jl_print_function(jl, "create-a-thread");
			jl->jl_ctx[i].thread =	SDL_CreateThread(fn, name, jl);
			jl->jl_ctx[i].thread_id =
				SDL_GetThreadID(jl->jl_ctx[i].thread);
			jl_print_return(jl, "create-a-thread");
			// Check if success
			if(jl->jl_ctx[i].thread == NULL) {
				jl_print_function(jl, "fail");
				jl_print(jl, "SDL_CreateThread failed: %s",
					SDL_GetError());
				jl_print_return(jl, "fail");
				exit(-1);
			}
			rtn = i;
			jl_thread_wait_stop(jl, &jl->wait);
			jl_print_return(jl, "i=");
			break;
		}
		jl_print_return(jl, "i=");
	}
	jl_print_function(jl, "ohyeah");
	if(rtn == -1) {
		jl_print(jl, "Cannot have more than 16 threads!");
		exit(-1);
	}
	JL_PRINT_DEBUG(jl, "Made thread #%d", rtn);
	jl_print_return(jl, "ohyeah");
	jl_print_return(jl, "jl-thread-new");
	return rtn;
}

/**
 * Return the ID of the current thread.
 * @param jl: The library context.
 * @returns: The thread ID number, 0 if main thread.
**/
uint8_t jl_thread_current(jl_t *jl) {
	SDL_threadID current_thread = SDL_ThreadID();
	uint8_t i, rtn = 0;

	// Skip main thread ( i = 1 )
	for(i = 1; i < 16; i++) {
		// Look for not init'd thread.
		if(jl->jl_ctx[i].thread_id == current_thread) {
			rtn = i;
			break;
		}
	}
	return rtn;
}

/**
 * Wait for a thread to exit.
 * @param jl: The library context.
 * @param threadnum: The thread id returned from jl_thread_new().
 * @returns: Value returned from the thread.
**/
int32_t jl_thread_old(jl_t *jl, uint8_t threadnum) {
	int32_t threadReturnValue = 0;

	SDL_WaitThread(jl->jl_ctx[threadnum].thread, &threadReturnValue);
	return threadReturnValue;
}

/**
 * Create a mutex ( lock for a thread's access to data )
 * @param jl: The library context.
 * @param mutex: Output mutex.
**/
void jl_thread_mutex_new(jl_t *jl, jl_mutex_t* mutex) {
	mutex->jl = jl;
	mutex->mutex = SDL_CreateMutex();
#if JL_DEBUG
	mutex->thread_id = JL_THREAD_MUTEX_UNLOCKED;
#endif
}

/**
 * Lock a mutex.
 * @param jl: The library context.
 * @param mutex: The mutex created by jl_thread_mutex_new().
**/
void jl_thread_mutex_lock(jl_mutex_t* mutex) {
	jl_t* jl = mutex->jl;
#if JL_DEBUG
	uint8_t current_thread = jl_thread_current(jl);
#endif
	double timer = 0.f;
	double timepass = 0.f;

	jl_sdl_timer(jl, &timer);
	// Test if mutex is uninit'd
#if JL_DEBUG
	if(!mutex || !jl) {
		jl_exit(jl, "Mutex is uninit'd\n");
	}
#endif
	int error;
	while((error = SDL_TryLockMutex(mutex->mutex))) {
		timepass += jl_sdl_timer(jl, &timer);
		if(timepass > 2.f)
			jl_exit(jl, "jl_thread_mutex_lock timeout %s\n",
				SDL_GetError());
	}
#if JL_DEBUG
	if(mutex->thread_id == current_thread)
		jl_exit(jl, "jl_thread_mutex_lock redundant\n");
	mutex->thread_id = current_thread;
#endif
}

/**
 * Unlock a mutex.
 * @param jl: The library context.
 * @param mutex: The mutex created by jl_thread_mutex_new().
**/
void jl_thread_mutex_unlock(jl_mutex_t* mutex) {
#if JL_DEBUG
	jl_t* jl = mutex->jl;
	// Test if mutex is uninit'd
	if(!mutex || !jl) {
		jl_exit(jl, "Mutex is uninit'd\n");
	}
	if(mutex->thread_id == JL_THREAD_MUTEX_UNLOCKED) {
		jl_exit(jl, "jl_thread_mutex_unlock redundant\n");
	}
	mutex->thread_id = JL_THREAD_MUTEX_UNLOCKED;
	if(SDL_UnlockMutex(mutex->mutex))
		jl_exit(jl, "SDL_UnlockMutex failed: %s\n", SDL_GetError());
#else
	SDL_UnlockMutex(mutex->mutex);
#endif
}

/**
 * Do a thread-safe copy of data from "src" to "dst".  This function will wait
 *	until no other threads are using the mutex before doing anything.
 * @param jl: The library context.
 * @param mutex: The mutex protecting the variable, made by
 *	jl_thread_mutex_new().
 * @param src: The mutex-protected variable.
 * @param dst: The non-protected variable.
 * @param size: The size of the data pointed to by "src" and "dst" ( must be the
 *	same)
**/
void jl_thread_mutex_cpy(jl_t *jl, jl_mutex_t* mutex, void* src, void* dst,
	uint32_t size)
{
	// Test if mutex is uninit'd
#if JL_DEBUG
	if(!mutex || !mutex->jl) {
		jl_exit(jl, "Mutex is uninit'd\n");
	}
#endif
	// Lock mutex
	jl_thread_mutex_lock(mutex);
	// Copy data.
	jl_mem_copyto(src, dst, size);
	// Give up for other threads
	jl_thread_mutex_unlock(mutex);
}

/**
 * Create a thread-protected variable.
 * @param jl: The library context.
 * @param pvar: The protected variable to initialize.
 * @param size: Size to allocate for pvar's data
**/
void jl_thread_pvar_init(jl_t* jl, jl_pvar_t* pvar, void* data, uint64_t size) {
	pvar->jl = jl;
	jl_thread_mutex_new(jl, &pvar->lock);
	pvar->data = data ? jl_mem_copy(jl, data, size) : jl_memi(jl, size);
	pvar->size = size;
}

/**
 * Get a protected variable's data.
 * @param pvar: The protected variable to lock.
 * @param data: Data.  NULL to get data, pointer to your pointer to data to
 *	unlock data - will be set to NULL so you can't use anymore.
 * @returns: Pointer to safe data to edit.
**/
void* jl_thread_pvar_edit(jl_pvar_t* pvar) {
	jl_thread_mutex_lock(&pvar->lock);
	return pvar->data;
}

/**
 * Drop editing a protected variable so other threads can access.
 * @param pvar: The protected variable to stop using.
 * @param data: Pointer to a pointer to pvar's data.
**/
void jl_thread_pvar_drop(jl_pvar_t* pvar, void** data) {
        jl_thread_mutex_unlock(&pvar->lock);
	*data = NULL;
}

void jl_thread_pvar_free(jl_pvar_t* pvar) {
	pvar->data = jl_mem(pvar->jl, pvar->data, 0);
	pvar->size = 0;
}

void jl_thread_wait(jl_t* jl, jl_wait_t* wait) {
	while(SDL_AtomicGet(&wait->wait));
}

void jl_thread_wait_init(jl_t* jl, jl_wait_t* wait) {
	jl_print_function(jl, "jl-wait-init");
	SDL_AtomicSet(&wait->wait, 1);
	jl_print_return(jl, "jl-wait-init");
}

void jl_thread_wait_stop(jl_t* jl, jl_wait_t* wait) {
	SDL_AtomicSet(&wait->wait, 0);
}

/**
 * Set a thread safe variable
**/
void al_safe_set(void* var, void* set, size_t size) {
	SDL_AtomicLock(var);
	jl_mem_copyto(set, var + sizeof(SDL_SpinLock), size);
	SDL_AtomicUnlock(var);
}

void al_safe_get(void* var, void* set, size_t size) {
	SDL_AtomicLock(var);
	jl_mem_copyto(var + sizeof(SDL_SpinLock), set, size);
	SDL_AtomicUnlock(var);
}

void al_safe_set_float(safe_float_t* var, float value) {
	al_safe_set(var, &value, sizeof(float));
}

float al_safe_get_float(safe_float_t* var) {
	float value;
	al_safe_get(var, &value, sizeof(float));
	return value;
}

//
// Internal functions
//

void jl_thread_init__(jl_t* jl) {
	uint8_t i;

	// Set all threads to null
	for(i = 0; i < 16; i++) jl->jl_ctx[i].thread = NULL;
}
