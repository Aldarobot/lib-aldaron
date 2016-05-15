// JL_LIB
	#include "../include/jl.h"
// LIBZIP
	#define ZIP_DISABLE_DEPRECATED //Don't allow the old functions.
	#include "zip.h"
// Standard Libraries
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdlib.h>
	#include <math.h>
	#include <dirent.h>
// Variable Map
	#include "jl_vm.h"

#define MAXFILELEN 1000 * 100000 //100,000 kb

//resolutions
#define JGR_STN 0 //standard 1280 by 960
#define JGR_LOW 1 //Low Res: 640 by 480
#define JGR_DSI 2 //DSi res: 256 by 192

#define VAR_POSITION 0
#define VAR_COLORS 1
#define VAR_TEXTUREI 2

// Printing
#if JL_PLAT == JL_PLAT_COMPUTER
	#define JL_PRINT(...) printf(__VA_ARGS__)
#else
	#define JL_PRINT(...) SDL_Log(__VA_ARGS__)
#endif

// Files
#define JL_FILE_SEPARATOR "/"
#define JL_ROOT_DIRNAME "JL_Lib"
#define JL_ROOT_DIR JL_ROOT_DIRNAME JL_FILE_SEPARATOR
#define JL_MAIN_DIR "PlopGrizzly_JLL"
#define JL_MAIN_MEF "media.zip"

// Replacement for NULL
#define STRT_NULL "(NULL)"
// target frames per second
#define JL_FPS 60

// Media To Include
char *jl_gem(void);
uint32_t jl_gem_size(void);

// Main - Prototypes
	str_t jl_file_convert__(jl_t* jl, str_t filename);
	jl_ctx_t* jl_thread_get_safe__(jl_t* jl);

	// LIB INITIALIZATION fn(Context)
	void _jl_cm_init(jvct_t* _jl);
	void jl_file_init__(jvct_t * _jl);
	jvct_t* jl_mem_init__(void);
	void jl_print_init__(jl_t* jl);
	void jl_thread_init__(jl_t* jl);
	void jl_mode_init__(jl_t* jl);
	void jl_sdl_init__(jl_t* jl);

	// LIB KILLS
	void jl_mem_kill__(jvct_t* jprg);
	void jl_file_kill__(jvct_t * _jl);
	void jl_print_kill__(jl_t* jl);

	// LIB THREAD INITS
	void jl_print_init_thread__(jl_t* jl, u8_t thread_id);
