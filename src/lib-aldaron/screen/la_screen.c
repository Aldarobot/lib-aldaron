/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "la_draw.h"
#include "la_memory.h"

#include <la_thread.h>
#include <la_window.h>
#include <la_ro.h>
#include <la_gui.h>

typedef struct {
	void* context;
	la_window_t* window;
	la_fn_t* loop;
	la_fn_t kill;
} la_main_thread_t;

extern SDL_atomic_t la_rmcexit;
SDL_atomic_t la_rmcwait;

void la_window_update_size(la_window_t*);
void la_mouse_resize__(la_window_t*);
void la_window_resize__(la_window_t*, uint32_t, uint32_t);

void la_window_draw__(void*, la_window_t*);

void* aldaron_data(void);
uint64_t aldaron_size(void);

void la_llgraphics_initshader_color__(la_window_t*);
void la_llgraphics_init__(la_window_t*);
void la_effects_init__(la_window_t*);
void la_window_init__(la_window_t*);

void la_window_update__(la_window_t*);

static void la_draw_programsresize__(void* context, la_window_t* window) {
	((la_draw_fn_t)la_safe_get_pointer(&window->functions.resize))
		(context, window);
}

static inline void la_draw_windowresize__(void* context, la_window_t* window) {
	uint32_t w = la_safe_get_uint32(&window->set_width);
	uint32_t h = la_safe_get_uint32(&window->set_height);

	la_window_update_size(window);
	if(w == 0) w = la_window_width(window);
	if(h == 0) h = la_window_height(window);

	la_print("Resizing to %dx%d....", w, h);
	la_window_resize__(window, w, h);
	la_print("User's resize....");
	la_draw_programsresize__(context, window);
	la_print("Resizing the mouse....");
	la_mouse_resize__(window);
	la_print("Resized.");
}

static void la_draw_loader(void* context, la_window_t* window) {
	la_gui_bg(window, window->textures.backdrop);
	la_ro_image_rect(window, &window->gl.temp_vo, window->textures.logo, 1.,
		90./251.);
	la_ro_move(&window->gl.temp_vo, (la_v3_t) {
		0.f, (la_ro_ar(window) - (90./251.)) / 2.f, 0.f });
	la_ro_draw(&window->gl.temp_vo);
}

static void la_draw_loader_f(void* context, la_window_t* window) {
	SDL_AtomicSet(&la_rmcwait, 0);
	((la_draw_fn_t)la_safe_get_pointer(&window->functions.fn))(
		context, window);
}

static void la_draw_loader_e(void* context, la_window_t* window) {
	window->textures.game = la_texture_fpk(window, &window->packagedata,
		"/landscape.png");
	window->textures.font = la_texture_fpk(window, &window->packagedata,
		"/font.png");
	window->textures.icon = la_texture_fpk(window, &window->packagedata,
		"/taskbar_items.png");
	la_draw_loader(context, window);
	la_screen_setprimary(window, la_draw_loader_f);
}

static void la_draw_loader_d(void* context, la_window_t* window) {
	la_llgraphics_initshader_color__(window);
	// Load other images....
	la_draw_loader(context, window);
	la_screen_setprimary(window, la_draw_loader_e);
}

static void la_draw_loader_c(void* context, la_window_t* window) {
	la_effects_init__(window);
	la_draw_loader(context, window);
	la_screen_setprimary(window, la_draw_loader_d);
}

static void la_draw_loader_b(void* context, la_window_t* window) {
	window->textures.logo = la_texture_fpk(window, &window->packagedata,
		"/logo.png");
	la_draw_loader(context, window);
	la_screen_setprimary(window, la_draw_loader_c);
}

static void la_draw_loader_a(void* context, la_window_t* window) {
	window->textures.backdrop = la_texture_fpk(window, &window->packagedata,
		"/backdrop.png");
	la_gui_bg(window, window->textures.backdrop);
	la_screen_setprimary(window, la_draw_loader_b);
}

static inline void la_draw_init__(void* context, la_window_t* window) {
	// Initialize subsystems
	la_print("Creating the window....");
	la_window_init__(window);
	la_print("Setting up OpenGL....");
	la_llgraphics_init__(window);
	la_print("Resize....");

	la_buffer_fromdata(&window->packagedata, aldaron_data(), aldaron_size());
	window->textures.cursor = la_texture_fpk(window, &window->packagedata,
		"/cursor.png");
	// Set window loops
	la_screen_setprimary(window, la_draw_loader_a);
	la_safe_set_pointer(&window->functions.secondary, la_draw_dont);
	la_safe_set_pointer(&window->functions.resize, la_draw_dont);
	la_safe_set_uint8(&window->needs_resize, 2);

	la_print("Window Created!");
}

void la_draw_checkresize__(void* context, la_window_t* window) {
	uint8_t should_resize =
		la_safe_get_uint8(&window->needs_resize);
	la_safe_set_uint8(&window->needs_resize, 0);

	if(should_resize == 1) la_draw_programsresize__(context, window);
	if(should_resize == 2) la_draw_windowresize__(context, window);
//TODO: REMOVE
	la_window_clear(0.f, 1.f, 0.f, 1.f);
}

void la_window_loop__(void* context, la_window_t* window) {
	// Check for resize
	la_draw_checkresize__(context, window);
	// Deselect any pre-renderer.
	window->gl.cp = NULL;
	//Redraw screen.
	la_window_draw__(context, window);
	//Update Screen.
	la_window_update__(window);
}

void la_window_kill__(la_window_t* window) {
	SDL_AtomicSet(&la_rmcexit, 0);
#ifndef LA_ANDROID
	la_print("Destroying window....");
	SDL_DestroyWindow(window->wm.window);
#endif
	la_print("Killed DRAW Subsystem!");
}

static int32_t la_main_thread(la_main_thread_t* ctx) {
	while(SDL_AtomicGet(&la_rmcwait));
	while(SDL_AtomicGet(&la_rmcexit)) {
		// Poll For Input & Regulate FPS
		la_port_input(ctx->window);
		// Program loop
		(*(ctx->loop))(ctx->context);
	}
	la_print("Kill The Loop -> 0.");
	ctx->kill(ctx->context);
	la_print("Exit Program -> 0.");
	return 0;
}

/**
 * Create a window.
 * @param window: The window.
 * @param fn_: Graphic initialization function run on graphical thread.
**/
void la_window_start__(void* context, la_window_t* window, la_draw_fn_t fn_,
	la_fn_t* loop, la_fn_t fnc_kill)
{
	la_thread_t mti; // Main Thread Id

	// Set init function
	la_safe_set_pointer(&window->functions.fn, fn_);
	// Initialize subsystems
	la_draw_init__(context, window);
	// Main loop delay
	SDL_AtomicSet(&la_rmcwait, 1);
	// Branch a new thread.
	la_main_thread_t ctx = (la_main_thread_t) { context, window,
		loop, fnc_kill };
	la_thread_new(&mti, (la_thread_fn_t)la_main_thread, "la_main", &ctx);
	printf("SSSSSSSSSSSS# %p\n", mti.thread);
#ifndef LA_ANDROID
	// Redraw loop
	while(SDL_AtomicGet(&la_rmcexit)) la_window_loop__(context, window);
	// Wait on other thread
	la_thread_old(&mti);
	// Free stuff.
	la_window_kill__(window);
#endif
}

void la_draw_dont(void* context, la_window_t* window) { return; }

void la_screen_setprimary(la_window_t* window, la_draw_fn_t new_fn) {
	la_safe_set_pointer(&window->functions.primary, new_fn);
}

/**
 * Set the functions to be called when the window redraws.
 * @param onescreen: The function to redraw the screen when there's only 1 
 *  screen.
 * @param upscreen: The function to redraw the upper or primary display.
 * @param downscreen: The function to redraw the lower or secondary display.
 * @param resize: The function called when window is resized.
**/
void la_draw_fnchange(la_window_t* window, la_draw_fn_t primary,
	la_draw_fn_t secondary, la_draw_fn_t resize)
{
	la_safe_set_pointer(&window->functions.primary, primary);
	la_safe_set_pointer(&window->functions.secondary, secondary);
	la_safe_set_pointer(&window->functions.resize, resize);
	la_safe_set_uint8(&window->needs_resize, 1);
}

/**
 * Resize the window.
**/
void la_draw_resize(la_window_t* window, uint32_t w, uint32_t h) {
	la_safe_set_uint32(&window->set_width, w);
	la_safe_set_uint32(&window->set_height, h);
	la_safe_set_uint8(&window->needs_resize, 2);
}

#endif
