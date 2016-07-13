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
	"uniform sampler2D texture;\n"
	"varying vec2 texcoord;\n"
	"\n"
	"varying vec3 fragpos;\n"
	"\n"
	"uniform vec3 light_color;\n"
	"uniform vec3 norm;\n"
	"uniform vec3 lightPos;\n"
	"uniform vec3 ambient;\n"
	"\n"
	"void main() {\n"
	// Diffuse
	"	vec3 lightDir = normalize(lightPos - fragpos);\n"
	"	float diff = max(dot(norm, lightDir), 0.0);\n"
	"	vec3 diffuse = diff * light_color;\n"
	// Specular
	"	float specularStrength = 0.5f;\n"
	"	vec3 viewDir = normalize(-fragpos);\n"
	"	vec3 reflectDir = reflect(-lightDir, norm);\n"
	"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
	"	vec3 specular = specularStrength * spec * light_color;\n"
	// Result
	"	gl_FragColor = vec4(ambient + diffuse + specular, 1.0f) *\n"
	"		texture2D(texture, texcoord);\n"
	"}";

const char* JL_EFFECT_LIGHTV =
	GLSL_HEAD
	"uniform vec3 translate;\n"
	"uniform vec4 transform;\n"
	"\n"
	"attribute vec3 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"varying vec3 fragpos;\n"
	"\n"
	"void main() {\n"
	"	texcoord = texpos;\n"
	"	vec4 pos = transform * vec4(position + translate, 1.0);"
	"	fragpos = vec3(pos.x, pos.y, pos.z);"
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
	jlgr_effects_vo_light(jlgr, &jlgr->gl.temp_vo, (jl_vec3_t) {
		0.f, 0.f, 0.f }, jlgr->effects.normal, jlgr->effects.lightPos,
		jlgr->effects.colors, jlgr->effects.ambient);
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
	jlgr_opengl_transform_(jlgr, &jlgr->effects.alpha.shader,
		offs.x, offs.y, offs.z, 1., 1., 1., jl_gl_ar(jlgr));
	// Set Alpha Value In Shader
	jlgr_opengl_uniform1(jlgr, jlgr->effects.alpha.fade, a);
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
	jlgr_opengl_transform_(jlgr, &jlgr->effects.hue.shader,
		offs.x, offs.y, offs.z, 1., 1., 1., jl_gl_ar(jlgr));
	// Set Hue Value In Shader
	jlgr_opengl_uniform4(jlgr, jlgr->effects.hue.new_color,
		c[0], c[1], c[2], c[3]);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.hue.shader);
}

void jlgr_effects_vo_light(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs,
	jl_vec3_t normal, jl_vec3_t lightPos, float color[], float ambient[])
{
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.light.shader);
	// Translate by offset vector
	jlgr_opengl_transform_(jlgr, &jlgr->effects.light.shader,
		offs.x, offs.y, offs.z, 1., 1., 1., jl_gl_ar(jlgr));
	// Set Hue Value In Shader
	jlgr_opengl_uniform3(jlgr, jlgr->effects.light.color,
		color[0], color[1], color[2]);
	jlgr_opengl_uniform3(jlgr, jlgr->effects.light.norm,
		normal.x, normal.y, normal.z);
	jlgr_opengl_uniform3(jlgr, jlgr->effects.light.lightPos,
		lightPos.x, lightPos.y, lightPos.z);
	jlgr_opengl_uniform3(jlgr, jlgr->effects.light.ambient,
		ambient[0], ambient[1], ambient[2]);
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

void jlgr_effects_light(jlgr_t* jlgr, jl_vec3_t normal, jl_vec3_t lightPos,
	float c[], float ambient[])
{
	jl_mem_copyto(c, jlgr->effects.colors, sizeof(float) * 3);
	jl_mem_copyto(c, jlgr->effects.ambient, sizeof(float) * 3);
	jlgr->effects.normal = normal;
	jlgr->effects.lightPos = lightPos;
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_light__);
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
		&jlgr->effects.light.norm, "norm");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.light.lightPos, "lightPos");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.light.color, "light_color");
	jlgr_opengl_shader_uniform(jlgr, &jlgr->effects.light.shader,
		&jlgr->effects.light.ambient, "ambient");

	JL_PRINT_DEBUG(jlgr->jl, "MADE EFFECTS!");
}
