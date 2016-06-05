/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRsg.c
 *	sg AKA. Simple Graphics does the window handling.
**/
#include "JLGRinternal.h"

// SG Prototypes
void jl_gl_draw_prendered(jlgr_t* jlgr, jl_vo_t* pv);

// Constants
	//ALL IMAGES: 1024x1024
	#define TEXTURE_WH 1024*1024 
	//1bpp Bitmap = 1048832 bytes (Color Key(256)*RGBA(4), 1024*1024)
	#define IMG_FORMAT_LOW 1048832 
	//2bpp HQ bitmap = 2097664 bytes (Color Key(256*2=512)*RGBA(4), 2*1024*1024)
	#define IMG_FORMAT_MED 2097664
	//3bpp Picture = 3145728 bytes (No color key, RGB(3)*1024*1024)
	#define IMG_FORMAT_PIC 3145728
	//
	#define IMG_SIZE_LOW (1+strlen(JL_IMG_HEADER)+(256*4)+(1024*1024)+1)
	
//Functions:

//Get a pixels RGBA values from a surface and xy
uint32_t _jl_sg_gpix(/*in */ SDL_Surface* surface, int32_t x, int32_t y) {
	i32_t bpp = surface->format->BytesPerPixel;
	u8_t *p = (uint8_t *)surface->pixels + (y * surface->pitch) + (x * bpp);
	uint32_t color_orig;
	uint32_t color;
	uint8_t* out_color = (void*)&color;

	if(bpp == 1) {
		color_orig = *p;
	}else if(bpp == 2) {
		color_orig = *(u16_t *)p;
	}else if(bpp == 3) {
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			color_orig = p[0] << 16 | p[1] << 8 | p[2];
		else
			color_orig = p[0] | p[1] << 8 | p[2] << 16;
	}else{ // 4
		color_orig = *(u32_t *)p;
	}
	SDL_GetRGBA(color_orig, surface->format, &(out_color[0]),
		&(out_color[1]), &(out_color[2]), &(out_color[3]));
	return color;
}

void _jl_sg_load_jlpx(jlgr_t* jlgr,data_t* data,void **pixels,int *w,int *h) {
	if(data == NULL) {
		jl_print(jlgr->jl, "NULL DATA!");
		jl_sg_kill(jlgr->jl);
	}else if(data->data[jlgr->sg.init_image_location] == 0) {
		return;
	}

	jl_print_function(jlgr->jl, "SG_Jlpx"); // {
	
	//Check If File Is Of Correct Format
	char *testing = malloc(strlen(JL_IMG_HEADER)+1);
	int32_t i, j;

	jl_mem_copyto(data->data + jlgr->sg.init_image_location, testing,
		strlen(JL_IMG_HEADER));
	testing[strlen(JL_IMG_HEADER)] = '\0';
	JL_PRINT_DEBUG(jlgr->jl, "header=\"%s\" @%d", testing,
		jlgr->sg.init_image_location);

	if(strcmp(testing, JL_IMG_HEADER) != 0) {
		jl_print(jlgr->jl, "error: bad file type:");
		jl_print(jlgr->jl, ":%s", testing);
		jl_print(jlgr->jl, "!=");
		jl_print(jlgr->jl, ":%s", JL_IMG_HEADER);
		jl_print(jlgr->jl, "couldn't load image #%d",
			jlgr->sg.image_id);
		jl_sg_kill(jlgr->jl);
	}
	uint8_t tester = data->data[jlgr->sg.init_image_location+strlen(JL_IMG_HEADER)];
	uint32_t FSIZE;
	if(tester == JL_IMG_FORMAT_FLS) {
		SDL_Surface *image;
		SDL_RWops *rw;
		void* img_file = NULL;
		uint32_t color = 0;
		data_t* pixel_data;

		JL_PRINT_DEBUG(jlgr->jl, "png/gif/jpeg etc.");
		data->curs = jlgr->sg.init_image_location+strlen(JL_IMG_HEADER)+1;
		jl_data_loadto(data, 4, &FSIZE);
		JL_PRINT_DEBUG(jlgr->jl, "File Size = %d", FSIZE);
		img_file = jl_memi(jlgr->jl, FSIZE);
		jl_data_loadto(data, FSIZE, img_file);
		rw = SDL_RWFromMem(img_file, FSIZE);
		if ((image = IMG_Load_RW(rw, 1)) == NULL) {
			jl_print(jlgr->jl, "Couldn't load image: %s",
				IMG_GetError());
			jl_sg_kill(jlgr->jl);
		}
		// Covert SDL_Surface.
		pixel_data = jl_data_make(image->w * image->h * 4);
		for(i = 0; i < image->h; i++) {
			for(j = 0; j < image->w; j++) {
				color = _jl_sg_gpix(image, j, i);
				jl_data_saveto(pixel_data, 4, &color);
			}
		}
		jlgr->sg.init_image_location += FSIZE + 6;
		//Set Return values
		*pixels = jl_data_tostring(jlgr->jl, pixel_data);
		*w = image->w;
		*h = image->h;
		// Clean-up
		SDL_free(image);
	}else{
		jl_print(jlgr->jl, "bad file type(must be 4) is: %d", tester);
		jl_sg_kill(jlgr->jl);
	}
	jl_print_return(jlgr->jl, "SG_Jlpx");
}

//loads next image in the currently loaded file.
static inline uint8_t _jl_sg_load_next_img(jlgr_t* jlgr) {
	void *fpixels = NULL;
	int fw;
	int fh;
	jl_print_function(jlgr->jl, "SG_Imgs");
	_jl_sg_load_jlpx(jlgr, jlgr->sg.image_data, &fpixels, &fw, &fh);
	if(fpixels == NULL) {
		JL_PRINT_DEBUG(jlgr->jl, "loaded %d", jlgr->sg.image_id);
		jlgr->jl->info = jlgr->sg.image_id;
		jl_print(jlgr->jl, "IL");
		jl_print_return(jlgr->jl, "SG_Imgs");
		return 0;
	}else{
		JL_PRINT_DEBUG(jlgr->jl, "creating image #%d....", jlgr->sg.igid);
		jl_gl_maketexture(jlgr, jlgr->sg.igid,
			jlgr->sg.image_id, fpixels, fw, fh, 0);
		JL_PRINT_DEBUG(jlgr->jl, "created image #%d:%d!", jlgr->sg.igid,
			jlgr->sg.image_id);
//		#endif
		jlgr->sg.image_id++;
		jl_print_return(jlgr->jl, "SG_Imgs");
		return 1;
	}
}

void jl_sg_add_some_imgs_(jlgr_t* jlgr, u16_t x) {
	m_u16_t i;
	for(i = 0; i < x; i++) {
		if(!_jl_sg_load_next_img(jlgr)) break;
	}
}

//Load the images in the image file
static inline void _jl_sg_init_images(jlgr_t* jlgr,data_t* data,u16_t gi,u16_t x){
	jlgr->sg.init_image_location = 0;
	jlgr->sg.image_id= 0; //Reset Image Id
	jlgr->sg.igid = gi;
	jlgr->sg.image_data = data;

	jl_print_function(jlgr->jl, "SG_InitImgs");
	JL_PRINT_DEBUG(jlgr->jl, "loading images....");
	JL_PRINT_DEBUG(jlgr->jl, "stringlength = %d", data->size);
//load textures
	if(x) jl_sg_add_some_imgs_(jlgr, x);
	else while(_jl_sg_load_next_img(jlgr));
	jl_print_return(jlgr->jl, "SG_InitImgs");
}

/**
 * Show Error On Screen if screen is available, otherwise print it out.
 * Quit, And Return -1 to show there was an error.
*/
void jl_sg_kill(jl_t* jl) {
	//TODO: Make Screen With Window Saying Error Message Followed By Prompt.
	//	Also, don't automatically quit, like it does now!  ERQT can be
	//	inline at that point.
	jl_print(jl, "Quitting On Error....");
	jl_print_stacktrace(jl);
	jl->mode.count = 0;
	exit(-1);
	// Program is stopped at this point.
}

static void jl_sg_add_image__(jl_t* jl, str_t pzipfile, u16_t pigid, u8_t x) {
	jl_print_function(jl, "SG_LImg");
	//Load Graphics
	data_t* img = jl_file_media(jl, "jlex/2/_img", pzipfile, jl_gem(),
		jl_gem_size());

	JL_PRINT_DEBUG(jl, "Loading Images....");
	if(img != NULL)
		_jl_sg_init_images(jl->jlgr, img, pigid, x);
	else
		JL_PRINT_DEBUG(jl, "Loaded 0 images!");
	JL_PRINT_DEBUG(jl, "Loaded Images...");
	jl_print_return(jl, "SG_LImg"); // }
}

/**
 * Load all images from a zipfile and give them ID's.
 * info: info is set to number of images loaded.
 * @param jl: library context
 * @param pzipfile: full file name of a zip file.
 * @param pigid: which image group to load the images into.
*/
void jl_sg_add_image(jl_t* jl, str_t pzipfile, u16_t pigid) {
	jl_sg_add_image__(jl, pzipfile, pigid, 0);
}

static void jl_sg_draw_up(jl_t* jl, uint8_t resize, void* data) {
	jlgr_t* jlgr = jl->jlgr;
	
	// Clear the screen.
	jl_gl_clear(jl->jlgr, 0, 127, 170, 255);
	// Run the screen's redraw function
	(jlgr->sg.cs == JL_SCR_UP) ? ((jl_fnct)jlgr->draw.redraw.lower)(jl) :
		((jl_fnct)jlgr->draw.redraw.upper)(jl);
}

static void jl_sg_draw_dn(jl_t* jl, uint8_t resize, void* data) {
	jlgr_t* jlgr = jl->jlgr;

	// Clear the screen.
	jl_gl_clear(jlgr, 255, 127, 0, 255);
	// Run the screen's redraw function
	(jlgr->sg.cs == JL_SCR_UP) ? ((jl_fnct)jlgr->draw.redraw.upper)(jl) :
		((jl_fnct)jlgr->draw.redraw.lower)(jl);
	// Draw Menu Bar & Mouse
	_jlgr_loopa(jl->jlgr);
}

// Double screen loop
static void _jl_sg_loop_ds(jlgr_t* jlgr) {
	// Draw upper screen - alternate screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.up, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.up);
	// Draw lower screen - default screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.dn, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.dn);
}

// Single screen loop
static void _jl_sg_loop_ss(jlgr_t* jlgr) {
	// Draw lower screen - default screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.dn, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.dn);
}

// Run the current loop.
void _jl_sg_loop(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "SG_LP");
	((jlgr_fnct)jlgr->sg.loop)(jlgr);
	jl_print_return(jlgr->jl, "SG_LP");
}

static void jl_sg_init_ds_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rcrd = {
		0., 0.,
		1., .5 * jlgr->wm.ar
	};

	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.up, &rcrd);
	rcrd.y = .5 * jlgr->wm.ar;
	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.dn, &rcrd);
	// Set double screen loop.
	jlgr->sg.loop = _jl_sg_loop_ds;
	if(jlgr->sg.cs == JL_SCR_SS) jlgr->sg.cs = JL_SCR_DN;
	//
	//jlgr->sg.bg.up->pr->ar = jlgr->wm.ar / 2.;
	//jlgr->sg.bg.dn->pr->ar = jlgr->wm.ar / 2.;
}

static void jl_sg_init_ss_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rcrd = {
		0.f, 0.f,
		1., jlgr->wm.ar
	};

	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.dn, &rcrd);
	// Set single screen loop.
	jlgr->sg.loop = _jl_sg_loop_ss;
	jlgr->sg.cs = JL_SCR_SS;
	//
	jlgr->sg.bg.dn.pr.ar = jlgr->wm.ar;
}

void jl_sg_resz__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	// Check screen count.
	if(jlgr->sg.cs == JL_SCR_SS)
		jl_sg_init_ss_(jl);
	else
		jl_sg_init_ds_(jl);
}

void jl_sg_initb__(jlgr_t* jlgr) {
	// Resize for 2 screen Default - so they initialize.
	jlgr->sg.cs = JL_SCR_DN;
	jl_sg_init_ds_(jlgr->jl);
	jlgr->sg.cs = JL_SCR_SS;
}

void jl_sg_inita__(jlgr_t* jlgr) {
	jlgr_redraw_t redraw = { jl_dont, jl_dont, jl_dont, jl_dont };
	jl_rect_t rc = { 0., 0., 1., jl_gl_ar(jlgr) };

	//Set Up Variables
	jlgr->gl.textures = NULL;
	jlgr->gl.tex.uniforms.textures = NULL;
	jlgr->sg.image_id = 0; //Reset Image Id
	jlgr->sg.igid = 0; //Reset Image Group Id
	// Initialize redraw routines to do nothing.
	jl_mem_copyto(&redraw, &(jlgr->draw.redraw), sizeof(jlgr_redraw_t));
	// Load Graphics
	jlgr->gl.allocatedg = 0;
	jlgr->gl.allocatedi = 0;
	jl_sg_add_image__(jlgr->jl,
		(void*)jl_file_get_resloc(jlgr->jl, JL_MAIN_DIR, JL_MAIN_MEF),
		0, 1);
	// Create upper and lower screens
	jlgr_sprite_init(jlgr, &jlgr->sg.bg.up, rc,
		jlgr_sprite_dont, jl_sg_draw_up, NULL, 0, NULL, 0);
	jlgr_sprite_init(jlgr, &jlgr->sg.bg.dn, rc,
		jlgr_sprite_dont, jl_sg_draw_dn, NULL, 0, NULL, 0);
	// Flip upside-down
	jlgr->sg.bg.up.pr.scl.y = -1.;
	jlgr->sg.bg.dn.pr.scl.y = -1.;
}
