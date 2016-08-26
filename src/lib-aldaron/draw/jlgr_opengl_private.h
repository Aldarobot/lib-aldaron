#if JL_GLTYPE == JL_GLTYPE_SDL_GL2  // SDL OpenGL 2
	#include "SDL_opengl.h"
	#include "SDL_opengl_glext.h"
#elif JL_GLTYPE == JL_GLTYPE_OPENGL2 // OpenGL 2
	#if JL_PLAT == JL_PLAT_COMPUTER
		#include "lib/glext.h"
	#else
		#error "JL_GLTYPE_OPENGL2 ain't supported by non-pc comps, man!"
	#endif
	#include "lib/glew/glew.h"
	#define JL_GLTYPE_HAS_GLEW
#elif JL_GLTYPE == JL_GLTYPE_SDL_ES2 // SDL OpenGLES 2
	#include "SDL_opengles2.h"
#elif JL_GLTYPE == JL_GLTYPE_OPENES2 // OpenGLES 2
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#endif

#ifdef GL_ES_VERSION_2_0
	#define GLSL_HEAD "#version 100\nprecision highp float;\n"
#else
	#define GLSL_HEAD "#version 100\n"
#endif

void jlgr_opengl_draw_arrays_(la_window_t* jlgr, GLenum mode, uint8_t count);
