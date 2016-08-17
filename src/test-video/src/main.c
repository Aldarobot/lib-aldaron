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

void tv_edit_loop(jl_t* jl) {
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

void tv_edit_init(jl_t* jl) {
	jlgr_loop_set(jl->jlgr, tv_wdns, la_dont, tv_wdns, la_dont);
}

static inline void tv_init_modes(jl_t* jl) {
	jl_mode_set(jl, MODE_EDIT, (jl_mode_t) {
		tv_edit_init, tv_edit_loop, jl_dont
	});
}

static inline void tv_init_camera(jlgr_t* jlgr, ctx_t* ctx) {
	const char* error;
	if((error = car_camera_init(&ctx->camera, 0, 640, 480, &ctx->video_stream)))
		tv_panic(jlgr->jl, "car_camera_init error %s:", error);
	ctx->video_stream_texture = la_texture_new(jlgr, NULL, 640, 480, 3);
}

static void tv_init(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Loading Camera....");
	tv_init_camera(jlgr, jl_get_context(jl));
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Loaded Camera!");

	// Create the modes & initialize one.
	tv_init_modes(jl);
	//Set mode
	jl_mode_switch(jl, MODE_EDIT);
}

static void tv_main(jl_t* jl) {
	jlgr_init(jl, 0, tv_init); // Open Window
}

int main(int argc, char* argv[]) {
	return la_start(tv_main, tv_exit, "Test-Video!", sizeof(ctx_t));
}
