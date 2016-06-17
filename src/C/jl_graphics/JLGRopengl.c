#include "JLGRinternal.h"
#include "jl_opengl.h"

// Shader Code

#ifdef GL_ES_VERSION_2_0
	#define GLSL_HEAD "#version 100\nprecision highp float;\n"
#else
	#define GLSL_HEAD "#version 100\n"
#endif

static char *source_frag_clr = 
	GLSL_HEAD
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vec4(vcolor.rgb * vcolor.a, vcolor.a);\n"
	"}\n\0";
	
static char *source_vert_clr = 
	GLSL_HEAD
	"uniform vec3 translate;\n"
	"uniform vec4 transform;\n"
	"\n"
	"attribute vec3 position;\n"
	"attribute vec4 acolor;\n"
	"\n"
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = transform * vec4(position + translate, 1.0);\n"
	"	vcolor = acolor;\n"
	"}\n\0";

static char *source_frag_tex_premult = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform float multiply_alpha;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	vcolor.a *= multiply_alpha;\n"
	"	gl_FragColor = vec4(vcolor.rgb * vcolor.a, vcolor.a);\n"
	"}\n\0";

static char *source_frag_tex_charmap = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform float multiply_alpha;\n"
	"uniform vec4 new_color;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	if((vcolor.r < 0.1) && (vcolor.g < 0.1) &&"
	"	   (vcolor.b < 0.1) && (vcolor.a > .9))\n"
	"		vcolor = new_color;\n"
	"	vcolor.a *= multiply_alpha;\n"
	"	gl_FragColor = vec4(vcolor.rgb * vcolor.a, vcolor.a);\n"
	"}\n\0";

static char *source_frag_tex = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = texture2D(texture, texcoord);\n"
	"}\n\0";

static char *source_vert_tex = 
	GLSL_HEAD
	"uniform vec3 translate;\n"
	"uniform vec4 transform;\n"
	"\n"
	"attribute vec3 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	texcoord = texpos;\n"
	"	gl_Position = transform * vec4(position + translate, 1.0);\n"
	"}\n\0";
	
// Full texture
static const float DEFAULT_TC[] = {
	0., 1.,
	0., 0.,
	1., 0.,
	1., 1.
};

// Prototypes:

//static void jl_gl_depthbuffer_set__(jlgr_t* jlgr, u16_t w, u16_t h);
//static void _jl_gl_depthbuffer_bind(jlgr_t* jlgr, uint32_t db);
//static void _jl_gl_depthbuffer_make(jlgr_t* jlgr, uint32_t *db);
//static void jl_gl_depthbuffer_off__(jlgr_t* jlgr);
static void jl_gl_framebuffer_addtx__(jlgr_t* jlgr, u32_t tx);
static void jl_gl_framebuffer_adddb__(jlgr_t* jlgr, u32_t db);
static void jl_gl_framebuffer_status__(jlgr_t* jlgr);
static void jl_gl_framebuffer_use__(jlgr_t* jlgr, jl_pr_t* pr);

// Definitions:
#ifdef JL_DEBUG_LIB
	#define JL_GL_ERROR(jlgr, x, fname) jl_gl_get_error___(jlgr, x, fname)
	#define JL_EGL_ERROR(jlgr, x, fname) jl_gl_egl_geterror__(jlgr, x, fname)
#else
	#define JL_GL_ERROR(jlgr, x, fname) ;
	#define JL_EGL_ERROR(jlgr, x, fname) ;
#endif

// Functions:

#ifdef JL_DEBUG_LIB
	static void jl_gl_get_error___(jlgr_t* jlgr, int width, str_t fname) {
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
		jl_sg_kill(jlgr->jl);
	}
#endif

static void jl_gl_buffer_use__(jlgr_t* jlgr, GLuint *buffer) {
	// Make buffer if not initialized.
	if(*buffer == 0) {
		glGenBuffers(1, buffer);
#ifdef JL_DEBUG_LIB
		JL_GL_ERROR(jlgr, 0,"buffer gen");
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

// Set the Data for VBO "buffer" to "buffer_data" with "buffer_size"
static void jl_gl_buffer_set__(jlgr_t* jlgr, GLuint *buffer,
	const void *buffer_data, u16_t buffer_size)
{
	//Bind Buffer "buffer"
	jl_gl_buffer_use__(jlgr, buffer);
	//Set the data
	glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), buffer_data,
		GL_DYNAMIC_DRAW);
	JL_GL_ERROR(jlgr, buffer_size, "buffer data");
}

static void jl_gl_buffer_old__(jlgr_t* jlgr, uint32_t *buffer) {
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
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					jl_print(jlgr->jl,
						"Could not compile shader:%s",buf);
					free(buf);
					jl_sg_kill(jlgr->jl);
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
	GLuint vertexShader = jl_gl_load_shader(jlgr, GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		jl_print(jlgr->jl, "couldn't load vertex shader");
		jl_sg_kill(jlgr->jl);
	}

	GLuint pixelShader = jl_gl_load_shader(jlgr, GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		jl_print(jlgr->jl, "couldn't load fragment shader");
		jl_sg_kill(jlgr->jl);
	}

	GLuint program = glCreateProgram();
	JL_GL_ERROR(jlgr, 0,"glCreateProgram");
	if (program) {
		GLint linkStatus = GL_FALSE;

		glAttachShader(program, vertexShader);
		JL_GL_ERROR(jlgr, 0,"glAttachShader (vertex)");
		glAttachShader(program, pixelShader);
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
					free(buf);
					jl_sg_kill(jlgr->jl);
				}else{
					jl_print(jlgr->jl, "failed malloc");
					jl_sg_kill(jlgr->jl);
				}
			}else{
				glDeleteProgram(program);
				jl_print(jlgr->jl, "no info log");
				jl_sg_kill(jlgr->jl);
			}
		}
	}else{
		jl_print(jlgr->jl, "Failed to load program");
		exit(-1);
	}
	return program;
}

static void jl_gl_texture_make__(jlgr_t* jlgr, uint32_t *tex) {
	glGenTextures(1, tex);
#ifdef JL_DEBUG_LIB
	if(!(*tex)) {
		JL_GL_ERROR(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
		jl_print(jlgr->jl, "jl_gl_texture_make__: GL tex = 0");
		exit(-1);
	}
	JL_GL_ERROR(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
#endif
}

// Set the bound texture.  pm is the pixels 0 - blank texture.
static void jl_gl_texture_set__(jlgr_t* jlgr, u8_t* pm, u16_t w, u16_t h,
	u8_t bytepp)
{
	GLenum format = bytepp == 3 ? GL_RGB : GL_RGBA;
	glTexImage2D(
		GL_TEXTURE_2D, 0,		/* target, level */
		format,				/* internal format */
		w, h, 0,			/* width, height, border */
		format, GL_UNSIGNED_BYTE,	/* external format, type */
		pm				/* pixels */
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

static void jl_gl_texture__bind__(jlgr_t* jlgr, uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	JL_GL_ERROR(jlgr, tex,"jl_gl_texture_bind__: glBindTexture");
}

// Bind a texture.
#ifdef JL_DEBUG_LIB
static void jl_gl_texture_bind__(jlgr_t* jlgr, uint32_t tex) {
	if(tex == 0) {
		jl_print(jlgr->jl, "jl_gl_texture_bind__: GL tex = 0");
		jl_sg_kill(jlgr->jl);
	}
	jl_gl_texture__bind__(jlgr, tex);
}
#else
	#define jl_gl_texture_bind__(jlgr, tex) jl_gl_texture__bind__(jlgr, tex)
#endif

// Unbind a texture
static void jl_gl_texture_off__(jlgr_t* jlgr) {
	jl_gl_texture__bind__(jlgr, 0);
}

// Make & Bind a new texture.
static void jl_gl_texture_new__(jlgr_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp)
{
	jl_print_function(jlgr->jl, "jl_gl_texture_new__");
	// Make the texture
	jl_gl_texture_make__(jlgr, tex);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, *tex);
	// Set texture
	jl_gl_texture_set__(jlgr, px, w, h, bytepp);
	// Set the texture parametrs.
	jl_gl_texpar_set__(jlgr);
	jl_print_return(jlgr->jl, "jl_gl_texture_new__");
}

/*
// Make & Bind a new depth buffer.
static void jl_gl_depthbuffer_new__(jlgr_t* jlgr,m_u32_t*db ,u16_t w,u16_t h) {
	// Make the depth buffer.
	_jl_gl_depthbuffer_make(jlgr, db);
	// Bind the depth buffer
	_jl_gl_depthbuffer_bind(jlgr, *db);
	// Set the depth buffer
	jl_gl_depthbuffer_set__(jlgr, w, h);
}
*/
// Make a texture - doesn't free "pixels"
uint32_t jl_gl_maketexture(jlgr_t* jlgr, void* pixels, int width, int height,
	u8_t bytepp)
{
	uint32_t texture;

	jl_print_function(jlgr->jl, "GL_MkTex");
	if (!pixels) {
		jl_print(jlgr->jl, "null pixels");
		jl_sg_kill(jlgr->jl);
	}
	JL_PRINT_DEBUG(jlgr->jl, "generating texture (%d,%d)",width,height);
	// Make the texture.
	jl_gl_texture_new__(jlgr, &texture, pixels, width, height, bytepp);
	jl_print_return(jlgr->jl, "GL_MkTex");
	return texture;
}

//Lower Level Stuff
static inline void _jl_gl_usep(jlgr_t* jlgr, GLuint prg) {
#ifdef JL_DEBUG_LIB
	if(!prg) {
		jl_print(jlgr->jl, "shader program uninit'd!");
		exit(-1);
	}
#endif
	glUseProgram(prg);
	JL_GL_ERROR(jlgr, prg, "glUseProgram");
}

static void _jl_gl_setalpha(jlgr_t* jlgr, float a) {
	glUniform1f(jlgr->gl.tex.uniforms.multiply_alpha, a);
	JL_GL_ERROR(jlgr, 0,"glUniform1f");
}

static void jl_gl_uniform3f__(jlgr_t* jlgr, GLint uv, float x, float y, float z)
{
	glUniform3f(uv, x, y, z);
	JL_GL_ERROR(jlgr, 0,"glUniform3f");
}

static void jl_gl_uniform4f__(jlgr_t* jlgr, GLint uv, float x, float y, float z,
	float w)
{
	glUniform4f(uv, x, y, z, w);
	JL_GL_ERROR(jlgr, 0,"glUniform4f");
}

//This pushes VBO "buff" up to the shader's vertex attribute "vertexAttrib"
//Set xyzw to 2 if 2D coordinates 3 if 3D. etc.
void _jl_gl_setv(jlgr_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
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

static void _jl_gl_draw_arrays(jlgr_t* jlgr, GLenum mode, uint8_t count) {
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
	JL_GL_ERROR(jlgr, 0,"glEnable( GL_BLEND )");
//	glEnable(GL_CULL_FACE);
//	JL_GL_ERROR(jlgr, 0,"glEnable( GL_CULL_FACE )");
	glBlendColor(0.f,0.f,0.f,0.f);
	JL_GL_ERROR(jlgr, 0,"glBlendColor");
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA,
		GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	JL_GL_ERROR(jlgr, 0,"glBlendFunc");
}

// Copy & Push vertices to a VBO.
static void jl_gl_vertices__(jlgr_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl)
{
	u16_t items = (vertices*3);

	// Copy Vertices
	jl_mem_copyto(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	jl_gl_buffer_set__(jlgr, gl, cv, items);
}

void jl_gl_vo_vertices(jlgr_t* jlgr, jl_vo_t* pv, const float *xyzw,
	uint8_t vertices)
{
	pv->vc = vertices;
	if(vertices) {
		// Re-Allocate pv->cv
		pv->cv = jl_mem(jlgr->jl, pv->cv, vertices * sizeof(float) * 3);
		// Set pv->cv & pv->gl
		jl_gl_vertices__(jlgr, xyzw, vertices, pv->cv, &pv->gl);
	}
}

void jl_gl_vo_free(jlgr_t* jlgr, jl_vo_t *pv) {
	// Free GL VBO
	jl_gl_buffer_old__(jlgr, &pv->gl);
	// Free GL Texture Buffer
	jl_gl_buffer_old__(jlgr, &pv->bt);
	// Free Converted Vertices & Colors
	if(pv->cv) pv->cv = jl_mem(jlgr->jl, pv->cv, 0);
	if(pv->cc) pv->cc = jl_mem(jlgr->jl, pv->cc, 0);
	// Free main structure
	pv = jl_mem(jlgr->jl, (void**)&pv, 0);
}

static void _jl_gl_col_begin(jlgr_t* jlgr, jl_vo_t* pv) {
	//Free anything old
	if(pv->cc != NULL) pv->cc = jl_mem(jlgr->jl, pv->cc, 0);
}

// TODO: MOVE
void jl_gl_pbo_new(jlgr_t* jlgr, jl_tex_t* texture, u8_t* pixels,
	u16_t w, u16_t h, u8_t bpp)
{
	jl_print_function(jlgr->jl, "GL_PBO_NEW");
	jl_gl_texture_make__(jlgr, &(texture->gl_texture));
	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	jl_gl_texpar_set__(jlgr);
	jl_gl_texture_set__(jlgr, pixels, w, h, bpp);
//	jl_gl_buffer_set__(jlgr,&texture->gl_buffer, pixels, w * h * bpp);
	jl_gl_buffer_use__(jlgr, &(texture->gl_buffer));
	glBufferData(GL_ARRAY_BUFFER, w * h * bpp, pixels, GL_DYNAMIC_DRAW);
	JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glBufferData");
	jl_print_return(jlgr->jl, "GL_PBO_NEW");
}
// TODO: MOVE
void jl_gl_pbo_set(jlgr_t* jlgr, jl_tex_t* texture, u8_t* pixels,
	u16_t w, u16_t h, u8_t bpp)
{
	GLenum format = GL_RGBA;

	if(bpp == 3) format = GL_RGB;

	// Bind Texture &
	//jl_gl_texture_bind__(jlgr, jlgr->gl.cp->tx);
	// Copy to texture.
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
	//	jlgr->gl.cp->w, jlgr->gl.cp->h,
	//	format, GL_UNSIGNED_BYTE, pixels);
	//JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glTexSubImage2D");

	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	jl_gl_buffer_use__(jlgr, &texture->gl_buffer);

	glBufferSubData(GL_ARRAY_BUFFER, 0, w * h * bpp, pixels);
	JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glBufferData");

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
		w, h, format, GL_UNSIGNED_BYTE, pixels);
	JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glTexSubImage2D");
}

static inline void jl_gl_viewport__(jlgr_t* jlgr, u16_t w, u16_t h) {
	glViewport(0, 0, w, h);
	JL_GL_ERROR(jlgr, w * h, "glViewport");
}

static void jl_gl_framebuffer_free__(jlgr_t* jlgr, m_u32_t *fb) {
	glDeleteFramebuffers(1, fb);
	JL_GL_ERROR(jlgr, *fb, "glDeleteFramebuffers");
	*fb = 0;
}

static void jl_gl_framebuffer_make__(jlgr_t* jlgr, m_u32_t *fb) {
	glGenFramebuffers(1, fb);
	if(!(*fb)) {
		jl_print(jlgr->jl, "jl_gl_framebuffer_make__: GL FB = 0");
		jl_sg_kill(jlgr->jl);
	}
	JL_GL_ERROR(jlgr, *fb,"glGenFramebuffers");
}

static void jl_gl_pr_obj_make_tx__(jlgr_t* jlgr, jl_pr_t *pr) {
	// Make a new texture for pre-renderering.  The "NULL" sets it blank.
	jl_gl_texture_new__(jlgr, &(pr->tx), NULL, pr->w, pr->h, 0);
	jl_gl_texture_off__(jlgr);
}

static void jl_gl_framebuffer_off__(jlgr_t* jlgr) {
	// Unbind the texture.
	jl_gl_texture_off__(jlgr);
	// Unbind the depthbuffer.
	// jl_gl_depthbuffer_off__(jlgr);
	// Unbind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	JL_GL_ERROR(jlgr, 0,"jl_gl_framebuffer_off__: glBindFramebuffer");
	// Render to the whole screen.
	jl_gl_viewport_screen(jlgr);
}

static void jl_gl_framebuffer_init__(jlgr_t* jlgr, jl_pr_t* pr) {
	if(pr->fb == 0 || pr->tx == 0/* || pr->db == 0*/) {
		// Make frame buffer
		jl_gl_framebuffer_make__(jlgr, &pr->fb);

		// Make a new Depthbuffer.
		//jl_gl_depthbuffer_new__(jlgr, &(pr->db), pr->w, pr->h);
		//jl_gl_depthbuffer_off__(jlgr);

		// Make the texture.
		jl_gl_pr_obj_make_tx__(jlgr, pr);

		// Make & Bind a new Framebuffer.
		// Recursively bind the framebuffer.
		jl_gl_framebuffer_use__(jlgr, pr);
		// Attach depth and texture buffer.
		jl_gl_framebuffer_addtx__(jlgr, pr->tx);
		jl_gl_framebuffer_adddb__(jlgr, pr->db);
		jl_gl_framebuffer_status__(jlgr);

		// Set Viewport to image and clear.
		jl_gl_viewport__(jlgr, pr->w, pr->h);
		// Clear the pre-renderer.
		jl_gl_clear(jlgr, 0, 0, 0, 0);
	}
}

static void jl_gl_framebuffer_use__(jlgr_t* jlgr, jl_pr_t* pr) {
	jl_print_function(jlgr->jl, "jl_gl_framebuffer_use__");
	jl_gl_framebuffer_init__(jlgr, pr);
	if(pr->w == 0) {
		jl_print(jlgr->jl,
		 "jl_gl_framebuffer_use__ failed: 'w' must be more than 0");
		jl_sg_kill(jlgr->jl);
	}else if(pr->h == 0) {
		jl_print(jlgr->jl,
		 "jl_gl_framebuffer_use__ failed: 'h' must be more than 0");
		jl_sg_kill(jlgr->jl);
	}else if((pr->w > GL_MAX_TEXTURE_SIZE)||(pr->h > GL_MAX_TEXTURE_SIZE)) {
		jl_print(jlgr->jl, "_jl_gl_pr_obj_make() failed:");
		jl_print(jlgr->jl, "w = %d,h = %d", pr->w, pr->h);
		jl_print(jlgr->jl, "texture is too big for graphics card.");
		jl_sg_kill(jlgr->jl);
	}
	// Bind the texture.
	jl_gl_texture_bind__(jlgr, pr->tx);
	// Bind the depthbuffer.
	//_jl_gl_depthbuffer_bind(jlgr, pr->db);
	// Bind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, pr->fb);
	JL_GL_ERROR(jlgr, pr->fb,"glBindFramebuffer");
	// Render on the whole framebuffer [ lower left -> upper right ]
	jl_gl_viewport__(jlgr, pr->w, pr->h);
	jl_print_return(jlgr->jl, "jl_gl_framebuffer_use__");
}

// add a texture to a framebuffer object.
static void jl_gl_framebuffer_addtx__(jlgr_t* jlgr, u32_t tx) {
	// Set "*tex" as color attachment #0.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tx, 0);
	JL_GL_ERROR(jlgr, tx,"jl_gl_framebuffer_addtx: glFramebufferTexture2D");
}

// add a depthbuffer to a framebuffer object.
static void jl_gl_framebuffer_adddb__(jlgr_t* jlgr, u32_t db) {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, db);
	JL_GL_ERROR(jlgr, db,"make pr: glFramebufferRenderbuffer");
}

static void jl_gl_framebuffer_status__(jlgr_t* jlgr) {
	// Check to see if framebuffer was made properly.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		jl_print(jlgr->jl, "Frame buffer not complete!");
		jl_sg_kill(jlgr->jl);
	}
}

static void _jl_gl_texture_free(jlgr_t* jlgr, uint32_t *tex) {
	glDeleteTextures(1, tex);
	JL_GL_ERROR(jlgr, 0, "_jl_gl_texture_free: glDeleteTextures");
	*tex = 0;
}

/*
static void _jl_gl_depthbuffer_free(jlgr_t* jlgr, uint32_t *db) {
	glDeleteRenderbuffers(1, db);
	JL_GL_ERROR(jlgr,*db,"_jl_gl_depthbuffer_free: glDeleteRenderbuffers");
	*db = 0;
}

static void _jl_gl_depthbuffer_make(jlgr_t* jlgr, uint32_t *db) {
	glGenRenderbuffers(1, db);
	if(!(*db)) {
		jl_print(jlgr->jl, "_jl_gl_depthbuffer_make: GL buff=0");
		jl_sg_kill(jlgr->jl);
	}
	JL_GL_ERROR(jlgr,*db,"make pr: glGenRenderbuffers");
}

static void jl_gl_depthbuffer_set__(jlgr_t* jlgr, u16_t w, u16_t h) {
	if(!w) {
		jl_print(jlgr->jl, "jl_gl_depthbuffer_set: w = 0");
		jl_sg_kill(jlgr->jl);
	}
	if(!h) {
		jl_print(jlgr->jl, "jl_gl_depthbuffer_set: h = 0");
		jl_sg_kill(jlgr->jl);
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	JL_GL_ERROR(jlgr, w, "make pr: glRenderbufferStorage");
}

static void _jl_gl_depthbuffer_bind(jlgr_t* jlgr, uint32_t db) {
	if(db == 0) {
		jl_print(jlgr->jl, "_jl_gl_depthbuffer_bind: GL db = 0");
		jl_sg_kill(jlgr->jl);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, db);
	JL_GL_ERROR(jlgr, db,"_jl_gl_depthbuffer_bind: glBindRenderbuffer");
}

static void jl_gl_depthbuffer_off__(jlgr_t* jlgr) {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	JL_GL_ERROR(jlgr, 0,"jl_gl_depthbuffer_off__: glBindRenderbuffer");
}
*/

static void _jl_gl_txtr(jlgr_t* jlgr, jl_vo_t** pv, uint8_t a, uint8_t is_rt) {
	if((*pv) == NULL) (*pv) = &jlgr->gl.temp_vo;
	// Set Simple Variabes
	(*pv)->a = ((float)a) / 255.f;
	// Make sure non-textured colors aren't attempted
	if((!is_rt) && ((*pv)->cc != NULL))
		(*pv)->cc = jl_mem(jlgr->jl, (*pv)->cc, 0);
}

static inline uint8_t _jl_gl_set_shader(jlgr_t* jlgr, jl_vo_t* pv) {
	uint8_t isTextured = pv->cc == NULL;
	_jl_gl_usep(jlgr, isTextured ? jlgr->gl.prgs.texture : jlgr->gl.prgs.color);
	return isTextured;
}

// Prepare to draw a solid color
static inline void _jl_gl_draw_colr(jlgr_t* jlgr, jl_vo_t* pv) {
	// Bind Colors to shader
	_jl_gl_setv(jlgr, &pv->bt, jlgr->gl.clr.attr.acolor, 4);
}

// Prepare to draw a texture with texture coordinates "tc". 
static void _jl_gl_draw_txtr(jlgr_t* jlgr, f32_t a, u32_t tx, uint32_t* tc) {
	jl_print_function(jlgr->jl, "OPENGL/Draw Texture");
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, tc, jlgr->gl.tex.attr.texpos, 2);
	// Set Alpha Value In Shader
	_jl_gl_setalpha(jlgr, a);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, tx);
	jl_print_return(jlgr->jl, "OPENGL/Draw Texture");
}

static void jl_gl_draw_vertices(jlgr_t* jlgr, uint32_t* gl, i32_t attr) {
	_jl_gl_setv(jlgr, gl, attr, 3);
}

static void jl_gl_draw_final__(jlgr_t* jlgr, u8_t rs, u32_t vc) {
	_jl_gl_draw_arrays(jlgr, rs ? GL_TRIANGLES : GL_TRIANGLE_FAN, vc);
}

static void _jl_gl_draw_onts(jlgr_t* jlgr, uint32_t* gl, uint8_t isTextured,
	u8_t rs, u32_t vc)
{
	// Update the position variable in shader.
	jl_gl_draw_vertices(jlgr, gl, isTextured ?
		jlgr->gl.tex.attr.position : jlgr->gl.clr.attr.position);
	// Draw the image on the screen!
	jl_gl_draw_final__(jlgr, rs, vc);
}

/************************/
/***  ETOM Functions  ***/
/************************/

// Set the viewport to the screen size.
void jl_gl_viewport_screen(jlgr_t* jlgr) {
	jl_gl_viewport__(jlgr, jlgr->wm.w, jlgr->wm.h);
}

void jl_gl_pr_use_(jlgr_t* jlgr, jl_pr_t* pr) {
	// Render to the framebuffer.
	if(pr) jl_gl_framebuffer_use__(jlgr, pr);
	else jl_gl_framebuffer_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = pr;
}

// Stop drawing to a pre-renderer.
void jl_gl_pr_off(jlgr_t* jlgr) {
	// Render to the screen
	jl_gl_framebuffer_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = NULL;
}

// Set vertices for a polygon.
void jl_gl_poly(jlgr_t* jlgr, jl_vo_t* pv, uint8_t vertices, const float *xyzw) {
	const float FS_RECT[] = {
		0.,jl_gl_ar(jlgr),0.,
		0.,0.,0.,
		1.,0.,0.,
		1.,jl_gl_ar(jlgr),0.
	};

	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	if(xyzw == NULL) xyzw = FS_RECT;
	// Rendering Style = polygon
	pv->rs = 0;
	// Set the vertices of vertex object "pv"
	jl_gl_vo_vertices(jlgr, pv, xyzw, vertices);
}

// Set vertices for vector triangles.
void jl_gl_vect(jlgr_t* jlgr, jl_vo_t* pv, uint8_t vertices, const float *xyzw) {
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Rendering Style = triangles
	pv->rs = 1;
	// Set the vertices of vertex object "pv"
	jl_gl_vo_vertices(jlgr, pv, xyzw, vertices);
}

// Set colors to "cc" in vertex oject "pv" - cc will be freed when done
void jl_gl_clrc(jlgr_t* jlgr, jl_vo_t* pv, jl_ccolor_t* cc) {
	_jl_gl_col_begin(jlgr, pv); // Free "pv->cc" if non-null
	pv->cc = cc;
	// Set Color Buffer "pv->bt" to "pv->cc"
	jl_gl_buffer_set__(jlgr, &pv->bt, pv->cc, pv->vc * 4);
}

//Convert color to solid
jl_ccolor_t* jl_gl_clrcs(jlgr_t* jlgr, u8_t *rgba, uint32_t vc) {
	int i;
	//Allocate memory
	jl_ccolor_t* cc = jl_memi(jlgr->jl, vc * sizeof(float) * 4);

	//Set RGBA for each vertex
	for(i = 0; i < vc; i++) { 
		cc[(i * 4) + 0] = ((double) rgba[0]) / 255.;
		cc[(i * 4) + 1] = ((double) rgba[1]) / 255.;
		cc[(i * 4) + 2] = ((double) rgba[2]) / 255.;
		cc[(i * 4) + 3] = ((double) rgba[3]) / 255.;
	}
	return cc;
}

//Convert Color To Gradient
jl_ccolor_t* jl_gl_clrcg(jlgr_t* jlgr, u8_t *rgba, uint32_t vc) {
	int i;
	//Allocate memory
	jl_ccolor_t* cc = jl_memi(jlgr->jl, vc * sizeof(float) * 4);

	//Set RGBA for each vertex
	for(i = 0; i < vc; i++) { 
		cc[(i * 4) + 0] = ((double) rgba[(i * 4) + 0]) / 255.;
		cc[(i * 4) + 1] = ((double) rgba[(i * 4) + 1]) / 255.;
		cc[(i * 4) + 2] = ((double) rgba[(i * 4) + 2]) / 255.;
		cc[(i * 4) + 3] = ((double) rgba[(i * 4) + 3]) / 255.;
	}
	return cc;
}

// Set Texturing to Gradient Color "rgba" { (4 * vertex count) values }
void jl_gl_clrg(jlgr_t* jlgr, jl_vo_t* pv, u8_t *rgba) {
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	_jl_gl_col_begin(jlgr, pv);
	jl_gl_clrc(jlgr, pv, jl_gl_clrcg(jlgr, rgba, pv->vc));
}

// Set Texturing to Solid Color "rgba" { 4 values }
void jl_gl_clrs(jlgr_t* jlgr, jl_vo_t* pv, u8_t *rgba) {
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	_jl_gl_col_begin(jlgr, pv);
	jl_gl_clrc(jlgr, pv, jl_gl_clrcs(jlgr, rgba, pv->vc));
}

// Set texturing to a bitmap
void jl_gl_txtr(jlgr_t* jlgr,jl_vo_t* pv,u8_t map,u8_t a, uint32_t tx) {
	_jl_gl_txtr(jlgr, &pv, a, 0);
	pv->tx = tx;
	if(!pv->tx) {
		jl_print(jlgr->jl, "Error: Texture=0!");
		jl_print_stacktrace(jlgr->jl);
		exit(-1);
	}
	jl_gl_vo_txmap(jlgr, pv, map);
}

// Set texturing to a bitmap
void jl_gl_txtr_(jlgr_t* jlgr, jl_vo_t* pv, u8_t map, u8_t a, u32_t tx) {
	_jl_gl_txtr(jlgr, &pv, a, 0);
	pv->tx = tx;
	jl_gl_vo_txmap(jlgr, pv, map);
}

//TODO:MOVE
// Shader true if texturing, false if coloring
// X,Y,Z are all [0. -> 1.]
// X,Y are turned into [-.5 -> .5] - center around zero.
static void jl_gl_translate__(jlgr_t* jlgr, int32_t shader, uint32_t which,
	float x, float y, float z, f64_t ar)
{
	// Determine which shader to use
	_jl_gl_usep(jlgr, which);
	// Set the uniforms
	glUniform3f(shader, x - (1./2.), y - (ar/2.), z);
	JL_GL_ERROR(jlgr, 0,"glUniform3f - translate");	
}

//TODO:MOVE
static void jl_gl_transform__(jlgr_t* jlgr, int32_t shader, uint32_t which,
	float x, float y, float z, f64_t ar)
{
	_jl_gl_usep(jlgr, which);
	glUniform4f(shader, 2. * x, 2. * (y / ar), 2. * z, 1.f);
	JL_GL_ERROR(jlgr, 0,"glUniform3f - transform");
}

void jl_gl_transform_pr_(jlgr_t* jlgr, jl_pr_t* pr, float x, float y, float z,
	float xm, float ym, float zm)
{
	jl_print_function(jlgr->jl, "OPENGL TRANSFORM!");
	f64_t ar = jl_gl_ar(jlgr);

	jl_gl_translate__(jlgr, jlgr->gl.prm.uniforms.translate,
		jlgr->gl.prgs.preblend, x, y, z, ar);
	jl_gl_transform__(jlgr, jlgr->gl.prm.uniforms.transform,
		jlgr->gl.prgs.preblend, xm, ym, zm, ar);
	jl_print_return(jlgr->jl, "OPENGL TRANSFORM!");
}

void jl_gl_transform_vo_(jlgr_t* jlgr, jl_vo_t* vo, float x, float y, float z,
	float xm, float ym, float zm)
{
	f64_t ar = jl_gl_ar(jlgr);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;

	jl_gl_translate__(jlgr, (vo->cc == NULL) ?
		jlgr->gl.tex.uniforms.translate:jlgr->gl.clr.uniforms.translate,
		(vo->cc == NULL) ? jlgr->gl.prgs.texture : jlgr->gl.prgs.color,
		x, y, z, ar);
	jl_gl_transform__(jlgr, (vo->cc == NULL) ?
		jlgr->gl.tex.uniforms.transform:jlgr->gl.clr.uniforms.transform,
		(vo->cc == NULL) ? jlgr->gl.prgs.texture : jlgr->gl.prgs.color,
		xm, ym, zm, ar);
}

void jl_gl_transform_chr_(jlgr_t* jlgr, float x, float y, float z,
	float xm, float ym, float zm)
{
	f64_t ar = jl_gl_ar(jlgr);

	jl_gl_translate__(jlgr, jlgr->gl.chr.uniforms.translate,
		jlgr->gl.prgs.character, x, y, z, ar);
	jl_gl_transform__(jlgr, jlgr->gl.chr.uniforms.transform,
		jlgr->gl.prgs.character, xm, ym, zm, ar);
}

//Draw object with "vertices" vertices.  The vertex data is in "x","y" and "z".
//"map" refers to the charecter map.  0 means don't zoom in to one character.
//Otherwise it will zoom in x16 to a single charecter
/**
 * If "pv" is NULL then draw what's on the temporary buffer
 * Else render vertex object "pv" on the screen.
*/
void jl_gl_draw(jlgr_t* jlgr, jl_vo_t* pv) {
	jl_print_function(jlgr->jl, "OPENGL/Draw");
	// Use Temporary Vertex Object If no vertex object.
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Determine which shader to use: texturing or coloring?
	uint8_t isTextured = _jl_gl_set_shader(jlgr, pv);
	// Set texture and transparency if texturing.  If colors: bind colors
	if(pv->cc) _jl_gl_draw_colr(jlgr, pv);
	else _jl_gl_draw_txtr(jlgr, pv->a, pv->tx, &pv->bt);
	// Draw onto the screen.
	_jl_gl_draw_onts(jlgr, &pv->gl, isTextured, pv->rs, pv->vc);
	jl_print_return(jlgr->jl, "OPENGL/Draw");
}

/**
 * If "pv" is NULL then draw what's on the temporary buffer
 * Else render vertex object "pv" on the screen.
*/
void jl_gl_draw_chr(jlgr_t* jlgr, jl_vo_t* pv,
	m_f32_t r, m_f32_t g, m_f32_t b, m_f32_t a)
{
	// Use Temporary Vertex Object If no vertex object.
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Set Shader
	_jl_gl_usep(jlgr, jlgr->gl.prgs.character);
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, &pv->bt, jlgr->gl.chr.attr.texpos, 2);
	// Set Alpha Value In Shader
	glUniform1f(jlgr->gl.chr.uniforms.multiply_alpha, pv->a);
	JL_GL_ERROR(jlgr, 0,"jl_gl_draw_chr: glUniform1f");
	// Set New Color In Shader
	jl_gl_uniform4f__(jlgr, jlgr->gl.chr.uniforms.new_color, r, g, b, a);
	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, pv->tx);
	JL_GL_ERROR(jlgr, pv->tx, "jl_gl_draw_chr: glBindTexture");
	// Update the position variable in shader.
	jl_gl_draw_vertices(jlgr, &pv->gl, jlgr->gl.chr.attr.position);
	// Draw the image on the screen!
	jl_gl_draw_final__(jlgr, pv->rs, pv->vc);
}

// Draw the pre-rendered texture.
void jl_gl_draw_pr_(jl_t* jl, jl_pr_t* pr) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jlgr->jl, "DRAW PR!");
	// Initialize Framebuffer, if not already init'd
	jl_gl_framebuffer_init__(jlgr, pr);
	// Use pre-mixed texturing shader.
	_jl_gl_usep(jlgr, jlgr->gl.prgs.preblend);
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, &jlgr->gl.default_tc, jlgr->gl.prm.attr.texpos, 2);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, pr->tx);
	// Update the position variable in shader.
	_jl_gl_setv(jlgr, &pr->gl, jlgr->gl.prm.attr.position, 3);
	// Draw the image on the screen!
	_jl_gl_draw_arrays(jlgr, GL_TRIANGLE_FAN, 4);
	jl_print_return(jlgr->jl, "DRAW PR!");
}

int32_t _jl_gl_getu(jlgr_t* jlgr, GLuint prg, char *var) {
	int32_t a = 0;
	if((a = glGetUniformLocation(prg, var)) == -1) {
		jl_print(jlgr->jl, ":opengl: bad name; is: %s", var);
		jl_sg_kill(jlgr->jl);
	}
	JL_GL_ERROR(jlgr, a,"glGetUniformLocation");
	return a;
}

void _jl_gl_geta(jlgr_t* jlgr, GLuint prg, m_i32_t *attrib, const char *title) {
	if((*attrib = glGetAttribLocation(prg, title)) == -1) {
		jl_print(jlgr->jl, 
			"attribute name is either reserved or non-existant");
		jl_sg_kill(jlgr->jl);
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

static inline void _jl_gl_init_shaders(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "making GLSL programs....");
	jlgr->gl.prgs.preblend = jl_gl_glsl_prg_create(jlgr,
		source_vert_tex, source_frag_tex);
	jlgr->gl.prgs.texture = jl_gl_glsl_prg_create(jlgr,
		source_vert_tex, source_frag_tex_premult);
	jlgr->gl.prgs.character = jl_gl_glsl_prg_create(jlgr,
		source_vert_tex, source_frag_tex_charmap);
	jlgr->gl.prgs.color = jl_gl_glsl_prg_create(jlgr,
		source_vert_clr, source_frag_clr);
	JL_PRINT_DEBUG(jlgr->jl, "made programs.");

	JL_PRINT_DEBUG(jlgr->jl, "setting up shaders....");
	// Texture
	jlgr->gl.prm.uniforms.textures =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.preblend, "texture");
	jlgr->gl.tex.uniforms.textures =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.texture, "texture");
	jlgr->gl.chr.uniforms.textures =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.character, "texture");
	// Multipy alpha
	jlgr->gl.tex.uniforms.multiply_alpha =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.texture,
			"multiply_alpha");
	jlgr->gl.chr.uniforms.multiply_alpha =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.character,
			"multiply_alpha");
	// Translate Vector
	jlgr->gl.prm.uniforms.translate =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.preblend, "translate");
	jlgr->gl.tex.uniforms.translate =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.texture, "translate");
	jlgr->gl.chr.uniforms.translate =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.character, "translate");
	jlgr->gl.clr.uniforms.translate =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.color, "translate");
	// Transform Vector
	jlgr->gl.prm.uniforms.transform =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.preblend, "transform");
	jlgr->gl.tex.uniforms.transform =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.texture, "transform");
	jlgr->gl.chr.uniforms.transform =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.character, "transform");
	jlgr->gl.clr.uniforms.transform =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.color, "transform");
	// New Color
	jlgr->gl.chr.uniforms.new_color =
		_jl_gl_getu(jlgr, jlgr->gl.prgs.character, "new_color");
	//
	JL_PRINT_DEBUG(jlgr->jl, "setting up prm shader attrib's....");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.preblend,
		&jlgr->gl.prm.attr.position, "position");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.preblend,
		&jlgr->gl.prm.attr.texpos, "texpos");
	JL_PRINT_DEBUG(jlgr->jl, "setting up tex shader attrib's....");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.texture,
		&jlgr->gl.tex.attr.position, "position");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.texture,
		&jlgr->gl.tex.attr.texpos, "texpos");
	JL_PRINT_DEBUG(jlgr->jl, "setting up chr shader attrib's....");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.character,
		&jlgr->gl.chr.attr.position, "position");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.character,
		&jlgr->gl.chr.attr.texpos, "texpos");
	JL_PRINT_DEBUG(jlgr->jl, "setting up clr shader attrib's....");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.color,
		&jlgr->gl.clr.attr.position, "position");
	_jl_gl_geta(jlgr, jlgr->gl.prgs.color,
		&jlgr->gl.clr.attr.acolor, "acolor");
	JL_PRINT_DEBUG(jlgr->jl, "set up shaders.");
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
	jl_gl_vo_init(jlgr, &jlgr->gl.temp_vo);
	JL_PRINT_DEBUG(jlgr->jl, "making default texc buff!");
	// Default GL Texture Coordinate Buffer
	jl_gl_buffer_set__(jlgr, &jlgr->gl.default_tc, DEFAULT_TC, 8);
	JL_PRINT_DEBUG(jlgr->jl, "made temp vo & default tex. c. buff!");
	jl_print_return(jlgr->jl, "GL_Init");
}

static void jl_gl_pr_set__(jl_pr_t *pr, f32_t w, f32_t h, u16_t w_px) {
	const float ar = h / w; // Aspect Ratio.
	const float h_px = w_px * ar; // Height in pixels.

	// Set width and height.
	pr->w = w_px;
	pr->h = h_px;
	// Set aspect ratio.
	pr->ar = ar;
}

/**	  @endcond	  **/
/************************/
/*** Global Functions ***/
/************************/

/**
 * Create an empty vertex object.
 * @param jl: The library context.
 * @param vo: A uninitialized vertex object - to initailize with 0 vertices.
**/
void jl_gl_vo_init(jlgr_t* jlgr, jl_vo_t* vo) {
	// GL VBO
	vo->gl = 0;
	// GL Texture Coordinate Buffer
	vo->bt = 0;
	// Converted Vertices
	vo->cv = NULL;
	// Vertex Count
	vo->vc = 0;
	// Converted Colors
	vo->cc = NULL;
	// Rendering Style = Polygon
	vo->rs = 0;
	// Texture
	vo->tx = 0;
}

/**
 * Change the character map for a texture.
 * @param jl: The library context.
 * @param pv: The vertext object to change.
 * @param map: The character value to map.
**/
void jl_gl_vo_txmap(jlgr_t* jlgr, jl_vo_t* pv, u8_t map) {
	if(map) {
		int32_t cX = map%16;
		int32_t cY = map/16;
		double CX = ((double)cX)/16.;
		double CY = ((double)cY)/16.;
		float tex1[] = {
			(DEFAULT_TC[0]/16.) + CX, (DEFAULT_TC[1]/16.) + CY,
			(DEFAULT_TC[2]/16.) + CX, (DEFAULT_TC[3]/16.) + CY,
			(DEFAULT_TC[4]/16.) + CX, (DEFAULT_TC[5]/16.) + CY,
			(DEFAULT_TC[6]/16.) + CX, (DEFAULT_TC[7]/16.) + CY
		};
		jl_gl_buffer_set__(jlgr, &pv->bt, tex1, 8);
	}else{
		jl_gl_buffer_set__(jlgr, &pv->bt, DEFAULT_TC, 8);
	}
}

/**
 * Get the Aspect Ratio of the pre-renderer in use.
 * @param jl: The library context.
**/
double jl_gl_ar(jlgr_t* jlgr) {
	uint8_t thread = jl_thread_current(jlgr->jl);

	if(thread)
		return jlgr->gl.cp ? jlgr->gl.cp->ar : jlgr->wm.ar;
	else
		return jlgr->wm.ar;
}

/**
 * Get the Width for the pre-renderer in use.
**/
uint32_t jl_gl_w(jlgr_t* jlgr) {
	uint32_t w;

	w = jlgr->gl.cp ? jlgr->gl.cp->w : jlgr->wm.w;
	return w;
}

/**
 * Clear the screen with a color
 * @param jl: The library context.
 * @param r: The amount of red [ 0 - 255 ]
 * @param g: The amount of green [ 0 - 255 ]
 * @param b: The amount of blue [ 0 - 255 ]
 * @param a: The translucency [ 0 - 255 ]
**/
void jl_gl_clear(jlgr_t* jlgr, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	glClearColor(((double)r)/255., ((double)g)/255.,
		((double)b)/255., ((double)a)/255.);
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClearColor");
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClear");
}

/**
 * THREAD: Draw thread only.
 * Resize a prerenderer.
 * @param jl: The library context.
 * @param pr: The pre-renderer to resize.
 * @param w: The display width. [ 0. - 1. ]
 * @param h: The display height. [ 0. - 1. ]
 * @param w_px: The resolution in pixels along the x axis [ 1- ]
**/
void jl_gl_pr_rsz(jlgr_t* jlgr, jl_pr_t *pr, f32_t w, f32_t h, u16_t w_px) {
	const float xyzw[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f
	};

	// If pre-renderer is initialized, reset.
	if(pr->fb) {
		_jl_gl_texture_free(jlgr, &(pr->tx));
//		_jl_gl_depthbuffer_free(jlgr, &(pr->db));
		jl_gl_framebuffer_free__(jlgr, &(pr->fb));
		jl_gl_buffer_old__(jlgr, &(pr->gl));
	}
	// Create the VBO.
	jl_gl_vertices__(jlgr, xyzw, 4, pr->cv, &pr->gl);
	// Set width, height and aspect ratio.
	jl_gl_pr_set__(pr, w, h, w_px);
}

/**
 * THREAD: any
 * Make a new pre-renderer.  Call to jl_gl_pr_rsz(); is necessary after this.
 * @param jlgr: The library context.
 * @param w: The display width. [ 0. - 1. ]
 * @param h: The display height. [ 0. - 1. ]
 * @param w_px: The resolution in pixels along the x axis [ 1- ]
**/
void jl_gl_pr_new(jlgr_t* jlgr, jl_pr_t * pr, float w, float h, uint16_t w_px) {
	jl_print_function(jlgr->jl, "GL_PR_NEW");
	// Set the initial pr structure values - Nothings made yet.
	pr->tx = 0;
	pr->db = 0;
	pr->fb = 0;
	pr->gl = 0;
	// Clear pr->cv
	jl_mem_clr(pr->cv, 4*sizeof(float)*3);
	// Set width, height and aspect ratio.
	jl_gl_pr_set__(pr, w, h, w_px);
	// Set transformation
	pr->scl = (jl_vec3_t) { 1., 1., 1. };
	jl_print_return(jlgr->jl, "GL_PR_NEW");
}

/**
 * Draw a pre-rendered texture.
 * @param jlgr: The library context.
 * @param pr: The pre-rendered texture.
 * @param vec: The vector of offset/translation.
 * @param scl: The scale factor.
**/
void jl_gl_pr_draw(jlgr_t* jlgr, jl_pr_t* pr, jl_vec3_t* vec, jl_vec3_t* scl) {
	if(vec == NULL) {
		jl_gl_transform_pr_(jlgr, pr,
			0.f, 0.f, 0.f, 1., 1., 1.);
	}else if(scl == NULL) {
		jl_gl_transform_pr_(jlgr, pr,
			vec->x, vec->y, vec->z, 1., 1., 1.);
	}else{
		jl_gl_transform_pr_(jlgr, pr,
			vec->x, vec->y, vec->z, scl->x, scl->y, scl->z);	
	}
	jl_gl_draw_pr_(jlgr->jl, pr);
}

void jl_gl_pr(jlgr_t* jlgr, jl_pr_t * pr, jl_fnct par__redraw) {
	jl_pr_t* oldpr = jlgr->gl.cp;

	if(!pr) {
		jl_print(jlgr->jl, "Drawing on lost pre-renderer.");
		jl_sg_kill(jlgr->jl);
	}
	// Use the vo's pr
	jl_gl_pr_use_(jlgr, pr);
	// Render to the pr.
	par__redraw(jlgr->jl);
	// Go back to the previous pre-renderer.
	jl_gl_pr_use_(jlgr, oldpr);
}

/***	  @cond	   ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void jl_gl_resz__(jlgr_t* jlgr) {
}

void jl_gl_init__(jlgr_t* jlgr) {
#ifdef JL_GLTYPE_HAS_GLEW
	if(glewInit()!=GLEW_OK) {
		jl_print(jlgr->jl, "glew fail!(no sticky)");
		jl_sg_kill(jlgr->jl);
	}
#endif
	jlgr->gl.cp = NULL;
	_jl_gl_make_res(jlgr);
	//Set uniform values
	_jl_gl_usep(jlgr, jlgr->gl.prgs.color);
	jl_gl_uniform3f__(jlgr, jlgr->gl.clr.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->gl.clr.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);
	_jl_gl_usep(jlgr, jlgr->gl.prgs.preblend);
	jl_gl_uniform3f__(jlgr, jlgr->gl.prm.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->gl.prm.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);
	_jl_gl_usep(jlgr, jlgr->gl.prgs.texture);
	jl_gl_uniform3f__(jlgr, jlgr->gl.tex.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->gl.tex.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);
	// Make sure no pre-renderer is activated.
	jl_gl_pr_off(jlgr);
}

/**	  @endcond	  **/
/***   #End of File   ***/
