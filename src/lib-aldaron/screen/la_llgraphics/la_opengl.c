/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_llgraphics.h>
#include <la_string.h>

#include "la_draw.h"
#include "la_opengl__.h"
#include "la_thread.h"
#include "la_memory.h"

extern float la_banner_size;

const char *LA_SHADER_CLR_FRAG =
	GLSL_HEAD
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vec4(vcolor.rgba);\n"
	"}";

const char *LA_SHADER_CLR_VERT =
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

const char *LA_SHADER_TEX_FRAG =
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(texture, texcoord);\n"
	"}";

const char *LA_SHADER_TEX_VERT =
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

#ifdef LA_DEBUG
static void la_opengl_error__(int data, const char* fname) {
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

#if defined(LA_WINDOWS)
	
#endif

static inline void la_opengl_enable(int32_t what) {
	glEnable(what);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glEnable()");
#endif
}

static inline void la_opengl_disable(int32_t what) {
	glDisable(what);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glDisable()");
#endif
}

static inline uint32_t la_opengl_genbuffer(void) {
	uint32_t buffer;
	glGenBuffers(1, &buffer);
#ifdef LA_DEBUG
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
#ifdef LA_DEBUG
	la_opengl_error__(0, "buffer free");
#endif
}

static inline void la_opengl_bindbuffer(uint32_t buffer) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
#ifdef LA_DEBUG
	la_opengl_error__(buffer, "bind buffer");
#endif
}

static inline void la_opengl_bufferdata(const void* data, uint32_t size) {
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
#ifdef LA_DEBUG
	la_opengl_error__(size, "buffer data");
#endif
}

static inline void la_opengl_attachshader(uint32_t program, uint32_t shader) {
	glAttachShader(program, shader);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glAttachShader()");
#endif
}

static inline uint32_t la_opengl_makeprogram(void) {
	uint32_t program = glCreateProgram();
#ifdef LA_DEBUG
	la_opengl_error__(0,"glCreateProgram");
	if (!program) {
		la_panic("Failed to load program");
	}
#endif
	return program;
}

static inline uint32_t la_opengl_makeshader(uint32_t type) {
	uint32_t shader = glCreateShader(type);
#ifdef LA_DEBUG
	la_opengl_error__(shader, "Couldn't create shader");
	if(!shader)
		la_panic("Couldn't create shader: opengl context not made?");
#endif
	return shader;
}

static inline void la_opengl_shader_setsource(uint32_t shader, const char* src){
	glShaderSource(shader, 1, &src, NULL);
#ifdef LA_DEBUG
	la_opengl_error__(shader, "glShaderSource");
#endif
}

static inline void la_opengl_shader_compile(uint32_t shader) {
	glCompileShader(shader);
#ifdef LA_DEBUG
	la_opengl_error__(shader, "glCompileShader");
#endif
}

static inline void la_opengl_program_link(uint32_t program) {
	glLinkProgram(program);
#ifdef LA_DEBUG
	la_opengl_error__(program, "glLinkProgram");
#endif
}

#ifdef LA_DEBUG

static int32_t la_opengl_shader_getiv(uint32_t shader, uint32_t get_what) {
	int32_t value;

	glGetShaderiv(shader, get_what, &value);
	la_opengl_error__(shader, "glGetShaderiv");
	return value;
}

static int32_t la_opengl_program_getiv(uint32_t program, uint32_t get_what) {
	int32_t value;

	glGetProgramiv(program, get_what, &value);
	la_opengl_error__(program,"glGetProgramiv");
	return value;
}

static inline void la_opengl_shader_kill(uint32_t shader) {
	glDeleteShader(shader);
	la_opengl_error__(shader, "glDeleteShader");
}

static inline void la_opengl_program_kill(uint32_t program) {
	glDeleteProgram(program);
	la_opengl_error__(program, "glDeleteProgram");
}

static inline void
la_opengl_shader_infolog(uint32_t shader, void* data, uint32_t size) {
	glGetShaderInfoLog(shader, size, NULL, data);
	la_opengl_error__(shader, "glGetShaderInfoLog");
}

static inline void
la_opengl_program_infoLog(uint32_t program, void* data, uint32_t size) {
	glGetProgramInfoLog(program, size, NULL, data);
	la_opengl_error__(program, "glGetProgramInfoLog");
}

static inline void la_opengl_program_validate(uint32_t program) {
	glValidateProgram(program);
	la_opengl_error__(program, "glValidateProgram");
}

#endif

static uint32_t la_opengl_texture_make__(void) {
	uint32_t tex;

	glGenTextures(1, &tex);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glGenTextures");
	if(!tex) la_panic("glGenTextures => 0");
#endif
	return tex;
}

static void la_opengl_texture_2d__(uint8_t* px,uint32_t w,uint32_t h,uint8_t a){
	GLenum format = a ? GL_RGBA : GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format,
		GL_UNSIGNED_BYTE, px);
#ifdef LA_DEBUG
	la_opengl_error__(w, "glTexImage2D");
#endif
}

// TODO: ADD PARAM
static inline void la_opengl_texture_param__(uint32_t filter) {
	glTexParameteri(GL_TEXTURE_2D, filter, GL_NEAREST);
#ifdef LA_DEBUG
	la_opengl_error__(filter, "glTexParameteri");
#endif
}

static inline void la_opengl_texture_bind__(uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
#ifdef LA_DEBUG
	la_opengl_error__(tex, "la_opengl_texture_bind__: glBindTexture");
#endif
}

static inline void la_opengl_texture_free__(uint32_t tex) {
	glDeleteTextures(1, &tex);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glDeleteTextures");
#endif
}

static inline int32_t la_opengl_uniform__(GLuint prg, const char *var) {
	int32_t a = glGetUniformLocation(prg, var);
#ifdef LA_DEBUG
	if(a == -1) la_panic("opengl: bad name; is: %s", var);
	la_opengl_error__(a, "glGetUniformLocation");
#endif
	return a;
}

static inline int32_t la_opengl_attribute__(GLuint prg, const char *var) {
	int32_t a = glGetAttribLocation(prg, var);
#ifdef LA_DEBUG
	if(a == -1) {
		la_print("for name \"%s\":", var);
		la_panic("attribute name is either reserved or non-existant");
	}
#endif
	glEnableVertexAttribArray(a);
#ifdef LA_DEBUG
	la_opengl_error__(a, "glEnableVertexAttribArray");
#endif
	return a;
}

static inline void la_opengl_use_program__(GLuint prg) {
#ifdef LA_DEBUG
	if(!prg) la_panic("shader program uninit'd!");
#endif
	glUseProgram(prg);
#ifdef LA_DEBUG
	la_opengl_error__(prg, "glUseProgram");
#endif
}

static inline void la_opengl_uniform_1int__(int32_t uv, int32_t v1) {
	glUniform1i(uv, v1);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_1int__");
#endif
}

static inline void la_opengl_uniform_2int__(int32_t uv, int32_t v1, int32_t v2){
	glUniform2i(uv, v1, v2);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_2int__");
#endif
}

static inline void la_opengl_uniform_3int__(int32_t uv, int32_t v1, int32_t v2,
	int32_t v3)
{
	glUniform3i(uv, v1, v2, v3);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_3int__");
#endif
}

static inline void la_opengl_uniform_4int__(int32_t uv, int32_t v1, int32_t v2,
	int32_t v3, int32_t v4)
{
	glUniform4i(uv, v1, v2, v3, v4);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_4int__");
#endif
}

static inline void la_opengl_uniform_1float__(int32_t uv, float v1) {
	glUniform1f(uv, v1);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_1float__");
#endif
}

static inline void la_opengl_uniform_2float__(int32_t uv, float v1,
	float v2)
{
	glUniform2f(uv, v1, v2);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_2float__");
#endif
}

static inline void la_opengl_uniform_3float__(int32_t uv, float v1,
	float v2, float v3)
{
	glUniform3f(uv, v1, v2, v3);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_3float__");
#endif
}

static inline void la_opengl_uniform_4float__(int32_t uv, float v1,
	float v2, float v3, float v4)
{
	glUniform4f(uv, v1, v2, v3, v4);
#ifdef LA_DEBUG
	la_opengl_error__(uv, "la_opengl_uniform_4float__");
#endif
}

static inline void la_opengl_attribute_pointer(int32_t attrib,uint8_t elementc){
	glVertexAttribPointer(attrib, elementc, GL_FLOAT, GL_FALSE, 0, 0);
#ifdef LA_DEBUG
	la_opengl_error__(attrib, "la_opengl_attribute_pointer");
#endif
}

static inline void la_opengl_draw_arrays(uint32_t mode, uint32_t count) {
	glDrawArrays(mode, 0, count);
#ifdef LA_DEBUG
	la_opengl_error__(0,"glDrawArrays");
#endif
}

static inline void la_opengl_blend_default__(void) {
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
		GL_DST_ALPHA);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glBlendFuncSeparate");
#endif
}

static void la_opengl_viewport(uint32_t w, uint32_t h) {
	glViewport(0, 0, w, h);
#ifdef LA_DEBUG
	la_opengl_error__(w, "glViewport");
#endif
}

static inline uint32_t la_opengl_gen_framebuffer(void) {
	uint32_t fb;

	glGenFramebuffers(1, &fb);
#ifdef LA_DEBUG
	if(!fb) la_panic("la_opengl_gen_framebuffer: GL FB = 0");
	la_opengl_error__(fb, "la_opengl_gen_framebuffer");
#endif
	return fb;
}

static inline void la_opengl_framebuffer_bind(uint32_t framebuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
#ifdef LA_DEBUG
	la_opengl_error__(framebuffer, "glBindFramebuffer");
#endif
}

static inline void la_opengl_framebuffer_tx2d(uint32_t texture) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, texture, 0);
#ifdef LA_DEBUG
	la_opengl_error__(texture, "glFramebufferTexture2D");
#endif
}

static inline void la_opengl_framebuffer_free(uint32_t fb) {
	glDeleteFramebuffers(1, &fb);
#ifdef LA_DEBUG
	la_opengl_error__(fb, "glDeleteFramebuffers");
#endif
}

static inline void la_opengl_clear(float r, float g, float b, float a){
	glClearColor(r, g, b, a);
#ifdef LA_DEBUG
	la_opengl_error__(a, "la_opengl_clear/glClearColor");
#endif
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#ifdef LA_DEBUG
	la_opengl_error__(a, "la_opengl_clear/glClear");
#endif
}

// Static LLGRAPHICS FUNCTIONS

static void la_llgraphics_buffer_use__(uint32_t *buffer) {
	// Make buffer if not initialized.
	if(*buffer == 0) *buffer = la_opengl_genbuffer();
	// Bind the buffer
	la_opengl_bindbuffer(*buffer);
}

static uint32_t la_llgraphics_load_shader__(GLenum type, const char* src) {
	uint32_t shader = la_opengl_makeshader(type);

	la_opengl_shader_setsource(shader, src);
	la_opengl_shader_compile(shader);
#ifdef LA_DEBUG
	if (!la_opengl_shader_getiv(shader, GL_COMPILE_STATUS)) {
		int32_t infoLen =
			la_opengl_shader_getiv(shader, GL_INFO_LOG_LENGTH);

		if (!infoLen) la_panic("Failed to compile shader: No details.");
		char buf[infoLen];

		la_opengl_shader_infolog(shader, buf, infoLen);
		la_opengl_shader_kill(shader);
		la_panic("Could not compile %s:%s", (type == GL_VERTEX_SHADER) ?
			"vertex shader" : "fragment shader", buf);
	}
#endif
	return shader;
}

static inline uint32_t
la_llgraphics_shader_new__(const char* vert, const char* frag) {
	uint32_t vertexShader =
		la_llgraphics_load_shader__(GL_VERTEX_SHADER, vert);
	uint32_t fragmentShader =
		la_llgraphics_load_shader__(GL_FRAGMENT_SHADER, frag);
	uint32_t program = la_opengl_makeprogram();

	la_opengl_attachshader(program, vertexShader);
	la_opengl_attachshader(program, fragmentShader);
	la_opengl_program_link(program);
#ifdef LA_DEBUG
	la_opengl_program_validate(program);
	if(la_opengl_program_getiv(program, GL_LINK_STATUS) != GL_TRUE) {
		GLint bufLength =
			la_opengl_program_getiv(program, GL_INFO_LOG_LENGTH);

		if (bufLength) {
			char buf[bufLength];

			la_opengl_program_infoLog(program, buf, bufLength);
			la_panic("Could not link program: %s", buf);
		}else{
			la_opengl_program_kill(program);
			la_panic("no info log");
		}
	}
#endif
	return program;
}

static int32_t la_llgraphics_uniform__(la_window_t* window, la_shader_t* glsl,
	const char* name, int16_t index)
{
	// Bind Shader
	la_llgraphics_shader_bind(glsl);
	// Get uniform
	if(index == -1) {
		return la_opengl_uniform__(glsl->program, name);
	}else{
		const char* ntext = la_string_fint(index);
		char name2[strlen(name) + 1 + strlen(ntext) + 2];

		la_memory_copy(name, name2, strlen(name));
		name2[strlen(name)] = '[';
		la_memory_copy(ntext, name2 + strlen(name) + 1, strlen(ntext));
		name2[strlen(name) + 1 + strlen(ntext)] = ']';
		name2[strlen(name) + 1 + strlen(ntext) + 1] = '\0';
		return la_opengl_uniform__(glsl->program, name2);
	}
}

static inline void la_llgraphics_uniform_matrix(int32_t uniform, float* matrix){
	glUniformMatrix4fv(uniform, 1, 0, matrix);
#ifdef LA_DEBUG
	la_opengl_error__(uniform, "matrix_object - scale");
#endif
}

// LLGRAPHICS API

void la_llgraphics_buffer_set_(uint32_t *buffer, const void *buffer_data,
	uint16_t buffer_size)
{
	//Bind Buffer "buffer"
	la_llgraphics_buffer_use__(buffer);
	//Set the data
	la_opengl_bufferdata(buffer_data, buffer_size * sizeof(float));
}

void la_llgraphics_buffer_free(uint32_t buffer) {
	la_opengl_freebuffer(buffer);
}

void la_llgraphics_texture_bind(uint32_t tex) {
#ifdef LA_DEBUG
	if(tex == 0)
		la_panic("la_llgraphics_texture_bind(): tex = 0");
#endif
	la_opengl_texture_bind__(tex);
}

void la_llgraphics_texture_unbind(void) {
	la_opengl_texture_bind__(0);
}

void la_llgraphics_texture_free(uint32_t tex) {
	la_opengl_texture_free__(tex);
}

// TODO: window is not needed as argument
void la_llgraphics_uniformf(la_window_t* window, la_shader_t* glsl, float* x,
	uint8_t num_elements, const char* name, int16_t index)
{
	int32_t uv = la_llgraphics_uniform__(window, glsl, name, index);

	// Push data to uniform
	switch(num_elements) {
		case 1:
			la_opengl_uniform_1float__(uv, x[0]);
			break;
		case 2:
			la_opengl_uniform_2float__(uv, x[0], x[1]);
			break;
		case 3:
			la_opengl_uniform_3float__(uv, x[0], x[1], x[2]);
			break;
		case 4:
			la_opengl_uniform_4float__(uv, x[0], x[1], x[2], x[3]);
			break;
		default:
			la_print("num_elements must be 1-4");
			break;
	}
}

void la_llgraphics_uniformi(la_window_t* window, la_shader_t* glsl, int32_t* x,
	uint8_t num_elements, const char* name, int16_t index)
{
	int32_t uv = la_llgraphics_uniform__(window, glsl, name, index);

	switch(num_elements) {
		case 1:
			la_opengl_uniform_1int__(uv, x[0]);
			break;
		case 2:
			la_opengl_uniform_2int__(uv, x[0], x[1]);
			break;
		case 3:
			la_opengl_uniform_3int__(uv, x[0], x[1], x[2]);
			break;
		case 4:
			la_opengl_uniform_4int__(uv, x[0], x[1], x[2], x[3]);
			break;
		default:
			la_print("num_elements must be 1-4");
			break;
	}
}

void la_llgraphics_attribute_set(uint32_t* buffer, uint32_t attrib,
	uint8_t elementc)
{
	// Bind Buffer
	la_llgraphics_buffer_use__(buffer);
	// Set Vertex Attrib Pointer
	la_opengl_attribute_pointer(attrib, elementc);
}

void la_llgraphics_draw_triangles(uint32_t count) {
	la_opengl_draw_arrays(GL_TRIANGLES, count);
}

void la_llgraphics_draw_trianglefan(uint32_t count) {
	la_opengl_draw_arrays(GL_TRIANGLE_FAN, count);
}

void la_llgraphics_push_vertices(const float *xyzw, uint32_t vertices,
	float* cv, uint32_t* gl)
{
	uint32_t items = (vertices*3);

	// Copy Vertices
	la_memory_copy(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	la_llgraphics_buffer_set_(gl, cv, items);
}

// TODO: MOVE
/**
 * Create a new texture object.
**/
uint32_t la_texture_new(la_window_t* window, uint8_t* pixels, uint32_t w,
	uint32_t h, uint8_t bpp)
{
	uint32_t texture = la_opengl_texture_make__();

	la_opengl_texture_bind__(texture);
	la_opengl_texture_param__(GL_TEXTURE_MIN_FILTER);
	la_opengl_texture_param__(GL_TEXTURE_MAG_FILTER);
	la_opengl_texture_2d__(pixels, w, h, !bpp);
	return texture;
}

// TODO: MOVE
/**
 * Set a texture object.
 * @param texture: The texture to modify.
 * @param pixels: The pixels to set the texture to.
 * @param w, h: The dimensions
 * @param bpp: The bytes per pixel.  Must be 3 or 4.
**/
void la_texture_set(la_window_t* window, uint32_t texture, uint8_t* pixels,
	uint32_t w, uint32_t h, uint8_t bpp)
{
	GLenum format = GL_RGBA;

	if(bpp == 3) format = GL_RGB;

	// Bind Texture
	la_opengl_texture_bind__(texture);
	// Copy to texture.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE,
		pixels);
#ifdef LA_DEBUG
	la_opengl_error__(0, "glTexSubImage2D");
#endif
}

void la_llgraphics_viewport(uint32_t w, uint32_t h) {
	la_opengl_viewport(w, h);
}

uint32_t la_llgraphics_framebuffer_make(void) {
	return la_opengl_gen_framebuffer();
}

void la_llgraphics_framebuffer_bind(uint32_t fb) {
	la_opengl_framebuffer_bind(fb);
}

void la_llgraphics_framebuffer_addtx(uint32_t texture) {
	la_opengl_framebuffer_tx2d(texture);
}

void la_llgraphics_framebuffer_free(uint32_t fb) {
	la_opengl_framebuffer_free(fb);
}

void la_llgraphics_setmatrix(la_shader_t* sh, la_v3_t scalev, la_v3_t rotatev,
	la_v3_t translatev, la_v3_t lookv, float ar)
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
	la_llgraphics_uniform_matrix(sh->uniforms.scale_object, scale);
	la_llgraphics_uniform_matrix(sh->uniforms.rotate_object, rotate_object);
	la_llgraphics_uniform_matrix(sh->uniforms.translate_object, translate);
	la_llgraphics_uniform_matrix(sh->uniforms.rotate_camera, rotate_scene);
	la_llgraphics_uniform_matrix(sh->uniforms.project_scene, projection);
}

void la_llgraphics_shader_bind(la_shader_t* sh) {
	la_opengl_use_program__(sh->program);
}

void la_llgraphics_shader_make(la_shader_t* glsl, const char* vert,
	const char* frag, uint8_t has_tex)
{
	const char* vertShader = vert ? vert : LA_SHADER_TEX_VERT;
	// Program
	glsl->program = la_llgraphics_shader_new__(vertShader, frag);
	// Matrices
	glsl->uniforms.scale_object = la_opengl_uniform__(glsl->program,
		"scale_object");
	glsl->uniforms.rotate_object = la_opengl_uniform__(glsl->program,
		"rotate_object");
	glsl->uniforms.translate_object = la_opengl_uniform__(glsl->program,
		"translate_object");
	glsl->uniforms.rotate_camera=la_opengl_uniform__(glsl->program,
		"rotate_camera");
	glsl->uniforms.project_scene=la_opengl_uniform__(glsl->program,
		"project_scene");
	// Position Attribute
	glsl->attributes.position = la_opengl_attribute__(glsl->program,
		"position");
	// If custom vertex shader, don't assume texture is defined
	if(has_tex) {
		// Texture
		glsl->uniforms.texture = la_opengl_uniform__(glsl->program, "texture");
		// Texture Position Attribute
		glsl->attributes.texpos_color =
			la_opengl_attribute__(glsl->program, "texpos");
	}else{
		// Color Attribute
		glsl->attributes.texpos_color =
			la_opengl_attribute__(glsl->program, "acolor");
	}
}

void la_llgraphics_clear(float r, float g, float b, float a) {
	la_opengl_clear(r, g, b, a);
}

// END API

void la_llgraphics_init__(la_window_t* window) {
	window->gl.cp = NULL;
	// Disable dither for high quality faster output.
	la_opengl_disable(GL_DITHER);
	la_opengl_enable(GL_CULL_FACE);
	la_opengl_enable(GL_BLEND);
	la_opengl_blend_default__();
//	la_opengl_enable(GL_DEPTH_TEST); // TODO: enable / disable functions 3d
	// Create shaders and set up attribute/uniform variable communication
	la_print("Making Shader: texture");
	la_llgraphics_shader_make(&window->gl.prg.texture, NULL,
		LA_SHADER_TEX_FRAG, 1);
	// Default GL Texture Coordinate Buffer
	la_llgraphics_buffer_set_(&window->gl.default_tc, DEFAULT_TC, 8);
	la_llgraphics_buffer_set_(&window->gl.upsidedown_tc, UPSIDEDOWN_TC, 8);
	la_print("made temp vo & default tex. c. buff!");
}

void la_llgraphics_initshader_color__(la_window_t* window) {
	la_print("Making Shader: color");
	la_llgraphics_shader_make(&window->gl.prg.color, LA_SHADER_CLR_VERT,
		LA_SHADER_CLR_FRAG, 0);
}

#endif
