#include "main.h"
#include "la_effect.h"

static jlgr_t window;

static void ex_redraw(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	ctx_t* ctx = la_context(jl);
	la_light_t light = {
		(jl_vec3_t) { al_safe_get_float(&jlgr->main.ct.msx),
			al_safe_get_float(&jlgr->main.ct.msy) },
		(jl_vec3_t) { 1.f, 1.f, 1.f },
		.25f
	};

// Draw
	jlgr_vo_draw(jlgr, &ctx->vo1);
// Light
	la_effect_light(&ctx->vo1, &light, 1, (jl_vec3_t) { 1.f, 1.f, 1.f });
}

void ex_down(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		ctx_t* ctx = la_context(jlgr->jl);

		if(ctx->hasMenu) {
			ctx->hasMenu = 0;
		}else{
			ctx->hasMenu = 1;
		}
	}
}

void ex_edit_loop(jl_t* jl) {
	jlgr_input_do(jl->jlgr, JL_INPUT_MENU, ex_down, NULL);
	jlgr_menu_loop(jl->jlgr);
}

void ex_wdns(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_text_draw(jlgr, "testing""\xCA""1234567890",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, .0625f });
	ex_redraw(jl);
	jlgr_menu_draw(jlgr, 0);
}

void ex_wups(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	float fontcolor[] = { 0.f, 0.f, 0.f, 1.f };

	jl_gl_clear(jlgr, 1., 1., 1., 1.);
	jlgr_text_draw(jlgr, "this IS upper",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, fontcolor, .0625f});
}

// Called when window is made/resized.
static void ex_edit_resz(jl_t* jl) {
	ctx_t* ctx = la_context(jl);
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc1 = { 0.f, 0.f, 1.f, jl_gl_ar(jl->jlgr) };
	jl_rect_t rc2 = { 0.f, 0.f, 2.f, 1.f };
	float colors[] = { 1.f, 1.f, 1.f, 1.f };

	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resizing Window....");
	jlgr_vo_set_image(jlgr, &(ctx->vo1), rc1, jlgr->textures.game);
	jlgr_vo_set_rect(jlgr, &(ctx->vo2), rc2, colors, 0);
	jlgr_menu_draw(jlgr, 1);
	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resize'd Window....");
}

void ex_edit_init(jl_t* jl) {
	jlgr_loop_set(jl->jlgr, ex_wdns, ex_wups, ex_wdns, ex_edit_resz);
}

static inline void ex_init_modes(jl_t* jl) {
	// Initialize a mode.
	jl_mode_set(jl, EX_MODE_EDIT,
		(jl_mode_t) { ex_edit_init, ex_edit_loop, jl_dont });
	// Switch to the mode.
	jl_mode_switch(jl, EX_MODE_EDIT);
}

static inline void ex_init_tasks(jlgr_t* jlgr) {
	jlgr_menu_addicon_flip(jlgr);
	jlgr_menu_addicon_slow(jlgr);
//	jlgr_menu_addicon_name(jl);
}

static void ex_graphical_init(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Initializing");
	ex_init_tasks(jlgr);
	ex_init_modes(jl);
}

int main(int argc, char* argv[]) {
	la_start(la_dont, la_dont, "Lib Aldaron Test Program", sizeof(ctx_t));
	la_window_init(&window, ex_graphical_init);
}
