/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

/** Structure for lights. */
typedef struct {
	la_v3_t position;
	la_v3_t color;
	float power;
}la_light_t;

void la_effect_light(la_vo_t* vo, la_light_t* lights, uint8_t light_count,
	la_v3_t material_brightness);
