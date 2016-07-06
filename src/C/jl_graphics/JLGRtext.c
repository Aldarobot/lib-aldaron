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
 * @param jlgr: The library context
 * @param str: The text to draw
 * @param loc: The position to draw it at
 * @param f: The font to use.
**/
void jlgr_text_draw(jlgr_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f) {
	if(str == NULL) return;

	const uint8_t *text = (void*)str;
	uint32_t i;
	uint8_t bold = 0;
	jl_rect_t rc = { loc.x, loc.y, f.size, f.size };
	jl_vec3_t tr = { 0., 0., 0. };
	jl_vo_t* vo = &jlgr->gl.temp_vo;

	jlgr_vo_set_image(jlgr, vo, rc, jlgr->textures.font);
	for(i = 0; i < strlen(str); i++) {
		// Check for special characters
		if(text[i] == '\n') {
			tr.x = 0, tr.y += f.size;
			continue;
		}else if(text[i] == '\x01') {
			if(strncmp(&str[i], JLGR_TEXT_BOLD, 2) == 0) {
				bold = 1;
			}else if(strncmp(&str[i], JLGR_TEXT_ALIGNC, 2) == 0) {
				uint32_t n = jl_mem_string_upto(&str[i] + 2,
					'\n');
				tr.x -= (f.size * n) / 2.f;
			}else if(strncmp(&str[i], JLGR_TEXT_ALIGNR, 2) == 0) {
				uint32_t n = jl_mem_string_upto(&str[i] + 2,
					'\n');
				tr.x -= f.size * n;
			}
			i++;
			continue;
		}
		// Set character
		jlgr_vo_txmap(jlgr, vo, 16, 16, text[i]);
		// Special Drawing
		if(bold) {
			float x = tr.x; int i;
			for(i = 0; i < 3; i++) {
				tr.x += .05 * f.size;
				jlgr_effects_vo_hue(jlgr, vo, tr, f.colors);
			}
			tr.x = x;
		}
		// Draw character.
		jlgr_effects_vo_hue(jlgr, vo, tr, f.colors);
		// Advance cursor.
		tr.x += f.size * ( 3. / 4. );
	}
}

void jlgr_text_init__(jlgr_t* jlgr) {
}
