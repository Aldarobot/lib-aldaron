#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

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
	#define JL_GL_ERROR(jlgr, x, fname) jlgr_opengl_error__(jlgr, x, fname)
#else
	#define JL_GL_ERROR(jlgr, x, fname) ;
#endif

#ifdef JL_DEBUG
	static void jlgr_opengl_error__(jlgr_t* jlgr, int width, const char* fname) {
		uint8_t thread = jl_thread_current(jlgr->jl);
		if(thread != 1) {
			jl_print(jlgr->jl, "\"%s\" is on the Wrong Thread: %d",
				fname, thread);
			jl_print(jlgr->jl, "Must be on thread 1!");
			jl_print_stacktrace(jlgr->jl);
			exit(-1);
		}

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
			JL_PRINT("Max texture size: %d/%d\n", width, a);
		}else{
			fstrerr = "opengl: unknown error!\n";
		}
		jl_print(jlgr->jl, "error: %s:%s (%d)",fname,fstrerr,width);
		exit(-1);
	}
#endif

static void jl_gl_buffer_use__(jlgr_t* jlgr, uint32_t *buffer) {
	// Make buffer if not initialized.
	if(*buffer == 0) {
		glGenBuffers(1, buffer);
#ifdef JL_DEBUG
		jlgr_opengl_error__(jlgr, 0,"buffer gen");
		if(*buffer == 0) {
			jl_print(jlgr->jl,
				"buffer is made wrongly on thread #%d!",
				jl_thread_current(jlgr->jl));
			exit(-1);
		}
#endif
	}
	// Bind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);
	JL_GL_ERROR(jlgr, *buffer, "bind buffer");
}

void jlgr_opengl_buffer_set_(jlgr_t* jlgr, uint32_t *buffer,
	const void *buffer_data, uint16_t buffer_size)
{
	//Bind Buffer "buffer"
	jl_gl_buffer_use__(jlgr, buffer);
	//Set the data
	glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), buffer_data,
		GL_DYNAMIC_DRAW);
	JL_GL_ERROR(jlgr, buffer_size, "buffer data");
}

void jlgr_opengl_buffer_old_(jlgr_t* jlgr, uint32_t *buffer) {
	glDeleteBuffers(1, buffer);
	JL_GL_ERROR(jlgr, 0,"buffer free");
	*buffer = 0;
}

GLuint jl_gl_load_shader(jlgr_t* jlgr, GLenum shaderType, const char* pSource) {
	GLuint shader = glCreateShader(shaderType);
	JL_GL_ERROR(jlgr, 0,"couldn't create shader");
	if (shader) {
		GLint compiled = 0;

		glShaderSource(shader, 1, &pSource, NULL);
		JL_GL_ERROR(jlgr, 0,"glShaderSource");
		glCompileShader(shader);
		JL_GL_ERROR(jlgr, 0,"glCompileShader");
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		JL_GL_ERROR(jlgr, 0,"glGetShaderiv");
		if (!compiled) {
			GLint infoLen = 0;
			char* buf;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			JL_GL_ERROR(jlgr, 1,"glGetShaderiv");
			if (infoLen) {
				buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen,
						NULL, buf);
					const char* msg =
						(shaderType==GL_VERTEX_SHADER)?
						"vertex shader":"fragment shader";
					jl_print(jlgr->jl,
						"Could not compile %s:%s",msg,buf);
					exit(-1);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint jl_gl_glsl_prg_create(jlgr_t* jlgr, const char* pVertexSource,
	const char* pFragmentSource)
{
	GLuint vertexShader =
		jl_gl_load_shader(jlgr, GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		jl_print(jlgr->jl, "couldn't load vertex shader");
		exit(-1);
	}

	GLuint fragmentShader =
		jl_gl_load_shader(jlgr, GL_FRAGMENT_SHADER, pFragmentSource);
	if (!fragmentShader) {
		jl_print(jlgr->jl, "couldn't load fragment shader");
		exit(-1);
	}

	GLuint program = glCreateProgram();
	JL_GL_ERROR(jlgr, 0,"glCreateProgram");
	if (!program) {
		jl_print(jlgr->jl, "Failed to load program");
		exit(-1);
	}

	GLint linkStatus = GL_FALSE;

	glAttachShader(program, vertexShader);
	JL_GL_ERROR(jlgr, 0,"glAttachShader (vertex)");
	glAttachShader(program, fragmentShader);
	JL_GL_ERROR(jlgr, 0,"glAttachShader (fragment)");
	glLinkProgram(program);
	JL_GL_ERROR(jlgr, 0,"glLinkProgram");
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	JL_GL_ERROR(jlgr, 0,"glGetProgramiv");
	glValidateProgram(program);
	JL_GL_ERROR(jlgr, 1,"glValidateProgram");
	if (linkStatus != GL_TRUE) {
		GLint bufLength = 0;
		char* buf;

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
		JL_GL_ERROR(jlgr, 1,"glGetProgramiv");
		if (bufLength) {
			buf = (char*) malloc(bufLength);
			if (buf) {
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				jl_print(jlgr->jl,
					"Could not link program:%s",buf);
				exit(-1);
			}else{
				jl_print(jlgr->jl, "failed malloc");
				exit(-1);
			}
		}else{
			glDeleteProgram(program);
			jl_print(jlgr->jl, "no info log");
			exit(-1);
		}
	}
	return program;
}

static void jl_gl_texture_make__(jlgr_t* jlgr, uint32_t *tex) {
	glGenTextures(1, tex);
#ifdef JL_DEBUG
	if(!(*tex)) {
		jlgr_opengl_error__(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
		jl_print(jlgr->jl, "jl_gl_texture_make__: GL tex = 0");
		exit(-1);
	}
	jlgr_opengl_error__(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
#endif
}

// Set the bound texture.  pm is the pixels 0 - blank texture.
static void jl_gl_texture_set__(jlgr_t* jlgr, uint8_t* px, uint16_t w, uint16_t h,
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
	JL_GL_ERROR(jlgr, w, "texture image 2D");
}

static void jl_gl_texpar_set__(jlgr_t* jlgr) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	JL_GL_ERROR(jlgr, 0,"glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	JL_GL_ERROR(jlgr, 1,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	JL_GL_ERROR(jlgr, 2,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	JL_GL_ERROR(jlgr, 3,"glTexParameteri");
}

static inline void jl_gl_texture__bind__(jlgr_t* jlgr, uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	JL_GL_ERROR(jlgr, tex,"jlgr_opengl_texture_bind_: glBindTexture");
}

// Bind a texture.
void jlgr_opengl_texture_bind_(jlgr_t* jlgr, uint32_t tex) {
#ifdef JL_DEBUG
	if(tex == 0) {
		jl_print(jlgr->jl, "jlgr_opengl_texture_bind_: GL tex = 0");
		exit(-1);
	}
#endif
	jl_gl_texture__bind__(jlgr, tex);
}

// Unbind a texture
void jlgr_opengl_texture_off_(jlgr_t* jlgr) {
	jl_gl_texture__bind__(jlgr, 0);
}

// Make & Bind a new texture.
void jlgr_opengl_texture_new_(jlgr_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp)
{
	jl_print_function(jlgr->jl, "jl_gl_texture_new");
	// Make the texture
	jl_gl_texture_make__(jlgr, tex);
	// Bind the texture
	jlgr_opengl_texture_bind_(jlgr, *tex);
	// Set texture
	jl_gl_texture_set__(jlgr, px, w, h, bytepp);
	// Set the texture parametrs.
	jl_gl_texpar_set__(jlgr);
	jl_print_return(jlgr->jl, "jl_gl_texture_new");
}

void jl_gl_texture_free_(jlgr_t* jlgr, uint32_t *tex) {
	glDeleteTextures(1, tex);
	JL_GL_ERROR(jlgr, 0, "glDeleteTextures");
	*tex = 0;
}

// Make a texture - doesn't free "pixels"
uint32_t jl_gl_maketexture(jlgr_t* jlgr, void* pixels,
	uint32_t width, uint32_t height, uint8_t bytepp)
{
	uint32_t texture;

	jl_print_function(jlgr->jl, "GL_MkTex");
	if (!pixels) {
		jl_print(jlgr->jl, "null pixels");
		exit(-1);
	}
	JL_PRINT_DEBUG(jlgr->jl, "generating texture (%d,%d)",width,height);
	// Make the texture.
	jlgr_opengl_texture_new_(jlgr, &texture, pixels, width, height, bytepp);
	jl_print_return(jlgr->jl, "GL_MkTex");
	return texture;
}

//Lower Level Stuff
static inline void jl_gl_usep__(jlgr_t* jlgr, GLuint prg) {
#ifdef JL_DEBUG
	if(!prg) {
		jl_print(jlgr->jl, "shader program uninit'd!");
		exit(-1);
	}
#endif
	glUseProgram(prg);
	JL_GL_ERROR(jlgr, prg, "glUseProgram");
}

/**
 * Set a uniform variable in a shader to a float.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param x: The float value.
**/
void jlgr_opengl_uniform1(jlgr_t* jlgr, int32_t uv, float x) {
	glUniform1f(uv, x);
	JL_GL_ERROR(jlgr, uv, "glUniform1f");
}

/**
 * Set a uniform variable in a shader to a vec3.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param x, y, z: The vec3 value.
**/
void jlgr_opengl_uniform3(jlgr_t* jlgr, int32_t uv, float x, float y, float z){
	glUniform3f(uv, x, y, z);
	JL_GL_ERROR(jlgr, uv, "glUniform3f");
}

/**
 * Set the effect uniform variable in a shader to a vec4.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param x, y, z, w: The vec4 value.
**/
void jlgr_opengl_uniform4(jlgr_t* jlgr, int32_t uv, float x, float y, float z,
	float w)
{
	glUniform4f(uv, x, y, z, w);
	JL_GL_ERROR(jlgr, uv, "glUniform4f");
}

/**
 * Set the effect uniform variable in a shader to a mat4.
 * @param jlgr: The library context.
 * @param uv: The uniform variable.
 * @param m: The mat4 value.
**/
void jlgr_opengl_uniformM(jlgr_t* jlgr, int32_t uv, float m[]) {
	glUniformMatrix4fv(uv, 1, 1, m);
	JL_GL_ERROR(jlgr, uv, "glUniformMatrix4fv");
}

//This pushes VBO "buff" up to the shader's vertex attribute "vertexAttrib"
//Set xyzw to 2 if 2D coordinates 3 if 3D. etc.
void jlgr_opengl_setv(jlgr_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw)
{
	// Bind Buffer
	jl_gl_buffer_use__(jlgr, buff);
	// Set Vertex Attrib Pointer
	glEnableVertexAttribArray(vertexAttrib);
	JL_GL_ERROR(jlgr, vertexAttrib,"glEnableVertexAttribArray");
	glVertexAttribPointer(
		vertexAttrib,	// attribute
		xyzw,		// x+y+z = 3
		GL_FLOAT,	// type
		GL_FALSE,	// normalized?
		0,		// stride
		0		// array buffer offset
	);
	JL_GL_ERROR(jlgr, 0,"glVertexAttribPointer");
}

void jlgr_opengl_draw_arrays_(jlgr_t* jlgr, GLenum mode, uint8_t count) {
	glDrawArrays(mode, 0, count);
	JL_GL_ERROR(jlgr, 0,"glDrawArrays");
}

static inline void _jl_gl_init_disable_extras(jlgr_t* jlgr) {
	glDisable(GL_DEPTH_TEST);
	JL_GL_ERROR(jlgr, 0, "glDisable(GL_DEPTH_TEST)");
	glDisable(GL_DITHER);
	JL_GL_ERROR(jlgr, 0, "glDisable(GL_DITHER)");
}

static inline void _jl_gl_init_enable_alpha(jlgr_t* jlgr) {
	glEnable(GL_BLEND);
	JL_GL_ERROR(jlgr, 0,"glEnable( BLEND )");
	glEnable(GL_CULL_FACE);
	JL_GL_ERROR(jlgr, 0,"glEnable( CULL FACE )");
	glBlendColor(0.f,0.f,0.f,0.f);
	JL_GL_ERROR(jlgr, 0,"glBlendColor");
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA,
		GL_DST_ALPHA);
	JL_GL_ERROR(jlgr, 0,"glBlendFunc");
}

// Copy & Push vertices to a VBO.
void jlgr_opengl_vertices_(jlgr_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl)
{
	uint16_t items = (vertices*3);

	// Copy Vertices
	jl_mem_copyto(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	jlgr_opengl_buffer_set_(jlgr, gl, cv, items);
}

// TODO: MOVE
void jl_gl_pbo_new(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp)
{
	jl_print_function(jlgr->jl, "GL_PBO_NEW");
	jl_gl_texture_make__(jlgr, &(texture->gl_texture));
	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	jl_gl_texpar_set__(jlgr);
	jl_gl_texture_set__(jlgr, pixels, w, h, bpp);
	jl_print_return(jlgr->jl, "GL_PBO_NEW");
}

// TODO: MOVE
void jl_gl_pbo_set(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp)
{
	GLenum format = GL_RGBA;

	if(bpp == 3) format = GL_RGB;

	// Bind Texture
	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	// Copy to texture.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE,
		pixels);
	JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glTexSubImage2D");
}

/************************/
/***  ETOM Functions  ***/
/************************/

void jlgr_opengl_viewport_(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	glViewport(0, 0, w, h);
	JL_GL_ERROR(jlgr, w * h, "glViewport");
}

void jl_opengl_framebuffer_make_(jlgr_t* jlgr, uint32_t *fb) {
	glGenFramebuffers(1, fb);
	if(!(*fb)) {
		jl_print(jlgr->jl, "jl_gl_framebuffer_make__: GL FB = 0");
		exit(-1);
	}
	JL_GL_ERROR(jlgr, *fb,"glGenFramebuffers");
}

void jlgr_opengl_framebuffer_bind_(jlgr_t* jlgr, uint32_t fb) {
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	JL_GL_ERROR(jlgr, fb, "glBindFramebuffer");
}

void jlgr_opengl_framebuffer_addtx_(jlgr_t* jlgr, uint32_t tx) {
	// Set "*tex" as color attachment #0.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tx, 0);
	JL_GL_ERROR(jlgr, tx,
		"jlgr_opengl_framebuffer_addtx_: glFramebufferTexture2D");
}

void jlgr_opengl_framebuffer_subtx_(jlgr_t* jlgr) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, 0, 0);
	JL_GL_ERROR(jlgr, 0,
		"jlgr_opengl_framebuffer_subtx_: glFramebufferTexture2D");
}

void jlgr_opengl_framebuffer_status_(jlgr_t* jlgr) {
	// Check to see if framebuffer was made properly.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		jl_print(jlgr->jl, "Frame buffer not complete!");
		exit(-1);
	}
}

void jl_opengl_framebuffer_free_(jlgr_t* jlgr, uint32_t *fb) {
	glDeleteFramebuffers(1, fb);
	JL_GL_ERROR(jlgr, *fb, "glDeleteFramebuffers");
	*fb = 0;
}

// Set the viewport to the screen size.
void jl_gl_viewport_screen(jlgr_t* jlgr) {
	jlgr_opengl_viewport_(jlgr, jlgr->wm.w, jlgr->wm.h);
}

/**
 * Set an objects scale, rotation, and translation for a shader.
 * Set Camera's Rotation
 * @param jlgr: The library context.
 * @param 
 * @param fov: Field of View.
 * @param ar: Aspect ratio
 * @param near: The near clipping pane.
 * @param far: The far clipping pane.
**/
void jlgr_opengl_matrix(jlgr_t* jlgr, jlgr_glsl_t* sh, jl_vec3_t scalev,
	jl_vec3_t rotatev, jl_vec3_t translatev, jl_vec3_t lookv,
	float fov, float ar, float near, float far)
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
		1.f, 0.f, 0.f, (translatev.x * 2.f) - 1.f,
		0.f, 1.f, 0.f, (translatev.y * 2.f / ar) - 1.f,
		0.f, 0.f, 1.f, (translatev.z * 2.f),
		0.f, 0.f, 0.f, 1.f
	};
	float rotate_scene[] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	// Projection Matrix
	near = 0.1f;
	far = 100.f;
	float angleOfView = 90.f;
	float do_scale = 1.f / tan(angleOfView * 0.5f * M_PI / 180.f);

	float projection[] = {
		do_scale, 0.f, 0.f, 0.f,
		0.f, do_scale, 0.f, 0.f,
		0.f, 0.f, 1.f, -1.f,
		0.f, 0.f, -1.f, 1.f
	};
	glUniformMatrix4fv(sh->uniforms.scale_object, 1, 1, scale);
	JL_GL_ERROR(jlgr, 0, "matrix_object - scale");
	glUniformMatrix4fv(sh->uniforms.rotate_object, 1, 1, rotate_object);
	JL_GL_ERROR(jlgr, 0, "matrix_object - rotate");
	glUniformMatrix4fv(sh->uniforms.translate_object, 1, 1, translate);
	JL_GL_ERROR(jlgr, 0, "matrix_object - translate");
	glUniformMatrix4fv(sh->uniforms.rotate_camera, 1, 1, rotate_scene);
	JL_GL_ERROR(jlgr, 0, "matrix_object - rotate world");
	glUniformMatrix4fv(sh->uniforms.project_scene, 1, 1, projection);
	JL_GL_ERROR(jlgr, 0, "matrix_object - projection");
}

/**
 * Bind shader ( Prepare to draw ).
 * @param jlgr: The library context.
 * @param sh: The shader to use.
**/
void jlgr_opengl_draw1(jlgr_t* jlgr, jlgr_glsl_t* sh) {
	// Select Shader.
	jl_gl_usep__(jlgr, sh->program);
}

static int32_t _jl_gl_getu(jlgr_t* jlgr, GLuint prg, const char *var) {
	int32_t a = glGetUniformLocation(prg, var);
#ifdef JL_DEBUG
	if(a == -1) {
		jl_print(jlgr->jl, ":opengl: bad name; is: %s", var);
		exit(-1);
	}
	jlgr_opengl_error__(jlgr, a,"glGetUniformLocation");
#endif
	return a;
}

void _jl_gl_geta(jlgr_t* jlgr, GLuint prg, int32_t *attrib, const char *title) {
	if((*attrib = glGetAttribLocation(prg, title)) == -1) {
		jl_print(jlgr->jl, "for name \"%s\":", title);
		jl_print(jlgr->jl,
			"attribute name is either reserved or non-existant");
		exit(-1);
	}
}

/***	  @cond	   ***/
/************************/
/*** Static Functions ***/
/************************/

static inline void _jl_gl_init_setup_gl(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "setting properties....");
	//Disallow Dither & Depth Test
	_jl_gl_init_disable_extras(jlgr);
	//Set alpha=0 to transparent
	_jl_gl_init_enable_alpha(jlgr);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	JL_PRINT_DEBUG(jlgr->jl, "set glproperties.");
}

/**
 * Create a GLSL shader program.
 * @param jlgr: The library context.
 * @param glsl: The jlgr_glsl_t object to initialize.
 * @param vert: The vertex shader - NULL for texture support.
 * @param frag: The fragment shader.
 * @param has_tex: Whether has texture or not.
**/
void jlgr_opengl_shader_init(jlgr_t* jlgr, jlgr_glsl_t* glsl, const char* vert,
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
void jlgr_opengl_shader_uniform(jlgr_t* jlgr, jlgr_glsl_t* glsl,
	int32_t* uniform, const char* name)
{
	*uniform = _jl_gl_getu(jlgr, glsl->program, name);
}

static inline void _jl_gl_init_shaders(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "Making Shader: texture");
	jlgr_opengl_shader_init(jlgr, &jlgr->gl.prg.texture, NULL,
		JL_SHADER_TEX_FRAG, 1);
	JL_PRINT_DEBUG(jlgr->jl, "Making Shader: color");
	jlgr_opengl_shader_init(jlgr, &jlgr->gl.prg.color, JL_SHADER_CLR_VERT,
		JL_SHADER_CLR_FRAG, 0);
	JL_PRINT_DEBUG(jlgr->jl, "Set up shaders!");
}

//Load and create all resources
static inline void _jl_gl_make_res(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "GL_Init");
	// Setup opengl properties
	_jl_gl_init_setup_gl(jlgr);
	// Create shaders and set up attribute/uniform variable communication
	_jl_gl_init_shaders(jlgr);
	//
	JL_PRINT_DEBUG(jlgr->jl, "making temporary vertex object....");
	jlgr_vo_init(jlgr, &jlgr->gl.temp_vo);
	JL_PRINT_DEBUG(jlgr->jl, "making default texc buff!");
	// Default GL Texture Coordinate Buffer
	jlgr_opengl_buffer_set_(jlgr, &jlgr->gl.default_tc, DEFAULT_TC, 8);
	jlgr_opengl_buffer_set_(jlgr, &jlgr->gl.upsidedown_tc, UPSIDEDOWN_TC, 8);
	JL_PRINT_DEBUG(jlgr->jl, "made temp vo & default tex. c. buff!");
	jl_print_return(jlgr->jl, "GL_Init");
}

/**	  @endcond	  **/
/************************/
/*** Global Functions ***/
/************************/

/**
 * Get the Aspect Ratio of the pre-renderer in use.
 * @param jlgr: The library context.
**/
float jl_gl_ar(jlgr_t* jlgr) {
	uint8_t thread = jl_thread_current(jlgr->jl);

	if(thread)
		return jlgr->gl.cp ? jlgr->gl.cp->ar : jlgr->wm.ar;
	else
		return jlgr->wm.ar;
}

/**
 * Get the Width for the pre-renderer in use.
 * @param jlgr: The library context.
**/
uint32_t jl_gl_w(jlgr_t* jlgr) {
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
void jl_gl_clear(jlgr_t* jlgr, float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClearColor");
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClear");
}

/***	  @cond	   ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void jl_gl_init__(jlgr_t* jlgr) {
#ifdef JL_GLTYPE_HAS_GLEW
	if(glewInit()!=GLEW_OK) {
		jl_print(jlgr->jl, "glew fail!(no sticky)");
		exit(-1);
	}
#endif
	jlgr->gl.cp = NULL;
	_jl_gl_make_res(jlgr);
	// Make sure no pre-renderer is activated.
	jlgr_pr_off(jlgr);
}

/**	  @endcond	  **/
/***   #End of File   ***/
