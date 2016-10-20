#include "main.h"
#include "la_effect.h"
#include "la_text.h"
#include "la_memory.h"
#include <la_window.h>

static void ex_redraw(ctx_t* ctx) {
	la_window_t* window = ctx->window;

	la_light_t light = {
		(la_v3_t) { la_safe_get_float(&window->mouse_x),
			la_safe_get_float(&window->mouse_y) },
		(la_v3_t) { 1.f, 1.f, 1.f },
		.25f
	};

// Draw
	la_ro_draw(&ctx->vo1);
// Light
	la_effect_light(&ctx->vo1, &light, 1, (la_v3_t) { 1.f, 1.f, 1.f });

	la_text(window, LA_PXMOVE("0.1", "0.1") LA_PXSIZE("0.1") LA_PRED "WHATS THAT");
}

void ex_edit_loop(ctx_t* ctx) {
	la_menu_loop(&ctx->menu);
}

void ex_wdns(ctx_t* ctx, la_window_t* window) {
	ex_redraw(ctx);
	la_text(window, "testing""\xCA""1234567890");
	la_menu_draw(&ctx->menu, 0);
}

void ex_wups(ctx_t* ctx, la_window_t* window) {
	float colors[] = { 0.f, 0.f, 0.f, 1.f };

	la_window_clear(1., 1., 1., 1.);
	la_text_centered(window, "this IS alternate", .1, colors);
	la_menu_draw(&ctx->menu, 0);
}

// Called when window is made/resized.
static void test_resize(ctx_t* ctx, la_window_t* window) {
	float colors[] = { 1.f, 1.f, 1.f, 1.f };

	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resizing Window....");
	la_ro_image_rect(window, &(ctx->vo1), window->textures.game, 1.f, 1.f);
	la_ro_plain_rect(window, &(ctx->vo2), colors, 2.f, 1.f);
	la_menu_draw(&ctx->menu, 1);
	la_print("EXXXXXXXXXXXXXXXXXXXXAMPLE Resize'd Window....");
}

void ex_edit_init(ctx_t* ctx) {
	la_draw_fnchange(ctx->window, (la_draw_fn_t) ex_wdns,
		(la_draw_fn_t) ex_wups,
		(la_draw_fn_t) test_resize);
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
	return la_start((la_fn_t) ex_init, (la_fn_t) ex_loop, la_dont,
		"Lib Aldaron Test Program", sizeof(ctx_t));
}
