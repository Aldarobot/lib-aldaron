#ifndef JLL
#define JLL

#include "la.h"

static inline void la_deprecated(const char* a, const char* b) {
#if JL_DEBUG
	printf("%s is deprecated: use %s\n", a, b);
#endif
}

static inline int32_t jl_start(jl_fnct a, const char* b, uint64_t c) {
	la_deprecated("jl_start", "la_start");
	return la_start(a, la_dont, b, c);
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
	la_panic(jl, format, arglist);
	va_end(arglist);
}

#endif
