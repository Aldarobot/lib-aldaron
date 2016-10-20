/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_VIDEO

#include "la_memory.h"
#include "JLGRprivate.h"
#include "SDL_image.h"
#include "jpeglib.h"
#include "la_video.h"

typedef long unsigned int jpeg_long_int_t;
typedef JSAMPROW jpeg_sample_row_t;
typedef struct jpeg_compress_struct jpeg_compress_struct_t;
typedef struct jpeg_error_mgr jpeg_error_mgr_t;

uint32_t la_texture_ssp__(SDL_Surface* surface, int32_t x, int32_t y);

void la_video_make_jpeg(la_buffer_t* rtn, uint8_t quality, uint8_t* pxdata,
	uint32_t w, uint32_t h)
{
	uint8_t* data = NULL;
	const int32_t row_stride = w * 3;
	jpeg_long_int_t data_size = 0;
	jpeg_compress_struct_t cinfo;
	jpeg_error_mgr_t jerr;
	jpeg_sample_row_t row_pointer[1];

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &data, &data_size);
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & pxdata[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	la_buffer_fromdata(rtn, data, data_size);
}

void la_video_load_jpeg(void* output, void* data, uint32_t size, uint32_t* w,
	uint32_t* h)
{
	SDL_Surface *image; //  Free'd by SDL_free(image);
	SDL_RWops *rw; // Free'd by SDL_RWFromMem
	uint32_t color = 0;
	uint8_t* pixels = output;

	rw = SDL_RWFromMem(data, size);
	if ((image = IMG_Load_RW(rw, 0)) == NULL)
		la_panic("Couldn't load image: %s", IMG_GetError());
	// Covert SDL_Surface.
	for(int i = 0; i < image->h; i++) {
		for(int j = 0; j < image->w; j++) {
			color = la_texture_ssp__(image, j, i);
			la_memory_copy(&color,
				&(pixels[((i * image->w) + j) * 3]), 3);
		}
	}
	//Set Return values
	*w = image->w;
	*h = image->h;
	// Clean-up
	SDL_FreeSurface(image);
	SDL_free(rw);
}

#endif
