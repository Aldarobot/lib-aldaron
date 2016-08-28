typedef struct {
	SDL_SpinLock lock;
	uint8_t value;
} safe_uint8_t;

typedef struct {
	SDL_SpinLock lock;
	float value;
} safe_float_t;

void al_safe_set(void* var, void* set, size_t size);
void al_safe_get(void* var, void* set, size_t size);
void al_safe_set_float(safe_float_t* var, float value);
float al_safe_get_float(safe_float_t* var);
void safe_set_uint8(safe_uint8_t* var, uint8_t value);
uint8_t safe_get_uint8(safe_uint8_t* var);
