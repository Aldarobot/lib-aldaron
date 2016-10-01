/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifdef JLGR_EFFECTS_TYPES
	#ifndef JLGR_EFFECTS_TYPES_H
	#define JLGR_EFFECTS_TYPES_H

	#include <la_math.h> // Defines INFINITY

	typedef struct{
		jlgr_glsl_t shader; // specifically for this many light sources.
		// Uniforms
		int32_t uniform_normal;
		int32_t uniform_shininess;
		// Colors
		la_v3_t diffuse;
		la_v3_t specular;
		//
		la_v3_t position;
		// 
		float power;
	}jlgr_effects_lightsource_t;

	typedef struct{
		// Colors
		float ambient[3];
		float diffuse[3];
		float specular[3];
		//
		la_v3_t direction;
	}jlgr_effects_light_directional_t;

	typedef struct{
		// Directional Light
		uint8_t has_directional;
		jlgr_effects_light_directional_t directional;
		// Point Light
		int32_t point_count;
		struct cl_array *lights;
		// Ambient light
		la_v3_t ambient;
	}jlgr_effects_light_t;

	#define LA_EFFECT_VARIABLE INFINITY

	#endif
#else
	#ifndef JLGR_EFFECTS_H
	#define JLGR_EFFECTS_H

	

	#endif
#endif
