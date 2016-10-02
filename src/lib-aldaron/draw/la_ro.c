/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

#include <la_pr.h>
#include <la_ro.h>
#include <la_memory.h>

extern float la_banner_size;

static inline void jlgr_vo_bounding_box(la_window_t* jlgr, la_ro_t* ro,
	const float *xyzw, uint32_t vertices)
{
	int i;
	ro->pr.cb.pos = (la_v3_t) { xyzw[0], xyzw[1], xyzw[2] };
	ro->pr.cb.ofs = (la_v3_t) { xyzw[0], xyzw[1], xyzw[2] };
	for(i = 1; i < vertices; i++) {
		la_v3_t new_vertex = {
			xyzw[(3 * i)],
			xyzw[(3 * i) + 1],
			xyzw[(3 * i) + 2] };

		if(new_vertex.x < ro->pr.cb.pos.x)
			ro->pr.cb.pos.x = new_vertex.x;
		if(new_vertex.y < ro->pr.cb.pos.y)
			ro->pr.cb.pos.y = new_vertex.y;
		if(new_vertex.z < ro->pr.cb.pos.z)
			ro->pr.cb.pos.z = new_vertex.z;

		if(new_vertex.x > ro->pr.cb.ofs.x)
			ro->pr.cb.ofs.x = new_vertex.x;
		if(new_vertex.y > ro->pr.cb.ofs.y)
			ro->pr.cb.ofs.y = new_vertex.y;
		if(new_vertex.z > ro->pr.cb.ofs.z)
			ro->pr.cb.ofs.z = new_vertex.z;
	}
	la_math_v3_sub(&ro->pr.cb.ofs, ro->pr.cb.pos);
}

static void jlgr_vo_vertices__(la_window_t* jlgr, la_ro_t* ro, const float *xyzw,
	uint32_t vertices)
{
	ro->vc = vertices;
	if(vertices) {
		// Re-Allocate ro->cc
		ro->cc = la_memory_resize(ro->cc, vertices * sizeof(float) * 4);
		// Re-Allocate ro->cv
		ro->cv = la_memory_resize(ro->cv, vertices * sizeof(float) * 3);
		// Set ro->cv & ro->gl
		jlgr_opengl_vertices_(jlgr, xyzw, vertices, ro->cv, &ro->gl);
		// Set bounding box
		jlgr_vo_bounding_box(jlgr, ro, xyzw, vertices);
		// Update pre-renderer
		jlgr_pr_resize(jlgr, &ro->pr, ro->pr.cb.ofs.x, ro->pr.cb.ofs.y,
			jlgr->wm.w * ro->pr.cb.ofs.x);
	}
	la_ro_move(ro, (la_v3_t) { 0.f, 0.f, 0.f });
}

static void jlgr_vo_color_buffer__(la_window_t* jlgr, la_ro_t* ro, float* cc) {
	ro->tx = 0;
	la_memory_copy(cc, ro->cc, ro->vc * 4 * sizeof(float));
	// Set Color Buffer "ro->bt" to "ro->cc"
	jlgr_opengl_buffer_set_(jlgr, &ro->bt, ro->cc, ro->vc * 4);
}

// Set vertices for a polygon.
static void jlgr_vo_poly__(la_window_t* jlgr, la_ro_t* ro, uint32_t vertices,
	const float *xyzw)
{
	const float FS_RECT[] = {
		0.,jl_gl_ar(jlgr),0.,
		0.,0.,0.,
		1.,0.,0.,
		1.,jl_gl_ar(jlgr),0.
	};

	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	if(xyzw == NULL) xyzw = FS_RECT;
	// Rendering Style = polygon
	ro->rs = 0;
	// Set the vertices of vertex object "ro"
	jlgr_vo_vertices__(jlgr, ro, xyzw, vertices);
}

void jlgr_vo_init__(la_window_t* window, la_ro_t* ro) {
	// Don't do anything if already init'd
	if(ro->window) return;
	//
	ro->window = window;
	// GL VBO
	ro->gl = 0;
	// GL Texture Coordinate Buffer
	ro->bt = 0;
	// Converted Vertices
	ro->cv = NULL;
	// Vertex Count
	ro->vc = 0;
	// Converted Colors
	ro->cc = NULL;
	// Rendering Style = Polygon
	ro->rs = 0;
	// Texture
	ro->tx = 0;
}

void jlgr_vo_exit__(la_ro_t* ro, const char* error) {
	if(ro->window == NULL) la_panic("Vertex object uninit'd: %s\n", error);
}

/**
 * Resize a vertex object to a rectangle.
 * @param jlgr: The library context.
 * @param ro: The vertex object.
 * @param rc: Rectangle to set it to.
**/
void la_ro_rect(la_window_t* window, la_ro_t* ro, float w, float h) {
	float rectangle_coords[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f };

	jlgr_vo_init__(window, ro);
	// Overwrite the vertex object
	jlgr_vo_poly__(window, ro, 4, rectangle_coords);
}

/**
 * Set a Vertex object to vector graphics.
 * @param jlgr: The library context.
 * @param ro: The vertex object to set.
**/
void jlgr_vo_set_vg(la_window_t* jlgr, la_ro_t *ro, uint16_t tricount,
	float* triangles, float* colors, uint8_t multicolor)
{
	jlgr_vo_init__(jlgr, ro);
	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	// Rendering Style = triangles
	ro->rs = 1;
	// Set the vertices of vertex object "ro"
	jlgr_vo_vertices__(jlgr, ro, triangles, tricount * 3);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, ro, colors);
	else jlgr_vo_color_solid(jlgr, ro, colors);
}

/**
 * Set a vertex object to a rectangle.
 * @param jl: The library context.
 * @param ro: The vertex object
 * @param rc: The rectangle coordinates.
 * @param colors: The color(s) to use - [ R, G, B, A ]
 * @param multicolor: If 0: Then 1 color is used.
 *	If 1: Then 1 color per each vertex is used.
**/
void jlgr_vo_set_rect(la_window_t* jlgr, la_ro_t *ro, jl_rect_t rc, float* colors,
	uint8_t multicolor)
{
	jlgr_vo_init__(jlgr, ro);
	la_ro_rect(jlgr, ro, rc.w, rc.h);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, ro, colors);
	else jlgr_vo_color_solid(jlgr, ro, colors);
}

void la_ro_color_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h)
{
	jlgr_vo_init__(window, ro);
	la_ro_rect(window, ro, w, h);
	// Texture the vertex object
	jlgr_vo_color_gradient(window, ro, colors);
}

void la_ro_plain_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h)
{
	jlgr_vo_init__(window, ro);
	la_ro_rect(window, ro, w, h);
	// Texture the vertex object
	jlgr_vo_color_solid(window, ro, colors);
}

/**
 * Set the vertex object's texturing to an image.
 * @param jlgr: The library context.
 * @param ro: The vertex object to modify.
 * @param img: The image to display on the vertex object.
**/
void jlgr_vo_image(la_window_t* jlgr, la_ro_t *ro, uint32_t img) {
	jlgr_vo_init__(jlgr, ro);
	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	// Make sure non-textured colors aren't attempted
	ro->tx = img;
#ifdef JL_DEBUG
	if(!ro->tx) la_panic("Error: Texture=0!");
#endif
	jlgr_vo_txmap(jlgr, ro, 0, 0, 0, -1);
}

/**
 * Set a vertex object to an Image.
 *
 * @param jlgr: The library context
 * @param ro: The vertex object
 * @param rc: the rectangle to draw the image in.
 * @param tex:  the ID of the image.
**/
void jlgr_vo_set_image(la_window_t* jlgr, la_ro_t *ro, jl_rect_t rc, uint32_t tex) {
	jlgr_vo_init__(jlgr, ro);
	//From bottom left & clockwise
	float Oone[] = {
		rc.x,		rc.y + rc.h,	0.f,
		rc.x,		rc.y,		0.f,
		rc.x + rc.w,	rc.y,		0.f,
		rc.x + rc.w,	rc.y + rc.h,	0.f };
	// Overwrite the vertex object
	jlgr_vo_poly__(jlgr, ro, 4, Oone);
	// Texture the vertex object
	jlgr_vo_image(jlgr, ro, tex);
}

/**
 * Change the character map for a texture.
 * @param jl: The library context.
 * @param ro: The vertext object to change.
 * @param w: How many characters wide the texture is.
 * @param h: How many characters high the texture is.
 * @param map: The character value to map.  -1 for full texture.
**/
void jlgr_vo_txmap(la_window_t* jlgr, la_ro_t* ro, uint8_t orientation,
	uint8_t w, uint8_t h, int16_t map)
{
	jlgr_vo_init__(jlgr, ro);
	const float *tc = orientation ?
		( orientation == 1 ? UPSIDEDOWN_TC : BACKWARD_TC )
		: DEFAULT_TC;
	if(map != -1) {
		float ww = (float)w;
		float hh = (float)h;
		float CX = ((float)(map%w))/ww;
		float CY = ((float)(map/w))/hh;
		float tex1[] = {
			(tc[0]/ww) + CX, (tc[1]/hh) + CY,
			(tc[2]/ww) + CX, (tc[3]/hh) + CY,
			(tc[4]/ww) + CX, (tc[5]/hh) + CY,
			(tc[6]/ww) + CX, (tc[7]/hh) + CY
		};
		jlgr_opengl_buffer_set_(jlgr, &ro->bt, tex1, 8);
	}else{
		jlgr_opengl_buffer_set_(jlgr, &ro->bt, tc, 8);
	}
}

/**
 * Change the coloring scheme for a vertex object to a gradient.
 * @param jl: The library context.
 * @param ro: The Vertex Object
 * @param rgba: { (4 * vertex count) values }
**/
void jlgr_vo_color_gradient(la_window_t* jlgr, la_ro_t* ro, float* rgba) {
	jlgr_vo_init__(jlgr, ro);
	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	jlgr_vo_color_buffer__(jlgr, ro, rgba);
}

/**
 * Change the coloring scheme for a vertex object to a solid.
 * @param jl: The library context.
 * @param ro: The Vertex Object
 * @param rgba: { 4 values }
**/
void jlgr_vo_color_solid(la_window_t* jlgr, la_ro_t* ro, float* rgba) {
	jlgr_vo_init__(jlgr, ro);
	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	float rgbav[4 * ro->vc];
	uint32_t i;

	for(i = 0; i < ro->vc; i++) {
		la_memory_copy(rgba, &(rgbav[i * 4]), 4 * sizeof(float));
	}
	jlgr_vo_color_buffer__(jlgr, ro, rgbav);
}

/**
 * Move a vertex object to a new location.
 * @param ro: The vertex object.
 * @param pos: The new position.
**/
void la_ro_move(la_ro_t* ro, la_v3_t pos) {
	jlgr_vo_exit__(ro, "Can't Move!");
	ro->pr.cb.pos = pos;
	ro->pr.cb.pos.y += la_banner_size;
}

/**
 * Draw vertex object.
 * @param jlgr: The library context.
 * @param ro: The vertex object to draw.
 * @param sh: The shader to use ( must be the same one used with
 *	jlgr_opengl_draw1(). )
**/
void jlgr_vo_draw2(la_window_t* jlgr, la_ro_t* ro, jlgr_glsl_t* sh) {
	jlgr_vo_exit__(ro, "Can't Draw2!");
	// Use Temporary Vertex Object If no vertex object.
	if(ro == NULL) ro = &jlgr->gl.temp_vo;
	if(ro->tx) {
		// Bind Texture Coordinates to shader
		jlgr_opengl_setv(jlgr, &ro->bt,
			jlgr->effects.alpha.shader.attributes.texpos_color, 2);
		// Bind the texture
		jlgr_opengl_texture_bind_(jlgr, ro->tx);
	}else{
		// Bind Colors to shader
		jlgr_opengl_setv(jlgr, &ro->bt,
			jlgr->gl.prg.color.attributes.texpos_color, 4);
	}
	// Update the position variable in shader.
	jlgr_opengl_setv(jlgr, &ro->gl, sh->attributes.position, 3);
	// Draw the image on the screen!
	jlgr_opengl_draw_arrays_(jlgr, ro->rs ? GL_TRIANGLES : GL_TRIANGLE_FAN,
		ro->vc);
}

/**
 * Draw a vertex object with offset by translation.
 * @param jlgr: The library context.
 * @param ro: The vertex object to draw.
**/
void jlgr_vo_draw(la_window_t* jlgr, la_ro_t* ro) {
	if(ro->window == NULL) la_panic("Can't Draw");
	jlgr_vo_exit__(ro, "Can't Draw!");
	jlgr_glsl_t* shader = ro->tx ?
		&jlgr->gl.prg.texture : &jlgr->gl.prg.color;

	jlgr_opengl_draw1(jlgr, shader);
	jlgr_opengl_matrix(jlgr, shader,
		(la_v3_t) { 1.f, 1.f, 1.f }, // Scale
		(la_v3_t) { 0.f, 0.f, 0.f }, // Rotate
		ro->pr.cb.pos, // Translate
		(la_v3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	jlgr_vo_draw2(jlgr, ro, shader);
}

void la_ro_draw(la_ro_t* ro) {
	jlgr_vo_draw(ro->window, ro);
}

/**
 * Draw a vertex object with effects.
 * @param jlgr: The library context.
 * @param ro: The vertex object.
**/
void la_ro_pr_draw(la_ro_t* ro, uint8_t orient) {
	jlgr_vo_exit__(ro, "Can't Draw Pre-Rendered!");
	jlgr_pr_draw(ro->window, &ro->pr, ro->pr.cb.pos, orient);
}

void la_ro_pr(void* context, la_window_t* window, la_ro_t* ro, la_fn_t drawfn) {
	la_pr(context, window, &ro->pr, drawfn);
}

/**
 * Free a vertex object.
 * @param jl: The library context
 * @param ro: The vertex object to free
**/
void jlgr_vo_free(la_window_t* jlgr, la_ro_t *ro) {
	jlgr_vo_exit__(ro, "Can't Free!");
	// Free GL VBO
	jlgr_opengl_buffer_old_(jlgr, &ro->gl);
	// Free GL Texture Buffer
	jlgr_opengl_buffer_old_(jlgr, &ro->bt);
	// Free Converted Vertices & Colors
	if(ro->cv) ro->cv = la_memory_resize(ro->cv, 0);
	if(ro->cc) ro->cc = la_memory_resize(ro->cc, 0);
	// Free main structure
	la_memory_free(ro);
}

#endif
