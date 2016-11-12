/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_VIDEO
#define LA_VIDEO

#include <la_config.h>
#ifndef LA_FEATURE_VIDEO
	#error "please add #define LA_FEATURE_VIDEO to your la_config.h"
#endif

#include <la_buffer.h>

void la_video_load_jpeg(void* output, void* data, uint32_t size, uint32_t* w,
	uint32_t* h);
void la_video_make_jpeg(la_buffer_t* rtn, uint8_t quality, uint8_t* pxdata,
	uint32_t w, uint32_t h);

#endif
