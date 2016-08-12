#include "main.h"

static void ex_redraw(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	ctx_t* ctx = jl_get_context(jl);

// Draw
	jlgr_vo_draw(jlgr, &ctx->vo1);
// Light
	jlgr_effects_light_begin(jlgr, &ctx->vo1);
	jlgr_effects_light_aa(jlgr, &ctx->vo1,
		(jl_vec3_t) { al_safe_get_float(&jlgr->main.ct.msx),
			al_safe_get_float(&jlgr->main.ct.msy), 0.f },
		(jl_vec3_t) { 1.f, 1.f, 1.f }, .5f, 1.f);
	jlgr_effects_light_end(jlgr);
	jlgr_effects_draw(jlgr, &ctx->vo1);
}

void ex_down(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		ctx_t* ctx = jl_get_context(jlgr->jl);

		if(ctx->hasMenu) {
			ctx->hasMenu = 0;
		}else{
			ctx->hasMenu = 1;
		}
	}
}

void ex_edit_loop(jl_t* jl) {
//	ctx_t* ctx = jl_get_context(jl);
//	int i;

	jlgr_input_do(jl->jlgr, JL_INPUT_MENU, ex_down, NULL);
//	for(i = 0; i < 3; i++)
//		jlgr_sprite_loop(jl->jlgr, &ctx->slider[i]);

	jlgr_menu_loop(jl->jlgr);
}

void ex_wdns(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jl, "wdns");
	jlgr_text_draw(jlgr, "testing""\xCA""1234567890",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, .0625f });
	ex_redraw(jl);
	jl_print_return(jl, "wdns");
	jlgr_menu_draw(jlgr, 0);
}

void ex_wups(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jl, "wups");
	float fontcolor[] = { 0.f, 0.f, 0.f, 1.f };

	jl_gl_clear(jlgr, 1., 1., 1., 1.);
	jlgr_text_draw(jlgr, "this IS upper",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, fontcolor, .0625f});
	jl_print_return(jl, "wups");
}

// Called when window is made/resized.
static void ex_edit_resz(jl_t* jl) {
	jl_print(jl, "Resizing Window....");

	ctx_t* ctx = jl_get_context(jl);
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc1 = { 0.2f, 0.2f, .1f, .1f };
	jl_rect_t rc2 = { 0.f, 0.f, 2.f, 1.f };
	float colors[] = { 1.f, 1.f, 1.f, 1.f };
//	float ar = jl_gl_ar(jlgr);
//	jl_rect_t rect[] = {
//		{ 0., ar - .05, 1./3., .05 },
//		{ 1./3., ar - .05, 1./3., .05 },
//		{ 2./3., ar - .05, 1./3., .05 }};
//	int i;

	jlgr_vo_set_image(jlgr, &(ctx->vo1), rc1, jlgr->textures.game);
	jlgr_vo_set_rect(jlgr, &(ctx->vo2), rc2, colors, 0);
//	for(i = 0; i < 3; i++)
//		jlgr_sprite_resize(jlgr, &ctx->slider[i], &rect[i]);

	jlgr_menu_draw(jlgr, 1);

	jl_print(jl, "Resize'd Window....");
}

void ex_edit_init(jl_t* jl) {
	jlgr_loop_set(jl->jlgr, ex_wdns, ex_wups, ex_wdns, ex_edit_resz);
}

static inline void ex_init_modes(jl_t* jl) {
	//Set mode data
	jl_mode_set(jl, EX_MODE_EDIT,
		(jl_mode_t) { ex_edit_init, ex_edit_loop, jl_dont });
//Leave terminal mode
	jl_mode_switch(jl, EX_MODE_EDIT);
}

static inline void ex_init_tasks(jlgr_t* jlgr) {
	jlgr_menu_addicon_flip(jlgr);
	jlgr_menu_addicon_slow(jlgr);
//	jlgr_menu_addicon_name(jl);
}

static inline void ex_init_objs(jlgr_t* jlgr) {
//	ctx_t* ctx = jl_get_context(jlgr->jl);
//	float ar = jl_gl_ar(jlgr);
//	jl_rect_t rect[] = {
//		{ 0., ar - .1, .5, .1 }, { .5, ar - .1, .5, .1 },
//		{ 0., ar - .2, .5, .1 }, { .5, ar - .2, .5, .1 },
//		{ 0., ar - .3, .5, .1 }, { .5, ar - .3, .5, .1 }};
//	uint8_t i;

//	for(i = 0; i < 3; i++)
//		jlgr_gui_slider(jlgr, &ctx->slider[i],
//			rect[i], 2, &ctx->s1[i], &ctx->s2[i]);
}

static void ex_graphical_init(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jl, "Example");
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Initializing");
	jl_print(jl, "Initializing Graphics....");
	ex_init_tasks(jlgr);
	ex_init_objs(jlgr);

	jl_print(jl, "Initialize'd Graphics....");

	// Create the modes & initialize one.
	ex_init_modes(jl);

	jl_print_return(jl, "Example");
}

void ex_init(jl_t* jl) {
	jl_print_function(jl, "Example");
	jl_print(jl,"Initializing....");
	// Open Window
	jlgr_init(jl, 0, ex_graphical_init);
	jl_print_return(jl, "Example");
}

int main(int argc, char* argv[]) {
	return jl_start(ex_init, "Example JL_Lib Program", sizeof(ctx_t));
}
