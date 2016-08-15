#include "main.h"
#include "la_video.h"

static void tv_exit(jl_t* jl) {
	ctx_t* ctx = jl_get_context(jl);
	const char* error;

	if((error = car_camera_kill(&ctx->camera)))
		la_panic(jl, "car_camera_kill error %s:", error);
}

static void tv_panic(jl_t* jl, const char* format, ...) {
	va_list arglist;

	tv_exit(jl);

	va_start( arglist, format );
	la_panic( jl, format, arglist );
	va_end( arglist );
}

void tv_down(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		ctx_t* ctx = jl_get_context(jlgr->jl);

		if(ctx->hasMenu) {
			ctx->hasMenu = 0;
		}else{
			ctx->hasMenu = 1;
		}
	}
}

void tv_edit_loop(jl_t* jl) {
	jlgr_input_do(jl->jlgr, JL_INPUT_MENU, tv_down, NULL);
//	for(i = 0; i < 3; i++)
//		jlgr_sprite_loop(jl->jlgr, &ctx->slider[i]);

	jlgr_menu_loop(jl->jlgr);
}

static void tv_wdns(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	ctx_t* ctx = jl_get_context(jl);
	const char* error;
	uint16_t w, h;

	if((error = car_camera_loop(&ctx->camera)))
		tv_panic(jl, "camera loop error: %s", error);
	la_video_load_jpeg(jl, ctx->pixels, ctx->video_stream, ctx->camera.size, &w, &h);
	la_texture_set(jlgr, ctx->video_stream_texture, ctx->pixels, 640, 480, 3);
	jlgr_draw_bg(jlgr, ctx->video_stream_texture, 0, 0, -1);
}

// Called when window is made/resized.
static void tv_edit_resz(jl_t* jl) {
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

void tv_edit_init(jl_t* jl) {
	jlgr_loop_set(jl->jlgr, tv_wdns, la_dont, tv_wdns, tv_edit_resz);
}

static inline void tv_init_modes(jl_t* jl) {
	//Set mode data
	jl_mode_set(jl, MODE_EDIT,
		(jl_mode_t) { tv_edit_init, tv_edit_loop, jl_dont });
//Leave terminal mode
	jl_mode_switch(jl, MODE_EDIT);
}

static inline void tv_init_tasks(jlgr_t* jlgr) {
	jlgr_menu_addicon_flip(jlgr);
	jlgr_menu_addicon_slow(jlgr);
//	jlgr_menu_addicon_name(jl);
}

static inline void tv_init_camera(jlgr_t* jlgr, ctx_t* ctx) {
	const char* error;
	if((error = car_camera_init(&ctx->camera, 0, 640, 480, &ctx->video_stream)))
		tv_panic(jlgr->jl, "car_camera_init error %s:", error);
	ctx->video_stream_texture = la_texture_new(jlgr, NULL, 640, 480, 3);
}

static void tv_init(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Initializing");
	jl_print(jl, "Initializing Graphics....");
	tv_init_tasks(jlgr);
	tv_init_camera(jlgr, jl_get_context(jl));
	jl_print(jl, "Initialize'd Graphics....");

	// Create the modes & initialize one.
	tv_init_modes(jl);
}

static void tv_main(jl_t* jl) {
	jlgr_init(jl, 0, tv_init); // Open Window
}

int main(int argc, char* argv[]) {
	return la_start(tv_main, tv_exit, "Test-Video!", sizeof(ctx_t));
}
