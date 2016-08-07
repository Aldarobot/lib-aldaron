/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRsprite.c
 *	Handles the sprites.
 */
#include "JLGRprivate.h"

static void jlgr_sprite_draw_to_pr__(jl_t *jl) {
	jl_sprite_t *sprite = jl_mem_temp(jl, NULL);

	jl_print_function(jl, "sprite->draw");
	((jlgr_sprite_draw_fnt)sprite->draw)(jl, sprite->resize, sprite->ctx_draw);
	jl_print_return(jl, "sprite->draw");
}

static void jlgr_sprite_redraw_tex__(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_print_function(jlgr->jl, "RedrawSpriteTex");
	jl_mem_temp(jlgr->jl, spr);
	jl_thread_mutex_lock(&spr->mutex);
	jlgr_pr(jlgr, &spr->pr, jlgr_sprite_draw_to_pr__);
	jl_thread_mutex_unlock(&spr->mutex);
	jl_print_return(jlgr->jl, "RedrawSpriteTex");
}

// Redraw a sprite
static inline void jlgr_sprite_redraw__(jlgr_t* jlgr, jl_sprite_t *spr) {
	spr->resize = 0;
	jl_print_function(jlgr->jl, "RedrawSprite");
	// If pre-renderer hasn't been intialized, initialize & redraw.
	if(!spr->pr.tx) jlgr_sprite_resize(jlgr, spr, NULL);
	// Else, Redraw texture.
	else jlgr_sprite_redraw_tex__(jlgr, spr);
	jl_print_return(jlgr->jl, "RedrawSprite");
	jl_print_function(jlgr->jl, "redraw-update");
	// Don't Redraw Again.
	jl_thread_mutex_lock(&spr->mutex);
	spr->update = 0;
	jl_thread_mutex_unlock(&spr->mutex);
	jl_print_return(jlgr->jl, "redraw-update");
}

static inline void jlgr_sprite_sync__(jl_t* jl, jl_sprite_t *spr, void* ctx) {
	jl_thread_mutex_cpy(jl, &spr->mutex, ctx, spr->ctx_draw,
		spr->ctx_draw_size);
}

//
// Exported Functions
//

/**
 * THREAD: Any thread.
 * Empty sprite loop. ( Don't do anything )
 * @param jl: The library context
 * @param sprite: The sprite
**/
void jlgr_sprite_dont(jl_t* jl, jl_sprite_t* sprite) { }

/**
 * THREAD: Main thread only.
 * Run a sprite's draw routine to draw on it's pre-rendered texture.
 *
 * @param jl: The library context
 * @param spr: Which sprite to draw.
 * @param ctx: Data to copy to drawing thread's context.
**/
void jlgr_sprite_redraw(jlgr_t* jlgr, jl_sprite_t *spr, void* ctx) {
	// Tell drawing thread to redraw.
	jl_thread_mutex_lock(&spr->mutex);
	spr->update = 1;
	jl_thread_mutex_unlock(&spr->mutex);
	// Copy drawing context up to drawing thread.
	if(ctx) jlgr_sprite_sync__(jlgr->jl, spr, ctx);
}

/**
 * THREAD: Draw thread only.
 * Render a sprite's pre-rendered texture onto the screen.
 *
 * @param jl: The library context.
 * @param spr: The sprite.
**/
void jlgr_sprite_draw(jlgr_t* jlgr, jl_sprite_t *spr) {
	// Redraw if needed.
	if(spr->update) jlgr_sprite_redraw__(jlgr, spr);
	// Draw onto screen
	jl_print_function(jlgr->jl, "sprite-draw");
	jl_thread_mutex_lock(&spr->mutex);
	jlgr_pr_draw(jlgr, &spr->pr, &spr->pr.cb.pos, spr->rs);
	jl_thread_mutex_unlock(&spr->mutex);
	jl_print_return(jlgr->jl, "sprite-draw");
}

/**
 * THREAD: Draw thread only.
 * Resize a sprite to the current window - and redraw.
 * @param jlgr: The library context.
 * @param spr: The sprite to use.
**/
void jlgr_sprite_resize(jlgr_t* jlgr, jl_sprite_t *spr, jl_rect_t* rc) {
	jl_print_function(jlgr->jl, "sprite-resize");
	jl_thread_mutex_lock(&spr->mutex);
	if(rc) {
		// Set collision box.
		spr->pr.cb.pos.x = rc->x; spr->pr.cb.pos.y = rc->y;
		spr->pr.cb.ofs.x = rc->w; spr->pr.cb.ofs.y = rc->h;
		// Set real dimensions
		spr->rw = rc->w;
		spr->rh = rc->h;
	}
	// Resize
	jlgr_pr_resize(jlgr, &spr->pr, spr->rw, spr->rh, jl_gl_w(jlgr) * spr->rw);
	//
	jl_thread_mutex_unlock(&spr->mutex);
	// Redraw
	spr->resize = 1;
	jlgr_sprite_redraw_tex__(jlgr, spr);
	//
	jl_print_return(jlgr->jl, "sprite-resize");
}

/**
 * THREAD: Main thread only.
 * Run a sprite's loop.
 * @param jl: The library context.
 * @param spr: Which sprite to loop.
**/
void jlgr_sprite_loop(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_print_function(jlgr->jl, "Sprite/Loop");
	((jlgr_sprite_loop_fnt)spr->loop)(jlgr->jl, spr);
	jl_print_return(jlgr->jl, "Sprite/Loop");
}

/**
 * THREAD: Main thread only.
 * Create a new sprite.
 *
 * @param jlgr: The library context.
 * @param sprite: The sprite to initialize.
 * @param rc: The rectangle bounding box & pre-renderer size.
 * @param loopfn: the loop function.
 * @param drawfn: the draw function.
 * @param main_ctx_size: how many bytes to allocate for the main context.
 * @param draw_ctx_size: how many bytes to allocate for the draw context.
 * @returns: the new sprite
**/
void jlgr_sprite_init(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rc,
	jlgr_sprite_loop_fnt loopfn, jlgr_sprite_draw_fnt drawfn,
	void* main_ctx, uint32_t main_ctx_size,
	void* draw_ctx, uint32_t draw_ctx_size)
{
	jl_t* jl = jlgr->jl;

	jl_print_function(jl, "sprite-init");
	// Set real dimensions
	sprite->rw = rc.w;
	sprite->rh = rc.h;
	// Set collision box.
	sprite->pr.cb.pos.x = rc.x; sprite->pr.cb.pos.y = rc.y;
	sprite->pr.cb.ofs.x = rc.w; sprite->pr.cb.ofs.y = rc.h;
	// Set draw function.
	sprite->draw = drawfn;
	// Set loop
	sprite->loop = loopfn;
	// Make mutex
	jl_thread_mutex_new(jl, &sprite->mutex);
	// Create main context.
	if(main_ctx_size)
		sprite->ctx_main = jl_mem_copy(jl, main_ctx, main_ctx_size);
	// Create draw context
	if(draw_ctx_size)
		sprite->ctx_draw = jl_mem_copy(jl, draw_ctx, draw_ctx_size);
	sprite->ctx_draw_size = draw_ctx_size;
	sprite->rs = 0;
	jl_print_return(jl, "sprite-init");
}

/**
 * THREAD: Main thread only.
**/
void jlgr_sprite_free(jlgr_t* jlgr, jl_sprite_t* sprite) {
}

/**
 * THREAD: Main thread only.
 * test if 2 sprites collide.
 *
 * @param 'jl': library context
 * @param 'spr1': sprite 1
 * @param 'spr2': sprite 2
 * @return 0: if the sprites don't collide in their bounding boxes.
 * @return 1: if the sprites do collide in their bounding boxes.
**/
uint8_t jlgr_sprite_collide(jlgr_t* jlgr, jl_pr_t *pr1, jl_pr_t *pr2) {
	if (
		(pr1->cb.pos.y >= (pr2->cb.pos.y+pr2->cb.ofs.y)) ||
		(pr1->cb.pos.x >= (pr2->cb.pos.x+pr2->cb.ofs.x)) ||
		(pr2->cb.pos.y >= (pr1->cb.pos.y+pr1->cb.ofs.y)) ||
		(pr2->cb.pos.x >= (pr1->cb.pos.x+pr1->cb.ofs.x)) )
	{
		return 0;
	}else{
		return 1;
	}
}

/**
 * Clamp a coordinate to an area.
 * @param xyz: The vertex to clamp.
 * @param area: The area to clamp it to.
 * @param rtn: The return vector.
**/
void jlgr_sprite_clamp(jl_vec3_t xyz, jl_area_t area, jl_vec3_t* rtn) {
	xyz.x -= area.pos.x;
	if(area.ofs.x != 0.f) xyz.x /= area.ofs.x;
	xyz.y -= area.pos.y;
	if(area.ofs.y != 0.f) xyz.y /= area.ofs.y;
	xyz.z -= area.pos.z;
	if(area.ofs.z != 0.f) xyz.z /= area.ofs.z;
	*rtn = xyz;
}

void* jlgr_sprite_getcontext(jl_sprite_t *sprite) {
	return sprite->ctx_main;
}

/**
 * THREAD: Draw thread only.
 * @parm sprite: The sprite to get the  contxt from.
**/
void* jlgr_sprite_getdrawctx(jl_sprite_t *sprite) {
	return sprite->ctx_draw;
}
