/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LIB_ALDARON_H
#define LIB_ALDARON_H

#include <stdint.h>
#include "clump.h" // LibClump

//Platform Declarations
#define JL_PLAT_COMPUTER 0 //PC/MAC
#define JL_PLAT_PHONE 1 //ANDROID/IPHONE
#define JL_PLAT_GAME 2 // 3DS
#if defined(__ANDROID__)
        #define JL_PLAT JL_PLAT_PHONE
	#define LA_PHONE
	#define LA_PHONE_ANDROID
#elif defined(__IPHONEOS__)
        #define JL_PLAT JL_PLAT_PHONE
	#define LA_PHONE
	#define LA_PHONE_APPLE
#else
        #define JL_PLAT JL_PLAT_COMPUTER
	#define LA_COMPUTER
#endif

//Determine Which OpenGL to use.

#define JL_GLTYPE_NO_SPRT 0 // No Support for OpenGL
// GLES version 2
#define JL_GLTYPE_SDL_GL2 1 // Include OpenGL with SDL
#define JL_GLTYPE_OPENGL2 2 // Include OpenGL with glut.
#define JL_GLTYPE_SDL_ES2 3 // Include OpenGLES 2 with SDL
#define JL_GLTYPE_OPENES2 4 // Include OpenGLES 2 standardly.
// Newer versions...

#define JL_GLTYPE JL_GLTYPE_NO_SPRT

// Platform Capabilities.
#if JL_PLAT == JL_PLAT_COMPUTER
	// All Linux Platforms
	#undef JL_GLTYPE
	#define JL_GLTYPE JL_GLTYPE_SDL_ES2
	// Windows
	// #define JL_GLTYPE JL_GLTYPE_SDL_GL2
#elif JL_PLAT == JL_PLAT_PHONE
	#undef JL_GLTYPE
	#define JL_GLTYPE JL_GLTYPE_SDL_ES2
#else
	#error "NO OpenGL support for this platform!"
#endif

//ERROR MESSAGES
typedef enum{
	JL_ERR_NERR, //NO ERROR
	JL_ERR_NONE, //Something requested is Non-existant
	JL_ERR_FIND, //Can not find the thing requested
	JL_ERR_NULL, //Something requested is empty/null
}jl_err_t;

typedef enum{
	JL_THREAD_PP_AA, // Push if acceptable
	JL_THREAD_PP_UA, // Push if acceptable, & make unacceptable until pull. 
	JL_THREAD_PP_FF, // Push forcefully.
	JL_THREAD_PP_UF, // Push forcefully, and make unacceptable until pull
}jl_thread_pp_t;

typedef void(*la_fn_t)(void* context);

void la_print(const char* format, ...);
void la_panic(const char* format, ...);
void la_dont(void* context);
const char* la_error(const char* format, ...);
int32_t la_start(void* fnc_init, la_fn_t fnc_loop, la_fn_t fnc_kill,
	const char* name, size_t ctx_size);

#endif
