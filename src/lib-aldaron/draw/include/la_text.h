#include "la_port.h"

#define LA_TEXT_CMD "\x04"

// Special Character Sets
#define LA_TEXT_CONTROL LA_TEXT_CMD "\x01"
#define LA_TEXT_IMAGE LA_TEXT_CMD "\x02"

// Font Styles.
#define LA_TEXT_XY LA_TEXT_CMD "\x10"
#define LA_TEXT_WH LA_TEXT_CMD "\x11"
#define LA_TEXT_ALIGN LA_TEXT_CMD "\x12"
#define LA_TEXT_WIDTH LA_TEXT_CMD "\x13"
#define LA_TEXT_COLOUR LA_TEXT_CMD "\x14"
#define LA_TEXT_UNDERLAY LA_TEXT_CMD "\x15"
#define LA_TEXT_SHADOWOFF LA_TEXT_CMD "\x16"

#define LA_TEXT_FX "%c%c%c%c"
#define LA_TEXT_MOVE LA_TEXT_XY LA_TEXT_FX LA_TEXT_FX
#define LA_TEXT_COLOR LA_TEXT_COLOUR LA_TEXT_FX LA_TEXT_FX LA_TEXT_FX LA_TEXT_FX
#define LA_TEXT_SHADOW LA_TEXT_UNDERLAY LA_TEXT_FX LA_TEXT_FX LA_TEXT_FX LA_TEXT_FX
#define LA_TEXT_SIZE LA_TEXT_WH LA_TEXT_FX

static inline int la_text_f2(float x, uint8_t i) {
	return ((char*)((void*)&x))[i];
}

#define LA_TEXT_F1(x) la_text_f2(x,0), la_text_f2(x,1), la_text_f2(x,2), la_text_f2(x,3)

#define LA_TEXT_F2(x, y) la_text_f2(x,0), la_text_f2(x,1), la_text_f2(x,2), la_text_f2(x,3), la_text_f2(y,0), la_text_f2(y,1), la_text_f2(y,2), la_text_f2(y,3)

#define LA_TEXT_F3(x, y, z) la_text_f2(x,0), la_text_f2(x,1), la_text_f2(x,2), la_text_f2(x,3), la_text_f2(y,0), la_text_f2(y,1), la_text_f2(y,2), la_text_f2(y,3), la_text_f2(z,0), la_text_f2(z,1), la_text_f2(z,2), la_text_f2(z,3)

#define LA_TEXT_F4(x, y, z, w) la_text_f2(x,0), la_text_f2(x,1), la_text_f2(x,2), la_text_f2(x,3), la_text_f2(y,0), la_text_f2(y,1), la_text_f2(y,2), la_text_f2(y,3), la_text_f2(z,0), la_text_f2(z,1), la_text_f2(z,2), la_text_f2(z,3), la_text_f2(w,0), la_text_f2(w,1), la_text_f2(w,2), la_text_f2(w,3)

void la_text(la_window_t* window, const char* format, ...);
