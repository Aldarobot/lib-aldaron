#include "JLGRprivate.h"
#include "SDL_filesystem.h"

#include "la_memory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "la_file.h"
#include "la_fileviewer.h"
#include "la_text.h"

static la_fileviewer_t* la_fileviewer = NULL;

static void jl_fl_user_select_check_extradir__(char *dirname) {
	if(dirname[strlen(dirname)-1] == '/' && strlen(dirname) > 1) {
		if(dirname[strlen(dirname)-2] == '/')
			dirname[strlen(dirname)-1] = '\0';
	}
}

// Return 1 on success
// Return 0 if directory not available
static uint8_t jl_fl_user_select_open_dir__(la_window_t* jlgr, char *dirname) {
	DIR *dir;
	struct dirent *ent;
	const char* converted_filename;

	jl_fl_user_select_check_extradir__(dirname);
	if(dirname[1] == '\0') {
//		jl_mem(jlgr->jl, dirname, 0);
		dirname = SDL_GetPrefPath("JL_Lib", "\0");
		jl_fl_user_select_check_extradir__(dirname);
	}
	la_fileviewer->dirname = dirname;
	la_fileviewer->cursor = 0;
	la_fileviewer->cpage = 0;
	converted_filename = la_fileviewer->dirname;
	cl_list_clear(la_fileviewer->filelist);
//UnComment to test file system conversion code.
	la_print("dirname=%s:%s\n", la_fileviewer->dirname, converted_filename);
	chdir(la_fileviewer->dirname);
	if ((dir = opendir (converted_filename)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(ent->d_name) + 1);
			memcpy(element, ent->d_name, strlen(ent->d_name));
			element[strlen(ent->d_name)] = '\0';
			cl_list_add(la_fileviewer->filelist, element);
		}
		closedir(dir);
		jl_cl_list_alphabetize(la_fileviewer->filelist);
	} else {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) { //Not a directory - is a file
			la_fileviewer->returnit = 1;
			la_fileviewer->dirname[strlen(dirname)-1] = '\0';
			// Exit this mode.
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else if(errsv == ENOENT) { // Directory Doesn't Exist
			return 1;
		}else if(errsv == EACCES) { // Doesn't have permission
			return 1;
		}else if((errsv == EMFILE) || (errsv == ENFILE) ||
			(errsv == ENOMEM)) //Not enough memory!
		{
			la_panic("FileViewer Can't Open Directory: Not Enough "
				"Memory!");
		}else{ //Unknown error
			la_panic("FileViewer Can't Open Directory: Unknown "
				"Error!");
		}
	}
	return 0;
}

/*static void jl_fl_user_select_up__(la_window_t* jlgr) {
	if((la_fileviewer->cursor > 0) || la_fileviewer->cpage) la_fileviewer->cursor--;
}

static void jl_fl_user_select_dn__(la_window_t* jlgr) {
	if(la_fileviewer->cursor + (la_fileviewer->cpage * (la_fileviewer->drawupto+1)) <
		cl_list_count(la_fileviewer->filelist) - 1)
	{
		la_fileviewer->cursor++;
	}
}

static void jl_fl_open_file__(la_window_t* jlgr, char *selecteditem) {
	char *newdir = jlgr_file_fullname__(jlgr,
		la_fileviewer->dirname, selecteditem);

	free(la_fileviewer->dirname);
	la_fileviewer->dirname = NULL;
	jl_fl_user_select_open_dir__(jlgr,newdir);
}

static void jl_fl_user_select_do__(la_window_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		struct cl_list_iterator *iterator;
		int i;
		char *stringtoprint;

		iterator = cl_list_iterator_create(la_fileviewer->filelist);
		for(i = 0; i < cl_list_count(la_fileviewer->filelist); i++) {
			stringtoprint = cl_list_iterator_next(iterator);
			if(i ==
				la_fileviewer->cursor + //ON PAGE
				(la_fileviewer->cpage * (la_fileviewer->drawupto+1))) //PAGE
			{
				la_fileviewer->selecteditem = stringtoprint;
				cl_list_iterator_destroy(iterator);
				break;
			}
		}
		if(strcmp(la_fileviewer->selecteditem, "..") == 0) {
			for(i = strlen(la_fileviewer->dirname)-2; i > 0; i--) {
				if(la_fileviewer->dirname[i] == '/') break;
				else la_fileviewer->dirname[i] = '\0';
			}
			jl_fl_user_select_open_dir__(jlgr,la_fileviewer->dirname);
		}else if(strcmp(la_fileviewer->selecteditem, ".") == 0) {
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else{
			jl_fl_open_file__(jlgr, la_fileviewer->selecteditem);
		}
	}
}*/

static void jl_fl_btn_makefile_press__(la_window_t* jlgr) {
	la_fileviewer->prompt = 1;
}

static void jl_fl_btn_makefile_loop__(jl_t* jl, jl_sprite_t* sprite) {
	la_window_t* jlgr = jl->jlgr;

	//TODO: make graphic: 0, 1, 1, 255
	if(la_fileviewer->newfiledata)
		jlgr_glow_button_draw(jlgr, &la_fileviewer->btns[0], "+ New File",
			jl_fl_btn_makefile_press__);
}

static void jl_fl_btn_makefile_draw__(jl_t* jl, uint8_t resize, void* ctx) {
	la_window_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, rc, jlgr->textures.icon);
	jlgr_vo_txmap(jlgr, &jlgr->gl.temp_vo, 0, 16, 16, 9);
	jlgr_vo_move(&jlgr->gl.temp_vo, tr);
	jlgr_vo_draw(jlgr, &jlgr->gl.temp_vo);
}

static void jl_fl_btn_makefolder_loop__(jl_t* jl, jl_sprite_t* sprite) {
	la_window_t* jlgr = jl->jlgr;
	
	//TODO: make graphic: 0, 1, 2, 255,
	jlgr_glow_button_draw(jlgr, &la_fileviewer->btns[1], "+ New Folder",
		jl_fl_btn_makefile_press__);
}

static void jl_fl_btn_makefolder_draw__(jl_t* jl, uint8_t resize, void* ctx) {
	la_window_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, rc, jlgr->textures.icon);
	jlgr_vo_txmap(jlgr, &jlgr->gl.temp_vo, 0, 16, 16, 10);
	jlgr_vo_move(&jlgr->gl.temp_vo, tr);
	jlgr_vo_draw(jlgr, &jlgr->gl.temp_vo);
}

/**
 * Open directory for file viewer.
 * If '!' is put at the beginning of "program_name", then it's treated as a
 *	relative path instead of a program name.
 * @param jl: The library context
 * @param directory: relative or absolute path to open
 * @param newfiledata: any new files created with the fileviewer will
 *	automatically be saved with this data.
 * @param newfilesize: size of "newfiledata"
 * @returns 1: if can't open the directory. ( Doesn't exist, Bad permissions )
 * @returns 0: on success.
**/
uint8_t la_fileviewer_init(la_window_t* window, la_fileviewer_t* fileviewer,
	const char* directory, void *newfiledata, uint64_t newfilesize)
{
	char *path = directory ? la_memory_makecopy(directory, strlen(directory) + 1) : getenv("HOME");
	jl_rect_t rc1 = { 0., 0., .1, .1 };
	jl_rect_t rc2 = { 0., 0., .1, .1 };

	fileviewer->window = window;
	fileviewer->returnit = 0;
	fileviewer->newfiledata = newfiledata;
	fileviewer->newfilesize = newfilesize;
	fileviewer->prompt = 0;
	la_buffer_init(&fileviewer->promptstring);
	fileviewer->filelist = cl_list_create();
	//Create the variables
	jlgr_sprite_init(window, &fileviewer->btns[0], rc1,
		jl_fl_btn_makefile_loop__, jl_fl_btn_makefile_draw__,
		NULL, 0, NULL, 0);
	jlgr_sprite_init(window, &fileviewer->btns[1], rc2,
		jl_fl_btn_makefolder_loop__, jl_fl_btn_makefolder_draw__,
		NULL, 0, NULL, 0);
	//
	jlgr_vo_set_image(window, &fileviewer->file, (jl_rect_t) { 0., 0., .2, .2}, window->textures.icon);
	la_fileviewer = fileviewer;
	return jl_fl_user_select_open_dir__(window, path);
}

/**
 * Get the results from the file viewer.
 * @param jlgr: Library Context.
 * @returns: If done, name of selected file.  If not done, NULL is returned.
**/
const char* la_fileviewer_loop(la_fileviewer_t* fileviewer) {
/*	if(la_fileviewer->cursor > la_fileviewer->drawupto) {
		la_fileviewer->cursor = 0;
		la_fileviewer->cpage++;
	}
	if(la_fileviewer->cursor < 0) {
		la_fileviewer->cursor = la_fileviewer->drawupto;
		la_fileviewer->cpage--;
	}
	// Sprite loops
	jlgr_sprite_loop(fileviewer->window, &la_fileviewer->btns[0]);
	jlgr_sprite_loop(fileviewer->window, &la_fileviewer->btns[1]);
	// Free if done.
	if(fileviewer->returnit) {
		cl_list_destroy(fileviewer->filelist);
		return fileviewer->dirname;
	}else{
		return NULL;
	}*/
	// Continue
	return NULL;
}

void la_fileviewer_draw(la_fileviewer_t* fileviewer) {
	struct cl_list_iterator *iterator;
	int i;
	char *stringtoprint;
	la_window_t* window = fileviewer->window;
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t alternate = 0;

	la_fileviewer->drawupto = ((int)(20.f * jl_gl_ar(window))) - 1;

	iterator = cl_list_iterator_create(la_fileviewer->filelist);

	jlgr_fill_image_set(window, window->textures.backdrop, 0, 0, -1);
	jlgr_fill_image_draw(window);

	//Draw files
	for(i = 0; i < cl_list_count(la_fileviewer->filelist); i++) {
		float offset = alternate ? 0.15f : 0.2f;

		alternate = !alternate;
		stringtoprint = cl_list_iterator_next(iterator);
		if(strcmp(stringtoprint, "..") && strcmp(stringtoprint, ".")) {
			uint8_t state = la_file_exist(stringtoprint);

			jlgr_vo_txmap(window, &fileviewer->file, 0,
				16, 16, state == FILE_TYPE_DIR ? 12 : 11);
			jlgr_vo_move(&fileviewer->file, (jl_vec3_t) { x * 0.2f, 0.1f + (y * 0.2f), 0.f});
			jlgr_vo_draw(window, &fileviewer->file);
		// (jl_vec3_t) { x * 0.2f, 0.1f + (y * 0.2f) }
			la_text(window, LA_PXWIDTH("0.5") LA_PXSIZE("0.025") LA_PXMOVE("%f", "%f") LA_PBLACK "%s", .035 + (x * 0.2f), offset + (y * 0.2f), stringtoprint);
			x++;
			if(x > 4) x = 0, y++;
		}
		if(i > 10) break;
	}
 	cl_list_iterator_destroy(iterator);
	// Draw prompt
//	if(la_fileviewer->prompt) {
/*		if(jlgr_draw_textbox(jlgr, .02, jl_gl_ar(jlgr) - .06, .94, .02,
			la_fileviewer->promptstring))
		{
			char *name = jlgr_file_fullname__(jlgr,
				la_fileviewer->dirname,
				(char*)la_fileviewer->promptstring->data);
			name[strlen(name) - 1] = '\0';
			jl_file_save(jlgr->jl, la_fileviewer->newfiledata,
				name, la_fileviewer->newfilesize);
			jl_fl_user_select_open_dir__(jlgr, la_fileviewer->dirname);
			la_fileviewer->prompt = 0;
		}*/
//	}else{
//		jlgr_text_draw(window, ">", (jl_vec3_t) {
//			.02, .08 + (.04 * la_fileviewer->cursor), 0. },
//			window->font);
//		jlgr_text_draw(window, la_fileviewer->dirname,
//			(jl_vec3_t) { .02, jl_gl_ar(window) - .02, 0. },
//			(jl_font_t) { window->textures.icon, 0,
//				window->fontcolor, .02});
//		jlgr_input_do(jlgr, JL_INPUT_SELECT, jl_fl_user_select_do__, NULL);
//	}
}
