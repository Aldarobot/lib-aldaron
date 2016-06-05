#include "jl_pr.h"

#if JL_PLAT == JL_PLAT_PHONE
	#include <jni.h>

	char* JL_FL_BASE = NULL;
#endif

//Initialize The Libraries Needed At Very Beginning: The Base Of It All
static inline jl_t* jl_init_essential__(void) {
	JL_PRINT("Starting JL_Lib - Version "JL_VERSION"....");
	// Memory
	jvct_t* _jl = jl_mem_init__(); // Create The Library Context
	// Printing to terminal
	jl_print_init__(_jl->jl);
	return _jl->jl;
}

static inline void jl_init_libs__(jl_t* jl) {
	JL_PRINT_DEBUG(jl, "Initializing threads....");
	jl_thread_init__(jl);
	JL_PRINT_DEBUG(jl, "Initializing file system....");
	jl_file_init__(jl->_jl);
	JL_PRINT_DEBUG(jl, "Initializing modes....");
	jl_mode_init__(jl);
	JL_PRINT_DEBUG(jl, "Initializing time....");
	jl_sdl_init__(jl);
	JL_PRINT_DEBUG(jl, "Initialized!");
//	jlgr_draw_msge(_jl->jl, 0, 0, 0, "INITIALIZATION COMPLETE!");
}

static inline void jl_init__(jl_t* jl,jl_fnct _fnc_init_,str_t nm,u64_t ctx1s) {
	jl_print(jl, "Initializing subsystems....");
	// Run the library's init function.
	jl_init_libs__(jl);
	// Allocate the program's context.
	jl->prg_context = jl_memi(jl, ctx1s);
	jl->name = jl_mem_copy(jl, nm, strlen(nm));
	// Run the program's init function.
	_fnc_init_(jl);
	jl_print(jl, "Started JL_Lib!");
}

static void jl_time_reset__(jl_t* jl, u8_t on_time) {
	if(jl->jlgr) {
		jlgr_t* jlgr = jl->jlgr;

		if((jlgr->sg.changed = ( jlgr->sg.on_time != on_time)))
			jlgr->sg.on_time = on_time;
	}
}

//return how many seconds passed since last call
static inline void jl_seconds_passed__(jl_t* jl) {
	uint16_t fps;

	jl->time.psec = jl_sdl_timer(jl, &jl->time.timer);
	fps = (uint16_t)(1. / jl->time.psec);
	// Tell if fps is 60 fps or better
	jl_time_reset__(jl, fps >= JL_FPS);
}

static inline void main_loop__(jl_t* jl) {
	jl_fnct loop_ = jl->mode.mode.loop;

	// Check the amount of time passed since last frame.
	jl_seconds_passed__(jl);
	// Run the user's mode loop.
	loop_(jl);
}

static inline int jl_kill__(jl_t* jl, int rc) {
	jvct_t* _jl = jl->_jl;

	jl_print(jl, "Quitting...."); //Exited properly
//	jlgr_draw_msge(jl, 0, 0, 0, "Quiting JL-Lib....");
	if(_jl->me.status == JL_STATUS_EXIT) {
		rc = JL_RTN_FAIL_IN_FAIL_EXIT;
		JL_PRINT("\n!! double error!\n");
		JL_PRINT("!! exiting with return value %d\n", rc);
		exit(rc);
	}
	// Set status to Exiting
	_jl->me.status = JL_STATUS_EXIT;
	jl_file_kill__(_jl);
	jl_print_kill__(jl);
	JL_PRINT("PRINTG KILL'd\n");
	jl_mem_kill__(_jl);
	JL_PRINT("PRINTG KILL'd\n");
	JL_PRINT("[\\JL_Lib] ");
	if(!rc) JL_PRINT("| No errors ");
	JL_PRINT("| Exiting with return value %d |\n", rc);
	return rc;
}

// EXPORT FUNCTIONS

/**
 * Do Nothing
 * @param jl: The library's context.
**/
void jl_dont(jl_t* jl) { }

/**
 * Get the program's context.
 * @param jl: The library's context.
**/
void* jl_get_context(jl_t* jl) {
	return jl->prg_context;
}

/**
 * Start JL_Lib.  Returns when program is closed.
 * @param fnc_init_: The function initialize the program.
 * @param fnc_kill_: The function to free memory before exiting.
 * @param name: The name of the program, used for storage / window name etc.
 * @param ctx_size: The size of the program context.
**/
int jl_start(jl_fnct fnc_init_, jl_fnct fnc_kill_, str_t name, u64_t ctx_size) {
	//Set Up Memory And Logging
	jl_t* jl = jl_init_essential__();

	// Initialize JL_lib!
	jl_init__(jl, fnc_init_, name, ctx_size);
	// Run the Loop
	while(jl->mode.count) main_loop__(jl);
	// Run Program's Kill Routine
	fnc_kill_(jl);
	// Kill the program
	return jl_kill__(jl, JL_RTN_SUCCESS);
}

#if JL_PLAT == JL_PLAT_PHONE

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeJlSendData( JNIEnv *env, jobject obj,
	jstring data)
{
	// Enable SDL standard application logging
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
	//
	SDL_Log("nativeJlSendData\n");
	JL_FL_BASE = (*env)->GetStringUTFChars(env, data, 0);
	SDL_Log("nativeJlSendData \"%s\"\n", JL_FL_BASE);
}

#endif

/**
 * @mainpage
 * @section Library Description
 * 
*/
