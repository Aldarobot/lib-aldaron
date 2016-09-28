#ifndef JLL
#define JLL

#include "la.h"

void la_print(const char* format, ...);

static inline void la_deprecated(const char* a, const char* b) {
#if JL_DEBUG
	printf("%s is deprecated: use %s\n", a, b);
#endif
}

static inline int32_t jl_start(jl_fnct a, const char* b, uint64_t c) {
	la_deprecated("jl_start", "la_start");
	la_print("You may not use jl_start anymore.");
	return -1;
}

#define jl_dont la_dont

static inline void jl_exit(void* jl, const char* format, ...) {
	la_deprecated("jl_exit", "la_panic");

	va_list arglist;

	va_start(arglist, format);
	la_panic(format, arglist);
	va_end(arglist);
}

#define jl_thread_wait(jl, wait) la_signal_wait(wait)
#define jl_thread_wait_init(jl, wait) la_signal_init(wait)
#define jl_thread_wait_stop(jl, wait) la_signal_send(wait)

#define jl_memi(jl, size) la_memory_allocate(size)
#define jl_mem_tbiu() la_memory_used()
#define jl_mem_clr(mem, size) la_memory_clear(mem, size)
#define jl_mem_copyto(src, dest, size) la_memory_copy(src, dest, size)
#define jl_mem_copy(jl, src, size) la_memory_makecopy(src, size)

#define jl_time_get(jl) la_time()

#endif
