#include <stdint.h>
#include <stdio.h>
#include "la_media.h"

// Get Embeded Media
void *jl_gem(void) {
	return JLmedia;
}

uint32_t jl_gem_size(void) {
	return JLmedia_size;
}
