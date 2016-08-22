#include "JLprivate.h"
#include "jlau.h"
#include "jlgr.h"
#if JL_PLAT == JL_PLAT_PHONE
#include "SDL_system.h"
#else
#include "SDL.h"
#endif

void jlau_kill(jlau_t* jlau);
void jlgr_kill(jlgr_t* jlgr);

void jl_mode_loop__(jl_t* jl);

#if JL_PLAT == JL_PLAT_PHONE
	#include <jni.h>
	#include "SDL_log.h"

	const char* LA_FILE_ROOT = NULL;
	const char* LA_FILE_LOG = NULL;
	char la_keyboard_press = 0;
#endif

SDL_atomic_t la_running;
float la_banner_size = 0.f;

//Initialize The Libraries Needed At Very Beginning: The Base Of It All
static inline jl_t* la_init_essential__(void) {
	// Memory
	jl_t* jl = jl_mem_init_(); // Create The Library Context
	// Printing to terminal
	jl_print_init__(jl);
	return jl;
}

static inline void jl_init_libs__(jl_t* jl) {
	JL_PRINT_DEBUG(jl, "Initializing threads....");
	jl_thread_init__(jl);
	JL_PRINT_DEBUG(jl, "Initializing file system....");
	jl_file_init_(jl);
	JL_PRINT_DEBUG(jl, "Initializing modes....");
	jl_mode_init__(jl);
	JL_PRINT_DEBUG(jl, "Initializing time....");
	jl_sdl_init__(jl);
	JL_PRINT_DEBUG(jl, "Initializing SDL....");
	SDL_Init(0);
	JL_PRINT_DEBUG(jl, "Initialized!");
}

static inline void la_init__(jl_t* jl, jl_fnct _fnc_init_, const char* nm,
	uint64_t ctx1s)
{
	//
	jl->loop = main_loop_;
	JL_PRINT_DEBUG(jl, "Initializing subsystems....");
	// Run the library's init function.
	jl_init_libs__(jl);
	// Allocate the program's context.
	jl->prg_context = jl_memi(jl, ctx1s);
	jl->name = jl_mem_copy(jl, nm, strlen(nm) + 1);
	// Run the program's init function.
	_fnc_init_(jl);
	// Run the mode loop
	jl_mode_loop__(jl);
	//
	jl->time.timer = jl_time_get(jl);
	JL_PRINT_DEBUG(jl, "Started JL_Lib!");
	SDL_AtomicSet(&la_running, 1);
}

static void jl_time_reset__(jl_t* jl, uint8_t on_time) {
	if(jl->jlgr) {
		jlgr_t* jlgr = jl->jlgr;

		if((jlgr->sg.changed = ( jlgr->sg.on_time != on_time)))
			jlgr->sg.on_time = on_time;
	}
}

//return how many seconds passed since last call
static inline void jl_seconds_passed__(jl_t* jl) {
	uint8_t isOnTime;

	jl->time.psec = jl_time_regulatefps(jl, &jl->time.timer, &isOnTime);
	jl_time_reset__(jl, isOnTime);
}

static inline int la_kill__(jl_t* jl, jl_fnct _fnc_kill_, int32_t rc) {
	if(jl->jlgr) jlgr_kill(jl->jlgr);
	if(jl->jlau) jlau_kill(jl->jlau);
	_fnc_kill_(jl);
	JL_PRINT_DEBUG(jl, "Killing SDL....");
	SDL_Quit();
	JL_PRINT_DEBUG(jl, "Killing Printing....");
	jl_print_kill__(jl);
	jl_mem_kill_(jl);
	JL_PRINT("[\\JL_Lib] ");
	if(!rc) JL_PRINT("| No errors ");
	JL_PRINT("| Exiting with return value %d |\n", rc);
	return rc;
}

void main_loop_(jl_t* jl) {
	jl_fnct loop_ = jl->mode.mode.loop;

	// Check the amount of time passed since last frame.
	jl_seconds_passed__(jl);
	// Run the user's mode loop.
	loop_(jl);
	// Run the mode loop
	jl_mode_loop__(jl);
}

// EXPORT FUNCTIONS

/**
 * Exit The program on an error.
**/
void la_panic(jl_t* jl, const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	printf( format, arglist );
	va_end( arglist );

	jl_print_stacktrace(jl);
}

/**
 * Do Nothing
 * @param jl: The library's context.
**/
void la_dont(jl_t* jl) { }

/**
 * Get the program's context.
 * @param jl: The library's context.
**/
void* la_context(jl_t* jl) {
	return jl->prg_context;
}

/**
 * Start JL_Lib.  Returns when program is closed.
 * @param fnc_init: The function initialize the program.
 * @param fnc_kill: The function to free anything that needs to be freed.
 * @param name: The name of the program, used for storage / window name etc.
 * @param ctx_size: The size of the program context.
**/
int32_t la_start(jl_fnct fnc_init, jl_fnct fnc_kill, const char* name,
	size_t ctx_size)
{
	//Set Up Memory And Logging
	jl_t* jl = la_init_essential__();

	// Initialize JL_lib!
	la_init__(jl, fnc_init, name, ctx_size);
	// Run the Loop
	while(jl->mode.count && SDL_AtomicGet(&la_running))
		((jl_fnct)jl->loop)(jl);
	// Kill the program
	return la_kill__(jl, fnc_kill, 0);
}

#if JL_PLAT == JL_PLAT_PHONE

/*#include "la_file.h" // Needed for printing to file

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeLaSetFiles(JNIEnv *env, jobject obj,
	jstring data, jstring logfile)
{
	// Enable SDL standard application logging
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
	//
	SDL_Log("nativeJlSendData\n");
	JL_FL_BASE = (*env)->GetStringUTFChars(env, data, 0);
	LA_FILE_LOG = (*env)->GetStringUTFChars(env, logfile, 0);
	SDL_Log("nativeJlSendData \"%s\"\n", JL_FL_BASE);
}

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeLaExit(JNIEnv *env, jobject obj) {
	SDL_AtomicSet(&la_running, 0);
}

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeLaFraction(JNIEnv *env, jobject obj,
	float fraction)
{
	la_banner_size = fraction;
}

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativeLaPrint(JNIEnv *env, jobject obj,
	jstring data)
{
	const char* string_data = (*env)->GetStringUTFChars(env, data, 0);
	la_file_append(LA_FILE_LOG, string_data, strlen(string_data));
}

//

JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLInputConnection_nativeLaType(JNIEnv *env, jobject obj,
	jstring data)
{
	la_keyboard_press = *((*env)->GetStringUTFChars(env, data, 0));
}*/

int SDL_main(char* argv[], int argc) {
	main(argv, argc);
}

#endif
