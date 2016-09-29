/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_DRAW
#define LA_DRAW

#include "jl.h"

#include "SDL_events.h"
#include "la_safe.h"

#define JLGR_TEXT_CMD "\x01"
#define JLGR_TEXT_BOLD JLGR_TEXT_CMD "\x01"
#define JLGR_TEXT_ITALIC JLGR_TEXT_CMD "\x02"
#define JLGR_TEXT_THIN JLGR_TEXT_CMD "\x03"
#define JLGR_TEXT_NORMAL JLGR_TEXT_CMD "\x04"
#define JLGR_TEXT_ALIGNL JLGR_TEXT_CMD "\x10"
#define JLGR_TEXT_ALIGNC JLGR_TEXT_CMD "\x11"
#define JLGR_TEXT_ALIGNR JLGR_TEXT_CMD "\x12"
#define JLGR_TEXT_ALIGNJ JLGR_TEXT_CMD "\x13"

typedef enum {
	JL_SCR_UP,
	JL_SCR_DN,
	JL_SCR_SS,
}jlgr_which_screen_t;

// Types:

// Coordinate Structures
typedef struct{
	float x, y, w, h;
}jl_rect_t;

// Collision Box / Line / Etc.
typedef struct{
	jl_vec3_t pos; // Position ( X/Y/Z )
	jl_vec3_t ofs; // Position Offset ( W/H/D )
}jl_area_t;

// Graphical stuff

// Pre-renderer
typedef struct {
	// What to render
	uint32_t tx;	// ID to texture.
	uint32_t fb;	// ID to Frame Buffer
	uint16_t w, h;	// Width and hieght of texture
	// Render Area
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	float ar;	// Aspect Ratio: h:w
	float cv[4*3];	// Converted Vertices
	jl_area_t cb;	// 2D/3D collision box.
}jl_pr_t;

typedef struct{
	struct {
		int32_t position;
		int32_t texpos_color;
	}attributes;

	struct {
		int32_t texture;
		// Matrices
		int32_t scale_object;
		int32_t rotate_object;
		int32_t translate_object;
		int32_t rotate_camera;
		int32_t project_scene;
	}uniforms;

	uint32_t program;
}jlgr_glsl_t;

// jlgr_effects/types
#define JLGR_EFFECTS_TYPES
#include "la_effects.h"

//Vertex Object
typedef struct{
	void* window;	// Window
	// Basic:
	uint8_t rs;	// Rendering Style 0=GL_TRIANGLE_FAN 1=GL_TRIANGLES
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	uint32_t vc;	// # of Vertices
	float* cv;	// Converted Vertices
	uint32_t bt;	// Buffer for Texture coordinates or Color Vertices.
	// Coloring:
	float* cc;	// Colors
	// Texturing:
	uint32_t tx;	// ID to texture. [ 0 = Colors Instead ]
	// Pre-rendered effects (framebuffer):
	jl_pr_t pr;
}la_vo_t;

/**
 * Font type.
**/
typedef struct {
	/** The texture ID of the font. */
	int32_t tex;
	/** Whether to allow multiple colors ( gradient ). */
	uint8_t multicolor;
	/** The color value(s). */
	float* colors;
	/** The size to draw the text ( 0. - 1. ). */
	float size;
}jl_font_t;

typedef struct{
	char *opt;
	jl_fnct run;
}jl_popup_button_t;

#include "la_port.h"

typedef void (*la_draw_fn_t)(void* context, la_window_t* window);
typedef void (*jlgr_fnct)(la_window_t* window);

// la_draw.c
void la_draw_dont(void* context, la_window_t* window);

// JLGR.c:
float la_window_h(la_window_t* window);
void la_draw_fnchange(la_window_t* window, jl_fnct primary, jl_fnct secondary,
	jl_fnct resize);

// JLGRgraphics.c:
void jlgr_dont(la_window_t* jlgr);
void jlgr_fill_image_set(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, 
	int16_t c);
void jlgr_fill_image_draw(la_window_t* jlgr);
void jlgr_draw_bg(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, int16_t c);

// JLGRtext.c:
void jlgr_text_draw(la_window_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f);
void jlgr_draw_int(la_window_t* jlgr, int64_t num, jl_vec3_t loc, jl_font_t f);
void jlgr_draw_dec(la_window_t* jlgr, double num, uint8_t dec, jl_vec3_t loc,
	jl_font_t f);
void jlgr_draw_ctxt(la_window_t* jlgr, const char *str, float yy, float* color);
void jlgr_draw_loadscreen(la_window_t* jlgr, jl_fnct draw_routine);
void jlgr_draw_msge(la_window_t* jlgr, uint32_t tex, uint8_t c, char* format, ...);
void jlgr_term_msge(la_window_t* jlgr, char* message);
void jlgr_gui_textbox_init(la_window_t* jlgr, data_t* string);
uint8_t jlgr_gui_textbox_loop(la_window_t* jlgr);
void jlgr_gui_textbox_draw(la_window_t* jlgr, jl_rect_t rc);
void jlgr_notify(la_window_t* jlgr, const char* notification, ...);

// JLGRvo.c
void jlgr_vo_set_vg(la_window_t* jlgr, la_vo_t *vo, uint16_t tricount,
	float* triangles, float* colors, uint8_t multicolor);
void jlgr_vo_set_rect(la_window_t* jlgr, la_vo_t *vo, jl_rect_t rc, float* colors,
	uint8_t multicolor);
void jlgr_vo_image(la_window_t* jlgr, la_vo_t *vo, uint32_t img);
void jlgr_vo_set_image(la_window_t* jlgr, la_vo_t *vo, jl_rect_t rc, uint32_t tex);
void jlgr_vo_txmap(la_window_t* jlgr, la_vo_t* vo, uint8_t orientation,
	uint8_t w, uint8_t h, int16_t map);
void jlgr_vo_color_gradient(la_window_t* jlgr, la_vo_t* vo, float* rgba);
void jlgr_vo_color_solid(la_window_t* jlgr, la_vo_t* vo, float* rgba);
void jlgr_vo_move(la_vo_t* vo, jl_vec3_t pos);
void jlgr_vo_draw2(la_window_t* jlgr, la_vo_t* vo, jlgr_glsl_t* sh);
void jlgr_vo_draw(la_window_t* jlgr, la_vo_t* vo);
void jlgr_vo_draw_pr(la_window_t* jlgr, la_vo_t* vo);
void jlgr_vo_free(la_window_t* jlgr, la_vo_t *vo);

// JLGRpr.c
void jlgr_pr_off(la_window_t* jlgr);
void jlgr_pr_resize(la_window_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px);
void jlgr_pr_draw(la_window_t* jlgr, jl_pr_t* pr, jl_vec3_t vec, uint8_t orient);

// OpenGL
uint32_t la_texture_new(la_window_t* jlgr, uint8_t* pixels, uint16_t w, uint16_t h,
	uint8_t bpp);
void la_texture_set(la_window_t* jlgr, uint32_t texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp);
uint32_t jl_gl_maketexture(la_window_t* jlgr, void* pixels,
	uint32_t width, uint32_t height, uint8_t bytepp);
float jl_gl_ar(la_window_t* jlgr);
void jl_gl_clear(la_window_t* jlgr, float r, float g, float b, float a);

// JLGRopengl.c
void jlgr_opengl_uniform(la_window_t* jlgr, jlgr_glsl_t* glsl, float* x, uint8_t vec,
	const char* name, ...);
void jlgr_opengl_uniformi(la_window_t* jlgr, jlgr_glsl_t* glsl, int32_t* x,
	uint8_t vec, const char* name, ...);
void jlgr_opengl_uniform1(la_window_t* jlgr, uint8_t e, int32_t uv, float* x);
void jlgr_opengl_uniform1i(la_window_t* jlgr, uint8_t e, int32_t uv, int32_t* x);
void jlgr_opengl_uniform3(la_window_t* jlgr, uint8_t e, int32_t uv, float* xyz);
void jlgr_opengl_uniform4(la_window_t* jlgr, uint8_t e, int32_t uv, float* xyzw);
void jlgr_opengl_uniformM(la_window_t* jlgr, int32_t uv, float m[]);
void jlgr_opengl_shader_init(la_window_t* jlgr, jlgr_glsl_t* glsl, const char* vert,
	const char* frag, uint8_t has_tex);
void jlgr_opengl_shader_uniform(la_window_t* jlgr, jlgr_glsl_t* glsl,
	int32_t* uniform, const char* name);
void jlgr_opengl_draw1(la_window_t* jlgr, jlgr_glsl_t* sh);

// JLGReffects.c
void jlgr_effects_vo_alpha(la_window_t* jlgr, la_vo_t* vo, jl_vec3_t offs, float a);
void jlgr_effects_vo_hue(la_window_t* jlgr, la_vo_t* vo, jl_vec3_t offs, float c[]);
void jlgr_effects_vo_light(la_window_t* jlgr, la_vo_t* vo, jl_vec3_t offs,
	jl_vec3_t* material);
void jlgr_effects_hue(la_window_t* jlgr, float c[]);
void jlgr_effects_draw(la_window_t* jlgr, la_vo_t* vo);

void jlgr_effects_light(la_window_t* jlgr, jl_vec3_t* material);
void jlgr_effects_light_clear(la_window_t* jlgr);
void jlgr_effects_light_add(la_window_t* jlgr, jl_vec3_t point, float ambient[],
	float diffuse[], float specular[], float power);
void jlgr_effects_light_update(la_window_t* jlgr);

// SG
uint32_t jl_sg_add_image(la_window_t* jlgr, data_t* zipdata, const char* filename);

// JLGRwm.c
void jlgr_wm_setfullscreen(la_window_t* jlgr, uint8_t is);
void jlgr_wm_togglefullscreen(la_window_t* jlgr);
uint16_t jlgr_wm_getw(la_window_t* jlgr);
uint16_t jlgr_wm_geth(la_window_t* jlgr);
void jlgr_wm_setwindowname(la_window_t* jlgr, const char* window_name);

#endif
