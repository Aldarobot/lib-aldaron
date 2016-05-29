#include "JLGRinternal.h"

static char* jlgr_file_fullname__(jlgr_t* jlgr, char* selecteddir,
	char* selecteditem)
{
	jl_t* jl = jlgr->jl;
	jvct_t * _jl = jl->_jl;

	char *newdir = malloc(
		strlen(_jl->fl.dirname) +
		strlen(selecteditem) + 2);
	memcpy(newdir, _jl->fl.dirname,
		strlen(_jl->fl.dirname));
	memcpy(newdir + strlen(_jl->fl.dirname),
		selecteditem,
		strlen(selecteditem));
	newdir[strlen(_jl->fl.dirname) +
		strlen(selecteditem)] = '/';
	newdir[strlen(_jl->fl.dirname) +
		strlen(selecteditem) + 1] = '\0';
	return newdir;
}

static void _jl_fl_user_select_check_extradir(char *dirname) {
	if(dirname[strlen(dirname)-1] == '/' && strlen(dirname) > 1) {
		if(dirname[strlen(dirname)-2] == '/')
			dirname[strlen(dirname)-1] = '\0';
	}
}

// Return 1 on success
// Return 0 if directory not available
static uint8_t _jl_fl_user_select_open_dir(jlgr_t* jlgr, char *dirname) {
	DIR *dir;
	struct dirent *ent;
	jvct_t * _jl = jlgr->jl->_jl;
	str_t converted_filename;

	_jl_fl_user_select_check_extradir(dirname);
	if(dirname[1] == '\0') {
		jl_mem(jlgr->jl, dirname, 0);
		dirname = SDL_GetPrefPath("JL_Lib", "\0");
		_jl_fl_user_select_check_extradir(dirname);
	}
	_jl->fl.dirname = dirname;
	_jl->fl.cursor = 0;
	_jl->fl.cpage = 0;
	converted_filename = jl_file_convert__(jlgr->jl, _jl->fl.dirname);
	cl_list_clear(_jl->fl.filelist);
//UnComment to test file system conversion code.
	JL_PRINT_DEBUG(jlgr->jl, "dirname=%s:%s\n", _jl->fl.dirname,
		converted_filename);
	if ((dir = opendir (converted_filename)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(ent->d_name) + 1);
			memcpy(element, ent->d_name, strlen(ent->d_name));
			element[strlen(ent->d_name)] = '\0';
			cl_list_add(_jl->fl.filelist, element);
		}
		closedir(dir);
		jl_cl_list_alphabetize(_jl->fl.filelist);
	} else {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) { //Not a directory - is a file
			_jl->fl.returnit = 1;
			_jl->fl.dirname[strlen(dirname)-1] = '\0';
			_jl->fl.inloop = 0;
			// Exit this mode.
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else if(errsv == ENOENT) { // Directory Doesn't Exist
			return 0;
		}else if(errsv == EACCES) { // Doesn't have permission
			return 0;
		}else if((errsv == EMFILE) || (errsv == ENFILE) ||
			(errsv == ENOMEM)) //Not enough memory!
		{
			jl_print(jlgr->jl, "FileViewer Can't Open Directory:"
				"Not Enough Memory!");
			jl_sg_kill(jlgr->jl);
		}else{ //Unknown error
			jl_print(jlgr->jl, "FileViewer Can't Open Directory:"
				"Unknown Error!");
			jl_sg_kill(jlgr->jl);
		}
	}
	return 1;
}

/**
 * Open directory for file viewer.
 * If '!' is put at the beginning of "program_name", then it's treated as a
 *	relative path instead of a program name.
 * @param jl: The library context
 * @param program_name: program name or '!'+relative path
 * @param newfiledata: any new files created with the fileviewer will
 *	automatically be saved with this data.
 * @param newfilesize: size of "newfiledata"
 * @returns 0: if can't open the directory. ( Doesn't exist, Bad permissions )
 * @returns 1: on success.
**/
uint8_t jlgr_openfile_init(jlgr_t* jlgr, str_t program_name, void *newfiledata,
	uint64_t newfilesize)
{
	jvct_t * _jl = jlgr->jl->_jl;
	_jl->fl.returnit = 0;
	_jl->fl.inloop = 1;
	_jl->fl.newfiledata = newfiledata;
	_jl->fl.newfilesize = newfilesize;
	_jl->fl.prompt = 0;
	_jl->fl.promptstring = NULL;
	if(program_name[0] == '!') {
		char *path = jl_mem_copy(jlgr->jl,program_name,
			strlen(program_name));
		return _jl_fl_user_select_open_dir(jlgr, path);
	}else{
		return _jl_fl_user_select_open_dir(jlgr,
			SDL_GetPrefPath("JL_Lib",program_name));
	}
}

static void _jl_fl_user_select_up(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == JLGR_INPUT_PRESS_JUST) {
		jvct_t * _jl = jlgr->jl->_jl;
		if((_jl->fl.cursor > 0) || _jl->fl.cpage) _jl->fl.cursor--;
	}
}

static void _jl_fl_user_select_dn(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == JLGR_INPUT_PRESS_JUST) {
		jvct_t * _jl = jlgr->jl->_jl;
		if(_jl->fl.cursor + (_jl->fl.cpage * (_jl->fl.drawupto+1)) <
			cl_list_count(_jl->fl.filelist) - 1)
		{
			_jl->fl.cursor++;
		}
	}
}

static void _jl_fl_user_select_rt(jlgr_t* jlgr, jlgr_input_t input) {
	int i;
	for(i = 0; i < 5; i++) {
		_jl_fl_user_select_dn(jlgr, input);
	}
}

static void _jl_fl_user_select_lt(jlgr_t* jlgr, jlgr_input_t input) {
	int i;
	for(i = 0; i < 5; i++) {
		_jl_fl_user_select_up(jlgr, input);
	}
}

static void _jl_fl_open_file(jl_t*jl, char *selecteditem) {
	jvct_t * _jl = jl->_jl;
	char *newdir = jlgr_file_fullname__(jl->jlgr,
		_jl->fl.dirname, selecteditem);

	free(_jl->fl.dirname);
	_jl->fl.dirname = NULL;
	_jl_fl_user_select_open_dir(jl->jlgr,newdir);
}

static void _jl_fl_user_select_do(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		jvct_t * _jl = jlgr->jl->_jl;
		struct cl_list_iterator *iterator;
		int i;
		char *stringtoprint;

		iterator = cl_list_iterator_create(_jl->fl.filelist);
		for(i = 0; i < cl_list_count(_jl->fl.filelist); i++) {
			stringtoprint = cl_list_iterator_next(iterator);
			if(i ==
				_jl->fl.cursor + //ON PAGE
				(_jl->fl.cpage * (_jl->fl.drawupto+1))) //PAGE
			{
				_jl->fl.selecteditem = stringtoprint;
				cl_list_iterator_destroy(iterator);
				break;
			}
		}
		if(strcmp(_jl->fl.selecteditem, "..") == 0) {
			for(i = strlen(_jl->fl.dirname)-2; i > 0; i--) {
				if(_jl->fl.dirname[i] == '/') break;
				else _jl->fl.dirname[i] = '\0';
			}
			_jl_fl_user_select_open_dir(jlgr,_jl->fl.dirname);
		}else if(strcmp(_jl->fl.selecteditem, ".") == 0) {
			_jl->fl.inloop = 0;
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else{
			_jl_fl_open_file(jlgr->jl, _jl->fl.selecteditem);
		}
	}
}

/**
 * Run the file viewer.
 * @param jlgr: The jlgr library Context.
**/
void jlgr_openfile_loop(jlgr_t* jlgr) {
	jvct_t * _jl = jlgr->jl->_jl;
	struct cl_list_iterator *iterator;
	int i;
	char *stringtoprint;

	_jl->fl.drawupto = ((int)(20.f * jl_gl_ar(jlgr))) - 1;

	iterator = cl_list_iterator_create(_jl->fl.filelist);

	jlgr_fill_image_set(jlgr, 0, JL_IMGI_ICON, 1, 255);
	jlgr_fill_image_draw(jlgr);
	jlgr_draw_text(jlgr, "Select File", (jl_vec3_t) { .02, .02, 0. },
		jlgr->font);

	jlgr_input_do(jlgr, JL_CT_MAINUP, _jl_fl_user_select_up, NULL);
	jlgr_input_do(jlgr, JL_CT_MAINDN, _jl_fl_user_select_dn, NULL);
	jlgr_input_do(jlgr, JL_CT_MAINRT, _jl_fl_user_select_rt, NULL);
	jlgr_input_do(jlgr, JL_CT_MAINLT, _jl_fl_user_select_lt, NULL);
	//Draw files
	for(i = 0; i < cl_list_count(_jl->fl.filelist); i++) {
		stringtoprint = cl_list_iterator_next(iterator);
		if(strcmp(stringtoprint, "..") == 0) {
			stringtoprint = "//containing folder//";
		}else if(strcmp(stringtoprint, ".") == 0) {
			stringtoprint = "//this folder//";
		}
		if(i - (_jl->fl.cpage * (_jl->fl.drawupto+1)) >= 0)
			jlgr_draw_text(jlgr, stringtoprint, (jl_vec3_t) {
				.06,
				.08 + (jlgr->font.size *
					(i - (_jl->fl.cpage * (
					_jl->fl.drawupto+1)))), 0. },
				jlgr->font);
		if(i - (_jl->fl.cpage * (_jl->fl.drawupto+1)) >
			_jl->fl.drawupto - 1)
		{
			break;
		 	cl_list_iterator_destroy(iterator);
	 	}
	}
	if(_jl->fl.cursor > _jl->fl.drawupto) {
		_jl->fl.cursor = 0;
		_jl->fl.cpage++;
	}
	if(_jl->fl.cursor < 0) {
		_jl->fl.cursor = _jl->fl.drawupto;
		_jl->fl.cpage--;
	}
	if(_jl->fl.prompt) {
		if(jlgr_draw_textbox(jlgr, .02, jl_gl_ar(jlgr) - .06, .94, .02,
			&_jl->fl.promptstring))
		{
			char *name = jlgr_file_fullname__(jlgr,
				_jl->fl.dirname,
				(char*)_jl->fl.promptstring->data);
			name[strlen(name) - 1] = '\0';
			jl_file_save(jlgr->jl, _jl->fl.newfiledata,
				name, _jl->fl.newfilesize);
			_jl_fl_user_select_open_dir(jlgr, _jl->fl.dirname);
			_jl->fl.prompt = 0;
		}
	}else{
		jlgr_draw_text(jlgr, ">", (jl_vec3_t) {
			.02, .08 + (.04 * _jl->fl.cursor), 0. },
			jlgr->font);
		jlgr_draw_text(jlgr, _jl->fl.dirname,
			(jl_vec3_t) { .02, jl_gl_ar(jlgr) - .02, 0. },
			(jl_font_t) { 0, JL_IMGI_ICON, 0,
				jlgr->fontcolor, .02});
		jlgr_input_do(jlgr, JL_CT_SELECT, _jl_fl_user_select_do, NULL);
	}
	jlgr_sprite_loop(jlgr, _jl->fl.btns[0]);
	jlgr_sprite_loop(jlgr, _jl->fl.btns[1]);
}

/**
 * Get the results from the file viewer.
 * @param jl: Library Context.
 * @returns: If done, name of selected file.  If not done, NULL is returned.
**/
str_t jlgr_openfile_kill(jlgr_t* jlgr) {
	jvct_t * _jl = jlgr->jl->_jl;
	
	if(_jl->fl.returnit)
		return _jl->fl.dirname;
	else
		return NULL;
}

static void _jl_fl_btn_makefile_press(jlgr_t* jlgr, jlgr_input_t input) {
	jvct_t *_jl = jlgr->jl->_jl;
	_jl->fl.prompt = 1;
}

static void* _jl_fl_btn_makefile_loop(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jvct_t *_jl = jlgr->jl->_jl;

	//TODO: make graphic: 0, 1, 1, 255
	if(!_jl->fl.newfiledata) return NULL;
	jlgr_glow_button_draw(jlgr, _jl->fl.btns[0], "+ New File",
		_jl_fl_btn_makefile_press);
	return NULL;
}

static void _jl_fl_btn_makefile_draw(jl_t* jl, uint8_t resize, void* ctx) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vos_image(jlgr, jlgr->gl.temp_vo, rc, 0, JL_IMGI_ICON, 9, 255);
	jlgr_draw_vo(jlgr, jlgr->gl.temp_vo, &tr);
}

static void* _jl_fl_btn_makefolder_loop(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jvct_t *_jl = jlgr->jl->_jl;
	
	//TODO: make graphic: 0, 1, 2, 255,
	jlgr_glow_button_draw(jlgr, _jl->fl.btns[1], "+ New Folder",
		_jl_fl_btn_makefile_press);
	return NULL;
}

static void _jl_fl_btn_makefolder_draw(jl_t* jl, uint8_t resize, void* ctx) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vos_image(jlgr, jlgr->gl.temp_vo, rc, 0, JL_IMGI_ICON, 10, 255);
	jlgr_draw_vo(jlgr, jlgr->gl.temp_vo, &tr);
}

void jlgr_fl_init(jlgr_t* jlgr) {
	jvct_t* _jl = jlgr->jl->_jl;
	jl_rect_t rc1 = { .8, 0., .1, .1 };
	jl_rect_t rc2 = { .9, 0., .1, .1 };

	//Create the variables
	_jl->fl.filelist = cl_list_create();
	_jl->fl.inloop = 0;
	_jl->fl.btns[0] = jlgr_sprite_new(jlgr, rc1,
		_jl_fl_btn_makefile_loop, _jl_fl_btn_makefile_draw,
		NULL, 0, NULL, 0);
	_jl->fl.btns[1] = jlgr_sprite_new(jlgr, rc2,
		_jl_fl_btn_makefolder_loop, _jl_fl_btn_makefolder_draw,
		NULL, 0, NULL, 0);
	_jl->has.fileviewer = 1;
}
