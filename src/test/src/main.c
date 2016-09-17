#include "main.h"
#include "la_effect.h"
#include "la_text.h"
#include "la_memory.h"

static void ex_redraw(jl_t* jl) {
	la_window_t* window = jl->jlgr;
	ctx_t* ctx = la_context(jl);
	la_light_t light = {
		(jl_vec3_t) { la_safe_get_float(&window->mouse_x),
			la_safe_get_float(&window->mouse_y) },
		(jl_vec3_t) { 1.f, 1.f, 1.f },
		.25f
	};

// Draw
	jlgr_vo_draw(window, &ctx->vo1);
// Light
	la_effect_light(&ctx->vo1, &light, 1, (jl_vec3_t) { 1.f, 1.f, 1.f });

	la_text(window, LA_PXMOVE("0.5", "0.1") LA_PXSIZE("0.1") LA_PRED "WHATS THAT");
}

void ex_down(la_window_t* jlgr, jlgr_input_t input) {
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
	ctx_t* ctx = la_context(jl);

	la_menu_loop(&ctx->menu);
}

void ex_wdns(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;
	ctx_t* ctx = la_context(jl);

	jlgr_text_draw(jlgr, "testing""\xCA""1234567890",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, .0625f });
	ex_redraw(jl);
	la_menu_draw(&ctx->menu, 0);
}

void ex_wups(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	float fontcolor[] = { 0.f, 0.f, 0.f, 1.f };

	jl_gl_clear(jlgr, 1., 1., 1., 1.);
	jlgr_text_draw(jlgr, "this IS upper",
		(jl_vec3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, fontcolor, .0625f});
}

// Called when window is made/resized.
static void ex_edit_resz(jl_t* jl) {
	ctx_t* ctx = la_context(jl);
	la_window_t* jlgr = jl->jlgr;
	jl_rect_t rc1 = { 0.f, 0.f, 1.f, jl_gl_ar(jl->jlgr) };
	jl_rect_t rc2 = { 0.f, 0.f, 2.f, 1.f };
	float colors[] = { 1.f, 1.f, 1.f, 1.f };

	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resizing Window....");
	jlgr_vo_set_image(jlgr, &(ctx->vo1), rc1, jlgr->textures.game);
	jlgr_vo_set_rect(jlgr, &(ctx->vo2), rc2, colors, 0);
	la_menu_draw(&ctx->menu, 1);
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

static inline void ex_init_tasks(la_window_t* window) {
	ctx_t* ctx = la_context(window->jl);

	la_menu_init(&ctx->menu, window);
	la_menu_addicon_flip(&ctx->menu);
	la_menu_addicon_slow(&ctx->menu);
	la_menu_addicon_name(&ctx->menu);
}

static void ex_init(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Initializing");
	ex_init_tasks(jlgr);
	ex_init_modes(jl);
}

int main(int argc, char* argv[]) {
	return la_start(ex_init, la_dont, 1, "Lib Aldaron Test Program",
		sizeof(ctx_t));
}
