/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#if JL_GLTYPE == JL_GLTYPE_SDL_GL2  // SDL OpenGL 2
	#include "SDL_opengl.h"
	#include "SDL_opengl_glext.h"
#elif JL_GLTYPE == JL_GLTYPE_SDL_ES2 // SDL OpenGLES 2
	#include "SDL_opengles2.h"
#elif JL_GLTYPE == JL_GLTYPE_OPENES2 // OpenGLES 2
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#endif

//#ifdef GL_ES_VERSION_2_0
	#define GLSL_HEAD "#version 100\nprecision mediump float;\n"
//#else
//	#define GLSL_HEAD "#version 100\n"
//#endif

void jlgr_opengl_draw_arrays_(la_window_t* jlgr, GLenum mode, uint8_t count);
