/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

#include <la_ro.h>
#include <la_memory.h>

extern float la_banner_size;

void jlgr_opengl_viewport_(la_window_t* jlgr, uint16_t w, uint16_t h);
void jlgr_opengl_texture_new_(la_window_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp);
void jl_gl_texture_free_(la_window_t* jlgr, uint32_t *tex);
void jlgr_opengl_texture_off_(la_window_t* jlgr);
void jl_opengl_framebuffer_make_(la_window_t* jlgr, uint32_t *fb);
void jlgr_opengl_framebuffer_addtx_(la_window_t* jlgr, uint32_t tx);
void jlgr_opengl_framebuffer_status_(la_window_t* jlgr);
void jl_opengl_framebuffer_free_(la_window_t* jlgr, uint32_t *fb);
void jlgr_opengl_framebuffer_bind_(la_window_t* jlgr, uint32_t fb);

static void jlgr_pr_init__(la_window_t* jlgr, la_ro_t* ro);

static inline void jlgr_vo_bounding_box(la_window_t* jlgr, la_ro_t* ro,
	const float *xyzw, uint32_t vertices)
{
	int i;
	ro->cb.pos = (la_v3_t) { xyzw[0], xyzw[1], xyzw[2] };
	ro->cb.ofs = (la_v3_t) { xyzw[0], xyzw[1], xyzw[2] };
	for(i = 1; i < vertices; i++) {
		la_v3_t new_vertex = {
			xyzw[(3 * i)],
			xyzw[(3 * i) + 1],
			xyzw[(3 * i) + 2] };

		if(new_vertex.x < ro->cb.pos.x)
			ro->cb.pos.x = new_vertex.x;
		if(new_vertex.y < ro->cb.pos.y)
			ro->cb.pos.y = new_vertex.y;
		if(new_vertex.z < ro->cb.pos.z)
			ro->cb.pos.z = new_vertex.z;

		if(new_vertex.x > ro->cb.ofs.x)
			ro->cb.ofs.x = new_vertex.x;
		if(new_vertex.y > ro->cb.ofs.y)
			ro->cb.ofs.y = new_vertex.y;
		if(new_vertex.z > ro->cb.ofs.z)
			ro->cb.ofs.z = new_vertex.z;
	}
	la_math_v3_sub(&ro->cb.ofs, ro->cb.pos);
}

static void jlgr_pr_set__(la_ro_t *ro, float w, float h, uint16_t w_px) {
	const float ar = h / w; // Aspect Ratio.
	const float h_px = w_px * ar; // Height in pixels.

	// Set width and height.
	ro->pr.w = w_px;
	ro->pr.h = h_px;
	// Set aspect ratio.
	ro->pr.ar = ar;
}

static inline void la_ro_pr_resize__(la_window_t* jlgr, la_ro_t* ro, float w,
	float h, uint16_t w_px)
{
	const float xyzw[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f
	};

	// If pre-renderer is initialized, reset.
	if(ro->pr.fb) {
		jl_gl_texture_free_(jlgr, &(ro->pr.tx));
		jl_opengl_framebuffer_free_(jlgr, &(ro->pr.fb));
		la_llgraphics_buffer_free(ro->pr.gl);
	}
	// Create the VBO.
	jlgr_opengl_vertices_(jlgr, xyzw, 4, ro->pr.cv, &ro->pr.gl);
	// Set width, height and aspect ratio.
	jlgr_pr_set__(ro, w, h, w_px);
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
		la_ro_pr_resize__(jlgr, ro, ro->cb.ofs.x, ro->cb.ofs.y,
			jlgr->wm.w * ro->cb.ofs.x);
	}
	la_ro_move(ro, (la_v3_t) { 0.f, 0.f, 0.f });
}

static void jlgr_vo_color_buffer__(la_window_t* jlgr, la_ro_t* ro, float* cc) {
	ro->tx = 0;
	la_memory_copy(cc, ro->cc, ro->vc * 4 * sizeof(float));
	// Set Color Buffer "ro->bt" to "ro->cc"
	la_llgraphics_buffer_set_(&ro->bt, ro->cc, ro->vc * 4);
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

static void la_ro_init__(la_window_t* window, la_ro_t* ro) {
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

static inline la_window_t* la_ro_panic__(la_ro_t* ro, const char* error) {
#ifdef JL_DEBUG
	if(ro->window == NULL) la_panic("Render object uninit'd: %s\n", error);
#endif
	return ro->window;
}

static void jlgr_pr_use__(la_window_t* window, la_ro_t* ro) {
	jlgr_pr_init__(window, ro);
	if(ro->pr.w == 0) {
		la_panic("jlgr_pr_use__ failed: 'w' must be more than 0");
	}else if(ro->pr.h == 0) {
		la_panic("jlgr_pr_use__ failed: 'h' must be more than 0");
	}else if((ro->pr.w > GL_MAX_TEXTURE_SIZE)||(ro->pr.h > GL_MAX_TEXTURE_SIZE)) {
		la_print("_jl_gl_pr_obj_make() failed:");
		la_print("w = %d,h = %d", ro->pr.w, ro->pr.h);
		la_panic("texture is too big for graphics card.");
	}
	// Bind the texture.
	jlgr_opengl_texture_bind_(window, ro->pr.tx);
	// Bind the framebuffer.
	jlgr_opengl_framebuffer_bind_(window, ro->pr.fb);
	// Render on the whole framebuffer [ lower left -> upper right ]
	jlgr_opengl_viewport_(window, ro->pr.w, ro->pr.h);
	// Reset the aspect ratio.
	window->gl.cp = ro;
}

static void jlgr_pr_init__(la_window_t* jlgr, la_ro_t* ro) {
	if(ro->pr.fb == 0 || ro->pr.tx == 0) {
		// Make frame buffer
		jl_opengl_framebuffer_make_(jlgr, &ro->pr.fb);
		// Make the texture.
		jlgr_opengl_texture_new_(jlgr, &(ro->pr.tx), NULL, ro->pr.w,
			ro->pr.h, 0);
		jlgr_opengl_texture_off_(jlgr);
		// Recursively bind the framebuffer.
		jlgr_pr_use__(jlgr, ro);
		// Attach texture buffer.
		jlgr_opengl_framebuffer_addtx_(jlgr, ro->pr.tx);
		jlgr_opengl_framebuffer_status_(jlgr);
		// Set Viewport to image and clear.
		jlgr_opengl_viewport_(jlgr, ro->pr.w, ro->pr.h);
		// Clear the pre-renderer.
		jl_gl_clear(jlgr, 0.f, 0.f, 0.f, 0.f);
	}
}

// Stop drawing to a pre-renderer.
static inline void la_ro_pr_off__(la_window_t* window) {
	// Unbind the texture.
	jlgr_opengl_texture_off_(window);
	// Unbind the framebuffer.
	jlgr_opengl_framebuffer_bind_(window, 0);
	// Render to the whole screen.
	jl_gl_viewport_screen(window);
	// Reset the aspect ratio.
	window->gl.cp = NULL;
}

void la_ro_rect(la_window_t* window, la_ro_t* ro, float w, float h) {
	float rectangle_coords[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f };

	la_ro_init__(window, ro);
	// Overwrite the vertex object
	jlgr_vo_poly__(window, ro, 4, rectangle_coords);
}

void la_ro_color_tris(la_window_t* window, la_ro_t *ro,
	float* triangles, float* colors, uint32_t tricount)
{
	la_ro_init__(window, ro);
	// Rendering Style = triangles
	ro->rs = 1;
	// Set the vertices of vertex object "ro"
	jlgr_vo_vertices__(window, ro, triangles, tricount * 3);
	// Texture the vertex object
	la_ro_change_color(ro, colors);
}

void la_ro_plain_tris(la_window_t* window, la_ro_t *ro,
	float* triangles, float* colors, uint32_t tricount)
{
	la_ro_init__(window, ro);
	// Rendering Style = triangles
	ro->rs = 1;
	// Set the vertices of vertex object "ro"
	jlgr_vo_vertices__(window, ro, triangles, tricount * 3);
	// Texture the vertex object
	la_ro_change_plain(ro, colors);
}

void la_ro_color_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h)
{
	la_ro_init__(window, ro);
	la_ro_rect(window, ro, w, h);
	// Texture the vertex object
	la_ro_change_color(ro, colors);
}

void la_ro_plain_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h)
{
	la_ro_init__(window, ro);
	la_ro_rect(window, ro, w, h);
	// Texture the vertex object
	la_ro_change_plain(ro, colors);
}

void la_ro_image_rect(la_window_t* window, la_ro_t *ro, uint32_t tex, float w,
	float h)
{
	la_ro_init__(window, ro);
	//From bottom left & clockwise
	float Oone[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f };
	// Overwrite the vertex object
	jlgr_vo_poly__(window, ro, 4, Oone);
	// Texture the vertex object
	la_ro_change_image(ro, tex, 0, 0, -1, 0);
}

void la_ro_change_image(la_ro_t *ro, uint32_t img,
	uint8_t w, uint8_t h, int16_t map, uint8_t orient)
{
#ifdef JL_DEBUG
	if(img == 0) la_panic("Error: Texture=0!");
	la_ro_panic__(ro, "Can't Draw!");
#endif
	const float *tc = orient ?
		( orient == 1 ? UPSIDEDOWN_TC : BACKWARD_TC ) : DEFAULT_TC;

	// Make sure non-textured colors aren't attempted
	ro->tx = img;
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
		la_llgraphics_buffer_set_(&ro->bt, tex1, 8);
	}else{
		la_llgraphics_buffer_set_(&ro->bt, tc, 8);
	}
}

void la_ro_change_color(la_ro_t* ro, float* rgba) {
	la_window_t* window = la_ro_panic__(ro, "Change color.");

	jlgr_vo_color_buffer__(window, ro, rgba);
}

void la_ro_change_plain(la_ro_t* ro, float* rgba) {
	la_window_t* window = la_ro_panic__(ro, "Change plain.");

	float rgbav[4 * ro->vc];
	uint32_t i;

	for(i = 0; i < ro->vc; i++) {
		la_memory_copy(rgba, &(rgbav[i * 4]), 4 * sizeof(float));
	}
	jlgr_vo_color_buffer__(window, ro, rgbav);
}

void la_ro_move(la_ro_t* ro, la_v3_t pos) {
	la_ro_panic__(ro, "Can't Move!");
	ro->cb.pos = pos;
	ro->cb.pos.y += la_banner_size;
}

/**
 * Draw vertex object.
 * @param jlgr: The library context.
 * @param ro: The vertex object to draw.
 * @param sh: The shader to use ( must be the same one used with
 *	jlgr_opengl_draw1(). )
**/
void la_ro_draw_shader(la_ro_t* ro, jlgr_glsl_t* sh) {
	la_window_t* jlgr = la_ro_panic__(ro, "Can't Draw2!");

	// Use Temporary Vertex Object If no vertex object.
	if(ro == NULL) ro = &jlgr->gl.temp_vo;

	jlgr_opengl_matrix(jlgr, sh,
		(la_v3_t) { 1.f, 1.f, 1.f }, // Scale
		(la_v3_t) { 0.f, 0.f, 0.f }, // Rotate
		ro->cb.pos, // Translate
		(la_v3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));

	if(ro->tx) {
		// Bind Texture Coordinates to shader
		jlgr_opengl_setv(jlgr, &ro->bt,
			jlgr->gl.prg.texture.attributes.texpos_color, 2);
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

void la_ro_draw(la_ro_t* ro) {
	la_window_t* window = la_ro_panic__(ro, "Can't Draw!");
	jlgr_glsl_t* shader = ro->tx ?
		&window->gl.prg.texture : &window->gl.prg.color;

	jlgr_opengl_draw1(window, shader);
	la_ro_draw_shader(ro, shader);
}

void la_ro_pr_draw(la_ro_t* ro, uint8_t orient) {
	la_window_t* window = la_ro_panic__(ro, "Can't Draw Pre-Rendered!");

	// Initialize Framebuffer, if not already init'd
	jlgr_pr_init__(window, ro);
	// Bind texture shader.
	jlgr_opengl_draw1(window, &window->gl.prg.texture);
	// Transform
	jlgr_opengl_matrix(window, &window->gl.prg.texture,
		(la_v3_t) { 1.f, 1.f, 1.f }, // Scale
		(la_v3_t) { 0.f, 0.f, 0.f }, // Rotate
		ro->cb.pos, // Translate
		(la_v3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(window));
	// Bind Texture Coordinates to shader
	jlgr_opengl_setv(window, orient
		? &window->gl.upsidedown_tc : &window->gl.default_tc,
		window->gl.prg.texture.attributes.texpos_color, 2);
	// Bind the texture
	jlgr_opengl_texture_bind_(window, ro->pr.tx);
	// Update the position variable in shader.
	jlgr_opengl_setv(window, &ro->pr.gl,
		window->gl.prg.texture.attributes.position, 3);
	// Draw the image on the screen!
	jlgr_opengl_draw_arrays_(window, GL_TRIANGLE_FAN, 4);
}

void la_ro_pr(void* context, la_window_t* window, la_ro_t* ro, la_fn_t drawfn) {
	la_ro_t* oldpr = window->gl.cp;

	if(!ro) la_panic("Drawing on lost pre-renderer.");
	// Use the vo's pr
	jlgr_pr_use__(window, ro);
	// Render to the pr.
	drawfn(context);
	// Go back to the previous pre-renderer or none.
	oldpr ? jlgr_pr_use__(window, oldpr) : la_ro_pr_off__(window);
}

uint8_t la_ro_collide(la_ro_t *ro1, la_ro_t *ro2) {
	if (
		(ro1->cb.pos.y >= (ro2->cb.pos.y+ro2->cb.ofs.y)) ||
		(ro1->cb.pos.x >= (ro2->cb.pos.x+ro2->cb.ofs.x)) ||
		(ro2->cb.pos.y >= (ro1->cb.pos.y+ro1->cb.ofs.y)) ||
		(ro2->cb.pos.x >= (ro1->cb.pos.x+ro1->cb.ofs.x)) )
	{
		return 0;
	}else{
		return 1;
	}
}

void la_ro_clamp(la_v3_t xyz, la_ro_t *ro, la_v3_t* rtn) {
	xyz.x -= ro->cb.pos.x;
	if(ro->cb.ofs.x != 0.f) xyz.x /= ro->cb.ofs.x;
	xyz.y -= ro->cb.pos.y;
	if(ro->cb.ofs.y != 0.f) xyz.y /= ro->cb.ofs.y;
	xyz.z -= ro->cb.pos.z;
	if(ro->cb.ofs.z != 0.f) xyz.z /= ro->cb.ofs.z;
	*rtn = xyz;
}

void la_ro_free(la_ro_t *ro) {
#ifdef JL_DEBUG
	la_ro_panic__(ro, "Can't Free!");
#endif
	// Free GL VBO
	la_llgraphics_buffer_free(ro->gl);
	// Free GL Texture Buffer
	la_llgraphics_buffer_free(ro->bt);
	// Free Converted Vertices & Colors
	if(ro->cv) ro->cv = la_memory_free(ro->cv);
	if(ro->cc) ro->cc = la_memory_free(ro->cc);
}

#endif
