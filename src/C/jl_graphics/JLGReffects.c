#include "JLGRprivate.h"
#include "jlgr_opengl_private.h"

/** @cond */

void jlgr_opengl_framebuffer_subtx_(jlgr_t* jlgr);
void jlgr_opengl_framebuffer_addtx_(jlgr_t* jlgr, uint32_t tx);

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

const char* JL_EFFECT_LIGHT =
	GLSL_HEAD
/*	"uniform vec3 direction;\n"
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
*/
	"uniform sampler2D texture;\n"
	"varying vec2 texcoord;\n"
	"\n"
	"varying vec3 fragpos;\n"
	"\n"
	"uniform vec3 norm;\n"
	"uniform float shininess;\n"
	"\n"
	"#define NUM_LIGHTS %d\n"
	"\n"
	"struct PointLight{\n"
	"	vec3 position;\n"
	"\n"
	"	float constant;\n"
	"	float linear;\n"
	"	float quadratic;\n"
	"\n"
	"	vec3 ambient;\n"
	"	vec3 diffuse;\n"
	"	vec3 specular;\n"
	"};\n"
	"\n"
	"uniform PointLight pl[32];\n"
	"\n"
	"vec3 point_light(PointLight pointlight) {\n"
	"	vec3 difference = pointlight.position - fragpos;\n"
	"	vec3 lightDir = normalize(difference);\n"
	"	float distance = length(difference);\n"
	"	vec3 light = \n"
	// Ambient
	"		pointlight.ambient + \n"
	// Diffuse
	"		max(dot(norm, lightDir), 0.0) * pointlight.diffuse + \n"
	// Specular
	"		pow(max(dot(norm, normalize(lightDir + \n"
	"			normalize(-fragpos))), 0.0), shininess)\n"
	"			* pointlight.specular;\n"
	// Attenuation
	"	light *= 1.0 / (pointlight.constant +\n"
	"		pointlight.linear * distance +\n"
	"		pointlight.quadratic * (distance * distance));\n"
	"	return light;\n"
	"}\n"
	"\n"
	"void main() {\n"
	"	vec3 result = vec3(0.0, 0.0, 0.0);\n"
	// Point light
	"	for(int i = 0; i < NUM_LIGHTS; i++) result += point_light(pl[i]);\n"
	// Result
	"	gl_FragColor = vec4(result, 1.0) * texture2D(texture, texcoord);\n"
	"}";

const char* JL_EFFECT_LIGHTV =
	GLSL_HEAD
	"uniform mat4 scale_object;\n"
	"uniform mat4 rotate_object;\n"
	"uniform mat4 translate_object;\n"
	"uniform mat4 rotate_camera;\n"
	"uniform mat4 project_scene;\n"
	"\n"
	"attribute vec4 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"varying vec3 fragpos;\n"
	"\n"
	"void main() {\n"
	"	texcoord = texpos;\n"
	"	vec4 pos = project_scene * rotate_camera *\n"
	"		translate_object * rotate_object * scale_object *\n"
	"		position;\n"
	"	fragpos = pos.xyz;\n"
	"	gl_Position = pos;\n"
	"}";

static void jlgr_effect_pr_hue__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_opengl_framebuffer_subtx_(jlgr);
	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
		0., 0., 1., jl_gl_ar(jlgr) }, jlgr->gl.cp->tx);
	jlgr_opengl_framebuffer_addtx_(jlgr, jlgr->gl.cp->tx);
	jlgr_effects_vo_hue(jlgr, &jlgr->gl.temp_vo, (jl_vec3_t) {
		0.f, 0.f, 0.f }, jlgr->effects.colors);
}

static void jlgr_effect_pr_light__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_opengl_framebuffer_subtx_(jlgr);
	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
		0., 0., 1., jl_gl_ar(jlgr) }, jlgr->gl.cp->tx);
	jlgr_opengl_framebuffer_addtx_(jlgr, jlgr->gl.cp->tx);
	jlgr_effects_vo_light(jlgr, &jlgr->gl.temp_vo,
		(jl_vec3_t) { 0.f, 0.f, 0.f });
}

/** @endcond */

/**
 * Draw a vertex object with alpha effect.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset vector to translate by.
 * @param a: The alpha value to multiply each pixel by. [ 0.f - 1.f ]
**/
void jlgr_effects_vo_alpha(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float a) {
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.alpha.shader);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &jlgr->effects.alpha.shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		1.f, jl_gl_ar(jlgr), 0.f, 1.f);
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
void jlgr_effects_vo_hue(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float c[]) {
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.hue.shader);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &jlgr->effects.hue.shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		1.f, jl_gl_ar(jlgr), 0.f, 1.f);
	// Set Hue Value In Shader
	jlgr_opengl_uniform4(jlgr, 1, jlgr->effects.hue.new_color, c);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.hue.shader);
}

void jlgr_effects_vo_light(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs) {
	float normal[] = { 0.f, 0.f, 1.f };
	float shininess = 32.f;
#ifdef JL_DEBUG
	if(jlgr->effects.lights.point_count == 0) {
		jl_print(jlgr->jl, "jlgr_effects_vo_light: No lights have been"
			" created yet.");
		exit(-1);
	}
#endif
	jlgr_effects_lightsource_t* lightsource = cl_array_borrow(
		jlgr->effects.lights.lights, jlgr->effects.lights.point_count-1);
	// Bind shader
	jlgr_opengl_draw1(jlgr, &lightsource->shader);
	// Update uniforms for material.
	jlgr_opengl_uniform3(jlgr, 1, lightsource->uniform_normal,
		normal);
	jlgr_opengl_uniform1(jlgr, 1, lightsource->uniform_shininess,
		&shininess);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &lightsource->shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		1.f, jl_gl_ar(jlgr), 0.f, 1.f);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &lightsource->shader);
}

/**
 * Apply a hue change effect to everything that's been drawn on the current pre-
 * renderer so far.
 * @param jlgr: The library context.
 * @param c: The hue to make everything.
**/
void jlgr_effects_hue(jlgr_t* jlgr, float c[]) {
	jl_mem_copyto(c, jlgr->effects.colors, sizeof(float) * 4);
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_hue__);
}

/**
 * @param shininess: Material shininess.
**/
void jlgr_effects_light(jlgr_t* jlgr, float shininess) {
//	jl_mem_copyto(c, jlgr->effects.colors, sizeof(float) * 3);
//	jl_mem_copyto(c, jlgr->effects.ambient, sizeof(float) * 3);
//	jlgr->effects.normal = normal;
//	jlgr->effects.lightPos = lightPos;
//	jlgr->effects.colors[3] = shininess;
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_light__);
}

/**
 * Delete all light sources.
**/
void jlgr_effects_light_clear(jlgr_t* jlgr) {
	jlgr->effects.lights.has_directional = 0;
	jlgr->effects.lights.point_count = 0;
}

/**
 * Add a new light source.
 * @param jlgr: The library context.
 * @param point: Light source location.
 * @param ambient, diffuse, specular: Color and intensity.
 * @param c, l, q: Light properties.
**/
void jlgr_effects_light_add(jlgr_t* jlgr, jl_vec3_t point, float ambient[],
	float diffuse[], float specular[], float c, float l, float q)
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
	lightsource->ambient = (jl_vec3_t) {
		ambient[0], ambient[1], ambient[2]
	};
	lightsource->diffuse = (jl_vec3_t) {
		diffuse[0], diffuse[1], diffuse[2]
	};
	lightsource->specular = (jl_vec3_t) {
		specular[0], specular[1], specular[2]
	};
	lightsource->constant = c;
	lightsource->linear = l;
	lightsource->quadratic = q;
}

void jlgr_effects_light_update(jlgr_t* jlgr) {
	jl_print(jlgr->jl, "Light count = %d", jlgr->effects.lights.point_count);
	int i;
	jlgr_effects_lightsource_t* lightsource = cl_array_borrow(
		jlgr->effects.lights.lights, jlgr->effects.lights.point_count - 1);
	jl_print(jlgr->jl, "%p\n",lightsource);
	jlgr_glsl_t* shader = &(lightsource->shader);
	jl_print(jlgr->jl, "%p\n",shader);

	for(i = 0; i < jlgr->effects.lights.point_count; i++) {
		lightsource = cl_array_borrow(jlgr->effects.lights.lights, i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->position, 3,
			"pl[%d].position", i);
		jlgr_opengl_uniform(jlgr, shader,
			&lightsource->constant, 1,
			"pl[%d].constant", i);
		jlgr_opengl_uniform(jlgr, shader,
			&lightsource->linear, 1,
			"pl[%d].linear", i);
		jlgr_opengl_uniform(jlgr, shader,
			&lightsource->quadratic, 1,
			"pl[%d].quadratic", i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->ambient, 3,
			"pl[%d].ambient", i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->diffuse, 3,
			"pl[%d].diffuse", i);
		jlgr_opengl_uniform(jlgr, shader,
			(float*) &lightsource->specular, 3,
			"pl[%d].specular", i);
	}
}

void jlgr_effects_init__(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: ALPHA");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.alpha.shader, NULL,
		JL_EFFECT_ALPHA, 1);
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.alpha.shader,
		&jlgr->effects.alpha.fade, "multiply_alpha");

	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: HUE");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.hue.shader, NULL,
		JL_EFFECT_HUE, 1);
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.hue.shader,
		&jlgr->effects.hue.new_color, "new_color");

	JL_PRINT_DEBUG(jlgr->jl, "MADE EFFECTS!");
	jlgr->effects.lights.lights = cl_array_create(
		sizeof(jlgr_effects_lightsource_t), 0);
	jlgr_effects_light_clear(jlgr);
}
