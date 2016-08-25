#include "la_signal.h"

void la_signal_init(la_signal_t* wait) {
	SDL_AtomicSet(&wait->wait, 1);
}

void la_signal_wait(la_signal_t* wait) {
	while(SDL_AtomicGet(&wait->wait));
}

void la_signal_send(la_signal_t* wait) {
	SDL_AtomicSet(&wait->wait, 0);
}
