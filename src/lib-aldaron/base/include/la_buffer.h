#ifndef LA_BUFFER
#define LA_BUFFER

typedef data_t la_buffer_t;

void la_buffer_init(la_buffer_t* buffer);
void la_buffer_fdata(la_buffer_t* a, const void *data, uint32_t size);
uint8_t la_buffer_byte(la_buffer_t* buffer);
void la_buffer_resize(la_buffer_t* buffer);
void la_buffer_format(la_buffer_t* buffer, const char* format, ...);
void la_buffer_read(void* var, uint32_t varsize, la_buffer_t* buffer);
char* la_buffer_tostring(la_buffer_t* a);

#define jl_data_byte la_buffer_byte

#endif
