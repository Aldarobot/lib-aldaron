#include <stdint.h>
#include "la_buffer.h"

enum {
	FILE_TYPE_NONE,
	FILE_TYPE_DIR,
	FILE_TYPE_FILE,
	FILE_TYPE_UNKNOWN,
} la_file_type_t;

const char* la_file_basename(char* base, const char* filename);
const char* la_file_append(const char* filename, const void* data, size_t size);
const char* la_file_truncate(const char* filename);
uint8_t la_file_exist(const char* path);
const char* la_file_load(la_buffer_t* load, const char* file_name);
const char* la_file_mkdir(const char* path);
const char* la_file_resloc(const char* prg_folder, const char* rname);
