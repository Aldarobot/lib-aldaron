/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

// TODO: Move entire file contents into la_opengl.c

#if LA_GLTYPE == LA_GLTYPE_SDL_GL2  // SDL OpenGL 2
	#include "SDL_opengl.h"
	#include "SDL_opengl_glext.h"
#elif LA_GLTYPE == LA_GLTYPE_SDL_ES2 // SDL OpenGLES 2
	#include "SDL_opengles2.h"
#elif LA_GLTYPE == LA_GLTYPE_OPENES2 // OpenGLES 2
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#endif

//#ifdef GL_ES_VERSION_2_0
	#define GLSL_HEAD "#version 100\nprecision mediump float;\n"
//#else
//	#define GLSL_HEAD "#version 100\n"
//#endif

#define DEFAULT_TC (const float[]) { \
	0., 1., \
	0., 0., \
	1., 0., \
	1., 1. \
}

#define UPSIDEDOWN_TC (const float[]) { \
	0., 0., \
	0., 1., \
	1., 1., \
	1., 0. \
}

#define BACKWARD_TC (const float[]) {\
	1., 1., \
	1., 0., \
	0., 0., \
	0., 1. \
}
