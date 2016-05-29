/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRsprite.c
 *	Handles the sprites.
 */
#include "JLGRinternal.h"

static void jlgr_sprite_draw_to_pr__(jl_t *jl) {
	jl_sprite_t *sprite = jl_mem_temp(jl, NULL);

	((jlgr_sprite_draw_fnt)sprite->draw)(jl, 1, sprite->ctx_draw);
}

static void jlgr_sprite_redraw_tex__(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_mem_temp(jlgr->jl, spr);
	jl_gl_pr(jlgr, &spr->pr, jlgr_sprite_draw_to_pr__);
}

// Redraw a sprite
static inline void jlgr_sprite_redraw__(jlgr_t* jlgr, jl_sprite_t *spr) {
	// If pre-renderer hasn't been intialized, initialize & redraw.
	if(!spr->pr.tx) jlgr_sprite_resize(jlgr, spr, NULL);
	// Else, Redraw texture.
	else jlgr_sprite_redraw_tex__(jlgr, spr);
}

static inline void jlgr_sprite_sync__(jl_t* jl,jl_sprite_t *spr,void* ctx_draw){
	jl_thread_mutex_cpy(jl, spr->mutex, ctx_draw, spr->ctx_draw,
		spr->ctx_draw_size);
}

//
// Exported Functions
//

/**
 * THREAD: Any thread.
 * Empty sprite loop. ( Don't do anything )
 * @param jl: The library context
 * @param spr: The sprite
**/
void* jlgr_sprite_dont(jl_t* jl, jl_sprite_t* sprite) { return NULL; }

/**
 * THREAD: Main thread only.
 * Run a sprite's draw routine to draw on it's pre-rendered texture.
 *
 * @param jl: The library context
 * @param spr: Which sprite to draw.
**/
void jlgr_sprite_redraw(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_thread_mutex_lock(jlgr->jl, spr->mutex);
	spr->update = 1;
	jl_thread_mutex_unlock(jlgr->jl, spr->mutex);
}

/**
 * THREAD: Draw thread only.
 * Render a sprite's pre-rendered texture onto the screen.
 *
 * @param jl: The library context.
 * @param spr: The sprite.
**/
void jlgr_sprite_draw(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_thread_mutex_lock(jlgr->jl, spr->mutex);

	// Redraw if needed.
	if(spr->update) jlgr_sprite_redraw__(jlgr, spr);
	spr->update = 0;

	jl_gl_transform_pr_(jlgr, &spr->pr,
		spr->pr.cb.x, spr->pr.cb.y, spr->pr.cb.z,
		spr->pr.scl.x, spr->pr.scl.y, spr->pr.scl.z);

	jl_gl_draw_pr_(jlgr->jl, &spr->pr);
	//
	jl_thread_mutex_unlock(jlgr->jl, spr->mutex);
}

/**
 * THREAD: Draw thread only.
 * Resize a sprite to the current window - and redraw.
 * @param jlgr: The library context.
 * @param spr: The sprite to use.
**/
void jlgr_sprite_resize(jlgr_t* jlgr, jl_sprite_t *spr, jl_rect_t* rc) {
	if(rc) {
		// Set collision box.
		spr->pr.cb.x = rc->x; spr->pr.cb.y = rc->y;
		spr->pr.cb.w = rc->w; spr->pr.cb.h = rc->h;
		// Set real dimensions
		spr->rw = rc->w;
		spr->rh = rc->h;
	}
	// Resize
	jl_gl_pr_rsz(jlgr, &spr->pr, spr->rw, spr->rh, jl_gl_w(jlgr) * spr->rw);
	// Redraw
	jlgr_sprite_redraw_tex__(jlgr, spr);
	//
	jl_thread_mutex_unlock(jlgr->jl, spr->mutex);
}

/**
 * THREAD: Main thread only.
 * Run a sprite's loop.
 * @param jl: The library context.
 * @param spr: Which sprite to loop.
**/
void jlgr_sprite_loop(jlgr_t* jlgr, jl_sprite_t *spr) {
	jl_print_function(jlgr->jl, "Sprite/Loop");
	void* ctx_draw = ((jlgr_sprite_loop_fnt)spr->loop)(jlgr->jl, spr);
	jl_print_return(jlgr->jl, "Sprite/Loop");
	if(ctx_draw) jlgr_sprite_sync__(jlgr->jl, spr, ctx_draw);
}

/**
 * THREAD: Main thread only.
 * Create a new sprite.
 *
 * @param jlgr: The library context.
 * @param rc: The rectangle bounding box & pre-renderer size.
 * @param loopfn: the loop function.
 * @param drawfn: the draw function.
 * @param main_ctx_size: how many bytes to allocate for the main context.
 * @param draw_ctx_size: how many bytes to allocate for the draw context.
 * @returns: the new sprite
**/
jl_sprite_t* jlgr_sprite_new(jlgr_t* jlgr, jl_rect_t rc,
	jlgr_sprite_loop_fnt loopfn, jlgr_sprite_draw_fnt drawfn,
	void* main_ctx, uint32_t main_ctx_size,
	void* draw_ctx, uint32_t draw_ctx_size)
{
	jl_sprite_t *spr = NULL;

	spr = jl_memi(jlgr->jl, sizeof(jl_sprite_t));
	// Set real dimensions
	spr->rw = rc.w;
	spr->rh = rc.h;
	// Make pre-renderer
	jl_gl_pr_new(jlgr, &spr->pr, rc.w, rc.h, jl_gl_w(jlgr) * spr->rw);
	// Set collision box.
	spr->pr.cb.x = rc.x; spr->pr.cb.y = rc.y;
	spr->pr.cb.w = rc.w; spr->pr.cb.h = rc.h;
	// Set draw function.
	spr->draw = drawfn;
	// Set loop
	spr->loop = loopfn;
	// Make mutex
	spr->mutex = jl_thread_mutex_new(jlgr->jl);
	// Create main context.
	if(main_ctx_size)
		spr->ctx_main = jl_mem_copy(jlgr->jl, main_ctx, main_ctx_size);
	// Create draw context
	if(draw_ctx_size)
		spr->ctx_draw = jl_mem_copy(jlgr->jl, draw_ctx, draw_ctx_size);
	spr->ctx_draw_size = draw_ctx_size;
	return spr;
}

/**
 * THREAD: Main thread only.
**/
void jlgr_sprite_old(jlgr_t* jlgr, jl_sprite_t* sprite) {
	jl_thread_mutex_old(jlgr->jl, sprite->mutex);
	jl_mem(jlgr->jl, sprite, 0);
}

/**
 * THREAD: Main thread only.
 * test if 2 sprites collide.
 *
 * @param 'jl': library context
 * @param 'sprite1': sprite 1
 * @param 'sprite2': sprite 2
 * @return 0: if the sprites don't collide in their bounding boxes.
 * @return 1: if the sprites do collide in their bounding boxes.
**/
u8_t jlgr_sprite_collide(jlgr_t* jlgr,
	jl_sprite_t *sprite1, jl_sprite_t *sprite2)
{
	if (
		(sprite1->pr.cb.y >= (sprite2->pr.cb.y+sprite2->pr.cb.h)) ||
		(sprite1->pr.cb.x >= (sprite2->pr.cb.x+sprite2->pr.cb.w)) ||
		(sprite2->pr.cb.y >= (sprite1->pr.cb.y+sprite1->pr.cb.h)) ||
		(sprite2->pr.cb.x >= (sprite1->pr.cb.x+sprite1->pr.cb.w)) )
	{
		return 0;
	}else{
		return 1;
	}
}

void* jlgr_sprite_getcontext(jl_sprite_t *sprite) {
	return sprite->ctx_main;
}
