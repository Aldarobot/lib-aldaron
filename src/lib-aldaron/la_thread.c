/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */

#include "JLprivate.h"
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
	uint8_t current_thread = la_thread_current();
#endif
	double timer = 0.f;
	double timepass = 0.f;

	jl_sdl_timer(jl, &timer);
	// Test if mutex is uninit'd
#if JL_DEBUG
	if(!mutex) {
		la_panic("Mutex is NULL!");
	}
	if(!mutex->mutex || !jl) {
		la_panic("Mutex is uninit'd!");
	}
#endif
	int error;
	while((error = SDL_TryLockMutex(mutex->mutex))) {
		timepass += jl_sdl_timer(jl, &timer);
//		if(timepass > 2.f)
//			la_panic("jl_thread_mutex_lock timeout %s",
//				SDL_GetError());
	}
#if JL_DEBUG
	if(mutex->thread_id == current_thread)
		la_panic("jl_thread_mutex_lock redundant");
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
	if(!mutex) {
		la_panic("Mutex is NULL\n");
	}
	if(!mutex->mutex || !jl) {
		la_panic("Mutex is uninit'd!");
	}
	if(mutex->thread_id == JL_THREAD_MUTEX_UNLOCKED) {
		la_panic("jl_thread_mutex_unlock redundant\n");
	}
	mutex->thread_id = JL_THREAD_MUTEX_UNLOCKED;
	if(SDL_UnlockMutex(mutex->mutex))
		la_panic("SDL_UnlockMutex failed: %s\n", SDL_GetError());
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
	if(!mutex || !mutex->jl) la_panic("Mutex is uninit'd\n");
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
	pvar->data = data? jl_mem_copy(jl, data, size):la_memory_allocate(size);
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
