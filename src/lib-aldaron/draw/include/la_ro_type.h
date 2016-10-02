/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_RO_TYPE
#define LA_RO_TYPE

#include <stdint.h>

// Render-able Object
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
	struct {
		// What to render
		uint32_t tx;	// ID to texture.
		uint32_t fb;	// ID to Frame Buffer
		uint16_t w, h;	// Width and hieght of texture
		// Render Area
		uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
		float ar;	// Aspect Ratio: h:w
		float cv[4*3];	// Converted Vertices

	}pr;
	// Collision Detection
	jl_area_t cb;	// Collision box.
}la_ro_t;

#endif
