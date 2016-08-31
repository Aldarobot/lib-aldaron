#include "la_draw.h"

#ifdef LA_PHONE_ANDROID

#include "la_port.h"
#include "la_memory.h"

#include <jni.h>
#include <errno.h>

// OpenGLES for android.
#include <GLES/gl.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#define ANDROID_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Aldaron", __VA_ARGS__))

int main(int argc, char* argv[]);
void la_opengl_error__(int data, const char* fname);
void la_window_loop__(la_window_t* window);
void la_window_kill__(la_window_t* window);

extern const char* LA_FILE_ROOT;
extern const char* LA_FILE_LOG;
extern float la_banner_size;
extern SDL_atomic_t la_rmc;
la_window_t* la_window = NULL;

/**
 * Initialize an EGL context for the current display.
 */
static inline int window_init_display(la_window_t* window) {
	// initialize OpenGL ES and EGL

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

	const EGLint attrib_list [] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(window->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, window->app->window, NULL);
	context = eglCreateContext(display, config, NULL, attrib_list);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		la_print("Aldaron Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	window->display = display;
	window->context = context;
	window->surface = surface;
	window->width = w;
	window->height = h;
	return 0;
}

/**
 * Just the current frame in the display.
 */
void la_port_swap_buffers(la_window_t* window) {
	// If window exists, update
	if (window->display) eglSwapBuffers(window->display, window->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void window_term_display(la_window_t* window) {
	if (window->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(window->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (window->context != EGL_NO_CONTEXT) {
			eglDestroyContext(window->display, window->context);
		}
		if (window->surface != EGL_NO_SURFACE) {
			eglDestroySurface(window->display, window->surface);
		}
		eglTerminate(window->display);
	}
	window->display = EGL_NO_DISPLAY;
	window->context = EGL_NO_CONTEXT;
	window->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t window_handle_input(struct android_app* app, AInputEvent* event) {
	la_window_t* window = (la_window_t*)app->userData;
	int32_t input_type = AInputEvent_getType(event);

	if (input_type == AINPUT_EVENT_TYPE_MOTION) {
		
		return 1;
	}else if(input_type == AINPUT_EVENT_TYPE_KEY) {
		la_print("KEY EVENT");
		int32_t key_code = AKeyEvent_getKeyCode(event);
		switch(key_code) {
			case AKEYCODE_BACK:

				break;
			case AKEYCODE_CAMERA:
				la_print("Camera key pressed");
				break;
			case AKEYCODE_MENU:
				la_print("Menu key pressed");
				break;
			case AKEYCODE_DEL:
				la_print("Backspace key pressed");
				break;
			case AKEYCODE_ENTER:
				la_print("Enter key pressed");
				break;
			default:
				if(key_code >= AKEYCODE_A &&
					key_code <= AKEYCODE_PERIOD)
				{
					char key = AKeyEvent_getScanCode(event);
					la_print("Key Insert %s", &key);
				}
				break;
		}
	}
	return 1;
}

/**
 * Process the next main command.
 */
static void window_handle_cmd(struct android_app* app, int32_t cmd) {
	la_window_t* window = (la_window_t*)app->userData;
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			la_print("---------------------------- app save state");
			// The system has asked us to save our current state.  Do so.
//			window->app->savedState = malloc(sizeof(struct saved_state));
//			*((struct saved_state*)window->app->savedState) = NULL;//window->state;
//			window->app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if (window->app->window != NULL) {
				window_init_display(window);
				la_port_swap_buffers(window);
			}
			main(0, NULL);
			break;
		case APP_CMD_TERM_WINDOW:
			// The cross-platform window kill.
			la_window_kill__(window);
			// The window is being hidden or closed, clean it up.
			window_term_display(window);
			break;
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if (window->accelerometerSensor != NULL) {
				ASensorEventQueue_enableSensor(window->sensorEventQueue,
						window->accelerometerSensor);
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_setEventRate(window->sensorEventQueue,
						window->accelerometerSensor, (1000L/60)*1000);
			}
			break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if (window->accelerometerSensor != NULL) {
				ASensorEventQueue_disableSensor(window->sensorEventQueue,
						window->accelerometerSensor);
			}
			la_port_swap_buffers(window);
			break;
	}
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
	la_window_t* window = la_memory_allocate(sizeof(la_window_t));
	int ident;
	int events;
	struct android_poll_source* source;

	// Make sure glue isn't stripped.
	app_dummy();

	state->userData = window;
	state->onAppCmd = window_handle_cmd;
	state->onInputEvent = window_handle_input;
	window->app = state;

	// Prepare to monitor accelerometer
	window->sensorManager = ASensorManager_getInstance();
	window->accelerometerSensor = ASensorManager_getDefaultSensor(
		window->sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	window->sensorEventQueue = ASensorManager_createEventQueue(
		window->sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

//	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
//		window->state = *(struct saved_state*)state->savedState;
//	}

	// Run main():
	la_window = window;

	SDL_AtomicSet(&la_rmc, 1);
	// Window thread ( Drawing + Events ).
	while (SDL_AtomicGet(&la_rmc)) {
		// Poll Events
		ident = ALooper_pollAll(0, NULL, &events, (void**)&source);

		// Process this event.
		if (source != NULL) {
			source->process(state, source);
		}

		// If a sensor has data, process it now.
		if (ident == LOOPER_ID_USER) {
			if (window->accelerometerSensor != NULL) {
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(
						window->sensorEventQueue,
						&event, 1) > 0)
				{
					window->input.accel.x =
						event.acceleration.x;
					window->input.accel.y =
						event.acceleration.y;
					window->input.accel.z =
						event.acceleration.z;
				}
			}
		}
		// Run the cross-platform window loop.
		if(window->context) la_window_loop__(window);
		// Update the screen.
		la_port_swap_buffers(window);
	}
	la_print("port-android quitting....");
	// The cross-platform window kill.
	la_window_kill__(window);
	// The window is being hidden or closed, clean it up.
	window_term_display(window);
	la_print("port-android quitted....");
	exit(0);
	return;
}

void la_print(const char* format, ...) {
	char temp[256];
	va_list arglist;

	// Write to temp
	va_start( arglist, format );
	vsprintf( temp, format, arglist );
	va_end( arglist );

	la_file_append(LA_FILE_LOG, temp, strlen(temp)); // To File
	ANDROID_LOG("%s", temp); // To Logcat
}

void jl_mode_loop__(jl_t* jl);

void la_port_input(la_window_t* window) {
	// Touch Input
	window->input.touch.x = al_safe_get_float(&window->mouse_x);
	window->input.touch.y = al_safe_get_float(&window->mouse_y);
	window->input.touch.p = 0;
	if(safe_get_uint8(&window->in.touch.h)) {
		la_print("Just touch %d", safe_get_uint8(&window->in.touch.p));
		window->input.touch.h = 1;
		window->input.touch.p = safe_get_uint8(&window->in.touch.p);
		// Not just pressed anymore
		safe_set_uint8(&window->in.touch.h, 0);
	}else{
		window->input.touch.h = 0;
	}
	//
	if(safe_get_uint8(&window->in.back)) {
		la_print("Back");
		safe_set_uint8(&window->in.back, 0);
		jl_mode_exit(window->jl);
	}
}

JNIEXPORT void JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaSetFiles(JNIEnv *env, jobject obj,
	jstring data, jstring logfile)
{
	LA_FILE_ROOT = (*env)->GetStringUTFChars(env, data, 0);
	LA_FILE_LOG = (*env)->GetStringUTFChars(env, logfile, 0);
}

JNIEXPORT void JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaFraction(JNIEnv *env, jobject obj,
	float fraction)
{
	la_banner_size = fraction;
}

JNIEXPORT jint JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaDraw(JNIEnv *env, jobject obj) {
	la_window_loop__(la_window);
	if(SDL_AtomicGet(&la_rmc) == 0) return 100;
	else return 0;
}

JNIEXPORT void JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaResize(JNIEnv *env, jobject obj,
	jint w, jint h)
{
	// If not initialized, initialize.
	if(!la_window) {
		la_print("nativeLaResize %dx%d", w, h);
		// Allocate the window context.
		la_window = la_memory_allocate(sizeof(la_window_t));
		//
		la_window->width = w;
		la_window->height = h;
		// Run the main routine.
		main(0, NULL);
	}else{
		la_window->width = w;
		la_window->height = h;
	}
}

JNIEXPORT void JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaBack(JNIEnv *env, jobject obj) {
	la_print("Back key pressed");
	safe_set_uint8(&la_window->in.back, 1);
}

JNIEXPORT void JNICALL
Java_com_libaldaron_LibAldaronActivity_nativeLaTouch(JNIEnv *env, jobject obj,
	jint x, jint y, jint p, jint h)
{
	float xf = ((float)x) / ((float)la_window->width);
	float yf = ((float)y) / ((float)la_window->height);

	la_print("%fx%f", xf, yf);
	// Set location of virtual mouse.
	al_safe_set_float(&la_window->mouse_x, xf);
	al_safe_set_float(&la_window->mouse_y, yf * la_window->wm.ar);
	safe_set_uint8(&la_window->in.touch.p, p);
	if(h) safe_set_uint8(&la_window->in.touch.h, h);
}

#endif
