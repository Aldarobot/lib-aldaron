/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_PORT
#define LA_PORT

#include <stdint.h>

#ifdef LA_ANDROID
#include <EGL/egl.h>
#include <android/sensor.h>
#endif

#include <la_safe.h>
#include <la_buffer.h>
#include <la_math.h>
#include <la_ro_type.h>

// Print 
#define LA_PRESET "\x1B[0m"

#define LA_PBOLD "\x1B[1m"
#define LA_PBLINK "\x1B[5m"

#define LA_PBLACK "\x1B[30m"
#define LA_PRED "\x1B[31m"
#define LA_PGREEN "\x1B[32m"
#define LA_PYELLOW "\x1B[33m"
#define LA_PBLUE "\x1B[34m"
#define LA_PMAGENTA "\x1B[35m"
#define LA_PCYAN "\x1B[36m"
#define LA_PWHITE "\x1B[37m"

#define LA_PBLACKH "\x1B[40m"
#define LA_PREDH "\x1B[41m"
#define LA_PGREENH "\x1B[42m"
#define LA_PYELLOWH "\x1B[43m"
#define LA_PBLUEH "\x1B[44m"
#define LA_PMAGENTAH "\x1B[45m"
#define LA_PCYANH "\x1B[46m"
#define LA_PWHITEH "\x1B[47m"

// Print eXtension
#define LA_PXSIZE(size) "\x1B[f" size ";"
#define LA_PXMOVE(x, y) "\x1B[m" x "," y ";"
#define LA_PXCOLOR(r, g, b, a) "\x1B[c" r "," g "," b "," a ";"
#define LA_PXWIDTH(width) "\x1B[w" width ";"
#define LA_PXLIMIT(limit) "\x1B[l" limit ";"
#define LA_PXBGCOLOR(r, g, b, a) "\x1B[g" r "," g "," b "," a ";"
#define LA_PXBGOFF "\x1B[og"

//

#ifdef LA_DRAW

typedef struct{
	float x; // X Location ( -1.f to 1.f )
	float y; // Y Location ( -1.f to 1.f )
	float z; // Z Location / Rotation ( -1.f to 1.f )
	uint8_t p; // Pressure 0-255
	uint8_t h; // true if just pressed, otherwise false.
	uint8_t k; // Which key [ a-z, 0-9 , left/right click ]
}la_input_t;

typedef struct {
#ifdef LA_ANDROID
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;

	struct {
		struct {
			safe_uint8_t p;
			safe_uint8_t h;
		} touch;

		safe_uint8_t back;
	} in;
#endif
#ifdef LA_COMPUTER
	SDL_Event sdl_event;
#endif
	struct {
		la_input_t touch; // Touchscreen Tap
		la_input_t mouse; // Mouse Location / L/M/R Click
		la_input_t scroll; // Mouse Scroll Wheel
		la_input_t keyboard; // Physical and Virtual keyboard.
		la_input_t drag; // Drag with left click, or finger on touchscreen
		la_input_t joystick_a; // Physical or Virtual joystick (phone)
		la_input_t joystick_b; // Physical or Virtual joystick (phone)
		la_input_t lbutton; // L Button
		la_input_t rbutton; // R Button
		la_input_t zlbutton; // ZL Button
		la_input_t zrbutton; // ZR Button
		la_input_t accel; // Accelerometer

		char text[32]; // Unicode Text Input.
	} input;

	la_ro_t mouse;
	la_ro_t screen;

	safe_uint8_t needs_resize;
	safe_uint32_t set_width;
	safe_uint32_t set_height;
	safe_uint8_t has_2_screens; // If 2 screens are hooked up.

	struct {
		safe_pointer_t fn;
		safe_pointer_t resize;
		safe_pointer_t primary;
		safe_pointer_t secondary;
	} functions;

	struct {
		float colors[4];
	}effects;

	struct {
		la_shader_t shader_laa[32]; // Light Ambient Attenuation
		uint8_t shader_laa_init[32];

		la_shader_t shadow;
		la_shader_t alpha;
		la_shader_t hue;
	} effect;
	
	//Opengl Data
	struct {
		struct {
			la_shader_t texture;
			la_shader_t color;
		}prg;

		la_ro_t temp_vo;
		// Default texture coordinates.
		uint32_t default_tc;
		uint32_t upsidedown_tc;
		
		la_ro_t* cp; // Renderer currently being drawn on.
	}gl;

	// Gui
	struct {
		struct {
			la_ro_t whole_screen;
		}vos;
		struct {
			char message[256];
			uint16_t t;
			uint8_t c;
		}msge;
	}gui;

	// Window Management
	struct {
	#if defined(LA_COMPUTER)
		uint8_t fullscreen;

		SDL_Window* window;
		SDL_GLContext* glcontext;
	#endif
		char windowTitle[2][17];
		// The full width and height of the window.
		int32_t w, h;
		// Aspect Ratio of the window
		float ar;
	}wm;

	struct {
		uint32_t font; // Lib Aldaron Default font
		uint32_t logo; // Lib Aldaron Loading Logo
		uint32_t game; // Game Graphics
		uint32_t icon; // Icons
		uint32_t cursor;
		uint32_t backdrop;
	}textures;

	la_buffer_t packagedata;

	double timer;
	double psec;
	uint8_t on_time;

	// Mouse x & y
	safe_float_t mouse_x;
	safe_float_t mouse_y;
} la_window_t;

// These functions must be supported by all platforms.

// Poll for input: return 1 if back (escape) key pressed.
void la_port_input(la_window_t* window);
void la_port_swap_buffers(la_window_t* window);

#endif

void la_print(const char* format, ...);

#endif
