/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_EFFECT
#define LA_EFFECT

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

/** Structure for lights. */
typedef struct {
	la_v3_t position;
	la_v3_t color;
	float power;
}la_light_t;

void la_effect_light(la_ro_t* ro, la_light_t* lights, uint8_t light_count,
	la_v3_t material_brightness);
void la_effect_hue(la_ro_t* ro, float c[]);

#endif
