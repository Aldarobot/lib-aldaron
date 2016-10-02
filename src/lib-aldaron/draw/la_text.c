/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"

#include <la_text.h>
#include <la_string.h>
#include <la_ro.h>

#define COMPARE(towhat) ( strncmp(&temp[i], towhat, strlen(towhat)) == 0 )

static inline float la_text_readfloat(const char* input) {
	float* x = (void*)input;
	return *x;
}

void la_text(la_window_t* window, const char* format, ...) {
	va_list arglist;
	float colors[] = { 1.f, 1.f, 1.f, 1.f }; // Color of the font.
	float shadowcolor[] = { 0.5f, 0.5f, 0.5f, 1.f };
//	uint8_t bold;
//	uint8_t italic;
	uint8_t shadow = 0;
	int i = 0;
	float tabsize = 8.f; // How many spaces are in a tab.
	float distance = .75f; // X distance between letters ( X : Y )
	float width = 1.f;
	la_v3_t tr = { 0.f, 0.f, 0.f };
	float resetx = 0.f;
//	float resety = 0.f;
	uint32_t limit = 0;
	char temp[1024 + 1];
	float w = .0625f, h = .0625f;

	// Format the String...
	va_start( arglist, format );
	vsnprintf(temp, 1024, format, arglist);
	va_end( arglist );

	// Draw
	la_ro_image_rect(window,&window->gl.temp_vo,window->textures.font,w,h);
	while(1) {
		if(limit && i > limit) break;
		if(temp[i] == '\0') break;
		if(COMPARE("\x1B[")) {
			if(COMPARE(LA_PRED)) {
				i += strlen(LA_PRED);
				colors[0] = 1.f, colors[1] = 0.f, colors[2] = 0.f,
				colors[3] = 1.f;
			}else if(COMPARE(LA_PBLACK)) {
				i += strlen(LA_PBLACK);
				colors[0] = 0.f, colors[1] = 0.f, colors[2] = 0.f,
				colors[3] = 1.f;
			}else if(COMPARE("\x1B[f")) {
				char* end;

				i += strlen("\x1B[f");
				h = strtof(&temp[i], &end);
				w = width * h;
				i += end - &temp[i] + 1;
				la_ro_image_rect(window, &window->gl.temp_vo,
					window->textures.font, w, h);
			}else if(COMPARE("\x1B[m")) {
				char* end;

				i += strlen("\x1B[m");
				tr.x = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
				tr.y = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
			}else if(COMPARE("\x1B[c")) {
				char* end;

				i += strlen("\x1B[c");
				colors[0] = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
				colors[1] = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
				colors[2] = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
				colors[3] = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
			}else if(COMPARE("\x1B[w")) {
				char* end;

				i += strlen("\x1B[w");
				width = strtof(&temp[i], &end);
				w = width * h;
				i += end - &temp[i] + 1;
				la_ro_image_rect(window, &window->gl.temp_vo,
					window->textures.font, w, h);
			}else if(COMPARE("\x1B[l")) {
				char* end;

				i += strlen("\x1B[l");
				limit = strtof(&temp[i], &end);
				i += end - &temp[i] + 1;
			}else{
				la_print("%c", temp[i]);
				la_panic("Unknown Ansi Escape Sequence");
			}
//		}else if(COMPARE(LA_TEXT_CMD)) {
//			if(COMPARE(LA_TEXT_ALIGN)) {
//				i += 2;
//			}else if(COMPARE(LA_TEXT_UNDERLAY)) {
//				i += 2;
//				for(int k = 0; k < 4; k++) { // RGBA
//					shadowcolor[k] = la_text_readfloat(&temp[i]);
//					i += sizeof(float);
//				}
//				shadow = 1;
//			}else if(COMPARE(LA_TEXT_SHADOWOFF)) {
//				shadow = 0;
//			}
		}else if(COMPARE("\n")) {
			tr.x = resetx, tr.y += h;
			i++;
		}else if(COMPARE("\t")) {
			tr.x += tabsize * w * ( 3. / 4. );
			i++;
		}else{ // Single Byte Character.
			// Set character
			jlgr_vo_txmap(window,&window->gl.temp_vo,0,16,16,temp[i]);
			// Effects
			if(shadow) {
				jlgr_effects_vo_hue(window, &window->gl.temp_vo,
					(la_v3_t) { tr.x - 0.005, tr.y + 0.005,
						0.f }, shadowcolor);
			}
			// Draw character
			jlgr_effects_vo_hue(window,&window->gl.temp_vo, tr, colors);
			// Advance cursor.
			tr.x += w * distance;
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
void jlgr_text_draw(la_window_t* jlgr, const char* str, la_v3_t loc, jl_font_t f) {
	if(str == NULL) return;

	const uint8_t *text = (void*)str;
	uint32_t i;
	uint8_t bold = 0;
	jl_rect_t rc = { loc.x, loc.y, f.size, f.size };
	la_v3_t tr = { 0., 0., 0. };
	la_ro_t* vo = &jlgr->gl.temp_vo;

	la_ro_image_rect(jlgr, vo, jlgr->textures.font, rc.w, rc.h);
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
				uint32_t n = la_string_upto(&str[i] + 2,
					'\n');
				tr.x -= (f.size * n) / 2.f;
			}else if(strncmp(&str[i], JLGR_TEXT_ALIGNR, 2) == 0) {
				uint32_t n = la_string_upto(&str[i] + 2,
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

#endif
