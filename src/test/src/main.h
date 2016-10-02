#include <la_menu.h>
#include <la_mode.h>

typedef struct{
	la_window_t* window;
	la_mode_t mode;
	la_ro_t vo1;
	la_ro_t vo2;
	float s1[3];
	float s2[3];
	uint8_t hasMenu;
	uint32_t lightTex;
	la_menu_t menu;
}ctx_t;
