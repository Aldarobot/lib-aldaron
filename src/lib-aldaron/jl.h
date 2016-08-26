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

static inline void* jl_get_context(jl_t* jl) {
	la_deprecated("jl_get_context", "la_context");
	return la_context(jl);
}

#define jl_dont la_dont

static inline void jl_exit(jl_t* jl, const char* format, ...) {
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

#endif
