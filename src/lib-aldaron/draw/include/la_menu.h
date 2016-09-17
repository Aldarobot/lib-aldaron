#ifndef LA_MENU
#define LA_MENU

typedef struct {
	la_window_t* window;
	jl_mutex_t mutex;
	jl_vo_t menubar;

	// Protected ....
	// Used for all icons on the menubar.
	jl_vo_t icon;
	jl_vo_t shadow;
	// Redraw Functions for 10 icons.
	void* redrawfn[10];
	// Loop Functions for 10 icons.
	void* inputfn[10];
	// Cursor
	int8_t cursor;
	// What needs redrawing - -1 nothing -2 all
	int8_t redraw;
}la_menu_t;

typedef void(*la_menu_fn_t)(la_menu_t* menu);

void la_menu_init(la_menu_t* menu, la_window_t* window);
void la_menu_draw(la_menu_t* menu, uint8_t resize);
void la_menu_loop(la_menu_t* menu);
void la_menu_drawicon(la_menu_t* menu, uint32_t tex, uint8_t c);
void la_menu_addicon(la_menu_t* menu, la_menu_fn_t inputfn, la_menu_fn_t rdr);
void la_menu_addicon_flip(la_menu_t* menu);
void la_menu_addicon_slow(la_menu_t* menu);
void la_menu_addicon_name(la_menu_t* menu);

#endif
