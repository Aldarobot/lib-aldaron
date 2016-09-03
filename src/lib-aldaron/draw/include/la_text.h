#define LA_TEXT_CMD "\x04"
#define LA_TEXT_CONTROL LA_TEXT_CMD "\x01"
#define LA_TEXT_IMAGE LA_TEXT_CMD "\x02"
#define LA_TEXT_XY LA_TEXT_CMD "\x10"
#define LA_TEXT_SIZE LA_TEXT_CMD "\x11"
#define LA_TEXT_ALIGN LA_TEXT_CMD "\x12"
#define LA_TEXT_WIDTH LA_TEXT_CMD "\x13"

#define LA_TEXT_MOVE LA_TEXT_XY "%c%c%c%c%c%c%c%c"

static inline int la_text_f2(float x, uint8_t i) {
	return ((char*)((void*)&x))[i];
}

#define LA_TEXT_F2(x, y) la_text_f2(x,0), la_text_f2(x,1), la_text_f2(x,2), la_text_f2(x,3), la_text_f2(y,0), la_text_f2(y,1), la_text_f2(y,2), la_text_f2(y,3)

void la_text(la_window_t* window, const char* format, ...);
