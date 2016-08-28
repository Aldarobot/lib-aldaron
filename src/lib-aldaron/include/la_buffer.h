typedef data_t la_buffer_t;

void la_buffer_init(la_buffer_t* buffer);
uint8_t la_buffer_byte(la_buffer_t* buffer);
void la_buffer_resize(la_buffer_t* buffer);

#define jl_data_byte la_buffer_byte
