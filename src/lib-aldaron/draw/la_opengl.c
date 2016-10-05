/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"
#include "la_thread.h"
#include "la_memory.h"

extern float la_banner_size;

const char *JL_SHADER_CLR_FRAG = 
	GLSL_HEAD
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vec4(vcolor.rgba);\n"
	"}";

const char *JL_SHADER_CLR_VERT = 
	GLSL_HEAD
	"uniform mat4 scale_object;\n"
	"uniform mat4 rotate_object;\n"
	"uniform mat4 translate_object;\n"
	"uniform mat4 rotate_camera;\n"
	"uniform mat4 project_scene;\n"
	"\n"
	"attribute vec4 position;\n"
	"attribute vec4 acolor;\n"
	"\n"
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	vcolor = acolor;\n"
	"	gl_Position = project_scene * rotate_camera *\n"
	"		translate_object * rotate_object * scale_object *\n"
	"		position;\n"
	"}";

const char *JL_SHADER_TEX_FRAG = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(texture, texcoord);\n"
	"}";

const char *JL_SHADER_TEX_VERT = 
	GLSL_HEAD
	"uniform mat4 scale_object;\n"
	"uniform mat4 rotate_object;\n"
	"uniform mat4 translate_object;\n"
	"uniform mat4 rotate_camera;\n"
	"uniform mat4 project_scene;\n"
	"\n"
	"attribute vec4 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	texcoord = texpos;\n"
	"	gl_Position = project_scene * rotate_camera *\n"
	"		translate_object * rotate_object * scale_object *\n"
	"		position;\n"
	"}";

#ifdef JL_DEBUG
void la_opengl_error__(int data, const char* fname) {
	GLenum err= glGetError();
	if(err == GL_NO_ERROR) return;
	char *fstrerr;
	if(err == GL_INVALID_ENUM) {
		fstrerr = "opengl: invalid enum";
	}else if(err == GL_INVALID_VALUE) {
		fstrerr = "opengl: invalid value!!\n";
	}else if(err == GL_INVALID_OPERATION) {
		fstrerr = "opengl: invalid operation!!\n";
	}else if(err == GL_OUT_OF_MEMORY) {
		fstrerr = "opengl: out of memory ): "
			"!! (Texture too big?)\n";
		GLint a;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &a);
		la_print("Max texture size: %d/%d\n", data, a);
	}else{
		fstrerr = "opengl: unknown error!\n";
	}
	la_panic("error: %s:%s (%d)", fname, fstrerr, data);
}
#endif

// Wrapper around opengl / es

static inline void la_opengl_enable(int32_t what) {
	glEnable(what);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glEnable()");
#endif
}

static inline void la_opengl_disable(int32_t what) {
	glDisable(what);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glDisable()");
#endif
}

static inline uint32_t la_opengl_genbuffer(void) {
	uint32_t buffer;
	glGenBuffers(1, &buffer);
#ifdef JL_DEBUG
	la_opengl_error__(buffer, "glGenBuffers");
	if(buffer == 0) {
		la_panic("buffer is made wrongly on thread #%X!",
			la_thread_current());
	}
#endif
	return buffer;
}

static inline void la_opengl_freebuffer(uint32_t buffer) {
	glDeleteBuffers(1, &buffer);
#ifdef JL_DEBUG
	la_opengl_error__(0, "buffer free");
#endif
}

static inline void la_opengl_bindbuffer(uint32_t buffer) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
#ifdef JL_DEBUG
	la_opengl_error__(buffer, "bind buffer");
#endif
}

static inline void la_opengl_bufferdata(const void* data, uint32_t size) {
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
#ifdef JL_DEBUG
	la_opengl_error__(size, "buffer data");
#endif
}

static inline void la_opengl_attachshader(uint32_t program, uint32_t shader) {
	glAttachShader(program, shader);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glAttachShader()");
#endif
}

static inline uint32_t la_opengl_makeprogram(void) {
	uint32_t program = glCreateProgram();
#ifdef JL_DEBUG
	la_opengl_error__(0,"glCreateProgram");
	if (!program) {
		la_panic("Failed to load program");
	}
#endif
	return program;
}

// Other ....

static void jl_gl_buffer_use__(uint32_t *buffer) {
	// Make buffer if not initialized.
	if(*buffer == 0) *buffer = la_opengl_genbuffer();
	// Bind the buffer
	la_opengl_bindbuffer(*buffer);
}

void la_llgraphics_buffer_set_(uint32_t *buffer, const void *buffer_data,
	uint16_t buffer_size)
{
	//Bind Buffer "buffer"
	jl_gl_buffer_use__(buffer);
	//Set the data
	la_opengl_bufferdata(buffer_data, buffer_size * sizeof(float));
}

void la_llgraphics_buffer_free(uint32_t buffer) {
	la_opengl_freebuffer(buffer);
}

static uint32_t jl_gl_load_shader(GLenum type, const char* src) {
	GLuint shader = glCreateShader(type);
#ifdef JL_DEBUG
	GLint compiled;

	la_opengl_error__(0,"couldn't create shader");
	if (shader) {
#endif
		glShaderSource(shader, 1, &src, NULL);
#ifdef JL_DEBUG
		la_opengl_error__(0, "glShaderSource");
#endif
		glCompileShader(shader);
#ifdef JL_DEBUG
		la_opengl_error__(0, "glCompileShader");
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		la_opengl_error__(0, "glGetShaderiv");

		if (!compiled) {
			GLint infoLen = 0;
			char* buf;

			la_print("Failed Compile %dx%d:", shader, compiled);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			la_opengl_error__(1,"glGetShaderiv");

			if (infoLen) {
				buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen,
						NULL, buf);
					const char* msg =
						(type == GL_VERTEX_SHADER) ?
							"vertex shader":
							"fragment shader";
					la_panic("Could not compile %s:%s", msg,
						buf);
				}
				glDeleteShader(shader);
				shader = 0;
				la_panic("Failed to make shader: out of memory");
			}else{
				la_panic("Failed to make shader: no details.");
			}
		}
	}else{
		la_panic("Couldn't load shader!");
	}
#endif
	return shader;
}

static inline uint32_t la_opengl_shader_new(const char* vert, const char* frag){
#ifdef JL_DEBUG
	GLint linkStatus;
#endif
	uint32_t vertexShader = jl_gl_load_shader(GL_VERTEX_SHADER, vert);
	uint32_t fragmentShader = jl_gl_load_shader(GL_FRAGMENT_SHADER, frag);
	uint32_t program = la_opengl_makeprogram();

	la_print("Linking....");
	la_opengl_attachshader(program, vertexShader);
	la_opengl_attachshader(program, fragmentShader);
	glLinkProgram(program);
#ifdef JL_DEBUG
	la_opengl_error__(0,"glLinkProgram");
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	la_opengl_error__(0,"glGetProgramiv");
	glValidateProgram(program);
	la_opengl_error__(1,"glValidateProgram");
	if (linkStatus != GL_TRUE) {
		GLint bufLength = 0;
		char* buf;

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
		la_opengl_error__(1,"glGetProgramiv");
		if (bufLength) {
			buf = (char*) malloc(bufLength);
			if (buf) {
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				la_panic("Could not link program: %s", buf);
			}else{
				la_panic("failed malloc");
			}
		}else{
			glDeleteProgram(program);
			la_panic("no info log");
		}
	}
#endif
	la_print("Made program!");
	return program;
}

static void jl_gl_texture_make__(la_window_t* jlgr, uint32_t *tex) {
	glGenTextures(1, tex);
#ifdef JL_DEBUG
	if(!(*tex)) {
		la_opengl_error__(0, "jl_gl_texture_make__: glGenTextures");
		la_panic("jl_gl_texture_make__: GL tex = 0");
	}
	la_opengl_error__(0, "jl_gl_texture_make__: glGenTextures");
#endif
}

// Set the bound texture.  px is the pixels 0 - blank texture.
static void jl_gl_texture_set__(la_window_t* jlgr, uint8_t* px, uint16_t w, uint16_t h,
	uint8_t bytepp)
{
	GLenum format = (bytepp == 3) ? GL_RGB : GL_RGBA;
	glTexImage2D(
		GL_TEXTURE_2D, 0,		/* target, level */
		format,				/* internal format */
		w, h, 0,			/* width, height, border */
		format, GL_UNSIGNED_BYTE,	/* external format, type */
		px				/* pixels */
	);
#ifdef JL_DEBUG
	la_opengl_error__(w, "texture image 2D");
#endif
}

static void jl_gl_texpar_set__(la_window_t* jlgr) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef JL_DEBUG
	la_opengl_error__(0,"glTexParameteri");
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef JL_DEBUG
	la_opengl_error__(1,"glTexParameteri");
#endif
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#ifdef JL_DEBUG
	la_opengl_error__(2,"glTexParameteri");
#endif
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef JL_DEBUG
	la_opengl_error__(3,"glTexParameteri");
#endif
}

static inline void jl_gl_texture__bind__(la_window_t* jlgr, uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
#ifdef JL_DEBUG
	la_opengl_error__(tex, "jlgr_opengl_texture_bind_: glBindTexture");
#endif
}

// Bind a texture.
void jlgr_opengl_texture_bind_(la_window_t* jlgr, uint32_t tex) {
#ifdef JL_DEBUG
	if(tex == 0) la_panic("jlgr_opengl_texture_bind_: GL tex = 0");
#endif
	jl_gl_texture__bind__(jlgr, tex);
}

// Unbind a texture
void jlgr_opengl_texture_off_(la_window_t* jlgr) {
	jl_gl_texture__bind__(jlgr, 0);
}

// Make & Bind a new texture.
void jlgr_opengl_texture_new_(la_window_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp)
{
	// Make the texture
	jl_gl_texture_make__(jlgr, tex);
	// Bind the texture
	jlgr_opengl_texture_bind_(jlgr, *tex);
	// Set texture
	jl_gl_texture_set__(jlgr, px, w, h, bytepp);
	// Set the texture parametrs.
	jl_gl_texpar_set__(jlgr);
}

void jl_gl_texture_free_(la_window_t* jlgr, uint32_t *tex) {
	glDeleteTextures(1, tex);
	*tex = 0;
#ifdef JL_DEBUG
	la_opengl_error__(0, "glDeleteTextures");
#endif
}

// Make a texture - doesn't free "pixels"
uint32_t jl_gl_maketexture(la_window_t* jlgr, void* pixels,
	uint32_t width, uint32_t height, uint8_t bytepp)
{
	uint32_t texture;

	if (!pixels) la_panic("null pixels");
	la_print("generating texture (%d,%d)", width, height);
	// Make the texture.
	jlgr_opengl_texture_new_(jlgr, &texture, pixels, width, height, bytepp);
	return texture;
}

static inline int32_t _jl_gl_getu(GLuint prg, const char *var) {
	int32_t a = glGetUniformLocation(prg, var);
#ifdef JL_DEBUG
	if(a == -1) la_panic("opengl: bad name; is: %s", var);
	la_opengl_error__(a, "glGetUniformLocation");
#endif
	return a;
}

static inline int32_t _jl_gl_geta(GLuint prg, const char *title) {
	int32_t a = glGetAttribLocation(prg, title);
#ifdef JL_DEBUG
	if(a == -1) {
		la_print("for name \"%s\":", title);
		la_panic("attribute name is either reserved or non-existant");
	}
#endif
	return a;
}

//Lower Level Stuff
static inline void jl_gl_usep__(la_window_t* jlgr, GLuint prg) {
#ifdef JL_DEBUG
	if(!prg) la_panic("shader program uninit'd!");
#endif
	glUseProgram(prg);
#ifdef JL_DEBUG
	la_opengl_error__(prg, "glUseProgram");
#endif
}

/**
 * Modify a uniform variable
 * @param jlgr: The library context.
 * @param glsl: The shader the uniform variable belongs to.
 * @param x: The value to push.
 * @param vec: 1-4 for float-vec2-vec3-vec4
 * @param name: Format variable for uniform name.
**/
void jlgr_opengl_uniform(la_window_t* jlgr, jlgr_glsl_t* glsl, float* x, uint8_t vec,
	const char* name, ...)
{
#ifdef JL_DEBUG
	if(glsl == NULL) la_panic("jlgr_opengl_uniform: NULL SHADER");
#endif
	int32_t uv;
	char uniform_name[256];

	// Store the format in uniform_name.
	va_list arglist;
	va_start( arglist, name );
	vsprintf( uniform_name, name, arglist );
	va_end( arglist );
	// Bind Shader
	jlgr_opengl_draw1(jlgr, glsl);
	// Get uniform
	uv = _jl_gl_getu(glsl->program, uniform_name);

	switch(vec) {
		case 1:
			glUniform1f(uv, x[0]);
			break;
		case 2:
			glUniform2f(uv, x[0], x[1]);
			break;
		case 3:
			glUniform3f(uv, x[0], x[1], x[2]);
			break;
		case 4:
			glUniform4f(uv, x[0], x[1], x[2], x[3]);
			break;
		default:
			la_print("vec must be 1-4");
			break;
	}
#ifdef JL_DEBUG
	la_opengl_error__(uv, "glUniform..f");
#endif
}

/**
 * Modify a uniform variable
 * @param jlgr: The library context.
 * @param glsl: The shader the uniform variable belongs to.
 * @param x: The value to push.
 * @param vec: 1-4 for float-vec2-vec3-vec4
 * @param name: Format variable for uniform name.
**/
void jlgr_opengl_uniformi(la_window_t* jlgr, jlgr_glsl_t* glsl, int32_t* x,
	uint8_t vec, const char* name, ...)
{
	int32_t uv;
	char uniform_name[256];

	// Store the format in uniform_name.
	va_list arglist;
	va_start( arglist, name );
	vsprintf( uniform_name, name, arglist );
	va_end( arglist );
	// Bind Shader
	jlgr_opengl_draw1(jlgr, glsl);
	// Get uniform
	uv = _jl_gl_getu(glsl->program, uniform_name);

	switch(vec) {
		case 1:
			glUniform1i(uv, x[0]);
			break;
		case 2:
			glUniform2i(uv, x[0], x[1]);
			break;
		case 3:
			glUniform3i(uv, x[0], x[1], x[2]);
			break;
		case 4:
			glUniform4i(uv, x[0], x[1], x[2], x[3]);
			break;
		default:
			la_print("vec must be 1-4");
			break;
	}
#ifdef JL_DEBUG
	la_opengl_error__(uv, "glUniform..i");
#endif
}

//This pushes VBO "buff" up to the shader's vertex attribute "vertexAttrib"
//Set xyzw to 2 if 2D coordinates 3 if 3D. etc.
void jlgr_opengl_setv(la_window_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw)
{
	// Bind Buffer
	jl_gl_buffer_use__(buff);
	// Set Vertex Attrib Pointer
	glEnableVertexAttribArray(vertexAttrib);
#ifdef JL_DEBUG
	la_opengl_error__(vertexAttrib, "glEnableVertexAttribArray");
#endif
	glVertexAttribPointer(
		vertexAttrib,	// attribute
		xyzw,		// x+y+z = 3
		GL_FLOAT,	// type
		GL_FALSE,	// normalized?
		0,		// stride
		0		// array buffer offset
	);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glVertexAttribPointer");
#endif
}

void jlgr_opengl_draw_arrays_(la_window_t* jlgr, GLenum mode, uint8_t count) {
	glDrawArrays(mode, 0, count);
#ifdef JL_DEBUG
	la_opengl_error__(0,"glDrawArrays");
#endif
}

void jlgr_opengl_blend_default_(la_window_t* jlgr) {
	la_opengl_enable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
		GL_DST_ALPHA);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glBlendFuncSeparate");
#endif
}

void jlgr_opengl_blend_add_(la_window_t* jlgr) {
	la_opengl_enable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
#ifdef JL_DEBUG
	la_opengl_error__(0, "glBlendFunc");
#endif
}

void jlgr_opengl_blend_none_(la_window_t* jlgr) {
	la_opengl_disable(GL_BLEND);
}

// Copy & Push vertices to a VBO.
void jlgr_opengl_vertices_(la_window_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl)
{
	uint16_t items = (vertices*3);

	// Copy Vertices
	la_memory_copy(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	la_llgraphics_buffer_set_(gl, cv, items);
}

// TODO: MOVE
/**
 * Create a new texture object.
 * @param jlgr: The library context.
**/
uint32_t la_texture_new(la_window_t* jlgr, uint8_t* pixels, uint16_t w, uint16_t h,
	uint8_t bpp)
{
	uint32_t texture;

	jl_gl_texture_make__(jlgr, &texture);
	jl_gl_texture__bind__(jlgr, texture);
	jl_gl_texpar_set__(jlgr);
	jl_gl_texture_set__(jlgr, pixels, w, h, bpp);
	return texture;
}

// TODO: MOVE
/**
 * Set a texture object.
 * @param jlgr: The library context.
 * @param texture: The texture to modify.
 * @param pixels: The pixels to set the texture to.
 * @param w, h: The dimensions
 * @param bpp: The bytes per pixel.  Must be 3 or 4.
**/
void la_texture_set(la_window_t* jlgr, uint32_t texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp)
{
	GLenum format = GL_RGBA;

	if(bpp == 3) format = GL_RGB;

	// Bind Texture
	jl_gl_texture__bind__(jlgr, texture);
	// Copy to texture.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE,
		pixels);
#ifdef JL_DEBUG
	la_opengl_error__(0, "jl_gl_pbo_set__: glTexSubImage2D");
#endif
}

void jlgr_opengl_viewport_(la_window_t* jlgr, uint16_t w, uint16_t h) {
	glViewport(0, 0, w, h);
#ifdef JL_DEBUG
	la_opengl_error__(w * h, "glViewport");
#endif
}

void jl_opengl_framebuffer_make_(la_window_t* jlgr, uint32_t *fb) {
	glGenFramebuffers(1, fb);
#ifdef JL_DEBUG
	if(!(*fb)) la_panic("jl_gl_framebuffer_make__: GL FB = 0");
	la_opengl_error__(*fb,"glGenFramebuffers");
#endif
}

void jlgr_opengl_framebuffer_bind_(la_window_t* jlgr, uint32_t fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
#ifdef JL_DEBUG
	la_opengl_error__(fb, "glBindFramebuffer");
#endif
}

void jlgr_opengl_framebuffer_addtx_(la_window_t* jlgr, uint32_t tx) {
	// Set "*tex" as color attachment #0.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tx, 0);
#ifdef JL_DEBUG
	la_opengl_error__(tx,
		"jlgr_opengl_framebuffer_addtx_: glFramebufferTexture2D");
#endif
}

void jlgr_opengl_framebuffer_subtx_(la_window_t* jlgr) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, 0, 0);
#ifdef JL_DEBUG
	la_opengl_error__(0,
		"jlgr_opengl_framebuffer_subtx_: glFramebufferTexture2D");
#endif
}

void jlgr_opengl_framebuffer_status_(la_window_t* jlgr) {
	// Check to see if framebuffer was made properly.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		la_panic("Frame buffer not complete!");
}

void jl_opengl_framebuffer_free_(la_window_t* jlgr, uint32_t *fb) {
	glDeleteFramebuffers(1, fb);
#ifdef JL_DEBUG
	la_opengl_error__(*fb, "glDeleteFramebuffers");
#endif
	*fb = 0;
}

// Set the viewport to the screen size.
void jl_gl_viewport_screen(la_window_t* jlgr) {
	jlgr_opengl_viewport_(jlgr, jlgr->wm.w, jlgr->wm.h);
}

/**
 * Set an objects scale, rotation, and translation for a shader.
 * Set Camera's Rotation
 * @param jlgr: The library context.
 * @param 
 * @param ar: Aspect ratio
 * @param near: The near clipping pane.
 * @param far: The far clipping pane.
**/
void jlgr_opengl_matrix(la_window_t* jlgr, jlgr_glsl_t* sh, la_v3_t scalev,
	la_v3_t rotatev, la_v3_t translatev, la_v3_t lookv, float ar)
{
	float scale[] = {
		2. * scalev.x, 0.f, 0.f, 0.f,
		0.f, 2. * scalev.y / ar, 0.f, 0.f,
		0.f, 0.f, 2. * scalev.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	float rotate_object[] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	float translate[] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f, 
		(translatev.x * 2.f) - 1.f, (translatev.y * 2.f / ar) - 1.f,
			(translatev.z * 2.f), 1.f
	};
	float rotate_scene[] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	// Projection Matrix
	float angleOfView = 90.f;
	float do_scale = 1.f / tan(angleOfView * 0.5f * M_PI / 180.f);

	float projection[] = {
		do_scale, 0.f, 0.f, 0.f,
		0.f, do_scale, 0.f, 0.f,
		0.f, 0.f, 1.f, -1.f,
		0.f, 0.f, -1.f, 1.f
	};
	glUniformMatrix4fv(sh->uniforms.scale_object, 1, 0, scale);
#ifdef JL_DEBUG
	la_opengl_error__(0, "matrix_object - scale");
#endif
	glUniformMatrix4fv(sh->uniforms.rotate_object, 1, 0, rotate_object);
#ifdef JL_DEBUG
	la_opengl_error__(0, "matrix_object - rotate");
#endif
	glUniformMatrix4fv(sh->uniforms.translate_object, 1, 0, translate);
#ifdef JL_DEBUG
	la_opengl_error__(0, "matrix_object - translate");
#endif
	glUniformMatrix4fv(sh->uniforms.rotate_camera, 1, 0, rotate_scene);
#ifdef JL_DEBUG
	la_opengl_error__(0, "matrix_object - rotate world");
#endif
	glUniformMatrix4fv(sh->uniforms.project_scene, 1, 0, projection);
#ifdef JL_DEBUG
	la_opengl_error__(0, "matrix_object - projection");
#endif
}

/**
 * Bind shader ( Prepare to draw ).
 * @param jlgr: The library context.
 * @param sh: The shader to use.
**/
void jlgr_opengl_draw1(la_window_t* jlgr, jlgr_glsl_t* sh) {
	jl_gl_usep__(jlgr, sh->program);
}

/**
 * Create a GLSL shader program.
 * @param jlgr: The library context.
 * @param glsl: The jlgr_glsl_t object to initialize.
 * @param vert: The vertex shader - NULL for texture support.
 * @param frag: The fragment shader.
 * @param has_tex: Whether has texture or not.
**/
void jlgr_opengl_shader_init(la_window_t* jlgr, jlgr_glsl_t* glsl, const char* vert,
	const char* frag, uint8_t has_tex)
{
	const char* vertShader = vert ? vert : JL_SHADER_TEX_VERT;
	// Program
	glsl->program = la_opengl_shader_new(vertShader, frag);
	// Matrices
	glsl->uniforms.scale_object=_jl_gl_getu(glsl->program,"scale_object");
	glsl->uniforms.rotate_object=_jl_gl_getu(glsl->program,"rotate_object");
	glsl->uniforms.translate_object=
		_jl_gl_getu(glsl->program,"translate_object");
	glsl->uniforms.rotate_camera=_jl_gl_getu(glsl->program,"rotate_camera");
	glsl->uniforms.project_scene=_jl_gl_getu(glsl->program,"project_scene");
	// Position Attribute
	glsl->attributes.position = _jl_gl_geta(glsl->program, "position");
	// If custom vertex shader, don't assume texture is defined
	if(has_tex) {
		// Texture
		glsl->uniforms.texture = _jl_gl_getu(glsl->program, "texture");
		// Texture Position Attribute
		glsl->attributes.texpos_color = _jl_gl_geta(glsl->program,
			"texpos");
	}else{
		// Color Attribute
		glsl->attributes.texpos_color = _jl_gl_geta(glsl->program,
			"acolor");
	}
}

/**
 * Get the Aspect Ratio of the pre-renderer in use.
 * @param jlgr: The library context.
**/
float jl_gl_ar(la_window_t* jlgr) {
	return jlgr->gl.cp ? jlgr->gl.cp->ar : jlgr->wm.ar;
}

float la_window_h(la_window_t* window) {
	float ar = jl_gl_ar(window);
	return (1. - la_banner_size) * ar;
}

/**
 * Get the Width for the pre-renderer in use.
 * @param jlgr: The library context.
**/
uint32_t jl_gl_w(la_window_t* jlgr) {
	uint32_t w;

	w = jlgr->gl.cp ? jlgr->gl.cp->w : jlgr->wm.w;
	return w;
}

/**
 * Clear the screen with a color
 * @param jlgr: The library context.
 * @param r: The amount of red [ 0.f - 1.f ]
 * @param g: The amount of green [ 0.f - 1.f ]
 * @param b: The amount of blue [ 0.f - 1.f ]
 * @param a: The translucency [ 0.f - 1.f ]
**/
void jl_gl_clear(la_window_t* jlgr, float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
#ifdef JL_DEBUG
	la_opengl_error__(a, "jl_gl_clear(): glClearColor");
#endif
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#ifdef JL_DEBUG
	la_opengl_error__(a, "jl_gl_clear(): glClear");
#endif
}

void jl_gl_init__(la_window_t* jlgr) {
	jlgr->gl.cp = NULL;
	// Disable dither for high quality faster output.
	la_opengl_disable(GL_DITHER);
	la_opengl_enable(GL_CULL_FACE);
	jlgr_opengl_blend_default_(jlgr);
//	la_opengl_enable(GL_DEPTH_TEST); // TODO: enable / disable functions 3d
	// Create shaders and set up attribute/uniform variable communication
	la_print("Making Shader: texture");
	jlgr_opengl_shader_init(jlgr, &jlgr->gl.prg.texture, NULL,
		JL_SHADER_TEX_FRAG, 1);
	// Default GL Texture Coordinate Buffer
	la_llgraphics_buffer_set_(&jlgr->gl.default_tc, DEFAULT_TC, 8);
	la_llgraphics_buffer_set_(&jlgr->gl.upsidedown_tc, UPSIDEDOWN_TC, 8);
	la_print("made temp vo & default tex. c. buff!");
}

void la_llgraphics_initshader_color__(la_window_t* window) {
	la_print("Making Shader: color");
	jlgr_opengl_shader_init(window, &window->gl.prg.color, JL_SHADER_CLR_VERT,
		JL_SHADER_CLR_FRAG, 0);
}

#endif
