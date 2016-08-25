typedef int (*la_thread_fn_t)(void* data);

typedef struct {
	la_thread_fn_t fn; // (SDL_ThreadFunction) Pointer to the function
	void* thread; // (SDL_Thread) Pointer to the SDL thread
	int64_t id; // (SDL_threadID) Integer to define thread.
} la_thread_t;

const char* la_thread_new(la_thread_t* thread, la_thread_fn_t fn,
	const char* name, void* data);
