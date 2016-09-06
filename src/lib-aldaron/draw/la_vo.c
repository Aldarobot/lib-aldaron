/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRvo.c
 *	Vertex Buffer Objects -> draw things on the screen.
 */
#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

#include "la_memory.h"

extern float la_banner_size;

static inline void jlgr_vo_bounding_box(la_window_t* jlgr, jl_vo_t* vo,
	const float *xyzw, uint32_t vertices)
{
	int i;
	vo->pr.cb.pos = (jl_vec3_t) { xyzw[0], xyzw[1], xyzw[2] };
	vo->pr.cb.ofs = (jl_vec3_t) { xyzw[0], xyzw[1], xyzw[2] };
	for(i = 1; i < vertices; i++) {
		jl_vec3_t new_vertex = {
			xyzw[(3 * i)],
			xyzw[(3 * i) + 1],
			xyzw[(3 * i) + 2] };

		if(new_vertex.x < vo->pr.cb.pos.x)
			vo->pr.cb.pos.x = new_vertex.x;
		if(new_vertex.y < vo->pr.cb.pos.y)
			vo->pr.cb.pos.y = new_vertex.y;
		if(new_vertex.z < vo->pr.cb.pos.z)
			vo->pr.cb.pos.z = new_vertex.z;

		if(new_vertex.x > vo->pr.cb.ofs.x)
			vo->pr.cb.ofs.x = new_vertex.x;
		if(new_vertex.y > vo->pr.cb.ofs.y)
			vo->pr.cb.ofs.y = new_vertex.y;
		if(new_vertex.z > vo->pr.cb.ofs.z)
			vo->pr.cb.ofs.z = new_vertex.z;
	}
	jl_mem_vec_sub(&vo->pr.cb.ofs, vo->pr.cb.pos);
}

static void jlgr_vo_vertices__(la_window_t* jlgr, jl_vo_t* vo, const float *xyzw,
	uint32_t vertices)
{
	vo->vc = vertices;
	if(vertices) {
		// Re-Allocate vo->cc
		vo->cc = la_memory_resize(vo->cc, vertices * sizeof(float) * 4);
		// Re-Allocate vo->cv
		vo->cv = la_memory_resize(vo->cv, vertices * sizeof(float) * 3);
		// Set vo->cv & vo->gl
		jlgr_opengl_vertices_(jlgr, xyzw, vertices, vo->cv, &vo->gl);
		// Set bounding box
		jlgr_vo_bounding_box(jlgr, vo, xyzw, vertices);
		// Update pre-renderer
		jlgr_pr_resize(jlgr, &vo->pr, vo->pr.cb.ofs.x, vo->pr.cb.ofs.y,
			jlgr->wm.w * vo->pr.cb.ofs.x);
	}
	jlgr_vo_move(vo, (jl_vec3_t) { 0.f, 0.f, 0.f });
}

static void jlgr_vo_color_buffer__(la_window_t* jlgr, jl_vo_t* vo, float* cc) {
	vo->tx = 0;
	jl_mem_copyto(cc, vo->cc, vo->vc * 4 * sizeof(float));
	// Set Color Buffer "vo->bt" to "vo->cc"
	jlgr_opengl_buffer_set_(jlgr, &vo->bt, vo->cc, vo->vc * 4);
}

// Set vertices for a polygon.
static void jlgr_vo_poly__(la_window_t* jlgr, jl_vo_t* vo, uint32_t vertices,
	const float *xyzw)
{
	const float FS_RECT[] = {
		0.,jl_gl_ar(jlgr),0.,
		0.,0.,0.,
		1.,0.,0.,
		1.,jl_gl_ar(jlgr),0.
	};

	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	if(xyzw == NULL) xyzw = FS_RECT;
	// Rendering Style = polygon
	vo->rs = 0;
	// Set the vertices of vertex object "vo"
	jlgr_vo_vertices__(jlgr, vo, xyzw, vertices);
}

void jlgr_vo_init__(jl_t* jl, jl_vo_t* vo) {
	// Don't do anything if already init'd
	if(vo->jl) return;
	//
	vo->jl = jl;
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

void jlgr_vo_exit__(jl_vo_t* vo, const char* error) {
	if(vo->jl == NULL) la_panic("Vertex object uninit'd: %s\n", error);
}

/**
 * Resize a vertex object to a rectangle.
 * @param jlgr: The library context.
 * @param vo: The vertex object.
 * @param rc: Rectangle to set it to.
**/
void jlgr_vo_rect(la_window_t* jlgr, jl_vo_t* vo, jl_rect_t* rc) {
	jlgr_vo_init__(jlgr->jl, vo);
	if(rc) {
		float rectangle_coords[] = {
			0.f,		rc->h,		0.f,
			0.f,		0.f,		0.f,
			rc->w,		0.f,		0.f,
			rc->w,		rc->h,		0.f };

		// Overwrite the vertex object
		jlgr_vo_poly__(jlgr, vo, 4, rectangle_coords);
		//
		jlgr_vo_move(vo, (jl_vec3_t) { rc->x, rc->y, 0.f } );
	}
}

/**
 * Set a Vertex object to vector graphics.
 * @param jlgr: The library context.
 * @param vo: The vertex object to set.
**/
void jlgr_vo_set_vg(la_window_t* jlgr, jl_vo_t *vo, uint16_t tricount,
	float* triangles, float* colors, uint8_t multicolor)
{
	jlgr_vo_init__(jlgr->jl, vo);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	// Rendering Style = triangles
	vo->rs = 1;
	// Set the vertices of vertex object "vo"
	jlgr_vo_vertices__(jlgr, vo, triangles, tricount * 3);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, vo, colors);
	else jlgr_vo_color_solid(jlgr, vo, colors);
}

/**
 * Set a vertex object to a rectangle.
 * @param jl: The library context.
 * @param vo: The vertex object
 * @param rc: The rectangle coordinates.
 * @param colors: The color(s) to use - [ R, G, B, A ]
 * @param multicolor: If 0: Then 1 color is used.
 *	If 1: Then 1 color per each vertex is used.
**/
void jlgr_vo_set_rect(la_window_t* jlgr, jl_vo_t *vo, jl_rect_t rc, float* colors,
	uint8_t multicolor)
{
	jlgr_vo_init__(jlgr->jl, vo);
	jlgr_vo_rect(jlgr, vo, &rc);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, vo, colors);
	else jlgr_vo_color_solid(jlgr, vo, colors);
}

/**
 * Set the vertex object's texturing to an image.
 * @param jlgr: The library context.
 * @param vo: The vertex object to modify.
 * @param img: The image to display on the vertex object.
**/
void jlgr_vo_image(la_window_t* jlgr, jl_vo_t *vo, uint32_t img) {
	jlgr_vo_init__(jlgr->jl, vo);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	// Make sure non-textured colors aren't attempted
	vo->tx = img;
#ifdef JL_DEBUG
	if(!vo->tx) la_panic("Error: Texture=0!");
#endif
	jlgr_vo_txmap(jlgr, vo, 0, 0, 0, -1);
}

/**
 * Set a vertex object to an Image.
 *
 * @param jlgr: The library context
 * @param vo: The vertex object
 * @param rc: the rectangle to draw the image in.
 * @param tex:  the ID of the image.
**/
void jlgr_vo_set_image(la_window_t* jlgr, jl_vo_t *vo, jl_rect_t rc, uint32_t tex) {
	jlgr_vo_init__(jlgr->jl, vo);
	//From bottom left & clockwise
	float Oone[] = {
		rc.x,		rc.y + rc.h,	0.f,
		rc.x,		rc.y,		0.f,
		rc.x + rc.w,	rc.y,		0.f,
		rc.x + rc.w,	rc.y + rc.h,	0.f };
	// Overwrite the vertex object
	jlgr_vo_poly__(jlgr, vo, 4, Oone);
	// Texture the vertex object
	jlgr_vo_image(jlgr, vo, tex);
}

/**
 * Change the character map for a texture.
 * @param jl: The library context.
 * @param vo: The vertext object to change.
 * @param w: How many characters wide the texture is.
 * @param h: How many characters high the texture is.
 * @param map: The character value to map.  -1 for full texture.
**/
void jlgr_vo_txmap(la_window_t* jlgr, jl_vo_t* vo, uint8_t orientation,
	uint8_t w, uint8_t h, int16_t map)
{
	jlgr_vo_init__(jlgr->jl, vo);
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
		jlgr_opengl_buffer_set_(jlgr, &vo->bt, tex1, 8);
	}else{
		jlgr_opengl_buffer_set_(jlgr, &vo->bt, tc, 8);
	}
}

/**
 * Change the coloring scheme for a vertex object to a gradient.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { (4 * vertex count) values }
**/
void jlgr_vo_color_gradient(la_window_t* jlgr, jl_vo_t* vo, float* rgba) {
	jlgr_vo_init__(jlgr->jl, vo);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	jlgr_vo_color_buffer__(jlgr, vo, rgba);
}

/**
 * Change the coloring scheme for a vertex object to a solid.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { 4 values }
**/
void jlgr_vo_color_solid(la_window_t* jlgr, jl_vo_t* vo, float* rgba) {
	jlgr_vo_init__(jlgr->jl, vo);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	float rgbav[4 * vo->vc];
	uint32_t i;

	for(i = 0; i < vo->vc; i++) {
		jl_mem_copyto(rgba, &(rgbav[i * 4]), 4 * sizeof(float));
	}
	jlgr_vo_color_buffer__(jlgr, vo, rgbav);
}

/**
 * Move a vertex object to a new location.
 * @param vo: The vertex object.
 * @param pos: The new position.
**/
void jlgr_vo_move(jl_vo_t* vo, jl_vec3_t pos) {
	jlgr_vo_exit__(vo, "Can't Move!");
	vo->pr.cb.pos = pos;
	vo->pr.cb.pos.y += la_banner_size;
}

/**
 * Draw vertex object.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param sh: The shader to use ( must be the same one used with
 *	jlgr_opengl_draw1(). )
**/
void jlgr_vo_draw2(la_window_t* jlgr, jl_vo_t* vo, jlgr_glsl_t* sh) {
	jlgr_vo_exit__(vo, "Can't Draw2!");
	// Use Temporary Vertex Object If no vertex object.
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	if(vo->tx) {
		// Bind Texture Coordinates to shader
		jlgr_opengl_setv(jlgr, &vo->bt,
			jlgr->effects.alpha.shader.attributes.texpos_color, 2);
		// Bind the texture
		jlgr_opengl_texture_bind_(jlgr, vo->tx);
	}else{
		// Bind Colors to shader
		jlgr_opengl_setv(jlgr, &vo->bt,
			jlgr->gl.prg.color.attributes.texpos_color, 4);
	}
	// Update the position variable in shader.
	jlgr_opengl_setv(jlgr, &vo->gl, sh->attributes.position, 3);
	// Draw the image on the screen!
	jlgr_opengl_draw_arrays_(jlgr, vo->rs ? GL_TRIANGLES : GL_TRIANGLE_FAN,
		vo->vc);
}

/**
 * Draw a vertex object with offset by translation.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
**/
void jlgr_vo_draw(la_window_t* jlgr, jl_vo_t* vo) {
	if(vo->jl == NULL) la_panic("Can't Draw");
	jlgr_vo_exit__(vo, "Can't Draw!");
	jlgr_glsl_t* shader = vo->tx ?
		&jlgr->gl.prg.texture : &jlgr->gl.prg.color;

	jlgr_opengl_draw1(jlgr, shader);
	jlgr_opengl_matrix(jlgr, shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		vo->pr.cb.pos, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	jlgr_vo_draw2(jlgr, vo, shader);
}

/**
 * Draw a vertex object with effects.
 * @param jlgr: The library context.
 * @param vo: The vertex object.
**/
void jlgr_vo_draw_pr(la_window_t* jlgr, jl_vo_t* vo) {
	jlgr_vo_exit__(vo, "Can't Draw Pre-Rendered!");
	jlgr_pr_draw(jlgr, &vo->pr, vo->pr.cb.pos, 0);
}

void la_vo_pr(la_window_t* window, jl_vo_t* vo, jl_fnct drawfn) {
	jlgr_pr(window, &vo->pr, drawfn);
}

/**
 * Free a vertex object.
 * @param jl: The library context
 * @param vo: The vertex object to free
**/
void jlgr_vo_free(la_window_t* jlgr, jl_vo_t *vo) {
	jlgr_vo_exit__(vo, "Can't Free!");
	// Free GL VBO
	jlgr_opengl_buffer_old_(jlgr, &vo->gl);
	// Free GL Texture Buffer
	jlgr_opengl_buffer_old_(jlgr, &vo->bt);
	// Free Converted Vertices & Colors
	if(vo->cv) vo->cv = la_memory_resize(vo->cv, 0);
	if(vo->cc) vo->cc = la_memory_resize(vo->cc, 0);
	// Free main structure
	la_memory_free(vo);
}
