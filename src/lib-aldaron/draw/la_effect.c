/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

#include "la_memory.h"

#include <la_effect.h>
#include <la_ro.h>

void jlgr_opengl_framebuffer_subtx_(la_window_t* jlgr);
void jlgr_opengl_framebuffer_addtx_(la_window_t* jlgr, uint32_t tx);
void jlgr_opengl_blend_add_(la_window_t* jlgr);
void jlgr_opengl_blend_default_(la_window_t* jlgr);
void jlgr_opengl_blend_none_(la_window_t* jlgr);

extern float la_banner_size;

const char *JL_EFFECT_SHADOW = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vec4(0., 0., 0., texture2D(texture, texcoord).a);\n"
	"}";

const char* JL_EFFECT_ALPHA = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform float multiply_alpha;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	gl_FragColor = vec4(vcolor.rgb, vcolor.a * multiply_alpha);\n"
	"}";

const char* JL_EFFECT_HUE = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform vec4 new_color;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	mediump float grayscale = vcolor.r + vcolor.g + vcolor.b;\n"
	"	grayscale = grayscale / 3.0;\n"
	"	gl_FragColor = \n"
	"		vec4(new_color.r * grayscale, new_color.g * grayscale,"
	"		new_color.b * grayscale, new_color.a * vcolor.a);\n"
	"}";

const char* JL_EFFECT_DIRLIGHT =
	GLSL_HEAD
	"uniform vec3 direction;\n"
	"uniform vec3 direction_ambient;\n"
	"uniform vec3 direction_diffuse;\n"
	"uniform vec3 direction_specular;\n"
	"\n"
	"vec3 CalcDirLight(vec3 normal, vec3 viewDir) {\n"
	"	vec3 lightDir = normalize(-direction);\n"
	"	// Diffuse\n"
	"	float diff = max(dot(normal, lightDir), 0.0);\n"
	"	// Specular\n"
	"	vec3 reflectDir = reflect(-lightDir, normal);\n"
	"	float spec = pow(max(dot(viewDir, reflectDir), 0.0),\n"
	"		material.shininess);\n"
	"	// Combine results\n"
	"	vec3 ambient = direction_ambient *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	vec3 diffuse = direction_diffuse * diff *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	vec3 specular = direction_specular * spec *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	return (ambient + diffuse + specular);\n"
	"}\n"
	"\n"
	"void main() {\n"
	"	// Properties\n"
	"	vec3 norm = normalize(normal);\n"
	"	vec3 view_dir = normalize(-fragpos);\n"
	"	// Directional light\n"
	"	vec3 result = CalcDirLight(norm, view_dir);\n"
	"}";

const char* JL_EFFECT_LIGHT =
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"varying vec2 texcoord;\n"
	"\n"
	"varying vec3 fragpos;\n"
	"\n"
	"uniform vec3 norm;\n"
	"uniform float shininess;\n"
	"uniform float max_brightness;\n"
	"\n"
	"#define NUM_LIGHTS %d\n"
	"\n"
	"struct PointLight{\n"
	"	float power;\n"
	"	vec3 position;\n"
	"	vec3 diffuse;\n"
	"	vec3 specular;\n"
	"};\n"
	"\n"
	"uniform PointLight pl[32];\n"
	"uniform vec3 ambient;\n"
	"\n"
	"vec3 point_light(PointLight pointlight) {\n"
	"	vec3 difference = pointlight.position - fragpos;\n"
	"	vec3 lightDir = normalize(difference);\n"
	"	float distance = length(difference);\n"
	"	vec3 light = \n"
	// Diffuse
	"		max(dot(norm, lightDir), 0.0) * pointlight.diffuse + \n"
	// Specular
	"		pointlight.specular *\n"
	"			pow(max(dot(norm, normalize(lightDir - \n"
	"				normalize(fragpos))), 0.0), shininess);\n"
	// Attenuation
	"	light /= (pointlight.power * distance * distance);\n"
	"	return light;\n"
	"}\n"
	"\n"
	"void main() {\n"
	// Ambient
	"	vec3 result = ambient;\n"
	// Point light
	"	for(int i = 0; i < NUM_LIGHTS; i++) result += point_light(pl[i]);\n"
	// Result
	"	gl_FragColor = vec4(max_brightness * result, 1.0) * texture2D(texture, texcoord);\n"
	"}";

// Simple Point Light.
const char* JL_EFFECT_LIGHT_AA =
	GLSL_HEAD
	"#define NUM_LIGHTS %d\n"
	"\n"
	"uniform sampler2D texture;\n"
	"varying vec2 texcoord;\n"
	"varying vec3 fragpos;\n"
	"\n"
	// Material
	"uniform vec3 brightness;\n"
	"\n"
	// Light
	"uniform vec3 color[NUM_LIGHTS];\n"
	"uniform vec3 where[NUM_LIGHTS];\n"
	"uniform float power[NUM_LIGHTS];\n"
	"\n"
	"void main() {\n"
	"	vec3 light = vec3(0.0, 0.0, 0.0);\n"
	"	for(int i = 0; i < NUM_LIGHTS; i++) light += vec3(min(pow(power[i] * length(where[i] - fragpos), -2.0), 1.0) * brightness * color[i]);\n"
	"	gl_FragColor = vec4(light, 1.0) * texture2D(texture, texcoord);\n"
	"}";

const char* JL_EFFECT_LIGHTV =
	GLSL_HEAD
	"uniform mat4 scale_object;\n"
	"uniform mat4 rotate_object;\n"
	"uniform mat4 translate_object;\n"
	"uniform mat4 rotate_camera;\n"
	"uniform mat4 project_scene;\n"
	"uniform vec3 offset;\n"
	"\n"
	"attribute vec4 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"varying vec3 fragpos;\n"
	"\n"
	"void main() {\n"
	"	texcoord = texpos;\n"
	"	vec4 pos = translate_object * rotate_object * scale_object *\n"
	"		position;\n"
	"	fragpos = offset + position.xyz;\n"
	"	gl_Position = project_scene * rotate_camera * pos;\n"
	"}";

static inline void la_effect_format__(char* rtn, const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	vsnprintf( rtn, strlen(format) + 128, format, arglist );
	va_end( arglist );
}

static void jlgr_effect_pr_hue__(la_window_t* jlgr) {
	jlgr_opengl_framebuffer_subtx_(jlgr);
	la_ro_image_rect(jlgr, &jlgr->gl.temp_vo, jlgr->gl.cp->tx,
		1., jl_gl_ar(jlgr));
	jlgr_opengl_framebuffer_addtx_(jlgr, jlgr->gl.cp->tx);
	la_effect_hue(&jlgr->gl.temp_vo, jlgr->effects.colors);
}

/**
 * Draw a vertex object with alpha effect.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset vector to translate by.
 * @param a: The alpha value to multiply each pixel by. [ 0.f - 1.f ]
**/
void jlgr_effects_vo_alpha(la_window_t* jlgr, la_ro_t* vo, la_v3_t offs, float a) {
	if(a < 0.f) a = 0.f;
	if(a > 1.f) a = 1.f;
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effect.alpha);
	// Set Alpha Value In Shader
	jlgr_opengl_uniform(jlgr, &jlgr->effect.alpha, &a, 1,
		"multiply_alpha");
	// Draw on screen
	la_ro_draw_shader(vo, &jlgr->effect.alpha);
}

void la_effect_hue(la_ro_t* ro, float c[]) {
	la_window_t* window = ro->window;

	// Bind shader
	jlgr_opengl_draw1(window, &window->effect.hue);
	// Set Hue Value In Shader
	jlgr_opengl_uniform(window, &window->effect.hue, c, 4,
		"new_color");
	// Draw on screen
	la_ro_draw_shader(ro, &window->effect.hue);
}

void la_effect_light(la_ro_t* ro, la_light_t* lights, uint8_t light_count,
	la_v3_t material_brightness)
{
	la_window_t* jlgr = ro->window;
	int i;
	jlgr_glsl_t* shader = &jlgr->effect.shader_laa[light_count];

	// Create a shader if doesn't exist.
	if(!jlgr->effect.shader_laa_init[light_count]) {
		if(light_count) {
			char frag_code[strlen(JL_EFFECT_LIGHT_AA) + 128];

			la_effect_format__(frag_code, JL_EFFECT_LIGHT_AA, light_count);
			jlgr_opengl_shader_init(jlgr, shader, JL_EFFECT_LIGHTV,
				frag_code, 1);
		}else{
			jlgr_opengl_shader_init(jlgr, shader, NULL,
				JL_EFFECT_SHADOW, 1);
		}
		jlgr->effect.shader_laa_init[light_count] = 1;
	}
	// Bind shader
	jlgr_opengl_draw1(jlgr, shader);
	// Update uniforms for material.
	for(i = 0; i < light_count; i++) {
		float power = 1.f / lights[i].power;
		// Push the uniform
		jlgr_opengl_uniform(jlgr, shader, (float*)&lights[i].position,
			3, "where[%d]", i);
		jlgr_opengl_uniform(jlgr, shader, (float*)&lights[i].color,
			3, "color[%d]", i);
		jlgr_opengl_uniform(jlgr, shader, (float*)&power,
			1, "power[%d]", i);
	}
	if(light_count) {
		jlgr_opengl_uniform(jlgr, shader, (float*)&material_brightness,
			3, "brightness");
		jlgr_opengl_uniform(jlgr, shader, (float*)&ro->cb.pos, 3,
			"offset");
	}
	// Draw on screen
	la_ro_draw_shader(ro, shader);
}

/**
 * Apply a hue change effect to everything that's been drawn on the current pre-
 * renderer so far.
 * @param jlgr: The library context.
 * @param c: The hue to make everything.
**/
void jlgr_effects_hue(la_window_t* jlgr, float c[]) {
	la_memory_copy(c, jlgr->effects.colors, sizeof(float) * 4);
	la_ro_pr(jlgr, jlgr, jlgr->gl.cp, (la_fn_t) jlgr_effect_pr_hue__);
}

void jlgr_effects_init__(la_window_t* jlgr) {
	la_print("MAKING EFFECT: ALPHA");
	jlgr_opengl_shader_init(jlgr, &jlgr->effect.alpha, NULL,
		JL_EFFECT_ALPHA, 1);

	la_print("MAKING EFFECT: HUE");
	jlgr_opengl_shader_init(jlgr, &jlgr->effect.hue, NULL,
		JL_EFFECT_HUE, 1);

	la_print("MAKING EFFECT: SHADOW");
	jlgr_opengl_shader_init(jlgr, &jlgr->effect.shadow, NULL,
		JL_EFFECT_SHADOW, 1);

	la_print("MADE EFFECTS!");
}

#endif
