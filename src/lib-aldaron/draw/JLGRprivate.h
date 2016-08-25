#include "jlgr.h"
#include "JLprivate.h"

// Full texture
#define DEFAULT_TC (const float[]) { \
	0., 1., \
	0., 0., \
	1., 0., \
	1., 1. \
}

#define UPSIDEDOWN_TC (const float[]) { \
	0., 0., \
	0., 1., \
	1., 1., \
	1., 0. \
}

#define BACKWARD_TC (const float[]) {\
	1., 1., \
	1., 0., \
	0., 0., \
	0., 1. \
}

typedef enum{
	JLGR_ID_NULL,
	JLGR_ID_UNKNOWN,
	JLGR_ID_FLIP_IMAGE,
	JLGR_ID_SLOW_IMAGE,
	JLGR_ID_GOOD_IMAGE,
	JLGR_ID_TASK_MAX //how many taskbuttons
}jlgr_id_t;

typedef struct{
	uint8_t id;
	char string[256];
}jlgr_comm_notify_t;

uint32_t _jl_sg_gpix(/*in */ SDL_Surface* surface, int32_t x, int32_t y);
void jl_gl_viewport_screen(jlgr_t* jlgr);
void jlgr_opengl_matrix(jlgr_t* jlgr, jlgr_glsl_t* sh, jl_vec3_t scalev,
	jl_vec3_t rotatev, jl_vec3_t translatev, jl_vec3_t lookv, float ar);
void jl_gl_vo_free(jlgr_t* jlgr, jl_vo_t *pv);
uint32_t jl_gl_w(jlgr_t* jlgr);

// JLGRopengl.c
void jlgr_opengl_buffer_set_(jlgr_t* jlgr, uint32_t *buffer,
	const void *buffer_data, uint16_t buffer_size);
void jlgr_opengl_buffer_old_(jlgr_t* jlgr, uint32_t *buffer);
void jlgr_opengl_setv(jlgr_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw);
void jlgr_opengl_vertices_(jlgr_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl);
void jlgr_opengl_texture_bind_(jlgr_t* jlgr, uint32_t tex);

//DL
void _jl_sg_loop(jlgr_t* jlgr);
float jl_sg_seconds_past_(jl_t* jlc);

// Resize function
void jl_sg_resz__(jl_t* jlc);
void jlgr_resz(jlgr_t* jlgr, uint16_t x, uint16_t y);
void jlgr_menu_resize_(jlgr_t* jlgr);
// init functions.
void jl_wm_init__(jlgr_t* jlgr);
void jlgr_text_init__(jlgr_t* jlgr);
void jl_sg_init__(jlgr_t* jlgr);
void jl_gl_init__(jlgr_t* jlgr);
void jlgr_init__(jlgr_t* jlgr);
void jl_ct_init__(jlgr_t* jlgr);
void jlgr_fl_init(jlgr_t* jlgr);
void jlgr_menubar_init__(jlgr_t* jlgr);
void jlgr_mouse_init__(jlgr_t* jlgr);
void jlgr_thread_init(jlgr_t* jlgr, jl_fnct fn_);
void jlgr_effects_init__(jlgr_t* jlgr);
// loop
void jl_ct_loop__(jlgr_t* jlgr);
void jl_wm_loop__(jlgr_t* jlgr);
void _jlgr_loopa(jlgr_t* jlgr);
// kill
void jl_wm_kill__(jlgr_t* jlgr);
void jlgr_file_kill_(jlgr_t* jlgr);

//
void jl_wm_updatewh_(jlgr_t* jlgr);
