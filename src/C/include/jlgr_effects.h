#ifdef JLGR_EFFECTS_TYPES
	#ifndef JLGR_EFFECTS_TYPES_H
	#define JLGR_EFFECTS_TYPES_H

typedef struct{

}jlgr_effects_light_point_t;

typedef struct{
	// Colors
	float ambient[3];
	float diffuse[3];
	float specular[3];
	//
	jl_vec3_t direction;
}jlgr_effects_light_directional_t;

typedef struct{
	// Directional Light
	uint8_t has_directional;
	jlgr_effects_light_directional_t directional;
	// Point Light
	uint8_t point_count;
	// Colors
	float point_ambient[3*32];
	float point_diffuse[3*32];
	float point_specular[3*32];
	//
	float point_position[3*32];
	// Reach
	float point_constant[32];
	float point_linear[32];
	float point_quadratic[32];
	// Uniforms
	int32_t uniform_point_count;
	int32_t uniform_point;
	int32_t uniform_point_constant;
	int32_t uniform_point_linear;
	int32_t uniform_point_quadratic;
	int32_t uniform_point_ambient;
	int32_t uniform_point_diffuse;
	int32_t uniform_point_specular;
	int32_t uniform_normal;
	int32_t uniform_shininess;
}jlgr_effects_light_t;

	#endif
#else
	#ifndef JLGR_EFFECTS_H
	#define JLGR_EFFECTS_H

	

	#endif
#endif
