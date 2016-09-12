#ifndef LA_PORT
#define LA_PORT

#include <stdint.h>

#ifdef LA_PHONE_ANDROID
#include <EGL/egl.h>
#include <android/sensor.h>
#endif

#include "la_safe.h"

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

//

#ifdef LA_DRAW

typedef struct{
	float x; // X Location ( -1.f to 1.f )
	float y; // Y Location ( -1.f to 1.f )
	float z; // Z Location / Rotation ( -1.f to 1.f )
	uint8_t p; // Pressure 0-255
	uint8_t h; // true if just pressed, otherwise false.
	uint8_t k; // Which key [ a-z, 0-9 , left/right click ]
}jlgr_input_t;

typedef struct {
#ifdef LA_PHONE_ANDROID
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
		jlgr_input_t touch; // Touchscreen Tap
		jlgr_input_t mouse; // Mouse Location / L/M/R Click
		jlgr_input_t scroll; // Mouse Scroll Wheel
		jlgr_input_t keyboard; // Physical and Virtual keyboard.
		jlgr_input_t drag; // Drag with left click, or finger on touchscreen
		jlgr_input_t joystick_a; // Physical or Virtual joystick (phone)
		jlgr_input_t joystick_b; // Physical or Virtual joystick (phone)
		jlgr_input_t lbutton; // L Button
		jlgr_input_t rbutton; // R Button
		jlgr_input_t zlbutton; // ZL Button
		jlgr_input_t zrbutton; // ZR Button
		jlgr_input_t accel; // Accelerometer

		char text[32]; // Unicode Text Input.
	} input;

// from JLGR.h
	jl_t* jl;

	// For Programer's Use
	float fontcolor[4];
	jl_font_t font;
	jl_vo_t mouse;

	struct {
		jl_mutex_t mutex;

		struct {
			float timeTilVanish;
			char message[256];
		} notification;

		struct {
			jl_fnct fn;
			jlgr_redraw_t redraw;
		} functions;

		uint8_t needs_resize;
		uint16_t set_width;
		uint16_t set_height;
	} protected;

	// Window Info
	struct {
		uint32_t taskbar[5];
		uint32_t init_image_location;

		// If Matching FPS
		uint8_t on_time;
		uint8_t changed;
		
		// Each screen is a sprite.
		struct {
			jl_sprite_t up;
			jl_sprite_t dn;
		}bg;

		void* loop; // ( jlgr_fnct ) For upper or lower screen.
		uint8_t cs; // The current screen "jlgr_which_screen_t"
	}sg;

	struct {
		struct {
			jlgr_glsl_t shader;
		}shadow;

		struct {
			jlgr_glsl_t shader;
			int32_t fade;
		}alpha;

		struct {
			jlgr_glsl_t shader;
			int32_t new_color;
		}hue;

		struct {
			jlgr_glsl_t aa;

			int32_t norm;
			int32_t lightPos;
			int32_t color;
			int32_t ambient;
			int32_t shininess;

			// Shader drawing settings
			jl_vec3_t light_position;
			jl_vec3_t light_color;
			float light_power;
			jl_vec3_t material_brightness;
			int32_t light_texture;
		}light;

		float colors[4];
		jl_vec3_t* vec3;
		jl_vo_t* vo;

		jlgr_effects_light_t lights;
	}effects;

	struct {
		jlgr_glsl_t shader_laa[32]; // Light Ambient Attenuation
		uint8_t shader_laa_init[32];
	} effect;
	
	//Opengl Data
	struct {
		struct {
			jlgr_glsl_t texture;
			jlgr_glsl_t color;
		}prg;

		jl_vo_t temp_vo;
		// Default texture coordinates.
		uint32_t default_tc;
		uint32_t upsidedown_tc;
		
		jl_pr_t* cp; // Renderer currently being drawn on.
	}gl;

	struct {
		jl_mutex_t mutex;
		jl_vo_t menubar;

		// Protected ....
		// Used for all icons on the menubar.
		jl_vo_t icon;
		jl_vo_t shadow;
		// Redraw Functions for 10 icons.
		void* redrawfn[10];
		// Loop Functions for 10 icons.
		void* inputfn[10];
		// Cursor
		int8_t cursor;
		// What needs redrawing - -1 nothing -2 all
		int8_t redraw;
	}menu;

	// Gui
	struct {
		struct {
			char* window_name;
			char* message;
			jl_popup_button_t* btns;
		}popup;
		struct {
			jl_vo_t whole_screen;
		}vos;
		struct {
			char message[256];
			uint16_t t;
			uint8_t c;
		}msge;
		struct{
			data_t* string;
			float counter;
			uint8_t do_it;
			uint8_t cursor;
		}textbox;
	}gui;

	// Window Management
	struct {
	#if JL_PLAT == JL_PLAT_COMPUTER
		uint8_t fullscreen;

		SDL_Window* window;
		SDL_GLContext* glcontext;
	#endif
		char windowTitle[2][16];
		// The full width and height of the window.
		int32_t w, h;
		// Aspect Ratio of the window
		float ar;
	}wm;

	// File Manager
	struct {
		struct cl_list *filelist; //List of all files in working dir.
		int8_t cursor;
		uint8_t cpage;
		char *dirname;
		char *selecteditem;
		uint8_t returnit;
		uint8_t drawupto;
		uint8_t inloop;
		jl_sprite_t btns[2];
		void *newfiledata;
		uint64_t newfilesize;
		uint8_t prompt;
		data_t* promptstring;
	}fl;

	struct {
		uint32_t font; // JL_Lib font
		uint32_t logo; // JL_Lib Loading Logo
		uint32_t game; // Game Graphics
		uint32_t icon; // Icons
		uint32_t cursor;
	}textures;

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
