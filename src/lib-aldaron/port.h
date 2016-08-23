#ifndef LA_PORT
#define LA_PORT

#include "la.h"

#ifdef LA_PHONE_ANDROID
#include <EGL/egl.h>
#include <android/sensor.h>
#endif

#ifdef LA_PHONE_ANDROID
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};
#endif

typedef struct {
#ifdef LA_PHONE_ANDROID
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
#endif
} la_window_t;

// These functions must be supported by all platforms.
void la_port_swap_buffers(la_window_t* window);
void la_print(const char* format, ...);

#endif
