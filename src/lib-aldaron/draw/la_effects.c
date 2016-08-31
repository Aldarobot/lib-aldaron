#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"
#include "la_effect.h"
#include "la_memory.h"

/** @cond */

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

static void jlgr_effect_pr_hue__(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	jlgr_opengl_framebuffer_subtx_(jlgr);
	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
		0., 0., 1., jl_gl_ar(jlgr) }, jlgr->gl.cp->tx);
	jlgr_opengl_framebuffer_addtx_(jlgr, jlgr->gl.cp->tx);
	jlgr_effects_vo_hue(jlgr, &jlgr->gl.temp_vo, (jl_vec3_t) {
		0.f, 0.f, 0.f }, jlgr->effects.colors);
}

static void jlgr_effect_pr_light__(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	jlgr_opengl_framebuffer_subtx_(jlgr);
	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
		0., 0., 1., jl_gl_ar(jlgr) }, jlgr->gl.cp->tx);
	jlgr_opengl_framebuffer_addtx_(jlgr, jlgr->gl.cp->tx);
	jlgr_effects_vo_light(jlgr, &jlgr->gl.temp_vo,
		(jl_vec3_t) { 0.f, 0.f, 0.f }, jlgr->effects.vec3);
}

static void jlgr_effects_clear__(jl_t* jl) {
	jl_gl_clear(jl->jlgr, 0.f, 0.f, 0.f, 0.f);
}

/** @endcond */

void jlgr_effects_clear(la_window_t* jlgr, jl_vo_t* vo) {
	jlgr_pr(jlgr, &vo->pr, jlgr_effects_clear__);
}

/**
 * Draw a vertex object with alpha effect.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset vector to translate by.
 * @param a: The alpha value to multiply each pixel by. [ 0.f - 1.f ]
**/
void jlgr_effects_vo_alpha(la_window_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float a) {
	if(a < 0.f) a = 0.f;
	if(a > 1.f) a = 1.f;
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.alpha.shader);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &jlgr->effects.alpha.shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	// Set Alpha Value In Shader
	jlgr_opengl_uniform1(jlgr, 1, jlgr->effects.alpha.fade, &a);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.alpha.shader);
}

/**
 * Draw a vertex object, changing te hue of each pixel.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset to draw it at.
 * @param c: The new hue ( r, g, b, a ) [ 0.f - 1.f ]
**/
void jlgr_effects_vo_hue(la_window_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float c[]) {
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.hue.shader);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &jlgr->effects.hue.shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y + la_banner_size, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	// Set Hue Value In Shader
	jlgr_opengl_uniform4(jlgr, 1, jlgr->effects.hue.new_color, c);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.hue.shader);
}

/**
 * Draw a vertex object with effects.
 * @param jlgr: The library context.
 * @param vo: The vertex object.
**/
void jlgr_effects_draw(la_window_t* jlgr, jl_vo_t* vo) {
	jlgr_pr_draw(jlgr, &vo->pr, &vo->pr.cb.pos, 0);
}

/**
 * Draw a vertex object with light effect.
 * @param vo: The vertex object.
 * @param lights: The lights to shine on the vertex object.
 * @param light_count: How many lights are in the array "lights".
 * @param material_brightness: Maximum R, G, and B values.
**/
void la_effect_light(jl_vo_t* vo, la_light_t* lights, uint8_t light_count,
	jl_vec3_t material_brightness)
{
	la_window_t* jlgr = vo->jl->jlgr;
	int i;
	jlgr_glsl_t* shader = &jlgr->effect.shader_laa[light_count];
	float ar = jl_gl_ar(jlgr);

	// Create a shader if doesn't exist.
	if(!jlgr->effect.shader_laa_init[light_count]) {
		printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaa\n");
		if(light_count) {
			char frag_code[strlen(JL_EFFECT_LIGHT_AA) + 128];

			jl_mem_format2(frag_code, JL_EFFECT_LIGHT_AA, light_count);
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
		// Need to do some math.
//		jl_vec3_t light_position = (jl_vec3_t) {
//			(lights[i].position.x * 2.f)-1.f,
//			(lights[i].position.y * 2.f)-.5f,
//			(lights[i].position.z * 2.f),
//		};
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
		jlgr_opengl_uniform(jlgr, shader, (float*)&vo->pr.cb.pos, 3,
			"offset");
	}
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		vo->pr.cb.pos, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		ar);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, shader);
}

/**
 * @param material: material light properties
 *  ( shininess, max brightness, unused )
 *  or null for default material ( 32.f, 1.f, 0.f )
**/
void jlgr_effects_vo_light(la_window_t* jlgr, jl_vo_t* vo, jl_vec3_t offs,
	jl_vec3_t* material)
{
	jl_vec3_t default_material = (jl_vec3_t) { 32.f, 1.f, 0.f };
	if(material == NULL) material = &default_material;
	float normal[] = { 0.f, 0.f, 1.f };
#ifdef JL_DEBUG
	if(jlgr->effects.lights.point_count == 0)
		la_panic("jlgr_effects_vo_light: No lights have been created yet.");
#endif
	jlgr_effects_lightsource_t* lightsource = cl_array_borrow(
		jlgr->effects.lights.lights, jlgr->effects.lights.point_count-1);
	// Bind shader
	jlgr_opengl_draw1(jlgr, &lightsource->shader);
	// Update uniforms for material.
	jlgr_opengl_uniform3(jlgr, 1, lightsource->uniform_normal, normal);
	jlgr_opengl_uniform1(jlgr, 1, lightsource->uniform_shininess,&material->x);
	jlgr_opengl_uniform(jlgr, &lightsource->shader,&material->y, 1,
		"max_brightness");
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &lightsource->shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		jl_gl_ar(jlgr));
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &lightsource->shader);
}

/**
 * Apply a hue change effect to everything that's been drawn on the current pre-
 * renderer so far.
 * @param jlgr: The library context.
 * @param c: The hue to make everything.
**/
void jlgr_effects_hue(la_window_t* jlgr, float c[]) {
	jl_mem_copyto(c, jlgr->effects.colors, sizeof(float) * 4);
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_hue__);
}

/**
 * @param material: Material shininess / Max brightness
**/
void jlgr_effects_light(la_window_t* jlgr, jl_vec3_t* material) {
	jlgr->effects.vec3 = material;
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_light__);
}

/**
 * Delete all light sources.
**/
void jlgr_effects_light_clear(la_window_t* jlgr) {
	jlgr->effects.lights.has_directional = 0;
	jlgr->effects.lights.point_count = 0;
	jlgr->effects.lights.ambient = (jl_vec3_t) { 0.f, 0.f, 0.f };
}

/**
 * Add a new light source.
 * @param jlgr: The library context.
 * @param point: Light source location.
 * @param ambient, diffuse, specular: Color and intensity.
 * @param c, l, q: Light properties.
**/
void jlgr_effects_light_add(la_window_t* jlgr, jl_vec3_t point, float ambient[],
	float diffuse[], float specular[], float power)
{
	int which = jlgr->effects.lights.point_count;
	jlgr_effects_lightsource_t* lightsource;

	// Point Count
	jlgr->effects.lights.point_count++;
	// Add a light source.
	if(!(lightsource = cl_array_borrow(jlgr->effects.lights.lights, which))){
		char frag_code[strlen(JL_EFFECT_LIGHT) + 128];

		// Create a shader if doesn't exist.
		lightsource = cl_array_add(jlgr->effects.lights.lights);
		jl_mem_format2(frag_code, JL_EFFECT_LIGHT,
			jlgr->effects.lights.point_count);
		jlgr_opengl_shader_init(jlgr, &lightsource->shader,
			JL_EFFECT_LIGHTV, frag_code, 1);

		jlgr_opengl_shader_uniform(jlgr, &lightsource->shader,
			&lightsource->uniform_normal, "norm");
		jlgr_opengl_shader_uniform(jlgr, &lightsource->shader,
			&lightsource->uniform_shininess, "shininess");
	}
	// Point
	lightsource->position = (jl_vec3_t) {
		(point.x * 2.f)-1.f,
		(point.y * 2.f/jl_gl_ar(jlgr))-1.f,
		(point.z * 2.f)
	};
	jl_vec3_t add_ambient = (jl_vec3_t){ambient[0], ambient[1], ambient[2]};
	lightsource->diffuse = (jl_vec3_t) {
		diffuse[0], diffuse[1], diffuse[2]
	};
	lightsource->specular = (jl_vec3_t) {
		specular[0], specular[1], specular[2]
	};
	lightsource->power = 1.f / power;
	// Add ambient light.
	jl_mem_vec_add(&jlgr->effects.lights.ambient, &add_ambient);
}

void jlgr_effects_light_update(la_window_t* jlgr) {
	int i;
	jlgr_effects_lightsource_t* lightsource = cl_array_borrow(
		jlgr->effects.lights.lights, jlgr->effects.lights.point_count - 1);
	jlgr_glsl_t* shader = &(lightsource->shader);

	for(i = 0; i < jlgr->effects.lights.point_count; i++) {
		lightsource = cl_array_borrow(jlgr->effects.lights.lights, i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->position, 3,
			"pl[%d].position", i);
		jlgr_opengl_uniform(jlgr, shader,
			&lightsource->power, 1,
			"pl[%d].power", i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->diffuse, 3,
			"pl[%d].diffuse", i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->specular, 3,
			"pl[%d].specular", i);
	}
	jlgr_opengl_uniform(jlgr, shader,
		(float*) &jlgr->effects.lights.ambient, 3, "ambient");
}

void jlgr_effects_init__(la_window_t* jlgr) {
	la_print("MAKING EFFECT: ALPHA");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.alpha.shader, NULL,
		JL_EFFECT_ALPHA, 1);
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.alpha.shader,
		&jlgr->effects.alpha.fade, "multiply_alpha");

	la_print("MAKING EFFECT: HUE");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.hue.shader, NULL,
		JL_EFFECT_HUE, 1);
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.hue.shader,
		&jlgr->effects.hue.new_color, "new_color");

	la_print("MAKING EFFECT: SHADOW");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.shadow.shader,
		NULL, JL_EFFECT_SHADOW, 1);

	la_print("MADE EFFECTS!");
	jlgr->effects.lights.lights = cl_array_create(
		sizeof(jlgr_effects_lightsource_t), 0);
	jlgr_effects_light_clear(jlgr);
}
