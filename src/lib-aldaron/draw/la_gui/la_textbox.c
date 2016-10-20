/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_gui.h>
#include <la_text.h>
#include <la_ro.h>

/*static void la_textbox_cursor__(la_window_t* window) {
	if(input.h == 2) window->gui.textbox.counter += window->psec;
	if(input.h != 1 && window->gui.textbox.do_it != 1) return;
	switch(input.k) {
		case LA_INPUT_DIR_RT: {
			if(window->gui.textbox.string->curs <
			 window->gui.textbox.string->size)
				window->gui.textbox.string->curs++;
			la_input_typing_disable();
			window->gui.textbox.cursor = 1;
			window->gui.textbox.counter = 0.f;
			break;
		}
		case LA_INPUT_DIR_LT: {
			if(window->gui.textbox.string->curs)
				window->gui.textbox.string->curs--;
			la_input_typing_disable();
			window->gui.textbox.cursor = 1;
			window->gui.textbox.counter = 0.f;
			break;
		}
	}
}*/

/**
 * Set the data string for a textbox.
 * @param string: The string to store to.
**/
void la_textbox_edit(la_textbox_t* textbox, la_window_t* window,
	la_buffer_t* string, la_rect_t rc)
{
	textbox->window = window;
	textbox->string = string;
	textbox->rc = rc;
}

/**
 * Check for keyboard input and store in string.  Do not call before
 *	la_textbox_edit().
 * @returns 1: if return/enter is pressed.
 * @returns 0: if not.
**/
uint8_t la_textbox_loop(la_textbox_t* textbox) {
	int i;
	la_window_t* window = textbox->window;

//	textbox->counter += window->psec;
	if(textbox->counter > .5) {
		textbox->counter -= .5;
		textbox->do_it = 1;
		if(textbox->cursor) textbox->cursor = 0;
		else textbox->cursor = 1;
	}else{
		textbox->do_it = 0;
	}
#if defined(LA_COMPUTER)
//	la_textbox_cursor__(window);
#endif
	for(i = 0; i < strlen(window->input.text); i++)
		la_buffer_ins(textbox->string, window->input.text[i]);
	if(window->input.keyboard.h && window->input.keyboard.p) {
		switch(window->input.keyboard.k) {
		case '\b':
			if(textbox->string->curs == 0) return 0;
			textbox->string->curs--;
			la_buffer_del(textbox->string);
			break;
		case '\02':
			la_buffer_del(textbox->string);
			break;
		case '\n':
			return 1;
		default:
			break;
		}
	}
	return 0;
}

/**
 * Draw A Textbox.  Must be paired on main thread by la_textbox_loop().
 * @param rc: Dimensions to draw textbox.
*/
void la_textbox_draw(la_textbox_t* textbox) {
	la_window_t* window = textbox->window;
	float cursor_color[] = { 0.1f, 0.1f, 0.1f, 1.f };

//(la_rect_t) {
//			rc.x + (textbox->string->curs * rc.h * .75),
//			rc.y, rc.h * .05, rc.h }
	if(textbox->cursor) {
		la_ro_plain_rect(window, &window->gl.temp_vo, cursor_color,
			textbox->rc.h * .05, textbox->rc.h);
		la_ro_draw(&window->gl.temp_vo);
	}
	la_text(window, (char*)(textbox->string->data));
//		(la_v3_t) {rc.x, rc.y, 0.},
}

#endif
