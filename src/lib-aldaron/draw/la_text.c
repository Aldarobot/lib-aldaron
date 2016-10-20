/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_draw.h>
#include <la_text.h>
#include <la_string.h>
#include <la_ro.h>
#include <la_effect.h>

#define COMPARE(towhat) ( strncmp(&temp[i], towhat, strlen(towhat)) == 0 )

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
			la_ro_change_image(&window->gl.temp_vo,
				window->textures.font, 16, 16, temp[i], 0);
			// Effects
			if(shadow) {
				la_ro_move(&window->gl.temp_vo, (la_v3_t) {
					tr.x - 0.005, tr.y + 0.005, 0.f });
				la_effect_hue(&window->gl.temp_vo, shadowcolor);
			}
			// Draw character
			la_ro_move(&window->gl.temp_vo, tr);
			la_effect_hue(&window->gl.temp_vo, colors);
			// Advance cursor.
			tr.x += w * distance;
			i++;
		}
	}
}

void la_text_centered(la_window_t* window, const char *str, float y, float* color) {
	la_text(window,
		LA_PXMOVE("%f", "%f") LA_PXSIZE("%f")
		LA_PXCOLOR("%f", "%f", "%f", "%f") "%s",
		0.f, y, 1. / ((double)(strlen(str)-1)),
		color[0], color[1], color[2], color[3], str);
}

#endif
