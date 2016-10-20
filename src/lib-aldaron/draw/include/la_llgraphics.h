/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_LLGRAPHICS
#define LA_LLGRAPHICS

#include <la_draw.h>
#include <stdint.h>

void la_llgraphics_buffer_set_(uint32_t *buffer, const void *buffer_data,
	uint16_t buffer_size);
void la_llgraphics_buffer_free(uint32_t buffer);
void la_llgraphics_texture_bind(uint32_t tex);
void la_llgraphics_texture_unbind(void);
void la_llgraphics_texture_free(uint32_t tex);
void la_llgraphics_uniformf(la_window_t* window, jlgr_glsl_t* glsl, float* x,
	uint8_t num_elements, const char* name, int16_t index);
void la_llgraphics_uniformi(la_window_t* window, jlgr_glsl_t* glsl, int32_t* x,
	uint8_t num_elements, const char* name, int16_t index);
void la_llgraphics_attribute_set(uint32_t* buffer, uint32_t attrib,
	uint8_t elementc);
void la_llgraphics_push_vertices(const float *xyzw, uint32_t vertices,
	float* cv, uint32_t* gl);
void la_llgraphics_draw_triangles(uint32_t count);
void la_llgraphics_draw_trianglefan(uint32_t count);
void la_llgraphics_viewport(uint32_t w, uint32_t h);
uint32_t la_llgraphics_framebuffer_make(void);
void la_llgraphics_framebuffer_bind(uint32_t fb);
void la_llgraphics_framebuffer_addtx(uint32_t texture);
void la_llgraphics_framebuffer_free(uint32_t fb);
void la_llgraphics_setmatrix(jlgr_glsl_t* sh, la_v3_t scalev, la_v3_t rotatev,
	la_v3_t translatev, la_v3_t lookv, float ar);
void la_llgraphics_shader_bind(jlgr_glsl_t* sh);
void la_llgraphics_shader_make(jlgr_glsl_t* glsl, const char* vert,
	const char* frag, uint8_t has_tex);
void la_llgraphics_clear(float r, float g, float b, float a);

#endif
