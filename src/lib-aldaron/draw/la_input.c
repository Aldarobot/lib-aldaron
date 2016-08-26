/*
 * (c) Jeron A. Lau
 *
 * jl/input is a library for making input controllers compatible between
 * different
 * devices.
*/

#include "JLGRprivate.h"

#if JL_PLAT == JL_PLAT_COMPUTER
#elif JL_PLAT == JL_PLAT_PHONE
	extern char la_keyboard_press;
#endif

/*
 * "Preval" is a pointer to previous key pressed value.
 *	 0 if key isn't pressed
 *	 1 if key is just pressed
 *	 2 if key is held down
 *	 3 if key is released
 * "read" is the input variable.
*/
static void jl_ct_state__(int8_t *preval, const uint8_t read) {
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

static void jlgr_input_state__(la_window_t* jlgr, uint8_t read) {
/*	if(jlgr->input.used[jlgr->main.ct.current_event] == 0) {
		jl_ct_state__(&jlgr->input.states[jlgr->main.ct.current_event],
			read);
		jlgr->input.used[jlgr->main.ct.current_event] = 1;
	}*/
}

/*
 * Returns 0 if key isn't pressed
 * Returns 1 if key is just pressed
 * Returns 2 if key is held down
 * Returns 3 if key is released
*/
uint8_t jl_ct_key_pressed__(la_window_t *jlgr, uint8_t key) {
//	jl_ct_state__(&jlgr->main.ct.keyDown[key], jlgr->main.ct.keys[key]);
	return jlgr->main.ct.keyDown[key];
}

static void jlgr_input_press2__(la_window_t* jlgr, uint8_t countit) {
/*	jlgr_input_state__(jlgr, countit);
	//hrxypk
	jlgr->input.input.h = jlgr->input.states[jlgr->main.ct.current_event];
	jlgr->input.input.r = 0.;
	jlgr->input.input.x = al_safe_get_float(&jlgr->main.ct.msx);
	jlgr->input.input.y = al_safe_get_float(&jlgr->main.ct.msy);
	if(countit) {
		jlgr->input.input.k = 1;
		jlgr->input.input.p = 1.;
	}else{
		jlgr->input.input.k = 0;
		jlgr->input.input.p = 0.;
	}*/
}

void jl_ct_key(la_window_t *jlgr, jlgr_input_fnct inputfn, uint8_t key) {
/*	uint8_t a = jl_ct_key_pressed__(jlgr, key);

	jlgr->input.input.h = a;
	jlgr->input.input.r = 0.;
	if(a && (a!=3)) {
		jlgr->input.input.p = 1.;
		jlgr->input.input.k = key;
	}else{
		jlgr->input.input.p = 0.;
		jlgr->input.input.k = 0;
	}
	inputfn(jlgr, jlgr->input.input);*/
}

static void jlgr_input_fourdir(la_window_t *jlgr, jlgr_input_fnct inputfn,
	uint8_t up, uint8_t dn, uint8_t lt, uint8_t rt)
{
/*	uint8_t key;
	// XY
	if(up) {
		key = JLGR_INPUT_DIR_UP;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = -1.;
	}else if(dn) {
		key = JLGR_INPUT_DIR_DN;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = 1.;
	}else if(lt) {
		key = JLGR_INPUT_DIR_LT;
		jlgr->input.input.x = -1.;
		jlgr->input.input.y = 0.;
	}else if(rt) {
		key = JLGR_INPUT_DIR_RT;
		jlgr->input.input.x = 1.;
		jlgr->input.input.y = 0.;
	}else{
		key = JLGR_INPUT_DIR_NO;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = 0.;
	}
	// HRPK
	jlgr_input_state__(jlgr, key);
	jlgr->input.input.h = jlgr->input.states[jlgr->main.ct.current_event];
	jlgr->input.input.r = 0.;
	jlgr->input.input.p = key ? 1. : 0.;
	jlgr->input.input.k = key ? key : 0;
	inputfn(jlgr, jlgr->input.input);*/
}

// Get Input

static void jlgr_input_return__(la_window_t *jlgr, jlgr_input_fnct inputfn) {
//	jl_ct_key(jlgr, inputfn, SDL_SCANCODE_RETURN);
}

static void jlgr_input_arrow__(la_window_t *jlgr, jlgr_input_fnct inputfn) {
/*	jlgr_input_fourdir(jlgr, inputfn,
		jlgr->main.ct.keys[SDL_SCANCODE_UP],
		jlgr->main.ct.keys[SDL_SCANCODE_DOWN],
		jlgr->main.ct.keys[SDL_SCANCODE_LEFT],
		jlgr->main.ct.keys[SDL_SCANCODE_RIGHT]);*/
}

static void jlgr_input_wasd__(la_window_t *jlgr, jlgr_input_fnct inputfn) {
/*	jlgr_input_fourdir(jlgr, inputfn,
		jlgr->main.ct.keys[SDL_SCANCODE_W],
		jlgr->main.ct.keys[SDL_SCANCODE_S],
		jlgr->main.ct.keys[SDL_SCANCODE_A],
		jlgr->main.ct.keys[SDL_SCANCODE_D]);*/
}

/*void jlgr_input_touch_center(la_window_t *jlgr, jlgr_input_fnct inputfn) { //touch center
	jlgr_input_press2__(jlgr, jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy > .4f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy < .6f * jlgr->wm.ar) &&
		(jlgr->main.ct.msx > .4f) &&
		(jlgr->main.ct.msx < .6f));
	inputfn(jlgr, jlgr->input.input);
}*/

void jlgr_input_dirnear__(la_window_t *jlgr, jlgr_input_fnct inputfn) {
/*	float msx = al_safe_get_float(&jlgr->main.ct.msx);
	float msy = al_safe_get_float(&jlgr->main.ct.msy);
	uint8_t near_right = jlgr->main.ct.input.click &&
		(msx > .6f) &&
		(msx < .8f) &&
		(msy > .2f * jlgr->wm.ar) &&
		(msy < .8f * jlgr->wm.ar);
	uint8_t near_left = jlgr->main.ct.input.click &&
		(msx < .4f) &&
		(msx > .2f) &&
		(msy > .2f * jlgr->wm.ar) &&
		(msy < .8f * jlgr->wm.ar);
	uint8_t near_up = jlgr->main.ct.input.click &&
		(msy < .4f * jlgr->wm.ar) &&
		(msy > .2f * jlgr->wm.ar) &&
		(msx >.2f) &&
		(msx <.8f);
	uint8_t near_down = jlgr->main.ct.input.click &&
		(msy > .6f * jlgr->wm.ar) &&
		(msy < .8f * jlgr->wm.ar) &&
		(msx > .2f) &&
		(msx < .8f);

	jlgr_input_fourdir(jlgr,inputfn,near_up,near_down,near_left,near_right);*/
}

void jlgr_input_dirfar__(la_window_t *jlgr, jlgr_input_fnct inputfn) {
/*	float msx = al_safe_get_float(&jlgr->main.ct.msx);
	float msy = al_safe_get_float(&jlgr->main.ct.msy);
	uint8_t far_right = jlgr->main.ct.input.click && (msx>.8f);
	uint8_t far_left = jlgr->main.ct.input.click && (msx<.2f);
	uint8_t far_up = jlgr->main.ct.input.click && (msy<.2f * jlgr->wm.ar);
	uint8_t far_down = jlgr->main.ct.input.click && (msy>.8f * jlgr->wm.ar);

	jlgr_input_fourdir(jlgr,inputfn, far_up, far_down, far_left, far_right);*/
}

void jlgr_input_press__(la_window_t *jlgr, jlgr_input_fnct inputfn) {//Any touch
/*	jlgr_input_press2__(jlgr, jlgr->main.ct.input.click);
	inputfn(jlgr, jlgr->input.input);*/
}

//

void jlgr_input_dont(la_window_t* jlgr, jlgr_input_t input) { }

void jl_ct_key_menu(la_window_t *jlgr, jlgr_input_fnct inputfn) {
//	jl_ct_key(jlgr, inputfn, SDL_SCANCODE_APPLICATION); //xyrhpk
}

static inline void la_input_clamp_msy(la_window_t* jlgr, float* msy) {
	if(jlgr->sg.cs != JL_SCR_SS) {
		*msy -= .5;
		*msy *= 2.;
	}
	*msy *= jlgr->wm.ar;
}

static inline void jlgr_input_handle_events_platform_dependant__(la_window_t* jlgr) {
#if JL_PLAT == JL_PLAT_PHONE
	if( jlgr->main.ct.event.type==SDL_FINGERDOWN ) {
		float msy;

		jlgr->main.ct.input.click = 1;
		msy = jlgr->main.ct.event.tfinger.y;

		la_input_clamp_msy(jlgr, &msy);
		if(msy < 0.) jlgr->main.ct.input.click = 0;

		al_safe_set_float(&jlgr->main.ct.msx,
			jlgr->main.ct.event.tfinger.x);
		al_safe_set_float(&jlgr->main.ct.msy, msy);
	}else if( jlgr->main.ct.event.type==SDL_FINGERUP ) {
		jlgr->main.ct.input.click = 0;
	}else if( jlgr->main.ct.event.type==SDL_KEYDOWN || jlgr->main.ct.event.type==SDL_KEYUP) {
		if( jlgr->main.ct.event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
			jlgr->main.ct.input.back =
				(jlgr->main.ct.event.type==SDL_KEYDOWN); //Back Key
	}
#elif JL_PLAT == JL_PLAT_COMPUTER


#endif
}

static void jl_ct_handle_resize__(la_window_t* jlgr) {
	if(jlgr->main.ct.event.type==SDL_WINDOWEVENT) { //Resize
		switch(jlgr->main.ct.event.window.event) {
			case SDL_WINDOWEVENT_RESIZED: {
				jlgr_resz(jlgr,
					jlgr->main.ct.event.window.data1,
					jlgr->main.ct.event.window.data2);
				break;
			}
#if JL_PLAT == JL_PLAT_COMPUTER
			case SDL_WINDOWEVENT_CLOSE: {
				jlgr->main.ct.back = 1;
				break;
			}
#endif
			default: {
				break;
			}
		}
	}
}

/*static inline uint8_t jlgr_input_do__(la_window_t *jlgr, jlgr_control_t events) {
	int8_t event;

	#if JL_PLAT == JL_PLAT_COMPUTER
		event = events.computer;
	#elif JL_PLAT == JL_PLAT_PHONE
		event = events.phone;
	#else // Game
		event = events.game;
	#endif
	if(events.computer == JLGR_INPUT_NONE) return -1;
	if(jlgr->input.states[events.computer] == -1) {
		#if JL_PLAT == JL_PLAT_COMPUTER
			event = events.computer_backup;
		#elif JL_PLAT == JL_PLAT_PHONE
			event = events.phone_backup;
		#else // Game
			event = events.game_backup;
		#endif
		if(events.computer == JLGR_INPUT_NONE) return -1;
	}
	return event;
}*/

/**
 * THREAD: Main thread.
 * For a certain input, do something.
 * @param jlgr: The library context.
 * @param event: The event id
 * @param fn: The function to run for the event.
 * @param data: Parameter to "fn": "jlgr_input_t->data"
 * @returns -1 on failure, 0 on success.
*/
int8_t jlgr_input_do(la_window_t *jlgr, jlgr_control_t events, jlgr_input_fnct fn,
	void* data)
{
/*	int8_t event = jlgr_input_do__(jlgr, events);
	if(event == -1) return -1;
	void (*FunctionToRun_)(la_window_t *jlgr, jlgr_input_fnct fn) =
		jlgr->main.ct.getEvents[event];

	if(jlgr->main.ct.getEvents[event] == NULL) {
		la_print("Null Pointer: jlgr->main.ct.getEvents.Event");
		la_panic("event=%d", event);
	}
	// Get the input.
	jlgr->main.ct.current_event = event;
	// Run input function.
	jlgr->input.input.data = data;
	FunctionToRun_(jlgr, fn);*/
	return 0;
}

//Main Input Loop
void jl_ct_loop__(la_window_t* jlgr) {
	
	#if JL_PLAT == JL_PLAT_COMPUTER
		//Get Whether mouse is down or not and xy coordinates
		SDL_GetMouseState(&jlgr->main.ct.msxi,&jlgr->main.ct.msyi);

		int32_t mousex = jlgr->main.ct.msxi;
		int32_t mousey = jlgr->main.ct.msyi;
		//translate integer into float by clipping [0-1]
		float msy = ((float)mousey) / jlgr_wm_geth(jlgr);

		la_input_clamp_msy(jlgr, &msy);

		// F11 toggle fullscreen.
		if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_F11) == 1)
			jlgr_wm_togglefullscreen(jlgr);
	#endif
}

static inline void jl_ct_var_init__(la_window_t* jlgr) {
	int i;
	for(i = 0; i < 255; i++)
		jlgr->main.ct.keyDown[i] = 0;
	jlgr->main.ct.read_cursor = 0;
	for(i = 0; i < 32; i++)
		jlgr->main.ct.text_input[i] = 0;
	jlgr->main.ct.sc = 0;
}

void jl_ct_init__(la_window_t* jlgr) {
//	jl_ct_var_init__(jlgr);
	jlgr->jl->has.input = 1;
}

/**
 * Get any keys that are currently being typed.
 * If phone, pops up keyboard if not already up.  If nothing is being typed,
 * returns 0.
 * @param jlgr: The library context.
 * @returns: The key pressed.
*/
uint8_t jlgr_input_typing_get(la_window_t *jlgr) {
#ifdef LA_PHONE_ANDROID
#else
/*	uint8_t rtn = jlgr->main.ct.text_input[jlgr->main.ct.read_cursor];

	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_BACKSPACE) == 1) return '\b';
	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_DELETE) == 1) return '\02';
	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_RETURN) == 1) return '\n';

	if(rtn) jlgr->main.ct.read_cursor++;
	return rtn;*/
	return 0;
#endif
}

/**
 * Stop receiving input from jl_input_typing_get(). If phone, hides keyboard.
*/
void jlgr_input_typing_disable(void) {
#ifdef LA_PHONE_ANDROID
#else
//	SDL_StopTextInput();
#endif
}
