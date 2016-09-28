/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LIB_ALDARON_H
#define LIB_ALDARON_H

#include <stdint.h>
#include "clump.h" // LibClump
#include "la_signal.h"

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

typedef struct{
	float x, y, z;
}jl_vec3_t;

typedef struct{
	float x, y, z, w;
}jl_vec4_t;

//4 bytes of information about the string are included
typedef struct{
	uint8_t* data; //Actual String
	size_t size; //Allocated Space In String
	size_t curs; //Cursor In String
}data_t;

typedef struct{
	/** Library Context **/
	void* jl;
	/** Mutex **/
	SDL_mutex* mutex;
#if JL_DEBUG
	/** Thread **/
	uint8_t thread_id;
#endif
}jl_mutex_t;

// Thread-Protected Variable
typedef struct{
	void* jl;
	jl_mutex_t lock;	/** The mutex lock on the "data" */
	void* data;		/** The data attached to the mutex */
	size_t size;		/** Size of "data" */
}jl_pvar_t;

typedef void(*jl_fnct)(void* context);
typedef void(*jl_data_fnct)(void* context, void* data);
typedef void(*jl_print_fnt)(void* context, const char * print);

void la_print(const char* format, ...);
void la_panic(const char* format, ...);
void la_dont(void* context);
const char* la_error(const char* format, ...);
int32_t la_start(void* fnc_init, jl_fnct fnc_loop, jl_fnct fnc_kill,
	uint8_t openwindow, const char* name, size_t ctx_size);

// "JLmem.c"
void jl_mem_format(char* rtn, const char* format, ... );
void jl_mem_format2(char* rtn, const char* format, ...);
uint32_t jl_mem_random_int(uint32_t a);
double jl_mem_addwrange(double v1, double v2);
double jl_mem_difwrange(double v1, double v2);
void jl_mem_vec_add(jl_vec3_t* v1, const jl_vec3_t* v2);
void jl_mem_vec_sub(jl_vec3_t* v1, const jl_vec3_t v2);
uint32_t jl_mem_string_upto(const char* string, char chr);

// "cl.c"
void jl_cl_list_alphabetize(struct cl_list *list);
void jl_clump_list_iterate(void* context, struct cl_list *list, jl_data_fnct fn);

#endif
