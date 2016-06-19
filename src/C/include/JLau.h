#ifndef JLAU
#define JLAU

#include "jl.h"
#include "SDL_mixer.h"

// Types:
typedef struct {
	Mix_Music *_MUS;
	char _VOL;
}jlau_music_t;

typedef struct{
	jl_t* jl; // Library Context
	jlau_music_t next; // Which music to play next
	float fade; // Seconds Of Fade In
	double pofr; // Point Of Return (Where Music Should Start)
}jlau_t;

// Prototypes:
void jlau_load(jlau_t* jlau, jlau_music_t* music, uint8_t volumeChange,
	const void *data, uint32_t dataSize);
void jlau_play_music(jlau_t* jlau, jlau_music_t* music, float out, float in);
void jlau_mus_halt(float out);
uint8_t jlau_mus_playing(void);
void jlau_panning(uint8_t left, uint8_t toCenter);
void jlau_panning_default(void);
void jlau_add_audio(jlau_t* jlau, jlau_music_t* music, data_t* zipdata,
	const char* filename);
jlau_t* jlau_init(jl_t* jl);
void jlau_loop(jlau_t* jlau);
void jlau_kill(jlau_t* jlau);

#endif
