#include "la.h"

#ifdef LA_PHONE_ANDROID
#include "la_audio.h"
#include "la_memory.h"

#define JLAU_CHANNEL_MUSIC -2
#define JLAU_CHANNEL_SOUND -3
#define JLAU_CHANNEL_LOCK -4

/** @cond **/
static inline int32_t jlau_sec2ms__(float sec) {
	int32_t ms = ((int32_t)(sec * 1000.f));
	if(ms < 20) ms = 0;
	return ms;
}

/**
 * Load audio from data pointed to by "data" of length "dataSize" into "audio",
 * set volume of audio to "volumeChange"
 */
static inline void jlau_load(jlau_t* jlau, jlau_audio_t* audio,
	uint8_t volumeChange, const void *data, uint32_t dataSize,
	uint8_t isMusic)
{
}
/** @endcond **/

/** 
 * Fade in audio.
 * @param jlau: The jlau library context.
 * @param audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
*/
void jlau_play(jlau_t* jlau, jlau_audio_t* audio, float in, jl_vec3_t* vec) {
	int32_t ms = jlau_sec2ms__(in);

	if(audio->channel == JLAU_CHANNEL_MUSIC) {

	}else{

	}
}

void jlau_lock(jlau_t* jlau, jlau_audio_t* audio, float in, jl_vec3_t* vec) {
	if(audio->channel != JLAU_CHANNEL_LOCK) jlau_play(jlau, audio, in, vec);
	audio->channel = JLAU_CHANNEL_LOCK;
}

void jlau_pause(jlau_t* jlau, jlau_audio_t* audio) {
}

void jlau_resume(jlau_t* jlau, jlau_audio_t* audio) {
}

/**
 * Wait on an audio track to stop.  If it is stopped, fade in a new one.
 * @param jlau: The jlau library context.
 * @param w_audio: The audio to wait on, NULL to wait on music to finish.
 * @param n_audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
 * @returns 1: Audio has finished, and new audio is playing
 * @returns 0: Audio is still playing.
**/
uint8_t jlau_wait(jlau_t* jlau, jlau_audio_t* w_audio, jlau_audio_t* n_audio,
	float in, jl_vec3_t* vec)
{
	// If audio is still playing, quit
	//
	jlau_play(jlau, n_audio, in, vec);
	return 1;
}

/**
 * Fade out a currently playing audio (if there is any) for "out" seconds
 * @param audio: Audio to fade out, or NULL to fade out all audio.
 * @param out: How many seconds to fade audio.
*/
void jlau_stop(jlau_audio_t* audio, float out) {
}

/**
 * Load an audiotrack from a zipfile.
 * info: info is set to number of audiotracks loaded.
 * @param jlau: The library context
 * @param audio: Unintialized audio object.
 * @param zipdata: data for a zip file.
 * @param filename: Name of the audio file in the package.
 * @param music: Whether the audio is music or not.
*/
void jlau_add_audio(jlau_t* jlau, jlau_audio_t* audio, data_t* zipdata,
	const char* filename, uint8_t music)
{
	data_t aud;

	jl_file_pk_load_fdata(jlau->jl, &aud, zipdata, filename);
	la_print("Loading audiostuffs....");
	jlau_load(jlau, audio, 255, aud.data, aud.size, music);
	la_print("Loaded audiostuffs!");
}

jlau_t* jlau_init(jl_t* jl) {
	jlau_t* jlau = la_memory_allocate(sizeof(jlau_t));

	jlau->jl = jl;
	jl->jlau = jlau;

	jlau->num_channels = 0;
	return jlau;
}

void jlau_kill(jlau_t* jlau) {
}
#endif
