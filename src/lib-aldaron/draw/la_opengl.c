#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"
#include "la_thread.h"

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

void la_opengl_error__(int data, const char* fname) {
#ifdef JL_DEBUG
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
#endif
}

static void jl_gl_buffer_use__(la_window_t* jlgr, uint32_t *buffer) {
	la_opengl_error__(0, "PRE__glGenBuffers");
	// Make buffer if not initialized.
	if(*buffer == 0) {
		glGenBuffers(1, buffer);
#ifdef JL_DEBUG
		la_opengl_error__(*buffer, "glGenBuffers");
		if(*buffer == 0) {
			la_panic("buffer is made wrongly on thread #%X!",
				la_thread_current());
		}
#endif
	}
	// Bind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);
	la_opengl_error__(*buffer, "bind buffer");
}

void jlgr_opengl_buffer_set_(la_window_t* jlgr, uint32_t *buffer,
	const void *buffer_data, uint16_t buffer_size)
{
	//Bind Buffer "buffer"
	jl_gl_buffer_use__(jlgr, buffer);
	//Set the data
	glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), buffer_data,
		GL_DYNAMIC_DRAW);
	la_opengl_error__(buffer_size, "buffer data");
}

void jlgr_opengl_buffer_old_(la_window_t* jlgr, uint32_t *buffer) {
	glDeleteBuffers(1, buffer);
	la_opengl_error__(0,"buffer free");
	*buffer = 0;
}

GLuint jl_gl_load_shader(la_window_t* jlgr, GLenum shaderType, const char* pSource) {
	GLuint shader = glCreateShader(shaderType);
	la_opengl_error__(0,"couldn't create shader");
	if (shader) {
		GLint compiled = 0;

		glShaderSource(shader, 1, &pSource, NULL);
		la_opengl_error__(0, "glShaderSource");
		glCompileShader(shader);
		la_opengl_error__(0, "glCompileShader");
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		la_opengl_error__(0, "glGetShaderiv");
		if (!compiled) {
			GLint infoLen = 0;
			char* buf;

			la_print("Failed Compile %dx%d:", shader, compiled);
//			la_print("%s", pSource);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			la_opengl_error__(1,"glGetShaderiv");
			if (infoLen) {
				buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen,
						NULL, buf);
					const char* msg =
						(shaderType==GL_VERTEX_SHADER)?
						"vertex shader":"fragment shader";
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
	}
	return shader;
}

GLuint jl_gl_glsl_prg_create(la_window_t* jlgr, const char* pVertexSource,
	const char* pFragmentSource)
{
	la_print("Vertex Shader....");
	GLuint vertexShader = jl_gl_load_shader(jlgr, GL_VERTEX_SHADER,
		pVertexSource);
	if (!vertexShader) {
		la_panic("couldn't load vertex shader");
	}
	la_print("Fragment shader....");
	GLuint fragmentShader = jl_gl_load_shader(jlgr, GL_FRAGMENT_SHADER,
		pFragmentSource);
	if (!fragmentShader) {
		la_panic("couldn't load fragment shader");
	}
	la_print("Together Shader....");
	GLuint program = glCreateProgram();
	la_opengl_error__(0,"glCreateProgram");
	if (!program) {
		la_panic("Failed to load program");
	}

	la_print("Linking....");

	GLint linkStatus = GL_FALSE;

	glAttachShader(program, vertexShader);
	la_opengl_error__(0,"glAttachShader (vertex)");
	glAttachShader(program, fragmentShader);
	la_opengl_error__(0,"glAttachShader (fragment)");
	glLinkProgram(program);
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
	la_opengl_error__(w, "texture image 2D");
}

static void jl_gl_texpar_set__(la_window_t* jlgr) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	la_opengl_error__(0,"glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	la_opengl_error__(1,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	la_opengl_error__(2,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	la_opengl_error__(3,"glTexParameteri");
}

static inline void jl_gl_texture__bind__(la_window_t* jlgr, uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	la_opengl_error__(tex,"jlgr_opengl_texture_bind_: glBindTexture");
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
	la_opengl_error__(0, "glDeleteTextures");
	*tex = 0;
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

//Lower Level Stuff
static inline void jl_gl_usep__(la_window_t* jlgr, GLuint prg) {
#ifdef JL_DEBUG
	if(!prg) la_panic("shader program uninit'd!");
#endif
	glUseProgram(prg);
	la_opengl_error__(prg, "glUseProgram");
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
	jlgr_opengl_shader_uniform(jlgr, glsl, &uv, uniform_name);

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
	la_opengl_error__(uv, "glUniform..f");
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
	jlgr_opengl_shader_uniform(jlgr, glsl, &uv, uniform_name);

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
	la_opengl_error__(uv, "glUniform..i");
}

/**
 * Set a uniform variable in a shader to a float.
 * @param jlgr: The library context.
 * @param e: The number of elements in an array, or 1 if uniform isn't array.
 * @param uv: The uniform variable.
 * @param x: The float value.
**/
void jlgr_opengl_uniform1(la_window_t* jlgr, uint8_t e, int32_t uv, float* x) {
	glUniform1fv(uv, e, x);
	la_opengl_error__(uv, "glUniform1fv");
}

void jlgr_opengl_uniform1i(la_window_t* jlgr, uint8_t e, int32_t uv, int32_t* x) {
	glUniform1iv(uv, e, x);
	la_opengl_error__(uv, "glUniform1iv");
}

/**
 * Set a uniform variable in a shader to a vec3.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param xyz: The vec3 value.
**/
void jlgr_opengl_uniform3(la_window_t* jlgr, uint8_t e, int32_t uv, float* xyz) {
	glUniform3fv(uv, e, xyz);
	la_opengl_error__(uv, "glUniform3fv");
}

/**
 * Set the effect uniform variable in a shader to a vec4.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param x, y, z, w: The vec4 value.
**/
void jlgr_opengl_uniform4(la_window_t* jlgr, uint8_t e, int32_t uv, float* xyzw) {
	glUniform4fv(uv, e, xyzw);
	la_opengl_error__(uv, "glUniform4fv");
}

/**
 * Set the effect uniform variable in a shader to a mat4.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param m: The mat4 value.
**/
void jlgr_opengl_uniformM(la_window_t* jlgr, int32_t uv, float m[]) {
	glUniformMatrix4fv(uv, 1, 1, m);
	la_opengl_error__(uv, "glUniformMatrix4fv");
}

//This pushes VBO "buff" up to the shader's vertex attribute "vertexAttrib"
//Set xyzw to 2 if 2D coordinates 3 if 3D. etc.
void jlgr_opengl_setv(la_window_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw)
{
	// Bind Buffer
	jl_gl_buffer_use__(jlgr, buff);
	// Set Vertex Attrib Pointer
	glEnableVertexAttribArray(vertexAttrib);
	la_opengl_error__(vertexAttrib,"glEnableVertexAttribArray");
	glVertexAttribPointer(
		vertexAttrib,	// attribute
		xyzw,		// x+y+z = 3
		GL_FLOAT,	// type
		GL_FALSE,	// normalized?
		0,		// stride
		0		// array buffer offset
	);
	la_opengl_error__(0,"glVertexAttribPointer");
}

void jlgr_opengl_draw_arrays_(la_window_t* jlgr, GLenum mode, uint8_t count) {
	glDrawArrays(mode, 0, count);
	la_opengl_error__(0,"glDrawArrays");
}

void jlgr_opengl_blend_default_(la_window_t* jlgr) {
	glEnable(GL_BLEND);
	la_opengl_error__(0,"glEnable( BLEND )");
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
		GL_DST_ALPHA);
	la_opengl_error__(0, "glBlendFuncSeparate");
}

void jlgr_opengl_blend_add_(la_window_t* jlgr) {
	glEnable(GL_BLEND);
	la_opengl_error__(0,"glEnable( BLEND )");
	glBlendFunc(GL_ONE, GL_ONE);
	la_opengl_error__(0, "glBlendFunc");
}

void jlgr_opengl_blend_none_(la_window_t* jlgr) {
	glDisable(GL_BLEND);
	la_opengl_error__(0,"glDisable( BLEND )");
}

static inline void _jl_gl_init_disable_extras(la_window_t* jlgr) {
	glDisable(GL_DEPTH_TEST);
	la_opengl_error__(0, "glDisable(GL_DEPTH_TEST)");
	glDisable(GL_DITHER);
	la_opengl_error__(0, "glDisable(GL_DITHER)");
}

static inline void _jl_gl_init_enable_alpha(la_window_t* jlgr) {
	glEnable(GL_CULL_FACE);
	la_opengl_error__(0,"glEnable( CULL FACE )");
	glBlendColor(0.f,0.f,0.f,0.f);
	la_opengl_error__(0,"glBlendColor");
	jlgr_opengl_blend_default_(jlgr);
}

// Copy & Push vertices to a VBO.
void jlgr_opengl_vertices_(la_window_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl)
{
	uint16_t items = (vertices*3);

	// Copy Vertices
	jl_mem_copyto(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	jlgr_opengl_buffer_set_(jlgr, gl, cv, items);
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
	la_opengl_error__(0, "jl_gl_pbo_set__: glTexSubImage2D");
}

/************************/
/***  ETOM Functions  ***/
/************************/

void jlgr_opengl_viewport_(la_window_t* jlgr, uint16_t w, uint16_t h) {
	glViewport(0, 0, w, h);
	la_opengl_error__(w * h, "glViewport");
}

void jl_opengl_framebuffer_make_(la_window_t* jlgr, uint32_t *fb) {
	glGenFramebuffers(1, fb);
	if(!(*fb)) la_panic("jl_gl_framebuffer_make__: GL FB = 0");
	la_opengl_error__(*fb,"glGenFramebuffers");
}

void jlgr_opengl_framebuffer_bind_(la_window_t* jlgr, uint32_t fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	la_opengl_error__(fb, "glBindFramebuffer");
}

void jlgr_opengl_framebuffer_addtx_(la_window_t* jlgr, uint32_t tx) {
	// Set "*tex" as color attachment #0.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tx, 0);
	la_opengl_error__(tx,
		"jlgr_opengl_framebuffer_addtx_: glFramebufferTexture2D");
}

void jlgr_opengl_framebuffer_subtx_(la_window_t* jlgr) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, 0, 0);
	la_opengl_error__(0,
		"jlgr_opengl_framebuffer_subtx_: glFramebufferTexture2D");
}

void jlgr_opengl_framebuffer_status_(la_window_t* jlgr) {
	// Check to see if framebuffer was made properly.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		la_panic("Frame buffer not complete!");
}

void jl_opengl_framebuffer_free_(la_window_t* jlgr, uint32_t *fb) {
	glDeleteFramebuffers(1, fb);
	la_opengl_error__(*fb, "glDeleteFramebuffers");
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
void jlgr_opengl_matrix(la_window_t* jlgr, jlgr_glsl_t* sh, jl_vec3_t scalev,
	jl_vec3_t rotatev, jl_vec3_t translatev, jl_vec3_t lookv, float ar)
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
	la_opengl_error__(0, "matrix_object - scale");
	glUniformMatrix4fv(sh->uniforms.rotate_object, 1, 0, rotate_object);
	la_opengl_error__(0, "matrix_object - rotate");
	glUniformMatrix4fv(sh->uniforms.translate_object, 1, 0, translate);
	la_opengl_error__(0, "matrix_object - translate");
	glUniformMatrix4fv(sh->uniforms.rotate_camera, 1, 0, rotate_scene);
	la_opengl_error__(0, "matrix_object - rotate world");
	glUniformMatrix4fv(sh->uniforms.project_scene, 1, 0, projection);
	la_opengl_error__(0, "matrix_object - projection");
}

/**
 * Bind shader ( Prepare to draw ).
 * @param jlgr: The library context.
 * @param sh: The shader to use.
**/
void jlgr_opengl_draw1(la_window_t* jlgr, jlgr_glsl_t* sh) {
	jl_gl_usep__(jlgr, sh->program);
}

static int32_t _jl_gl_getu(la_window_t* jlgr, GLuint prg, const char *var) {
	int32_t a = glGetUniformLocation(prg, var);
#ifdef JL_DEBUG
	if(a == -1) la_panic("opengl: bad name; is: %s", var);
	la_opengl_error__(a, "glGetUniformLocation");
#endif
	return a;
}

void _jl_gl_geta(la_window_t* jlgr, GLuint prg, int32_t *attrib, const char *title) {
	if((*attrib = glGetAttribLocation(prg, title)) == -1) {
		la_print("for name \"%s\":", title);
		la_panic("attribute name is either reserved or non-existant");
	}
}

/***	  @cond	   ***/
/************************/
/*** Static Functions ***/
/************************/

static inline void _jl_gl_init_setup_gl(la_window_t* jlgr) {
	la_print("setting properties....");
	//Disallow Dither & Depth Test
	_jl_gl_init_disable_extras(jlgr);
	//Set alpha=0 to transparent
	_jl_gl_init_enable_alpha(jlgr);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	la_print("set glproperties.");
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
	glsl->program = jl_gl_glsl_prg_create(jlgr, vertShader, frag);
	// Matrices
	glsl->uniforms.scale_object =
		_jl_gl_getu(jlgr,glsl->program,"scale_object");
	glsl->uniforms.rotate_object =
		_jl_gl_getu(jlgr,glsl->program,"rotate_object");
	glsl->uniforms.translate_object =
		_jl_gl_getu(jlgr,glsl->program,"translate_object");
	glsl->uniforms.rotate_camera =
		_jl_gl_getu(jlgr,glsl->program,"rotate_camera");
	glsl->uniforms.project_scene =
		_jl_gl_getu(jlgr,glsl->program,"project_scene");
	// Position Attribute
	_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.position, "position");
	// If custom vertex shader, don't assume texture is defined
	if(has_tex) {
		// Texture
		glsl->uniforms.texture =
			_jl_gl_getu(jlgr, glsl->program, "texture");
		// Texture Position Attribute
		_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.texpos_color,
			 "texpos");
	}else{
		// Color Attribute
		_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.texpos_color,
			"acolor");
	}
}

/**
 * Get a uniform from shader.
 * @param jlgr: The library context.
 * @param glsl: The shader.
 * @param uniform: The uniform to get ( output ).
 * @param name: The name of the uniform.
**/
void jlgr_opengl_shader_uniform(la_window_t* jlgr, jlgr_glsl_t* glsl,
	int32_t* uniform, const char* name)
{
	*uniform = _jl_gl_getu(jlgr, glsl->program, name);
}

static inline void _jl_gl_init_shaders(la_window_t* jlgr) {
	la_print("Making Shader: texture");
	jlgr_opengl_shader_init(jlgr, &jlgr->gl.prg.texture, NULL,
		JL_SHADER_TEX_FRAG, 1);
	la_print("Making Shader: color");
	jlgr_opengl_shader_init(jlgr, &jlgr->gl.prg.color, JL_SHADER_CLR_VERT,
		JL_SHADER_CLR_FRAG, 0);
	la_print("Set up shaders!");
}

//Load and create all resources
static inline void _jl_gl_make_res(la_window_t* jlgr) {
	// Setup opengl properties
	_jl_gl_init_setup_gl(jlgr);
	// Create shaders and set up attribute/uniform variable communication
	_jl_gl_init_shaders(jlgr);
	// Default GL Texture Coordinate Buffer
	jlgr_opengl_buffer_set_(jlgr, &jlgr->gl.default_tc, DEFAULT_TC, 8);
	jlgr_opengl_buffer_set_(jlgr, &jlgr->gl.upsidedown_tc, UPSIDEDOWN_TC, 8);
	la_print("made temp vo & default tex. c. buff!");
}

/**	  @endcond	  **/
/************************/
/*** Global Functions ***/
/************************/

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
	la_opengl_error__(a, "jl_gl_clear(): glClearColor");
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	la_opengl_error__(a, "jl_gl_clear(): glClear");
}

/***	  @cond	   ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void jl_gl_init__(la_window_t* jlgr) {
#ifdef JL_GLTYPE_HAS_GLEW
	if(glewInit()!=GLEW_OK) la_panic("glew fail!(no sticky)");
#endif
	jlgr->gl.cp = NULL;
	_jl_gl_make_res(jlgr);
	// Make sure no pre-renderer is activated.
	jlgr_pr_off(jlgr);
}

/**	  @endcond	  **/
/***   #End of File   ***/
