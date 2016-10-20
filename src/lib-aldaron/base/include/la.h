/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LIB_ALDARON_H
#define LIB_ALDARON_H

#include <stdint.h>
#include "clump.h" // LibClump

#if defined(__ANDROID__)
	#define LA_PHONE
	#define LA_ANDROID
#elif defined(__IPHONEOS__) || defined(TARGET_OS_IPHONE) \
 || defined(TARGET_IPHONE_SIMULATOR)
	#define LA_PHONE
	#define LA_PHONE_APPLE
#else
	#define LA_COMPUTER
	#if defined(__unix__)
		#if defined(__APPLE__)
			#define LA_APPLE
		#else
			#define LA_LINUX
		#endif
		#define LA_UNIX
	// Any definiton used for windows
	#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) \
	 || defined (__WIN32__) || defined(__CYGWIN__) ||  defined(__MINGW32__)\
	 || defined(__BORLANDC__)
		#define LA_WINDOWS
	#endif
#endif

//Determine Which Graphics Library to use.
#define LA_GLTYPE_SDL_GL2 1 // Use OpenGL with SDL
#define LA_GLTYPE_SDL_ES2 3 // Use OpenGLES 2 with SDL
#define LA_GLTYPE_OPENES2 4 // Use OpenGLES 2 standardly.
#define LA_GLTYPE_VULKAN 5 // Use Vulkan

// Platform Capabilities.
#if defined(LA_COMPUTER)
	#if defined(LA_UNIX)
		#define LA_GLTYPE LA_GLTYPE_SDL_ES2
	#else
		#define LA_GLTYPE LA_GLTYPE_SDL_GL2
	#endif
#elif defined(LA_PHONE)
	#define LA_GLTYPE LA_GLTYPE_SDL_ES2
#else
	#error "NO OpenGL support for this platform!"
#endif

typedef void(*la_fn_t)(void* context);

void la_print(const char* format, ...);
void la_panic(const char* format, ...);
void la_dont(void* context);
const char* la_error(const char* format, ...);
int32_t la_start(void* fnc_init, la_fn_t fnc_loop, la_fn_t fnc_kill,
	const char* name, size_t ctx_size);

#endif
