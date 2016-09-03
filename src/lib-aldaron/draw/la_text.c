/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
#include "JLGRprivate.h"
#include "la_text.h"
#include "la_memory.h"

#define COMPARE(towhat) ( strncmp(&temp[i], towhat, strlen(towhat)) == 0 )

void la_text(la_window_t* window, const char* format, ...) {
	va_list arglist;
	float colors[] = { 1.f, 1.f, 1.f, 1.f }; // Color of the font.
	jl_rect_t rc = { 0.f, 0.f, .0625f, .0625f };
//	uint8_t bold;
//	uint8_t italic;
	int i = 0;
	float tabsize = 8.f; // How many spaces are in a tab.
	float distance = .75f; // X distance between letters ( X : Y )
	jl_vec3_t tr = { 0., 0., 0. };

	// Format the String...
	va_start( arglist, format );
	char temp[snprintf(NULL, 0, format, arglist) + 1];
	vsprintf(temp, format, arglist);
	va_end( arglist );

	// Draw
	jlgr_vo_set_image(window, &window->gl.temp_vo, rc, window->textures.font);
//	la_print("TEXTSTART:");
	while(1) {
//		la_print("PRINTOUT: %d", temp[i]);
		if(temp[i] == '\0') break;
		if(COMPARE(LA_TEXT_CMD)) {
			if(COMPARE(LA_TEXT_CONTROL)) {
				i += 2;
			}else if(COMPARE(LA_TEXT_IMAGE)) {
				i += 2;
			}else if(COMPARE(LA_TEXT_XY)) {
				float* temp2;
				i += 2;
				temp2 = (void*) &temp[i];
				tr.x += *temp2;
				i += sizeof(float);
				temp2 = (void*) &temp[i];
				tr.y += *temp2;
				i += sizeof(float);
				la_print("%fx%f", tr.x, tr.y);
			}else if(COMPARE(LA_TEXT_SIZE)) {
				i += 2;
			}else if(COMPARE(LA_TEXT_ALIGN)) {
				i += 2;
			}else if(COMPARE(LA_TEXT_WIDTH)) {
				i += 2;
			}
		}else if(COMPARE("\n")) {
			tr.x = 0, tr.y += rc.y;
		}else if(COMPARE("\t")) {
			tr.x += tabsize * rc.x * ( 3. / 4. );
		}else{ // Single Byte Character.
			// Set character
			jlgr_vo_txmap(window,&window->gl.temp_vo,0,16,16,temp[i]);
			// Draw character			
			jlgr_effects_vo_hue(window,&window->gl.temp_vo, tr, colors);
			// Advance cursor.
			tr.x += rc.w * distance;
			i++;
		}
	}
}

/**
 * Draw text on the current pre-renderer.
 * @param jlgr: The library context
 * @param str: The text to draw
 * @param loc: The position to draw it at
 * @param f: The font to use.
**/
void jlgr_text_draw(la_window_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f) {
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
		} else if(text[i] == '\t') {
			tr.x += 8.f * f.size * ( 3. / 4. );
			continue;
		} else if(text[i] == '\x01') {
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
		jlgr_vo_txmap(jlgr, vo, 0, 16, 16, text[i]);
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
