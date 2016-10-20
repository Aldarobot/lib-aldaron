/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_gui.h>
#include <la_text.h>

void la_notify(la_notify_t* notify, const char* notification, ...) {
	char message[256];
	va_list arglist;

	va_start( arglist, notification );
	vsprintf( message, notification, arglist );
	va_end( arglist );

	la_safe_set_string(&notify->message, message);
	la_safe_set_float(&notify->timeTilVanish, 4.5);
}

void la_notify_draw(la_window_t* window, la_notify_t* notify) {
	float time_until_vanish = la_safe_get_float(&notify->timeTilVanish);

	// Notifications:
	if(time_until_vanish > 0.f) {
		if(time_until_vanish > .5) {
			float color[] = { 1., 1., 1., 1. };
			la_text_centered(window, la_safe_get_string(
				&notify->message), 0, color);
		}else{
			float color[] = { 1., 1., 1., (time_until_vanish / .5)};
			la_text_centered(window, la_safe_get_string(
				&notify->message), 0, color);
		}
		la_safe_set_float(&notify->timeTilVanish,
			time_until_vanish - window->psec);
	}
}

#endif
