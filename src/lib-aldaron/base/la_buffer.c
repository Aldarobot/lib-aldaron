/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la.h"
#include "la_memory.h"
#include "la_buffer.h"

//
// Internal Functions
//

static void jl_data_truncate_curs__(data_t* pstr) {
	if(pstr->curs > pstr->size) {
		pstr->curs = pstr->size - 1;
	}
}

static void jl_data_increment(data_t* pstr, uint8_t incrementation) {
	pstr->curs += incrementation;
	jl_data_truncate_curs__(pstr);
}

//
// Exported Functions
//

/**
 * Clears an already existing string and resets it's cursor value.
 * @param pa: string to clear.
*/
/*void jl_data_clear(jl_t* jl, data_t* pa) {
	pa->curs = 0;
//	jl_data_resize(jl, pa, 0);
	la_memory_clear(pa->data, pa->size + 1);
}*/

/**
 * Allocates a "strt" of size "size" and returns it.
 * @param jl: The library context.
 * @param a: The jl_data_t to initialize.
 * @param size: How many bytes/characters to allocate.
 * @returns: A new initialized "strt".
*/
/*void jl_data_init(jl_t* jl, data_t* a, uint32_t size) {
	a->data = la_memory_allocate(size+1);
	a->size = size;
	a->curs = 0;
}*/

void la_buffer_init(la_buffer_t* buffer) {
	buffer->curs = 0;
	buffer->size = 8;
	buffer->data = la_memory_allocate(8);
}

/**
 * frees a "strt".
 * @param pstr: the "strt" to free
*/
void jl_data_free(data_t* pstr) {
	free(pstr->data);
}

void la_buffer_fdata(la_buffer_t* a, const void *data, uint32_t size) {
	a->curs = 0;
	a->size = size;
	a->data = la_memory_makecopy(data, size);
}

/**
 * Converts "string" into a data_t* and returns it.
 * @param string: String to convert
 * @returns: new "strt" with same contents as "string".
*/
void jl_data_mkfrom_str(data_t* a, const char* string) {
	return la_buffer_fdata(a, string, strlen(string));
}

void la_buffer_format(la_buffer_t* buffer, const char* format, ...) {
	va_list arglist;

	va_start(arglist, format);
	buffer->curs = vsnprintf(NULL, 0, format, arglist);
	va_end(arglist);
	la_buffer_resize(buffer);
	va_start(arglist, format);
	vsnprintf((void*)buffer->data, buffer->size, format, arglist);
	va_end(arglist);
}

uint8_t la_buffer_byte(la_buffer_t* buffer) {
	// Avoid memory errors.
	if(buffer->curs >= buffer->size)
		buffer->curs = buffer->size - 1;
	// Return byte at cursor.
	return buffer->data[buffer->curs];
}

/**
 * Get the byte at the cursor of "strt", and increment the cursor value
**/
uint8_t jl_data_get_byte(data_t* pstr) {
	uint8_t* area = ((void*)pstr->data) + pstr->curs;
	jl_data_increment(pstr, 1);
	return *area;
}

void la_buffer_read(void* var, uint32_t varsize, la_buffer_t* buffer) {
	void* area = buffer->data + buffer->curs;

	la_memory_copy(area, var, varsize);
	buffer->curs += varsize;
}

/**
 * Add variable data at the cursor of "pstr", and increment the cursor value.
 * @param pstr: the string to read.
 * @param: pval: the integer to add to "pstr"
*/
void la_buffer_add(la_buffer_t* buffer, const void* var, uint32_t varsize) {
	void* dest = ((void*)buffer->data) + buffer->curs;

	buffer->curs = buffer->curs + varsize;
	la_buffer_resize(buffer);
	la_memory_copy(var, dest, varsize);
}

/**
 * Add a byte ( "pvalue" ) at the cursor in a string ( "pstr" ), then increment
 * the cursor value [ truncated to the string size ]
 * @param pstr: The string to add a byte to.
 * @param pvalue: the byte to add to "pstr".
*/
void jl_data_add_byte(data_t* pstr, uint8_t pvalue) {
	jl_data_truncate_curs__(pstr);
	pstr->data[pstr->curs] = pvalue;
	jl_data_increment(pstr, 1);
}

void la_buffer_resize(la_buffer_t* buffer) {
	LA_BUFFER_RESIZE:
		if(buffer->curs < buffer->size) return;

		uint64_t oldsize = buffer->size;

		buffer->size *= 2;
		buffer->data = la_memory_resize(buffer->data, buffer->size);
		la_memory_clear(buffer->data + oldsize, oldsize);
		goto LA_BUFFER_RESIZE;
}

/*void jl_data_resize(jl_t *jl, data_t* pstr, uint32_t newsize) {
	pstr->size = newsize;
	pstr->data = la_memory_resize(pstr->data, newsize);
}*/

/**
 * Delete byte at cursor in string.
*/
void la_buffer_del(la_buffer_t* buffer) {
	int i;

	for(i = buffer->curs; i < buffer->size - 1; i++)
		buffer->data[i] = buffer->data[i+1];
	buffer->data[strlen((void*)buffer->data)] = '\0';
	jl_data_truncate_curs__(buffer);
}

/**
 * Inserts a byte at cursor in string pstr.  If not enough size is available,
 * the new memory will be allocated. Value 0 is treated as null byte - dont use.
*/
void la_buffer_ins(la_buffer_t* buffer, uint8_t pvalue) {
	la_print("inserting a byte...");
	la_buffer_resize(buffer);
	
	if(la_buffer_byte(buffer) == '\0') {
		// Append at end.
		buffer->data[buffer->curs] = pvalue;
		buffer->curs++;
		la_buffer_resize(buffer);
		buffer->data[buffer->curs] = '\0';
	}else{
		// Insert in the middle.
		size_t curs = buffer->curs;
		size_t string_len = strlen((void*)buffer->data);

		// Resize if not enough space
		buffer->curs = string_len + 1;
		la_buffer_resize(buffer);
		buffer->data[buffer->curs] = '\0';
		// Move data
		memmove(buffer->data + curs + 1, buffer->data + curs,
			string_len - curs);
		// Set byte
		buffer->data[curs] = pvalue;
		// Set cursor
		buffer->curs = curs + 1;
	}
	la_print("inserted a byte...");
}

/*void jl_data_insert_data(jl_t *jl, data_t* pstr, const void* data, uint32_t size) {
	// Add size
	jl_data_resize(jl, pstr, pstr->size + size);
	// Copy data.
	la_memory_copy(data, pstr->data + pstr->curs, size);
	// Increase cursor
	pstr->curs+=size;
}*/

/**
 * At the cursor in string 'a' replace 'bytes' bytes of 'b' at it's cursor.
 * jl_data_data(jl, { data="HELLO", curs=2 }, { "WORLD", curs=2 }, 2);
 *  would make 'a'
 *	"HELLO"-"LL" = "HE\0\0O"
 *	"WORLD"[2] and [3] = "RL"
 *	"HE"+"RL"+"O" = "HERLO"
 * @param jl: library context
 * @param a: string being modified
 * @param b: string being copied into 'a'
 * @param bytes: the number of bytes to copy over
 */
/*void jl_data_data(jl_t *jl, data_t* a, const data_t* b, uint64_t bytes) {
	int32_t i;
	uint32_t size = a->size;
	uint32_t sizeb = a->curs + bytes;

	if(a == NULL) la_panic("jl_data_data: NULL A STRING");
	else if(b == NULL) la_panic("jl_data_data: NULL B STRING");
	if(sizeb > size) size = sizeb;
	a->data = la_memory_resize(a->data, size + 1);
	for(i = 0; i < bytes; i++) {
		a->data[i + a->curs] = b->data[i + b->curs];
	}
	a->size = size;
	a->data[a->size] = '\0';
}*/

/**
 * Add string "b" at the end of string "a"
 * @param 'jl': library context
 * @param 'a': string being modified
 * @param 'b': string being appended onto "a"
 */
/*void jl_data_merg(jl_t *jl, data_t* a, const data_t* b) {
	a->curs = a->size;
	jl_data_data(jl, a, b, b->size);
}*/

/**
 * Truncate the string to a specific length.
 * @param 'jl': library context
 * @param 'a': string being modified
 * @param 'size': size to truncate to.
 */
/*void jl_data_trunc(jl_t *jl, data_t* a, uint32_t size) {
	a->curs = 0;
	a->size = size;
	a->data = la_memory_resize(a->data, a->size + 1);
}*/

/**
 * Get a string ( char * ) from a 'strt'.  Then, free the 'strt'.
 * @param jl: The library context.
 * @param a: the 'strt' to convert to a string ( char * )
 * @returns: a new string (char *) with the same contents as "a"
*/
char* la_buffer_tostring(la_buffer_t* a) {
	return (void*)a->data;
}

/**
 * Tests if the next thing in array script is equivalent to particle.
 * @param script: The array script.
 * @param particle: The phrase to look for.
 * @return 1: If particle is at the cursor.
 * @return 0: If particle is not at the cursor.
*/
uint8_t jl_data_test_next(data_t* script, const char* particle) {
	char * point = (void*)script->data + script->curs; //the cursor
	char * place = strstr(point, particle); //search for partical
	if(place == point) {//if partical at begining return true otherwise false
		return 1;
	}else{
		return 0;
	}
}

/**
 * Returns string "script" truncated to "psize" or to the byte "end" in
 * "script".  It is dependant on which happens first.
 * @param jl: The library context.
 * @param script: The array script.
 * @param end: byte to end at if found.
 * @param psize: maximum size of truncated "script" to return.
 * @returns: a "strt" that is a truncated array script.
*/
/*void jl_data_read_upto(jl_t* jl, data_t* compiled, data_t* script, uint8_t end,
	uint32_t psize)
{
	jl_data_init(jl, compiled, psize);
	compiled->curs = 0;
	while((la_buffer_byte(script) != end) && (la_buffer_byte(script) != 0)){
		strncat((void*)compiled->data,
			(void*)script->data + script->curs, 1);
		script->curs++;
		compiled->curs++;
	}
	jl_data_trunc(jl, compiled, compiled->curs);
}*/
