#include <stdint.h>

const char* la_file_basename(char* base, const char* filename);
const char* la_file_append(const char* filename, const void* data, size_t size);
const char* la_file_mkdir(const char* path);
