/*
 * me: memory manager
 * 
 * A simple memory library.  Includes creating variables, setting and
 * getting variables, and doing simple and complicated math functions on
 * the variables.  Has a specialized string type.
*/

#include "jl_pr.h"
#include <malloc.h>

/**
 * Return Amount Of Total Memory Being Used
 * @returns The total amount of memory being used in bytes.
**/
u64_t jl_mem_tbiu(void) {
	struct mallinfo mi;

#if JL_PLAT != JL_PLAT_PHONE
	malloc_trim(0); //Remove Free Memory
#endif
	mi = mallinfo();
	return mi.uordblks;
}

void jl_mem_leak_init(jl_t* jl) {
	jvct_t * jl_ = jl->_jl;

	jl_->me.usedmem = jl_mem_tbiu();
}

/**
 * Exit if there's been a memory leak since the last call to jl_mem_leak_init().
**/
void jl_mem_leak_fail(jl_t* jl, str_t fn_name) {
	jvct_t * jl_ = jl->_jl;

	if(jl_mem_tbiu() != jl_->me.usedmem) {
		jl_print(jl, "%s: Memory Leak Fail", fn_name);
		jl_sg_kill(jl);
	}
}

/**
 * Allocate, Resize, or Free Dynamic Memory.  All memory allocated by this
 * function is uninitialized.
 * To allocate dynamic memory:	void* memory = fn(jl, NULL, size);
 * To resize dynamic memory:	memory = fn(jl, memory, new_size);
 * To free dynamic memory:	memory = fn(jl, memory, 0);
 * @param jl: The library context.
 * @param a: Pointer to the memory to resize/free, or NULL if allocating memory.
 * @param size: # of bytes to resize to/allocate, or 0 to free.
**/
void *jl_mem(jl_t* jl, void *a, u64_t size) {
	if(size == 0) { // Free
		if(a == NULL) {
			jl_print(jl, "Double Free or free on NULL pointer");
			exit(-1);
		}else{
			free(a);
		}
		return NULL;
	}else{ // Allocate or Resize
		if((a = realloc(a, size)) == NULL) {
			jl_print(jl, "realloc() failed! Out of memory?");
			exit(-1);
		}
	}
	return a;
}

/**
 * Allocate & Initialize Dynamic Memory.  All memory allocated by this function
 * is initialized as 0.
 * @param jl: The library context.
 * @param size: # of bytes to allocate.
**/
void *jl_memi(jl_t* jl, u64_t size) {
	// Make sure size is non-zero.
	if(!size) {
		if(jl) jl_print(jl, "jl_memi(): size must be more than 0");
		else JL_PRINT("jl_memi(): size must be more than 0");
		exit(-1);
	}
	// Allocate Memory.
	void* a = jl_mem(jl, NULL, size);

	// Clear the memory.
	jl_mem_clr(a, size);
	// Return the memory
	return a;
}

/**
 * Clear memory pointed to by "mem" of size "size"
 * @param pmem: memory to clear
 * @param size: size of "mem"
**/
void jl_mem_clr(void* mem, u64_t size) {
	memset(mem, 0, size);
}

/**
 * Copy memory from one place to another.
 * @param src: The place to copy memory from
 * @param dst: The place to copy memory to
 * @param size: The size of src & dst in bytes.
**/
void jl_mem_copyto(const void* src, void* dst, u64_t size) {
	memcpy(dst, src, size);
}

/**
 * Copy "size" bytes of "src" to a new pointer of "size" bytes and return it.
 * @param jl: The library context.
 * @param src: source buffer
 * @param size: # of bytes of "src" to copy to "dst"
 * @returns: a new pointer to 
*/
void *jl_mem_copy(jl_t* jl, const void *src, u64_t size) {
	void *dest = jl_memi(jl, size);
	jl_mem_copyto(src, dest, size);
	return dest;
}

/**
 * Format a string.
**/
str_t jl_mem_format(jl_t* jl, str_t format, ... ) {
	if(format) {
		va_list arglist;
		void* temp = jl->jl_ctx[jl_thread_current(jl)].temp;

		va_start( arglist, format );
		vsprintf( temp, format, arglist );
		va_end( arglist );
		return temp;
	}else{
		return NULL;
	}
}

/**
 * Generate a random integer from 0 to "a"
 * @param a: 1 more than the maximum # to return
 * @returns: a random integer from 0 to "a"
*/
u32_t jl_mem_random_int(u32_t a) {
	return rand()%a;
}

/**
 * Save up to 256 bytes to a buffer, return the previous value of the buffer.
 * @param jl: The library context.
 * @param mem: The new memory to save to the buffer.
 * @param size: Size of pointer.
 * @returns: The old/previous value of the pointer.
**/
void *jl_mem_temp(jl_t* jl, void *mem) {
	void* rtn = jl->jl_ctx[jl_thread_current(jl)].temp_ptr;

	jl->jl_ctx[jl_thread_current(jl)].temp_ptr = mem;
	return rtn;
}

jvct_t* jl_mem_init__(void) {
	//Create a context for the currently loaded program
	jvct_t* _jl = jl_memi(NULL, sizeof(jvct_t));

	//Set the current program ID to 0[RESERVED DEFAULT]
	_jl->cprg = 0;
	//Prepare user data structure
	_jl->jl = jl_memi(NULL, sizeof(jl_t));
	_jl->jl->_jl = _jl;
	_jl->jl->errf = JL_ERR_NERR; // No error
	//Make sure that non-initialized things aren't used
	_jl->has.graphics = 0;
	_jl->has.fileviewer = 0;
	_jl->has.filesys = 0;
	_jl->has.input = 0;
	_jl->fl.inloop = 1;
	_jl->me.status = JL_STATUS_GOOD;
	return _jl;
}

void jl_mem_kill__(jvct_t* _jl) {
	free(_jl);
//	cl_list_destroy(g_vmap_list);
}
