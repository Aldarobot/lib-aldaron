#include "SDL_thread.h"

typedef struct{
	SDL_atomic_t wait;
}la_signal_t;

void la_signal_init(la_signal_t* wait);
void la_signal_wait(la_signal_t* wait);
void la_signal_send(la_signal_t* wait);
