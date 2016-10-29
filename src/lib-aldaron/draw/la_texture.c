/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "SDL_image.h"
#include "la_buffer.h"

#include <la_file.h>
#include <la_ro.h>
#include <la_window.h>

void la_mouse_draw__(la_window_t*);

uint32_t la_texture_ssp__(SDL_Surface* surface, int32_t x, int32_t y) {
	int32_t bpp = surface->format->BytesPerPixel;
	uint8_t *p = (uint8_t *)surface->pixels + (y * surface->pitch) + (x * bpp);
	uint32_t color_orig;
	uint32_t color;
	uint8_t* out_color = (void*)&color;

	if(bpp == 1) {
		color_orig = *p;
	}else if(bpp == 2) {
		color_orig = *(uint16_t *)p;
	}else if(bpp == 3) {
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			color_orig = p[0] << 16 | p[1] << 8 | p[2];
		else
			color_orig = p[0] | p[1] << 8 | p[2] << 16;
	}else{ // 4
		color_orig = *(uint32_t *)p;
	}
	SDL_GetRGBA(color_orig, surface->format, &(out_color[0]),
		&(out_color[1]), &(out_color[2]), &(out_color[3]));
	return color;
}

SDL_Surface* la_window_makesurface(la_window_t* window, la_buffer_t* data) {
	SDL_Surface *image;
	SDL_RWops *rw;

	la_print("File Size = %d", data->size);
	rw = SDL_RWFromMem(data->data + data->curs, data->size);
	if ((image = IMG_Load_RW(rw, 1)) == NULL)
		la_panic("Couldn't load image: %s", IMG_GetError());

	return image;
}

static inline void la_texture_load__(la_window_t* window, la_buffer_t* data,
	void **pixels, int *w, int *h)
{
	SDL_Surface *image;
	uint32_t color = 0;
	la_buffer_t pixel_data;
	int i, j;

	if(data->data[0] == 0) la_panic("NO DATA!");

	image = la_window_makesurface(window, data);
	// Covert SDL_Surface.
	la_buffer_init(&pixel_data);
	for(i = 0; i < image->h; i++) {
		for(j = 0; j < image->w; j++) {
			color = la_texture_ssp__(image, j, i);
			la_buffer_write(&pixel_data, &color, 4);
		}
	}
	//Set Return values
	*pixels = la_buffer_tostring(&pixel_data);
	*w = image->w;
	*h = image->h;
	// Clean-up
	SDL_free(image);
}

//Load the images in the image file
static inline uint32_t la_texture_fpk__(la_window_t* window,la_buffer_t* data) {
	void *fpixels = NULL;
	int fw;
	int fh;
	uint32_t rtn;

	la_print("size = %d", data->size);
//load textures
	la_texture_load__(window, data, &fpixels, &fw, &fh);
	la_print("creating image....");
	rtn = la_texture_new(window, fpixels, fw, fh, 0);
	la_print("created image!");
	return rtn;
}

/**
 * Load an image from a zipfile.
 * @param zipdata: data for a zip file.
 * @param filename: Name of the image file in the package.
 * @returns: Texture object.
*/
uint32_t la_texture_fpk(la_window_t* window, la_buffer_t* zipdata,
	const char* filename)
{
	la_buffer_t img;

	// Load image into "img"
	if(la_file_loadzip(&img, zipdata, filename))
		la_panic("add-image: pk_load_fdata failed!");

	la_print("Loading Image....");
	uint32_t rtn = la_texture_fpk__(window, &img);
	la_print("Loaded Image!");
	return rtn;
}

void la_window_icon(la_window_t* window,la_buffer_t* buffer,const char* fname) {
	la_buffer_t img;

	// Load data
	if(la_file_loadzip(&img, buffer, fname))
		la_panic("add-image: pk_load_fdata failed!");
	// Load image
	SDL_Surface* image = la_window_makesurface(window, &img);
	// Set icon
	SDL_SetWindowIcon(window->wm.window, image);
	// Free image
	SDL_free(image);
}

typedef struct{
	void* context;
	la_window_t* window;
}la_window_draw_t;

static void la_window_draw_flipped(la_window_draw_t* param) {
	la_draw_fn_t redraw = la_safe_get_pointer(
		&param->window->protected.functions.primary);

	// Clear the screen.
	la_window_clear(1., 1., 1., 1.);
	// Run the screen's redraw function
	redraw(param->context, param->window);
	// Draw Mouse
	#if defined(LA_COMPUTER)
	la_mouse_draw__(param->window);
	#endif
}

void la_window_draw__(void* context, la_window_t* window) {
	la_window_draw_t pass = (la_window_draw_t) { context, window };

	la_window_clear(1., 0., 0., 1.);
	// Draw over it.
	la_ro_pr(&pass, window, &window->screen, (la_fn_t) la_window_draw_flipped);
	la_ro_draw(&window->screen);
}

#endif
