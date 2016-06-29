/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRtext.c
 *	Draw text on screen.
 */
#include "JLGRprivate.h"

/**
 * Draw text on the current pre-renderer.
 * @param 'jl': library context
 * @param 'str': the text to draw
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
**/
void jlgr_text_draw(jlgr_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f) {
	if(str == NULL) return;

	const uint8_t *text = (void*)str;
	uint32_t i;
	jl_rect_t rc = { loc.x, loc.y, f.size, f.size };
	jl_vec3_t tr = { 0., 0., 0. };
	jl_vo_t* vo = &jlgr->gl.temp_vo;

	jlgr_vos_image(jlgr, vo, rc, jlgr->textures.font, 1.);
	for(i = 0; i < strlen(str); i++) {
		if(text[i] == '\n') {
			tr.x = 0, tr.y += f.size;
			continue;
		}
		//Font 0:0
		jl_gl_vo_txmap(jlgr, vo, 16, 16, text[i]);
		jl_gl_transform_chr_(jlgr, tr.x, tr.y, tr.z,
			1., 1., 1.);
		jl_gl_draw_chr(jlgr, vo, f.colors[0], f.colors[1], f.colors[2],
			f.colors[3]);
		tr.x += f.size * ( 3. / 4. );
	}
}

void jlgr_text_init__(jlgr_t* jlgr) {
}
