#include "main.h"
#include "la_effect.h"
#include "la_text.h"
#include "la_memory.h"

static void ex_redraw(ctx_t* ctx) {
	la_window_t* window = ctx->window;

	la_light_t light = {
		(la_v3_t) { la_safe_get_float(&window->mouse_x),
			la_safe_get_float(&window->mouse_y) },
		(la_v3_t) { 1.f, 1.f, 1.f },
		.25f
	};

// Draw
	jlgr_vo_draw(window, &ctx->vo1);
// Light
	la_effect_light(&ctx->vo1, &light, 1, (la_v3_t) { 1.f, 1.f, 1.f });

	la_text(window, LA_PXMOVE("0.1", "0.1") LA_PXSIZE("0.1") LA_PRED "WHATS THAT");
}

void ex_edit_loop(ctx_t* ctx) {
	la_menu_loop(&ctx->menu);
}

void ex_wdns(ctx_t* ctx) {
	la_window_t* jlgr = ctx->window;

	jlgr_text_draw(jlgr, "testing""\xCA""1234567890",
		(la_v3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, .0625f });
	ex_redraw(ctx);
	la_menu_draw(&ctx->menu, 0);
}

void ex_wups(ctx_t* ctx) {
	la_window_t* jlgr = ctx->window;

	float fontcolor[] = { 0.f, 0.f, 0.f, 1.f };

	jl_gl_clear(jlgr, 1., 1., 1., 1.);
	jlgr_text_draw(jlgr, "this IS upper",
		(la_v3_t) { 0., 0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, fontcolor, .0625f});
	la_menu_draw(&ctx->menu, 0);
}

// Called when window is made/resized.
static void test_resize(ctx_t* ctx) {
	la_window_t* jlgr = ctx->window;
	jl_rect_t rc1 = { 0.f, 0.f, 1.f, jl_gl_ar(jlgr) };
	jl_rect_t rc2 = { 0.f, 0.f, 2.f, 1.f };
	float colors[] = { 1.f, 1.f, 1.f, 1.f };

	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resizing Window....");
	jlgr_vo_set_image(jlgr, &(ctx->vo1), rc1, jlgr->textures.game);
	jlgr_vo_set_rect(jlgr, &(ctx->vo2), rc2, colors, 0);
	la_menu_draw(&ctx->menu, 1);
	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resize'd Window....");
}

void ex_edit_init(ctx_t* ctx) {
	la_draw_fnchange(ctx->window, (la_fn_t) ex_wdns, (la_fn_t) ex_wups,
		(la_fn_t) test_resize);
}

static inline void ex_init_tasks(ctx_t* ctx) {
	la_window_t* window = ctx->window;

	la_menu_init(&ctx->menu, window);
	la_menu_addicon_flip(&ctx->menu);
	la_menu_addicon_slow(&ctx->menu);
	la_menu_addicon_name(&ctx->menu);
}

static void ex_loop(ctx_t* ctx) {
	la_mode_run(ctx, ctx->mode);
}

static void ex_init(ctx_t* ctx, la_window_t* window) {
	ctx->window = window;
	jlgr_draw_msge(window, window->textures.logo, 0, "Initializing");
	ex_init_tasks(ctx);
	la_mode_init(ctx, &ctx->mode, (la_mode_t)
		{ ex_edit_loop, ex_edit_init, la_dont } );
}

int main(int argc, char* argv[]) {
	return la_start((la_fn_t) ex_init, (la_fn_t) ex_loop, la_dont, 1,
		"Lib Aldaron Test Program", sizeof(ctx_t));
}
