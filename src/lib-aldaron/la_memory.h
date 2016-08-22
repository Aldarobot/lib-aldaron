void* la_memory_clear(void* data, size_t size);
void* la_memory_copy(const void* src, void* dst, size_t size);
void* la_memory_stringcopy(const char* src, char* dst, size_t size);
void* la_memory_makecopy(void* data, size_t size);
void* la_memory(size_t size);
void* la_memory_free(void* data);
