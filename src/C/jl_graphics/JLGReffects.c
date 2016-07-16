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
/*	"uniform float material_shininess;\n"
	"\n"
	"uniform vec3 direction;\n"
	"uniform vec3 direction_ambient;\n"
	"uniform vec3 direction_diffuse;\n"
	"uniform vec3 direction_specular;\n"
	"\n"
	"\n"
	"uniform vec3 normal;\n"
	"uniform sampler2D texture;\n"
	"varying vec2 texcoord;\n"
	"varying vec3 fragpos;\n"
	"\n"
	"vec3 CalcPointLight(int which, vec3 normal, vec3 fragPos,\n"
	"	vec3 viewDir)\n"
	"{\n"
	"	vec3 lightDir = normalize(point[which] - fragPos);\n"
	"	// Diffuse shading\n"
	"	float diff = max(dot(normal, lightDir), 0.0);\n"
	"	// Specular shading\n"
	"	vec3 reflectDir = reflect(-lightDir, normal);\n"
	"	float spec = pow(max(dot(viewDir, reflectDir), 0.0),\n" 	
	"		material_shininess);\n"
	"	// Attenuation\n"
	"	float distance = length(point[which] - fragPos);\n"
	"	float attenuation = 1.0 / (point_constant[which] +\n"
	"		point_linear[which] *\n"
	"		distance + point_quadratic * (distance * distance));\n"
	"	// Combine results\n"
	"	vec3 ambient = point_ambient *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	vec3 diffuse = point_diffuse * diff *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	vec3 specular = point_specular * spec *\n"
	"		texture2D(texture, texcoord).rgb;\n"
	"	ambient *= attenuation;\n"
	"	diffuse *= attenuation;\n"
	"	specular *= attenuation;\n"
	"	return (ambient + diffuse + specular);\n"
	"}\n"
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
	"	// Point lights\n"
	"	for(int i = 0; i < point_count; i++)\n"
	"	result += CalcPointLight(point_count, norm, fragpos, view_dir);\n"
	"	gl_FragColor = vec4(result, 1.0);\n"
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
	"uniform int point_count;\n"
	"uniform vec3 point[32];\n"
	"uniform float point_constant[32];\n"
	"uniform float point_linear[32];\n"
	"uniform float point_quadratic[32];\n"
	"uniform vec3 point_ambient[32];\n"
	"uniform vec3 point_diffuse[32];\n"
	"uniform vec3 point_specular[32];\n"
	"\n"
	"void main() {\n"
	"	vec3 lightDir;\n"
	"	float diff;\n"
	"	vec3 halfwayDir;\n"
	"	float spec;\n"
	"	vec3 result = vec3(0.0, 0.0, 0.0);\n"
	"	for(int i = 0; i < point_count; i++) {\n"
	// Attenuation\n"
	"		float distance = length(point[i] - fragpos);\n"
	"		float attenuation = 1.0 / (point_constant[i] +\n"
	"			point_linear[i] * distance +\n"
	"			point_quadratic[i] * (distance * distance));\n"
	// Ambient
	"		result += point_ambient[i] * attenuation;\n"
	// Diffuse
	"		lightDir = normalize(point[i] - fragpos);\n"
	"		diff = max(dot(norm, lightDir), 0.0);\n"
	"		result += diff * point_diffuse[i] * attenuation;\n"
	// Specular
	"		halfwayDir = normalize(lightDir + normalize(-fragpos));\n"
	"		spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);\n"
	"		result += spec * point_specular[i] * attenuation;\n"
	"	}\n"
	// Result
	"	gl_FragColor = vec4(result, 1.0f) * texture2D(texture, texcoord);\n"
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
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader);
	// Update uniforms for material.
	jlgr_opengl_uniform3(jlgr, 1, jlgr->effects.lights.uniform_normal,
		normal);
	jlgr_opengl_uniform1(jlgr, 1, jlgr->effects.lights.uniform_shininess,
		&shininess);
	// Translate by offset vector
	jlgr_opengl_matrix(jlgr, &jlgr->effects.light.shader,
		(jl_vec3_t) { 1.f, 1.f, 1.f }, // Scale
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Rotate
		(jl_vec3_t) { offs.x, offs.y, offs.z }, // Translate
		(jl_vec3_t) { 0.f, 0.f, 0.f }, // Look
		1.f, jl_gl_ar(jlgr), 0.f, 1.f);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.hue.shader);
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
	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader);
	int32_t point_count = 0;

	jlgr->effects.lights.has_directional = 0;
	jlgr->effects.lights.point_count = 0;

	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader); // Bind shader
	jlgr_opengl_uniform1i(jlgr, 1, jlgr->effects.lights.uniform_point_count,
		&point_count);
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
	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader);
	int which = jlgr->effects.lights.point_count;
	// Point Count
	jlgr->effects.lights.point_count++;
	int32_t point_count = jlgr->effects.lights.point_count;
	jlgr_opengl_uniform1i(jlgr, 1, jlgr->effects.lights.uniform_point_count,
		&point_count);
	// Point
	jl_mem_copyto((float*) &point, jlgr->effects.lights.point_position +
		(sizeof(float) * 3 * which), sizeof(float) * 3);
	jl_mem_copyto(ambient, jlgr->effects.lights.point_ambient +
		(sizeof(float) * 3 * which), sizeof(float) * 3);
	jl_mem_copyto(diffuse, jlgr->effects.lights.point_diffuse +
		(sizeof(float) * 3 * which), sizeof(float) * 3);
	jl_mem_copyto(specular, jlgr->effects.lights.point_specular +
		(sizeof(float) * 3 * which), sizeof(float) * 3);
	jlgr->effects.lights.point_constant[which] = c;
	jlgr->effects.lights.point_linear[which] = l;
	jlgr->effects.lights.point_quadratic[which] = q;
}

void jlgr_effects_light_update(jlgr_t* jlgr) {
	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader);
	jlgr_opengl_uniform3(jlgr, 32, jlgr->effects.lights.uniform_point,
		jlgr->effects.lights.point_position);
	jlgr_opengl_uniform1(jlgr, 32, jlgr->effects.lights.uniform_point_constant,
		jlgr->effects.lights.point_constant);
	jlgr_opengl_uniform1(jlgr, 32, jlgr->effects.lights.uniform_point_linear,
		jlgr->effects.lights.point_linear);
	jlgr_opengl_uniform1(jlgr, 32, jlgr->effects.lights.uniform_point_quadratic,
		jlgr->effects.lights.point_quadratic);
	jlgr_opengl_uniform3(jlgr, 32, jlgr->effects.lights.uniform_point_ambient,
		jlgr->effects.lights.point_ambient);
	jlgr_opengl_uniform3(jlgr, 32, jlgr->effects.lights.uniform_point_diffuse,
		jlgr->effects.lights.point_diffuse);
	jlgr_opengl_uniform3(jlgr, 32, jlgr->effects.lights.uniform_point_specular,
		jlgr->effects.lights.point_specular);
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

	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: LIGHT");
	jlgr_opengl_shader_init(jlgr, &jlgr->effects.light.shader,
		JL_EFFECT_LIGHTV, JL_EFFECT_LIGHT, 1);

	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_count, "point_count");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point, "point");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_constant, "point_constant");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_linear, "point_linear");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_quadratic, "point_quadratic");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_ambient, "point_ambient");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_diffuse, "point_diffuse");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_point_specular, "point_specular");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_normal, "norm");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.lights.uniform_shininess, "shininess");

	JL_PRINT_DEBUG(jlgr->jl, "MADE EFFECTS!");
	jlgr_effects_light_clear(jlgr);
}
