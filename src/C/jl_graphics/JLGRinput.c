/*
 * (c) Jeron A. Lau
 *
 * jl/input is a library for making input controllers compatible between
 * different
 * devices.
*/

#include "JLGRinternal.h"

//Prototypes
	// ct.c
	uint8_t jl_ct_key_pressed(jlgr_t *jlgr, uint8_t key);

/*
 * "Preval" is a pointer to previous key pressed value.
 *	 0 if key isn't pressed
 *	 1 if key is just pressed
 *	 2 if key is held down
 *	 3 if key is released
 * "read" is the input variable.
*/
static void jl_ct_state__(uint8_t *preval, const uint8_t read) {
	if(*preval == JLGR_INPUT_PRESS_ISNT || *preval == JLGR_INPUT_PRESS_STOP)
		//If wasn't pressed: Just Pressed / Not pressed
		*preval = read ? JLGR_INPUT_PRESS_JUST : JLGR_INPUT_PRESS_ISNT;
	else if(read ==  0 && (*preval == JLGR_INPUT_PRESS_JUST ||
	 *preval == JLGR_INPUT_PRESS_HELD))
		//If Was Held Down And Now Isnt | 3: Release
		*preval = JLGR_INPUT_PRESS_STOP;
	else
		//2: Held Down
		*preval = JLGR_INPUT_PRESS_HELD;
}

static void jlgr_input_press__(jlgr_t* jlgr, u8_t countit) {
	jl_ct_state__(&jlgr->main.ct.states[jlgr->main.ct.current_event], countit);
	//hrxypk
	jlgr->input.input.h = jlgr->main.ct.states[jlgr->main.ct.current_event];
	jlgr->input.input.r = 0.;
	jlgr->input.input.x = jl_ct_gmousex(jlgr);
	jlgr->input.input.y = jl_ct_gmousey(jlgr);
	if(countit) {
		jlgr->input.input.k = 1;
		jlgr->input.input.p = 1.;
	}else{
		jlgr->input.input.k = 0;
		jlgr->input.input.p = 0.;
	}
}

void jl_ct_key(jlgr_t *jlgr, jlgr_input_fnct inputfn, uint8_t key) {
	uint8_t a = jl_ct_key_pressed(jlgr, key);

	jlgr->input.input.h = a;
	jlgr->input.input.r = 0.;
	if(a && (a!=3)) {
		jlgr->input.input.p = 1.;
		jlgr->input.input.k = key;
	}else{
		jlgr->input.input.p = 0.;
		jlgr->input.input.k = 0;
	}
	inputfn(jlgr, jlgr->input.input);
}

#if JL_PLAT == JL_PLAT_COMPUTER

	void jl_ct_key_retn(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_RETURN);
	}

	void jl_ct_key_arup(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_UP);
	}

	void jl_ct_key_ardn(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_DOWN);
	}

	void jl_ct_key_arlt(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_LEFT);
	}

	void jl_ct_key_arrt(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_RIGHT);
	}

	void jl_ct_key_keyw(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_W);
	}

	void jl_ct_key_keya(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_A);
	}

	void jl_ct_key_keys(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_S);
	}

	void jl_ct_key_keyd(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
		jl_ct_key(jlgr, inputfn, SDL_SCANCODE_D);
	}

#elif JL_PLAT == JL_PLAT_PHONE //if Android
	void tuch_cntr(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //touch center
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msy > .4f * jlgr->wm.ar) &&
			(jlgr->main.ct.msy < .6f * jlgr->wm.ar) &&
			(jlgr->main.ct.msx > .4f) &&
			(jlgr->main.ct.msx < .6f));
		inputfn(jlgr, jlgr->input.input);
	}
	//
	void tuch_nrrt(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //near right
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msx > .6f) &&
			(jlgr->main.ct.msx < .8f) &&
			(jlgr->main.ct.msy * jlgr->wm.ar>.2f) &&
			(jlgr->main.ct.msy * jlgr->wm.ar<.8f));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_nrlt(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //near left
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msx < .4f) &&
			(jlgr->main.ct.msx > .2f) &&
			(jlgr->main.ct.msy > .2f * jlgr->wm.ar) &&
			(jlgr->main.ct.msy < .8f * jlgr->wm.ar));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_nrup(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //near up
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msy < .4f * jlgr->wm.ar) &&
			(jlgr->main.ct.msy > .2f * jlgr->wm.ar) &&
			(jlgr->main.ct.msx >.2f) &&
			(jlgr->main.ct.msx <.8f));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_nrdn(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //near down
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msy > .6f * jlgr->wm.ar) &&
			(jlgr->main.ct.msy < .8f * jlgr->wm.ar) &&
			(jlgr->main.ct.msx > .2f) &&
			(jlgr->main.ct.msx < .8f));
		inputfn(jlgr, jlgr->input.input);
	}
	//

	void tuch_frrt(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//far right
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msx>.8f));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_frlt(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//far left
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msx<.2f));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_frup(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//far up
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msy<.2f * jlgr->wm.ar));
		inputfn(jlgr, jlgr->input.input);
	}

	void tuch_frdn(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//far down
		jlgr_input_press__(jlgr, jlgr->main.ct.input.click &&
			(jlgr->main.ct.msy>.8f * jlgr->wm.ar));
		inputfn(jlgr, jlgr->input.input);
	}
#endif

void press(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//Any touch
	jlgr_input_press__(jlgr, jlgr->main.ct.input.click);
	inputfn(jlgr, jlgr->input.input);
}

void jlgr_input_dont(jlgr_t* jlgr, jlgr_input_t input) { }

void jl_ct_key_menu(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	#if JL_PLAT == JL_PLAT_COMPUTER
	jl_ct_key(jlgr, inputfn, SDL_SCANCODE_APPLICATION); //xyrhpk
	#elif JL_PLAT == JL_PLAT_PHONE
	jl_ct_key(jlgr, inputfn, SDL_SCANCODE_MENU); //xyrhpk
	#endif
}

void jl_ct_txty(void) {
	SDL_StartTextInput();
}

void jl_ct_txtn(void) {
	SDL_StopTextInput();
}

float jl_ct_gmousex(jlgr_t* jlgr) {
	float x = jlgr->main.ct.msx;
//	x -= jlgr->gl.cp->cb.x;
//	x /= jlgr->gl.cp->cb.w;
	return x;
}

float jl_ct_gmousey(jlgr_t* jlgr) {
	float y = jlgr->main.ct.msy;
//	y -= jlgr->gl.cp->cb.y;
//	y /= jlgr->gl.cp->cb.h;
	return y;
}

static inline void jlgr_input_handle_events___platform_dependant(jlgr_t* jlgr) {
#if JL_PLAT == JL_PLAT_PHONE
	if( jlgr->main.ct.event.type==SDL_FINGERDOWN ) {
		jlgr->main.ct.msx = jlgr->main.ct.event.tfinger.x;
		jlgr->main.ct.input.click = 1;
		jlgr->main.ct.msy = jlgr->main.ct.event.tfinger.y * jlgr->wm.ar;
		if(jlgr->sg.cs != JL_SCR_SS) {
			jlgr->main.ct.msy = jlgr->main.ct.msy * 2.;
			jlgr->main.ct.msy -= jlgr->wm.ar;
			if(jlgr->main.ct.msy < 0.) jlgr->main.ct.input.click = 0;
		}

	}else if( jlgr->main.ct.event.type==SDL_FINGERUP ) {
		jlgr->main.ct.input.click = 0;
	}else if( jlgr->main.ct.event.type==SDL_KEYDOWN || jlgr->main.ct.event.type==SDL_KEYUP) {
		if( jlgr->main.ct.event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
			jlgr->main.ct.input.back =
				(jlgr->main.ct.event.type==SDL_KEYDOWN); //Back Key
	}
#elif JL_PLAT == JL_PLAT_COMPUTER
	uint8_t isNowDown = jlgr->main.ct.event.type == SDL_MOUSEBUTTONDOWN;
	uint8_t isNowUp = jlgr->main.ct.event.type == SDL_MOUSEBUTTONUP;
	if( isNowDown || isNowUp) {
		if(jlgr->main.ct.event.button.button == SDL_BUTTON_LEFT)
			jlgr->main.ct.input.click = isNowDown;
		else if(jlgr->main.ct.event.button.button == SDL_BUTTON_RIGHT)
			jlgr->main.ct.input.click_right = isNowDown;
		else if(jlgr->main.ct.event.button.button == SDL_BUTTON_MIDDLE)
			jlgr->main.ct.input.click_middle = isNowDown;
	}else if(jlgr->main.ct.event.wheel.type == SDL_MOUSEWHEEL) {
		//SDL 2.0.4 NYI
		uint8_t flip = 1;
			/*(direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1*/;
		int32_t x = flip * jlgr->main.ct.event.wheel.x;
		int32_t y = flip * jlgr->main.ct.event.wheel.y;
		if(jlgr->main.ct.event.wheel.y > 0)
			jlgr->main.ct.input.scroll_up = (y > 0) ? y : -y;
		else if(jlgr->main.ct.event.wheel.y < 0)
			jlgr->main.ct.input.scroll_down = (y > 0) ? y : -y;
		if(jlgr->main.ct.event.wheel.x > 0)
			jlgr->main.ct.input.scroll_right = (x > 0) ? x : -x;
		else if(jlgr->main.ct.event.wheel.x < 0)
			jlgr->main.ct.input.scroll_left = (x > 0) ? x : -x;
	}

#endif
}

static void jl_ct_handle_resize__(jlgr_t* jlgr) {
	if(jlgr->main.ct.event.type==SDL_WINDOWEVENT) { //Resize
		if((jlgr->main.ct.event.window.event == SDL_WINDOWEVENT_RESIZED)
			 && (SDL_GetWindowFromID(
				jlgr->main.ct.event.window.windowID) ==
					jlgr->wm.displayWindow->w))
		{
			JL_PRINT_DEBUG(jlgr->jl, "INPUT/RESIZE");
			jlgr_resz(jlgr,
				jlgr->main.ct.event.window.data1,
				jlgr->main.ct.event.window.data2);
		}
	}
}

static inline void jlgr_input_handle_events__(jlgr_t* jlgr) {
	if ( jlgr->main.ct.event.type == SDL_TEXTINPUT) {
//		JL_PRINT("%1s\n", &(jlgr->main.ct.event.text.text[0]));
		int i;
		for(i = 0; i < 32; i++)
			jlgr->main.ct.text_input[i] =
				jlgr->main.ct.event.text.text[i];
		jlgr->main.ct.read_cursor = 0;
	}else{
		jl_ct_handle_resize__(jlgr);
	}
	jlgr_input_handle_events___platform_dependant(jlgr);
}

static void jl_ct_testquit__(jlgr_t* jlgr) {
	if(jlgr->main.ct.back == 1) jl_mode_exit(jlgr->jl); // Back Button/Key
}

/**
 * THREAD: Main thread.
 * For a certain input, do something.
 * @param jlgr: The library context.
 * @param event: The event id
 * @param fn: The function to run for the event.
 * @param data: Parameter to "fn": "jlgr_input_t->data"
*/
void jlgr_input_do(jlgr_t *jlgr, uint8_t event, jlgr_input_fnct fn, void* data){
	void (*FunctionToRun_)(jlgr_t *jlgr, jlgr_input_fnct fn) =
		jlgr->main.ct.getEvents[event];

	if(jlgr->main.ct.getEvents[event] == NULL) {
		jl_print(jlgr->jl,"Null Pointer: jlgr->main.ct.getEvents.Event");
		jl_print(jlgr->jl,"event=%d", event);
		jl_sg_kill(jlgr->jl);
	}
	// Get the input.
	jlgr->main.ct.current_event = event;
	// Run input function.
	jlgr->input.input.data = data;
	FunctionToRun_(jlgr, fn);
}

void jl_ct_getevents_(jlgr_t* jlgr) {
	while(SDL_PollEvent(&jlgr->main.ct.event))
		jlgr_input_handle_events__(jlgr);
	jlgr->main.ct.keys = SDL_GetKeyboardState(NULL);
	//If Back key is pressed, then quit the program
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->main.ct.back = jl_ct_key_pressed(jlgr, SDL_SCANCODE_ESCAPE);
#elif JL_PLAT == JL_PLAT_PHONE
	jl_ct_state__(&jlgr->main.ct.back, jlgr->main.ct.input.back);
#endif
}

void jl_ct_quickloop_(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "INPUT_QUICKLOOP");
	jlgr->jl->has.quickloop = 1;
	if(jlgr->jl->has.input) {
		jl_ct_getevents_(jlgr);
		jl_ct_testquit__(jlgr);
	}else{
		while(SDL_PollEvent(&jlgr->main.ct.event))
			jl_ct_handle_resize__(jlgr);
	}
	jl_print_return(jlgr->jl, "INPUT_QUICKLOOP");
}

//Main Input Loop
void jl_ct_loop__(jlgr_t* jlgr) {
	if(jlgr->jl->has.quickloop) jlgr->jl->has.quickloop = 0;
	//Get the information on current events
	jl_ct_getevents_(jlgr);
	#if JL_PLAT == JL_PLAT_COMPUTER
		//Get Whether mouse is down or not and xy coordinates
		SDL_GetMouseState(&jlgr->main.ct.msxi,&jlgr->main.ct.msyi);

		int32_t mousex = jlgr->main.ct.msxi;
		int32_t mousey = jlgr->main.ct.msyi;
		//translate integer into float by clipping [0-1]
		jlgr->main.ct.msx = ((float)mousex) / jlgr_wm_getw(jlgr);
		jlgr->main.ct.msy = ((float)mousey) / jlgr_wm_geth(jlgr);
		if(jlgr->sg.cs != JL_SCR_SS) jlgr->main.ct.msy -= .5;
		jlgr->main.ct.msy *= jlgr->wm.ar;

		// Ignore mouse input on upper screen.
		if(jlgr->sg.cs != JL_SCR_SS && jlgr->main.ct.msy < 0.) {
			jlgr->main.ct.msy = 0.;
			// Show native cursor.
			if(!jlgr->main.ct.sc) {
				SDL_ShowCursor(SDL_ENABLE);
				jlgr->main.ct.sc = 1;
			}
		}else{
			// Show JL_Lib cursor
			if(jlgr->main.ct.sc) {
				SDL_ShowCursor(SDL_DISABLE);
				jlgr->main.ct.sc = 0;
			}
		}
		// F11 toggle fullscreen.
		if(jl_ct_key_pressed(jlgr, SDL_SCANCODE_F11) == 1)
			jlgr_wm_togglefullscreen(jlgr);
	#endif
	jl_ct_testquit__(jlgr);
	int i;
	for(i = 0; i < JL_CT_MAXX; i++) {
		jlgr->main.ct.checked[i] = 1;
	}
}

static inline void jlgr_input_fn_init__(jlgr_t* jlgr) {
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->main.ct.getEvents[JL_CT_COMP_RETN] = jl_ct_key_retn;
	jlgr->main.ct.getEvents[JL_CT_COMP_KEYW] = jl_ct_key_keyw;
	jlgr->main.ct.getEvents[JL_CT_COMP_KEYA] = jl_ct_key_keya;
	jlgr->main.ct.getEvents[JL_CT_COMP_KEYS] = jl_ct_key_keys;
	jlgr->main.ct.getEvents[JL_CT_COMP_KEYD] = jl_ct_key_keyd;
	jlgr->main.ct.getEvents[JL_CT_COMP_ARUP] = jl_ct_key_arup;
	jlgr->main.ct.getEvents[JL_CT_COMP_ARDN] = jl_ct_key_ardn;
	jlgr->main.ct.getEvents[JL_CT_COMP_ARRT] = jl_ct_key_arrt;
	jlgr->main.ct.getEvents[JL_CT_COMP_ARLT] = jl_ct_key_arlt;
//	jlgr->main.ct.getEvents[JL_CT_COMP_MSXY] = mous_over;
	jlgr->main.ct.getEvents[JL_CT_COMP_CLLT] = press;
	jlgr->main.ct.getEvents[JL_CT_COMP_MENU] = jl_ct_key_menu;
#elif JL_PLAT == JL_PLAT_PHONE
	jlgr->main.ct.getEvents[JL_CT_ANDR_TCCR] = tuch_cntr;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TFUP] = tuch_frup;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TFDN] = tuch_frdn;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TFRT] = tuch_frrt;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TFLT] = tuch_frlt;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TNUP] = tuch_nrup;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TNDN] = tuch_nrdn;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TNRT] = tuch_nrrt;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TNLT] = tuch_nrlt;
	jlgr->main.ct.getEvents[JL_CT_ANDR_TOUC] = press;
	jlgr->main.ct.getEvents[JL_CT_ANDR_MENU] = jl_ct_key_menu;
#endif
}

static inline void jl_ct_var_init__(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 255; i++)
		jlgr->main.ct.keyDown[i] = 0;
	jlgr->main.ct.read_cursor = 0;
	for(i = 0; i < 32; i++)
		jlgr->main.ct.text_input[i] = 0;
	jlgr->main.ct.sc = 0;
}

void jl_ct_init__(jlgr_t* jlgr) {
	jlgr_input_fn_init__(jlgr);
	jl_ct_var_init__(jlgr);
	jlgr->jl->has.input = 1;
}

/**
 * Get any keys that are currently being typed.  Output in Aski.
 * If phone, pops up keyboard if not already up.  If nothing is being typed,
 * returns 0.
*/
uint8_t jl_ct_typing_get(jlgr_t *jlgr) {
	if(!SDL_IsTextInputActive()) SDL_StartTextInput();
	uint8_t rtn = jlgr->main.ct.text_input[jlgr->main.ct.read_cursor];
	if(jl_ct_key_pressed(jlgr, SDL_SCANCODE_BACKSPACE) == 1) return '\b';
	if(jl_ct_key_pressed(jlgr, SDL_SCANCODE_DELETE) == 1) return '\02';
	if(jl_ct_key_pressed(jlgr, SDL_SCANCODE_RETURN) == 1) return '\n';
	if(!rtn) return 0;
	jlgr->main.ct.read_cursor++;
	return rtn;
}

/**
 * If phone, hides keyboard.
*/
void jl_ct_typing_disable(void) {
	SDL_StopTextInput();
}


/**
 * Returns 0 if key isn't pressed
 * Returns 1 if key is just pressed
 * Returns 2 if key is held down
 * Returns 3 if key is released
*/
uint8_t jl_ct_key_pressed(jlgr_t *jlgr, uint8_t key) {
	jl_ct_state__(&jlgr->main.ct.keyDown[key], jlgr->main.ct.keys[key]);
	return jlgr->main.ct.keyDown[key];
}
