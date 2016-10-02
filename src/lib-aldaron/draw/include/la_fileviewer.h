/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_FILEVIEWER
#define LA_FILEVIEWER

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

#include "la_draw.h"
#include "clump.h"
#include "la_buffer.h"
#include "la_menu.h"
#include "la_safe.h"

typedef struct{
	la_window_t* window;
	struct cl_list *filelist; //List of all files in working dir.
	int8_t cursor;
	uint8_t cpage;
	char *dirname;
	char *selecteditem;
	uint8_t returnit;
	uint8_t drawupto;
	void *newfiledata;
	uint64_t newfilesize;
	safe_uint8_t prompt;
	la_buffer_t promptstring;
	la_ro_t file;
	la_ro_t fade;
	la_menu_t menu;
}la_fileviewer_t;

uint8_t la_fileviewer_init(la_window_t* window, la_fileviewer_t* fileviewer,
	const char* directory, void *newfiledata, uint64_t newfilesize);
const char* la_fileviewer_loop(la_fileviewer_t* fileviewer);
void la_fileviewer_draw(la_fileviewer_t* fileviewer);

#endif
