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
#define JL_FL_PERMISSIONS ( S_IRWXU | S_IRWXG | S_IRWXO )
#define JL_FILE_SEPARATOR "/"

#if JL_PLAT == JL_PLAT_PHONE
	extern const char* LA_FILE_ROOT;
#endif

//static void* la_file_temp = NULL;
static char la_file_string[256];

// This function converts linux filenames to native filnames
char* jl_file_convert__(const char* filename) {
	char* newfilename = la_memory_makecopy(filename, strlen(filename) + 1);
// #if // Windows:
//	int i;
// #else  // Linux:
// #endif
	return newfilename;
}

/*static int
jl_file_save_(const void *file_data, const char *file_name, uint32_t bytes) {
	int errsv;
	ssize_t n_bytes;
	int fd;
	
	if(file_name == NULL)
		la_panic("Save[file_name]: is Null");
	else if(strlen(file_name) == 0)
		la_panic("Save[strlen]: file_name is Empty String");
	else if(!file_data)
		la_panic("Save[file_data]: file_data is NULL");

	char* converted_filename = jl_file_convert__(file_name);
	fd = open(converted_filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS);
	la_memory_free(converted_filename);

	if(fd <= 0) {
		errsv = errno;

		la_print("Save/Open: ");
		la_print("\tFailed to open file: \"%s\"",
			converted_filename);
		la_panic("\tWrite failed: %s", strerror(errsv));
	}
	int at = lseek(fd, 0, SEEK_END);
	n_bytes = write(fd, file_data, bytes);
	if(n_bytes <= 0) {
		errsv = errno;
		close(fd);
		la_print(":Save[write]: Write to \"%s\" failed:");
		la_panic("\"%s\"", strerror(errsv));
	}
	close(fd);
	return at;
}*/

static char* jl_file_get_root__(void) {
	la_buffer_t root_path;

	la_buffer_init(&root_path);
#if JL_PLAT == JL_PLAT_PHONE // Android:
	la_buffer_format(&root_path, "%s", LA_FILE_ROOT);
#else // Linux:
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
	char* root = jl_file_get_root__();
	la_buffer_t buffer;

	la_buffer_init(&buffer);
	la_buffer_format(&buffer, "%s/log.txt", root);

	la_memory_free(root);
	return la_buffer_tostring(&buffer);
}

// NON-STATIC Library Dependent Functions

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
	if((fd = open(filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS)) <= 0) {
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

/**
 * Check whether a file or directory exists.
 * @returns 0: If the file doesn't exist.
 * @returns 1: If the file does exist and is a directory.
 * @returns 2: If the file does exist and isn't a directory.
 * @returns 3: If the file exists and the user doesn't have permissions to open.
**/
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
	char* converted_filename = jl_file_convert__(filename);

	unlink(converted_filename);
	la_memory_free(converted_filename);
}

/**
 * Load a File from the file system.  Returns bytes loaded from "file_name"
 * @param jl: Library Context
 * @param load: Location to store loaded data.
 * @param file_name: file to load
 * @returns A readable "strt" containing the bytes from the file.
 */
const char* la_file_load(la_buffer_t* load, const char* file_name) {
	char* converted_filename = jl_file_convert__(file_name);
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

/**
 * Save file "filename" with contents "data" of size "dataSize" to package
 * "packageFileName"
 * @param jl: Library Context
 * @param packageFileName: Name of package to Save to
 * @param fileName: the file to Save to within the package.
 * @param data: the data to save to the file
 * @param dataSize: the # of bytes to save from the data to the file.
 * @returns 0: On success
 * @returns 1: If File is unable to be made.
 */
/*char jl_file_pk_save(jl_t* jl, const char* packageFileName,
	const char* fileName, void *data, uint64_t dataSize)
{
	char* converted = jl_file_convert__(packageFileName);

	struct zip *archive = zip_open(converted, ZIP_CREATE 
		| ZIP_CHECKCONS, NULL);
	if(archive == NULL)
		return 1;
	la_memory_free(converted);

	struct zip_source *s;
	if ((s=zip_source_buffer(archive, (void *)data, dataSize, 0)) == NULL) {
		zip_source_free(s);
		la_panic("src null error[replace]: %s",
			(char *)zip_strerror(archive));
	}
//	la_print("%d,%d,%d\n",archive,sb.index,s);
	zip_file_add(archive, fileName, s, ZIP_FL_OVERWRITE);
	zip_close(archive);
	return 0;
}

static void jl_file_pk_compress_iterate__(jl_t* jl, void* data) {
	char* name = la_file_temp;
	data_t read; la_file_load(&read, data);

	jl_file_pk_save(jl, name, data + strlen(name)-4, read.data, read.size);
	la_print("\t%s", data);
	jl_data_free(&read);
}*/

/**
 * Compress a folder in a package.
 * @param jl: The library context.
 * @param folderName: Name of the folder to compress.
 * @returns: Name of package, needs to be free'd.
**/
/*char* jl_file_pk_compress(jl_t* jl, const char* folderName) {
	// If last character is '/', then overwrite.
	uint32_t cursor = strlen(folderName);
	if(folderName[cursor - 1] == '/') cursor--;
	// Name+.zip\0
	char* pkName = la_memory_allocate(cursor + 5);
	la_memory_copy(folderName, pkName, cursor);
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
	//
	la_file_temp = pkName;
	// Find Filse
	struct cl_list * filelist = jl_file_dir_ls(jl, folderName, 1);
	// Save Files Into Package
	jl_clump_list_iterate(jl, filelist, jl_file_pk_compress_iterate__);
	// Free 
	cl_list_destroy(filelist);
	return pkName;
}*/

/**
 * Load a zip package from memory.
 * @param jl: The library context.
 * @param rtn: An empty data_t structure to return to.  Needs to be freed.
 * @param data: The data that contains the zip file.
 * @param file_name: The name of the file to load.
**/
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
	// Make a data_t* from the data.
	if(Read) la_buffer_fromdata(rtn, fileToLoad, Read);
	la_print("done.");
	return NULL;
}

/**
 * Create a folder (directory)
 * @param jl: library context
 * @param pfilebase: name of directory to create
 * @returns NULL: Either made directory, or it exists already.
 * @returns error: If there was an error.
*/
const char* la_file_mkdir(const char* path) {
	if(path == NULL) return NULL;
	if(path[0] == 0) return NULL;
	if(mkdir(path, JL_FL_PERMISSIONS)) {
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
/*
static int8_t jl_file_dirls__(jl_t* jl,const char* filename,uint8_t recursive,
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
			if(jl_file_dirls__(jl, element, recursive, filelist)||
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
	if(errsv == ENOTDIR) {
//		la_print("Can't Open Directory: Is a File!");
	}else if(errsv == ENOENT) {
//		la_print("Can't Open Directory: Doesn't Exist!");
	}else if(errsv == EACCES) {
//		la_print("Can't Open Directory: No Permission!");
	}else if((errsv == EMFILE) || (errsv == ENFILE) ||
		(errsv == ENOMEM)) //Not enough memory!
	{
		la_panic("Can't Open Directory: Not Enough Memory!\n");
	}else{ //Unknown error
		la_panic("Can't Open Directory: Unknown Error!\n");
	}
	return -1;
}*/

/**
 * List all of the files in a directory.
 * @param jl: The library context.
 * @param dirname: The name of the directory.
 * @param recursive: 0: list files and directories inside of "dirname", 1: list
 *	all files inside of "dirname" and inside of all other directories under
 *	dirname.
 * @returns: List of files inside of "dirname", needs to be freed with
 *	cl_list_destroy()
**/
/*struct cl_list * jl_file_dir_ls(jl_t* jl,const char* dirname,uint8_t recursive){
	struct cl_list * filelist = cl_list_create();
	char* converted_dirname = jl_file_convert__(dirname);
	char* end_character = &converted_dirname[strlen(converted_dirname) - 1];
	if(*end_character == '/') *end_character = '\0';

	if(jl_file_dirls__(jl, converted_dirname, recursive, filelist)) {
		la_memory_free(converted_dirname);
		cl_list_destroy(filelist);
		return NULL;
	}else{
		la_memory_free(converted_dirname);
		return filelist;
	}
}*/

/**
 * Get the designated location for a resource file. Resloc = Resource Location
 * @param jl: Library Context.
 * @param prg_folder: The name of the folder for all of the program's resources.
 *	For a company "Plop Grizzly" with game "Super Game":
 *		Pass: "PlopGrizzly_SG"
 *	For an individual game developer "Jeron Lau" with game "Cool Game":
 *		Pass: "JeronLau_CG"
 *	If prg_folder is NULL, uses the program name from jl_start.
 * @param rname: Name Of Resource File
 * @returns: The designated location for a resouce pack
*/
const char* la_file_resloc(const char* prg_folder, const char* rname) {
	char* root = jl_file_get_root__();
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
