#include <car.h>
#include "jl.h"
#include "jlgr.h"

enum {
	MODE_EDIT,
	MODE_MAXX,
} mode;

typedef struct{
	jl_vo_t vo1;
	jl_vo_t vo2;
	jl_sprite_t slider[3];
	float s1[3];
	float s2[3];
	uint8_t hasMenu;
	uint32_t lightTex;
	car_camera_t camera;
	void* video_stream;
	uint32_t video_stream_texture;
	uint8_t pixels[640*480*3];
}ctx_t;
