/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

#include <la_pr.h>

/** @cond */

void jl_opengl_framebuffer_make_(la_window_t* jlgr, uint32_t *fb);
void jlgr_opengl_framebuffer_bind_(la_window_t* jlgr, uint32_t fb);
void jlgr_opengl_framebuffer_addtx_(la_window_t* jlgr, uint32_t tx);
void jlgr_opengl_framebuffer_status_(la_window_t* jlgr);
void jl_opengl_framebuffer_free_(la_window_t* jlgr, uint32_t *fb);
void jlgr_opengl_texture_new_(la_window_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp);
void jlgr_opengl_texture_off_(la_window_t* jlgr);
void jl_gl_texture_free_(la_window_t* jlgr, uint32_t *tex);
static void jlgr_pr_use__(la_window_t* jlgr, jl_pr_t* pr);
void jlgr_opengl_viewport_(la_window_t* jlgr, uint16_t w, uint16_t h);

static void jlgr_pr_obj_make_tx__(la_window_t* jlgr, jl_pr_t *pr) {
	// Make a new texture for pre-renderering.  The "NULL" sets it blank.
	jlgr_opengl_texture_new_(jlgr, &(pr->tx), NULL, pr->w, pr->h, 0);
	jlgr_opengl_texture_off_(jlgr);
}

static void jlgr_pr_off__(la_window_t* jlgr) {
	// Unbind the texture.
	jlgr_opengl_texture_off_(jlgr);
	// Unbind the framebuffer.
	jlgr_opengl_framebuffer_bind_(jlgr, 0);
	// Render to the whole screen.
	jl_gl_viewport_screen(jlgr);
}

static void jlgr_pr_init__(la_window_t* jlgr, jl_pr_t* pr) {
	if(pr->fb == 0 || pr->tx == 0) {
		// Make frame buffer
		jl_opengl_framebuffer_make_(jlgr, &pr->fb);
		// Make the texture.
		jlgr_pr_obj_make_tx__(jlgr, pr);
		// Recursively bind the framebuffer.
		jlgr_pr_use__(jlgr, pr);
		// Attach texture buffer.
		jlgr_opengl_framebuffer_addtx_(jlgr, pr->tx);
		jlgr_opengl_framebuffer_status_(jlgr);
		// Set Viewport to image and clear.
		jlgr_opengl_viewport_(jlgr, pr->w, pr->h);
		// Clear the pre-renderer.
		jl_gl_clear(jlgr, 0.f, 0.f, 0.f, 0.f);
	}
}

static void jlgr_pr_use__(la_window_t* jlgr, jl_pr_t* pr) {
	jlgr_pr_init__(jlgr, pr);
	if(pr->w == 0) {
		la_panic("jlgr_pr_use__ failed: 'w' must be more than 0");
	}else if(pr->h == 0) {
		la_panic("jlgr_pr_use__ failed: 'h' must be more than 0");
	}else if((pr->w > GL_MAX_TEXTURE_SIZE)||(pr->h > GL_MAX_TEXTURE_SIZE)) {
		la_print("_jl_gl_pr_obj_make() failed:");
		la_print("w = %d,h = %d", pr->w, pr->h);
		la_panic("texture is too big for graphics card.");
	}
	// Bind the texture.
	jlgr_opengl_texture_bind_(jlgr, pr->tx);
	// Bind the framebuffer.
	jlgr_opengl_framebuffer_bind_(jlgr, pr->fb);
	// Render on the whole framebuffer [ lower left -> upper right ]
	jlgr_opengl_viewport_(jlgr, pr->w, pr->h);
}

static void jlgr_pr_set__(jl_pr_t *pr, float w, float h, uint16_t w_px) {
	const float ar = h / w; // Aspect Ratio.
	const float h_px = w_px * ar; // Height in pixels.

	// Set width and height.
	pr->w = w_px;
	pr->h = h_px;
	// Set aspect ratio.
	pr->ar = ar;
}

static void jlgr_pr_use2__(la_window_t* jlgr, jl_pr_t* pr) {
	// Render to the framebuffer.
	if(pr) jlgr_pr_use__(jlgr, pr);
	else jlgr_pr_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = pr;
}

/** @endcond */

// Stop drawing to a pre-renderer.
void jlgr_pr_off(la_window_t* jlgr) {
	// Render to the screen
	jlgr_pr_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = NULL;
}

/**
 * THREAD: Draw thread only.
 * Resize a prerenderer.
 * @param jlgr: The library context.
 * @param pr: The pre-renderer to resize.
 * @param w: The display width. [ 0. - 1. ]
 * @param h: The display height. [ 0. - 1. ]
 * @param w_px: The resolution in pixels along the x axis [ 1- ]
**/
void jlgr_pr_resize(la_window_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px){
	const float xyzw[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f
	};

	// If pre-renderer is initialized, reset.
	if(pr->fb) {
		jl_gl_texture_free_(jlgr, &(pr->tx));
		jl_opengl_framebuffer_free_(jlgr, &(pr->fb));
		jlgr_opengl_buffer_old_(jlgr, &(pr->gl));
	}
	// Create the VBO.
	jlgr_opengl_vertices_(jlgr, xyzw, 4, pr->cv, &pr->gl);
	// Set width, height and aspect ratio.
	jlgr_pr_set__(pr, w, h, w_px);
}

/**
 * Draw a pre-rendered texture.
 * @param jlgr: The library context.
 * @param pr: The pre-rendered texture.
 * @param vec: The vector of offset/translation.
 * @param orient: 1 for upside-down 0 for normal.
**/
void jlgr_pr_draw(la_window_t* jlgr, jl_pr_t* pr, la_v3_t vec, uint8_t orient) {
	// Initialize Framebuffer, if not already init'd
	jlgr_pr_init__(jlgr, pr);
	// Bind texture shader.
	jlgr_opengl_draw1(jlgr, &jlgr->gl.prg.texture);
	// Transform
	jlgr_opengl_matrix(jlgr, &jlgr->gl.prg.texture,
		(la_v3_t) { 1.f, 1.f, 1.f }, // Scale
		(la_v3_t) { 0.f, 0.f, 0.f }, // Rotate
		(la_v3_t) { vec.x, vec.y, vec.z }, // Translate
		(la_v3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	// Bind Texture Coordinates to shader
	jlgr_opengl_setv(jlgr, orient
		? &jlgr->gl.upsidedown_tc : &jlgr->gl.default_tc,
		jlgr->gl.prg.texture.attributes.texpos_color, 2);
	// Bind the texture
	jlgr_opengl_texture_bind_(jlgr, pr->tx);
	// Update the position variable in shader.
	jlgr_opengl_setv(jlgr, &pr->gl, jlgr->gl.prg.texture.attributes.position, 3);
	// Draw the image on the screen!
	jlgr_opengl_draw_arrays_(jlgr, GL_TRIANGLE_FAN, 4);
}

void la_pr(void* context, la_window_t* window, jl_pr_t* pr, jl_fnct redrawfn) {
	jl_pr_t* oldpr = window->gl.cp;

	if(!pr) la_panic("Drawing on lost pre-renderer.");
	// Use the vo's pr
	jlgr_pr_use2__(window, pr);
	// Render to the pr.
	redrawfn(context);
	// Go back to the previous pre-renderer.
	jlgr_pr_use2__(window, oldpr);
}
