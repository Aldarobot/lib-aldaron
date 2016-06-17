#ifndef JLGR
#define JLGR

#include "jl.h"

// Enum:
typedef enum {
	JL_SCR_UP,
	JL_SCR_DN,
	JL_SCR_SS,
}jlgr_which_screen_t;

// Types:

// Coordinate Structures
typedef struct{
	float x, y, w, h;
}jl_rect_t;

typedef struct{
	float x, y, z;
}jl_vec3_t;

// Collision Box / Line / Etc.
typedef struct{
	jl_vec3_t pos; // Position ( X/Y/Z )
	jl_vec3_t ofs; // Position Offset ( W/H/D )
}jl_area_t;

// Graphical stuff
typedef struct {
	SDL_Window* w;		// Window
	SDL_GLContext* c;	// GL Context
}jl_window_t;

typedef struct {
	m_u32_t gl_texture;
	m_u32_t gl_buffer;
	m_u16_t w, h;
	void* pixels;
}jl_tex_t;

// Pre-renderer
typedef struct {
	// What to render
	uint32_t tx;	// ID to texture.
	uint32_t db;	// ID to Depth Buffer
	uint32_t fb;	// ID to Frame Buffer
	uint16_t w, h;	// Width and hieght of texture
	// Render Area
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	float ar;	// Aspect Ratio: h:w
	float cv[4*3];	// Converted Vertices
	jl_area_t cb;	// 2D/3D collision box.
	jl_vec3_t scl;	// Scaling vector.
}jl_pr_t;

//Vertex Object
typedef struct{
	// Basic:
	uint8_t rs;	// Rendering Style 0=GL_TRIANGLE_FAN 1=GL_TRIANGLES
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	uint32_t vc;	// # of Vertices
	float* cv;	// Converted Vertices
	uint32_t bt;	// Buffer for Texture coordinates or Color Vertices.
	// Coloring:
	jl_ccolor_t* cc;// Converted Colors [ NULL = Texturing Instead ]
	// Texturing:
	uint32_t tx;	// ID to texture.
	float a;	// Converted Alpha.
}jl_vo_t;

typedef struct {
	m_i32_t tex; // Group ID, Image ID
	m_u8_t multicolor; // Allow Multiple Colors
	m_u8_t* colors; // The Colors
	m_f32_t size; // The Size
}jl_font_t;

typedef struct{
	SDL_mutex* mutex;	// The mutex for writing/reading ctx_draw.
	float rh, rw;		// Real Height & Width
	void* ctx_main;		// The sprite's context.
	void* ctx_draw;		// Information required for drawing.
	uint32_t ctx_draw_size;	// Size of "ctx_draw"
	void* loop;		// (jlgr_sprite_loop_fnt) Loop function
	void* kill;		// (jlgr_sprite_loop_fnt) Kill function
	void* draw;		// (jlgr_sprite_draw_fnt) Draw function
	uint8_t update;		// Whether sprite should redraw or not.
	jl_pr_t pr;		// Pre-renderer / collision box.
}jl_sprite_t;

typedef void (*jlgr_sprite_draw_fnt)(jl_t* jl, uint8_t resize, void* ctx_draw);
typedef void (*jlgr_sprite_loop_fnt)(jl_t* jl, jl_sprite_t* spr);

typedef struct{
	char *opt;
	jl_fnct run;
}jl_popup_button_t;

typedef struct{
	void* single;
	void* upper;
	void* lower;
	void* resize;
}jlgr_redraw_t;

typedef struct{
	uint8_t id;		// Packet ID
	uint16_t x, y;		// X(w), Y(h)
	jl_fnct fn;		// Function
}jlgr_thread_packet_t;

typedef struct{
	uint16_t x; // X Location
	uint16_t y; // Y Location
	float r; // Rotational Value in "pi radians" 2=full circle
	float p; // Pressure 0-1
	uint8_t h; // How long held down.
	uint8_t k; // Which key [ a-z , left/right click ]
	void* data; // Parameter
}jlgr_input_t;

typedef struct{
	jl_t* jl;

	// For Programer's Use
	uint8_t fontcolor[4];
	jl_font_t font;
	jl_sprite_t mouse; // Sprite to represent mouse pointer

	uint8_t thread; // Graphical Thread ID.
	SDL_mutex* mutex; // Mutex to lock wshare structure.
	jl_comm_t* comm2draw; // thread communication variable.
	jl_comm_t* comm2main; // commition variable for thread communication.
	struct {
		SDL_mutex *usr_ctx;
	}mutexs;

	struct{
		jlgr_input_t input;
	}input;

	struct {
		//Input Information
		struct {
			void* getEvents[JL_CT_MAXX];
			uint8_t states[JL_CT_MAXX];
			uint8_t checked[JL_CT_MAXX];

			float msx, msy;
			int msxi, msyi;

			SDL_Event event;
		
			const Uint8 *keys;

			struct {
				#if JL_PLAT == JL_PLAT_PHONE
					uint8_t back;
				#elif JL_PLAT == JL_PLAT_COMPUTER
					uint8_t click_right; // Or Ctrl-Click
					uint8_t click_middle; // Or Shift-Click
				#endif
				//Multi-Platform
				uint8_t click; // Or Click Left
				uint8_t scroll_right;
				uint8_t scroll_left;
				uint8_t scroll_up;
				uint8_t scroll_down;
			}input;

			uint8_t back; //Back Key, Escape Key, Start Button
			uint8_t keyDown[255];
			uint32_t sd; //NYI: stylus delete
		
			uint8_t sc;
			uint8_t text_input[32];
			uint8_t read_cursor;

			uint8_t current_event;
		}ct;

		m_u8_t rtn;
	} main;

	struct {
		m_u8_t rtn;
		jl_fnct fn;
		jlgr_redraw_t redraw;
	} draw;

	// Window Info
	struct {
		uint32_t taskbar[5];
		uint32_t init_image_location;

		// If Matching FPS
		uint8_t on_time;
		uint8_t changed;
		
		// Each screen is a sprite.
		struct {
			jl_sprite_t up;
			jl_sprite_t dn;
		}bg;

		void* loop; // ( jlgr_fnct ) For upper or lower screen.
		m_u8_t cs; // The current screen "jlgr_which_screen_t"
	}sg;
	
	//Opengl Data
	struct {
		struct {
			uint32_t texture;
			uint32_t preblend;
			uint32_t color;
			uint32_t character;
		}prgs;
		//PRG: TEX
		struct {
			struct {
				m_i32_t position;
				m_i32_t texpos;
			} attr;
			struct {
				m_i32_t textures;
				m_i32_t multiply_alpha;
				m_i32_t translate;
				m_i32_t transform;
			} uniforms;
		} tex;
		//PRG: PRM
		struct {
			struct {
				m_i32_t position;
				m_i32_t texpos;
			} attr;
			struct {
				m_i32_t textures;
				m_i32_t translate;
				m_i32_t transform;
			} uniforms;
		} prm;
		//PRG: CHR
		struct {
			struct {
				m_i32_t position;
				m_i32_t texpos;
			} attr;
			struct {
				m_i32_t textures;
				m_i32_t multiply_alpha;
				m_i32_t new_color;
				m_i32_t translate;
				m_i32_t transform;
			} uniforms;
		} chr;
		//PRG: CLR
		struct {
			struct {
				m_i32_t position;
				m_i32_t acolor;
			} attr;
			struct {
				m_i32_t translate;
				m_i32_t transform;
			} uniforms;
		} clr;
		jl_vo_t temp_vo;
		// Default texture coordinates.
		uint32_t default_tc;
		
		jl_pr_t* cp; // Renderer currently being drawn on.
	}gl;

	struct {
		jl_sprite_t menubar;
	}menubar;

	//Graphics
	struct {
		struct {
			double timeTilVanish;
			char message[256];
		} notification;
		struct {
			char* window_name;
			char* message;
			jl_popup_button_t* btns;
		}popup;
		struct {
			jl_vo_t whole_screen;
		}vos;
		struct {
			char message[256];
			m_u16_t t;
			m_u8_t c;
		}msge;
		data_t* textbox_string;
	}gr;

	// Window Management
	struct {
		uint8_t fullscreen;

		char windowTitle[2][16];
		jl_window_t* displayWindow;
		// The full width and height of the window.
		int32_t w, h;
		// Aspect Ratio of the window
		double ar;
	}wm;

	// File Manager
	struct {
		struct cl_list *filelist; //List of all files in working dir.
		int8_t cursor;
		uint8_t cpage;
		char *dirname;
		char *selecteditem;
		uint8_t returnit;
		uint8_t drawupto;
		uint8_t inloop;
		jl_sprite_t btns[2];
		void *newfiledata;
		uint64_t newfilesize;
		uint8_t prompt;
		data_t* promptstring;
	}fl;

	struct {
		uint32_t font; // JL_Lib font
		uint32_t logo; // JL_Lib Loading Logo
		uint32_t game; // Game Graphics
		uint32_t icon; // Icons
	} textures;

	double timer;
	double psec;
}jlgr_t;

typedef void(*jlgr_fnct)(jlgr_t* jlgr);
typedef void(*jlgr_input_fnct)(jlgr_t* jlgr, jlgr_input_t input);

// JLGR.c:
jlgr_t* jlgr_init(jl_t* jl, u8_t fullscreen, jl_fnct fn_);
void jlgr_loop_set(jlgr_t* jlgr, jl_fnct onescreen, jl_fnct upscreen,
	jl_fnct downscreen, jl_fnct resize);
void jlgr_loop(jlgr_t* jlgr);
void jlgr_kill(jlgr_t* jlgr);

// JLGRsprite.c
void jlgr_sprite_dont(jl_t* jl, jl_sprite_t* sprite);
void jlgr_sprite_redraw(jlgr_t* jlgr, jl_sprite_t *spr, void* ctx);
void jlgr_sprite_resize(jlgr_t* jlgr, jl_sprite_t *spr, jl_rect_t* rc);
void jlgr_sprite_loop(jlgr_t* jlgr, jl_sprite_t *spr);
void jlgr_sprite_draw(jlgr_t* jlgr, jl_sprite_t *spr);
void jlgr_sprite_init(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rc,
	jlgr_sprite_loop_fnt loopfn, jlgr_sprite_draw_fnt drawfn,
	void* main_ctx, uint32_t main_ctx_size,
	void* draw_ctx, uint32_t draw_ctx_size);
void jlgr_sprite_free(jlgr_t* jlgr, jl_sprite_t* sprite);
u8_t jlgr_sprite_collide(jlgr_t* jlgr,
	jl_sprite_t *sprite1, jl_sprite_t *sprite2);
void* jlgr_sprite_getcontext(jl_sprite_t *sprite);
void* jlgr_sprite_getdrawctx(jl_sprite_t *sprite);

// JLGRmenu.c
void jlgr_menu_toggle(jlgr_t* jlgr);
void jlgr_menu_draw_icon(jlgr_t* jlgr, uint32_t tex, u8_t c);
void jlgr_menu_addicon(jlgr_t* jlgr, jlgr_input_fnct inputfn, jlgr_fnct rdr);
void jlgr_menu_addicon_flip(jlgr_t* jlgr);
void jlgr_menu_addicon_slow(jlgr_t* jlgr);
void jlgr_menu_addicon_name(jlgr_t* jlgr);

// JLGRgraphics.c:
void jlgr_dont(jlgr_t* jlgr);
void jlgr_fill_image_set(jlgr_t* jlgr, uint32_t tex, uint8_t c, uint8_t a);
void jlgr_fill_image_draw(jlgr_t* jlgr);
void jlgr_draw_bg(jlgr_t* jlgr, uint32_t tex, u8_t c);
jl_ccolor_t* jlgr_convert_color(jlgr_t* jlgr, uint8_t *rgba, uint32_t vc,
	uint8_t gradient);
void jlgr_vo_color(jlgr_t* jlgr, jl_vo_t* pv, jl_ccolor_t* cc);
void jlgr_draw_vo(jlgr_t* jlgr, jl_vo_t* pv, jl_vec3_t* vec);
void jlgr_vos_vec(jlgr_t* jlgr, jl_vo_t *pv, uint16_t tricount,
	float* triangles, uint8_t* colors, uint8_t multicolor);
void jlgr_vos_rec(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc, u8_t* colors,
	uint8_t multicolor);
void jlgr_vos_image(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc,
	uint32_t tex, uint8_t c, uint8_t a);
void jlgr_vos_texture(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc,
	jl_tex_t* tex, u8_t c, u8_t a);
void jlgr_vo_old(jlgr_t* jlgr, jl_vo_t* pv);
void jlgr_draw_text(jlgr_t* jlgr, str_t str, jl_vec3_t xyz, jl_font_t f);
void jlgr_draw_int(jlgr_t* jlgr, i64_t num, jl_vec3_t loc, jl_font_t f);
void jlgr_draw_float(jlgr_t* jlgr, f64_t num, u8_t dec, jl_vec3_t loc,
	jl_font_t f);
void jlgr_draw_text_area(jlgr_t* jlgr, jl_sprite_t * spr, str_t txt);
void jlgr_draw_text_sprite(jlgr_t* jlgr,jl_sprite_t * spr, str_t txt);
void jlgr_draw_ctxt(jlgr_t* jlgr, char *str, float yy, uint8_t* color);
void jlgr_draw_loadscreen(jlgr_t* jlgr, jl_fnct draw_routine);
void jlgr_draw_msge(jlgr_t* jlgr, uint32_t tex, u8_t c, m_str_t format, ...);
void jlgr_term_msge(jlgr_t* jlgr, char* message);
void jlgr_slidebtn_rsz(jlgr_t* jlgr, jl_sprite_t * spr, str_t txt);
void jlgr_slidebtn_loop(jlgr_t* jlgr, jl_sprite_t * spr, float defaultx,
	float slidex, jlgr_input_fnct prun);
void jlgr_glow_button_draw(jlgr_t* jlgr, jl_sprite_t * spr,
	char *txt, jlgr_input_fnct prun);
uint8_t jlgr_draw_textbox(jlgr_t* jlgr, float x, float y, float w,
	float h, data_t* string);
void jlgr_gui_slider(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rectangle,
	u8_t isdouble, m_f32_t* x1, m_f32_t* x2);
void jlgr_notify(jlgr_t* jlgr, str_t notification);

// OpenGL
void jl_gl_pbo_new(jlgr_t* jlgr, jl_tex_t* texture, u8_t* pixels,
	u16_t w, u16_t h, u8_t bpp);
void jl_gl_pbo_set(jlgr_t* jlgr, jl_tex_t* texture, u8_t* pixels,
	u16_t w, u16_t h, u8_t bpp);
void jl_gl_vo_init(jlgr_t* jlgr, jl_vo_t* vo);
void jl_gl_vo_txmap(jlgr_t* jlgr, jl_vo_t* pv, u8_t map);
uint32_t jl_gl_maketexture(jlgr_t* jlgr, void* pixels, int width, int height,
	u8_t bytepp);
double jl_gl_ar(jlgr_t* jlgr);
void jl_gl_clear(jlgr_t* jlgr, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void jl_gl_pr_rsz(jlgr_t* jlgr, jl_pr_t *pr, f32_t w, f32_t h, u16_t w_px);
void jl_gl_pr_new(jlgr_t* jlgr, jl_pr_t * pr, float w, float h, uint16_t w_px);
void jl_gl_pr_draw(jlgr_t* jlgr, jl_pr_t* pr, jl_vec3_t* vec, jl_vec3_t* scl);
void jl_gl_pr(jlgr_t* jlgr, jl_pr_t * pr, jl_fnct par__redraw);

// video
void jl_vi_make_jpeg(jl_t* jl, data_t* rtn, uint8_t quality, uint8_t* pxdata,
	uint16_t w, uint16_t h);
m_u8_t* jlgr_load_image(jl_t* jl, data_t* data, m_u16_t* w, m_u16_t* h);

// SG
void jl_sg_kill(jl_t* jl);
uint32_t jl_sg_add_image(jlgr_t* jlgr, data_t* zipdata, const char* filename);

// JLGRinput.c
void jlgr_input_do(jlgr_t *jlgr, uint8_t event, jlgr_input_fnct fn, void* data);
void jlgr_input_dont(jlgr_t* jlgr, jlgr_input_t input);
void jl_ct_quickloop_(jlgr_t* jlgr);
float jl_ct_gmousex(jlgr_t* jlgr);
float jl_ct_gmousey(jlgr_t* jlgr);
uint8_t jl_ct_typing_get(jlgr_t* pusr);
void jl_ct_typing_disable(void);

// JLGRfiles.c
uint8_t jlgr_openfile_init(jlgr_t* jlgr, str_t program_name, void *newfiledata,
	uint64_t newfilesize);
void jlgr_openfile_loop(jlgr_t* jlgr);
str_t jlgr_openfile_kill(jlgr_t* jlgr);

// Window Management
void jlgr_wm_setfullscreen(jlgr_t* jlgr, uint8_t is);
void jlgr_wm_togglefullscreen(jlgr_t* jlgr);
uint16_t jlgr_wm_getw(jlgr_t* jlgr);
uint16_t jlgr_wm_geth(jlgr_t* jlgr);
void jlgr_wm_setwindowname(jlgr_t* jlgr, str_t window_name);

#endif
