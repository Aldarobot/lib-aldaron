/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la.h"
#include "SDL_filesystem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

// LIBZIP
#define ZIP_DISABLE_DEPRECATED //Don't allow the old functions.
#include "zip.h"

#include "la_file.h"
#include "la_memory.h"
#include "la_buffer.h"

#define PKFMAX 10000000
#define LA_FILE_PERMISSIONS ( S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH )
#if defined(LA_WINDOWS)
	#define LA_FILE_ROOT "C:\\Program Files\\aldaron"
#elif defined(LA_ANDROID)
	extern const char* LA_FILE_ROOT;
#endif

static char la_file_string[256];

// This function converts linux filenames to native filnames
static char* la_file_convert__(const char* filename) {
	char* newfilename = la_memory_makecopy(filename, strlen(filename) + 1);
#if defined(LA_WINDOWS)
	for(int i = 0; i < strlen(filename); i++) {
		if(newfilename[i] == '/')
			newfilename[i] = '\\';
	}
#endif
	return newfilename;
}

static char* la_file_get_root__(void) {
	la_buffer_t root_path;

	la_buffer_init(&root_path);
#if defined(LA_ANDROID) || defined(LA_WINDOWS)
	la_buffer_format(&root_path, "%s", LA_FILE_ROOT);
#else // UNIX
	la_buffer_format(&root_path, "%s/.aldaron", getenv("HOME"));
#endif
	const char* error = NULL;
	if((error = la_file_mkdir((char*) root_path.data))) {
		la_print((char*) root_path.data);
		la_panic("mkdir: %s", error);
	}
	return la_buffer_tostring(&root_path);
}

static char* la_file_log__(void) {
	char* root = la_file_get_root__();
	la_buffer_t buffer;

	la_buffer_init(&buffer);
	la_buffer_format(&buffer, "%s/log.txt", root);

	la_memory_free(root);
	return la_buffer_tostring(&buffer);
}

static void la_file_pk_compress_iterate__(void* context, void* data) {
	char* name = context;
	la_buffer_t read; la_file_load(&read, data);

	la_file_savezip(name, data + strlen(name) - 4, read.data, read.size);
	la_print("\t%s", data);
	la_buffer_free(&read);
}

static int8_t la_file_ls__(const char* filename,uint8_t recursive,
	struct cl_list * filelist)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (filename)) != NULL) {
		// print all the files and directories within directory
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(filename) +
				strlen(ent->d_name) + 3);
			memcpy(element, filename, strlen(filename));
			element[strlen(filename)] = '/';
			memcpy(element + strlen(filename) + 1, ent->d_name,
				strlen(ent->d_name));
			element[strlen(ent->d_name) + strlen(filename) + 1]
				= '\0';
			if(ent->d_name[0] == '.') continue;
			// Open And Read if directory
			if(la_file_ls__(element, recursive, filelist)||
				!recursive)
			{
				// If wasn't directory nor recursive add file.
				cl_list_add(filelist, element);
			}
		}
		closedir(dir);
		return 0;
	}
	//Couldn't open Directory
	int errsv = errno;
	if(errsv == ENOTDIR) { // Is a File
	}else if(errsv == ENOENT) { // Doesn't Exist
	}else if(errsv == EACCES) { // No Permission
	}else if((errsv == EMFILE) || (errsv == ENFILE) || (errsv == ENOMEM)) {
		la_panic("Can't Open Directory: Not Enough Memory!\n");
	}else{ //Unknown error
		la_panic("Can't Open Directory: Unknown Error!\n");
	}
	return -1;
}

const char* la_file_basename(char* base, const char* filename) {
	int i;

	for(i = strlen(filename) - 1; i > 0; i--) {
		if(filename[i] == '/') break;
	}
	la_memory_stringcopy(filename, base, i);
	return NULL;
}

const char* la_file_append(const char* filename,const void* data,uint32_t size){
	int fd;
	const char* error = NULL;
	char* fname = filename ?
		la_memory_makecopy(filename, strlen(filename) + 1) :
		la_file_log__();
	char directory[strlen(filename)];

	// If parent directory doesn't exist, make it.
	la_file_basename(directory, filename);
	if((error = la_file_mkdir(directory))) return error;
	// Open, and write out.
	if((fd = open(filename, O_RDWR | O_CREAT, LA_FILE_PERMISSIONS)) <= 0) {
		la_memory_free(fname);
		return la_error("Fail open \"%s\": \"%s\"", filename,
			strerror(errno));
	}
	lseek(fd, 0, SEEK_END);
	if(write(fd, data, size) <= 0) {
		close(fd);
		la_memory_free(fname);
		return la_error("Fail write \"%s\": \"%s\"", filename,
			strerror(errno));
	}
	close(fd);
	la_memory_free(fname);
	return NULL;
}

const char* la_file_truncate(const char* filename) {
	char* fname = filename ?
		la_memory_makecopy(filename, strlen(filename) + 1) :
		la_file_log__();
	if(truncate(fname, 0)) {
		la_memory_free(fname);
		return la_error("Couldn't Truncate");
	}
	la_memory_free(fname);
	return NULL;
}

uint8_t la_file_exist(const char* path) {
	DIR *dir;
	uint8_t rtn = FILE_TYPE_DIR;

	if ((dir = opendir (path)) == NULL) {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) //Not a directory - is a file
			rtn = FILE_TYPE_FILE;
		else if(errsv == ENOENT) // Directory Doesn't Exist
			rtn = FILE_TYPE_NONE;
		else if(errsv == EACCES) // Doesn't have permission
			rtn = FILE_TYPE_UNKNOWN;
		else if((errsv == EMFILE)||(errsv == ENFILE)||(errsv == ENOMEM))
			la_panic("file_exist: Out of Memory!");
		else //Unknown error
			la_panic("file_exist: Unknown Error!");
	}
	closedir(dir);
	return rtn; // Directory Does exist
}

void la_file_rm(const char* filename) {
	char* converted_filename = la_file_convert__(filename);

	unlink(converted_filename);
	la_memory_free(converted_filename);
}

const char* la_file_load(la_buffer_t* load, const char* file_name) {
	char* converted_filename = la_file_convert__(file_name);
	int fd = open(converted_filename, O_RDWR);
	int size = lseek(fd, 0, SEEK_END);
	unsigned char *file = malloc(size);
	int32_t bytes;
	
	if(fd <= 0) {
		int errsv = errno;

		la_print("\tFailed to load file: \"%s\"", converted_filename);
		la_print("\tBecause: %s", strerror(errsv));
		return la_error("file load failed");
	}
	lseek(fd, 0, SEEK_SET);
	if((bytes = read(fd, file, size)))
		la_buffer_fromdata(load, file, bytes);
	close(fd);
	la_memory_free(converted_filename);
	return NULL;
}

char la_file_savezip(const char* package_name, const char* file_name,
	void *data, uint64_t data_size)
{
	char* converted = la_file_convert__(package_name);
	struct zip *archive = zip_open(converted,ZIP_CREATE|ZIP_CHECKCONS,NULL);
	struct zip_source *s;

	if(archive == NULL)
		return 1;
	la_memory_free(converted);
	if ((s=zip_source_buffer(archive, (void *)data, data_size, 0)) == NULL){
		zip_source_free(s);
		la_panic("src null error[replace]: %s",
			(char *)zip_strerror(archive));
	}
	zip_file_add(archive, file_name, s, ZIP_FL_OVERWRITE);
	zip_close(archive);
	return 0;
}

char* la_file_compress(const char* folder) {
	// If last character is '/', then overwrite.
	uint32_t cursor = strlen(folder);
	if(folder[cursor - 1] == '/') cursor--;
	// Name+.zip\0
	char* pkName = la_memory_allocate(cursor + 5);
	la_memory_copy(folder, pkName, cursor);
	pkName[cursor] = '.';
	cursor++;
	pkName[cursor] = 'z';
	cursor++;
	pkName[cursor] = 'i';
	cursor++;
	pkName[cursor] = 'p';
	cursor++;
	pkName[cursor] = '\0';
	// Overwrite any existing package with same name
	la_file_rm(pkName);
	// Find Filse
	struct cl_list * filelist = la_file_ls(folder, 1);
	// Save Files Into Package
	la_list_iterate(pkName, filelist, la_file_pk_compress_iterate__);
	// Free 
	cl_list_destroy(filelist);
	return pkName;
}

const char* la_file_loadzip(la_buffer_t* rtn, la_buffer_t* data,
	const char* file_name)
{
	zip_error_t ze; ze.zip_err = ZIP_ER_OK;
	zip_source_t *file_data;
	int zerror = 0;
	int Read;

	file_data = zip_source_buffer_create(data->data, data->size, 0, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		la_print("couldn't make pckg buffer!");
		la_panic("because: \"%s\"", zip_error_strerror(&ze));
	}

	la_print("error check 2.");
	struct zip *zipfile = zip_open_from_source(file_data,
		ZIP_CHECKCONS | ZIP_RDONLY, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		zip_error_init_with_code(&ze, ze.zip_err);
		la_print("couldn't load pckg file");
		la_panic("because: \"%s\"", zip_error_strerror(&ze));
	}

	la_print("error check 3.");
	if(zipfile == NULL) {
		la_print("couldn't load zip because:");
		if(zerror == ZIP_ER_INCONS)
			la_panic("\tcorrupt file");
		else if(zerror == ZIP_ER_NOZIP)
			la_panic("\tnot a zip file");
		else
			la_panic("\tunknown error");
	}
	la_print("error check 4.");
	la_print((char *)zip_strerror(zipfile));
	la_print("loaded package.");
	unsigned char *fileToLoad = malloc(PKFMAX);
	la_print("opening file in package....");
	struct zip_file *file = zip_fopen(zipfile, file_name, ZIP_FL_UNCHANGED);
	la_print("call pass.");
	if(file == NULL) {
		la_print("couldn't open up file: \"%s\" in package:",
			file_name);
		la_print("because: %s", (void *)zip_strerror(zipfile));
		la_print(zip_get_name(zipfile, 0, ZIP_FL_UNCHANGED));
		return la_error("Generic Error");
	}
	la_print("opened file in package / reading opened file....");
	if((Read = zip_fread(file, fileToLoad, PKFMAX)) == -1)
		la_panic("file reading failed");
	if(Read == 0) {
		la_print("empty file, returning NULL.");
		return NULL;
	}
	la_print("la_file_loadzip: read %d bytes", Read);
	zip_close(zipfile);
	la_print("closed file.");
	// Make a la_buffer_t* from the data.
	if(Read) la_buffer_fromdata(rtn, fileToLoad, Read);
	la_print("done.");
	return NULL;
}

const char* la_file_mkdir(const char* path) {
	if(path == NULL) return NULL;
	if(path[0] == 0) return NULL;
	if(mkdir(path, LA_FILE_PERMISSIONS)) {
		int errsv = errno;
		if(errsv == EEXIST) {
			return NULL;
		}else if((errsv == EACCES) || (errsv == EROFS)) {
			return strerror(errsv); // Permission error
		}else{
			la_print("Couldn't mkdir %s because:%s", path,
				strerror(errsv));
			return strerror(errsv);
		}
	}else{
		return NULL;
	}
}

la_list_t* la_file_ls(const char* dirname, uint8_t recursive) {
	struct cl_list * filelist = cl_list_create();
	char* converted_dirname = la_file_convert__(dirname);
	char* end_character = &converted_dirname[strlen(converted_dirname) - 1];
	if(*end_character == '/') *end_character = '\0';

	if(la_file_ls__(converted_dirname, recursive, filelist)) {
		la_memory_free(converted_dirname);
		cl_list_destroy(filelist);
		return NULL;
	}else{
		la_memory_free(converted_dirname);
		return filelist;
	}
}

const char* la_file_resloc(const char* prg_folder, const char* rname) {
	char* root = la_file_get_root__();
	char* pdir = NULL;
	la_buffer_t buffer;

	la_print("ROOT: %s", root);
	la_buffer_init(&buffer);
	la_buffer_format(&buffer, "%s/%s", root, prg_folder);
	la_print("BUFFER: %s", (char*) buffer.data);

	la_memory_free(root);
	pdir = la_buffer_tostring(&buffer);

	// Make 'prg_folder' if it doesn't already exist.
	const char* error = NULL;
	if((error = la_file_mkdir(pdir))) {
		la_print("la_file_resloc: couldn't make \"%s\"", pdir);
		la_panic("mkdir: %s", error);
	}

	la_print("rname = %s", rname);

	// Format & Free
	sprintf(la_file_string, "%s/%s", pdir, rname);
	la_memory_free(pdir);

	la_print("fname = %s", la_file_string);

	return la_file_string;
}
