/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLfiles.c
 * 	This allows you to modify the file system.  It uses libzip.
 */
/** @cond **/
#include "JLprivate.h"
#include "SDL_filesystem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// LIBZIP
#define ZIP_DISABLE_DEPRECATED //Don't allow the old functions.
#include "zip.h"

#include "la_file.h"
#include "la_memory.h"

#define PKFMAX 10000000
#define JL_FL_PERMISSIONS ( S_IRWXU | S_IRWXG | S_IRWXO )

#if JL_PLAT == JL_PLAT_PHONE
	extern const char* LA_FILE_ROOT;
#endif

static void* la_file_temp = NULL;

// This function converts linux filenames to native filnames
char* jl_file_convert__(jl_t* jl, const char* filename) {
	data_t src; jl_data_mkfrom_str(&src, filename);
	data_t converted; jl_data_init(jl, &converted, 0);

	if(jl_data_test_next(&src, "!")) {
		src.curs++; // ignore
	}else{
		src.curs++; // ignore
		jl_data_merg(jl, &converted, &jl->fl.separator);
	}
	while(1) {
		data_t append; jl_data_read_upto(jl, &append, &src, '/', 300); 
		if(append.data[0] == '\0') break;
		jl_data_merg(jl, &converted, &append);
		if(jl_data_byte(&src) == '/')
			jl_data_merg(jl, &converted, &jl->fl.separator);
		src.curs++; // Skip '/'
		jl_data_free(&append);
	}
	jl_data_free(&src);
	return jl_data_tostring(jl, &converted);
}

static int jl_file_save_(jl_t* jl, const void *file_data, const char *file_name,
	uint32_t bytes)
{
	int errsv;
	ssize_t n_bytes;
	int fd;
	
	if(file_name == NULL)
		la_panic("Save[file_name]: is Null");
	else if(strlen(file_name) == 0)
		la_panic("Save[strlen]: file_name is Empty String");
	else if(!file_data)
		la_panic("Save[file_data]: file_data is NULL");

	const char* converted_filename = jl_file_convert__(jl, file_name);
	fd = open(converted_filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS);

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
}

static inline void jl_file_reset_cursor__(const char* file_name) {
	int fd = open(file_name, O_RDWR);
	lseek(fd, 0, SEEK_SET);
	close(fd);
}

static inline void jl_file_get_root__(jl_t * jl) {
	data_t root_path;

#if JL_PLAT == JL_PLAT_PHONE
	data_t root_dir;

	la_print("Get external storage directory.");
	jl_data_mkfrom_str(&root_path, LA_FILE_ROOT);
	la_print("Append JL_ROOT_DIR.");
	jl_data_mkfrom_str(&root_dir, JL_ROOT_DIR);
	la_print("Merging root_path and root_dir.");
	jl_data_merg(jl, &root_path, &root_dir);
	la_print("Free root_dir.");
	jl_data_free(&root_dir);
#elif JL_PLAT_RPI
	data_t root_dir;

	la_print("Get external storage directory.");
	jl_data_mkfrom_str(&root_path, "/home/pi/.local/share/");
	la_print("Append JL_ROOT_DIR.");
	jl_data_mkfrom_str(&root_dir, JL_ROOT_DIR);
	la_print("Merging root_path and root_dir.");
	jl_data_merg(jl, &root_path, &root_dir);
	la_print("Free root_dir.");
	jl_data_free(&root_dir);
#else
	// Get the operating systems prefered path
	char* pref_path = SDL_GetPrefPath(JL_ROOT_DIRNAME, "\0");

	if(!pref_path) {
		la_panic("This platform has no pref path!");
	}
	// Erase extra non-needed '/'s
	pref_path[strlen(pref_path) - 1] = '\0';
	// Set root path to pref path
	jl_data_mkfrom_str(&root_path, pref_path);
	// Free the pointer to pref path
	SDL_free(pref_path);
#endif
	// Make "-- JL_ROOT_DIR"
	const char* error = NULL;
	if((error = la_file_mkdir((char*) root_path.data))) {
		la_print((char*) root_path.data);
		la_panic("mkdir: %s", error);
	}
	// Set paths.root & free root_path
	jl->fl.paths.root = jl_data_tostring(jl, &root_path);
	la_print("Root Path=\"%s\"", jl->fl.paths.root);
}

static inline void jl_file_get_errf__(jl_t * jl) {
	data_t fname; jl_data_mkfrom_str(&fname, "errf.txt");
	// Add the root path
	data_t errfs; jl_data_mkfrom_str(&errfs, jl->fl.paths.root);

	// Add the file name
	jl_data_merg(jl, &errfs, &fname);
	// Free fname
	jl_data_free(&fname);
	// Set paths.errf & free errfs
	jl->fl.paths.errf = jl_data_tostring(jl, &errfs);
}

// NON-STATIC Library Dependent Functions

/** @endcond **/

const char* la_file_basename(char* base, const char* filename) {
	int i;

	for(i = strlen(filename) - 1; i > 0; i--) {
		if(filename[i] == '/') break;
	}
	la_memory_stringcopy(filename, base, i);
	return NULL;
}

const char* la_file_append(const char* filename, const void* data, size_t size){
	int fd;
	const char* error = NULL;
	char directory[strlen(filename)];

	// If parent directory doesn't exist, make it.
	la_file_basename(directory, filename);
	if((error = la_file_mkdir(directory))) return error;
	// Open, and write out.
	if((fd = open(filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS)) <= 0) {
		la_print("la_file_append fail open \"%s\" because\"%s\"",
			filename, strerror(errno));
		error = strerror(errno);
		return error;
	}
	lseek(fd, 0, SEEK_END);
	if(write(fd, data, size) <= 0) {
		la_print("la_file_append fail write \"%s\" because\"%s\"",
			filename, strerror(errno));
		error = strerror(errno);
	}
	close(fd);
	return error;
}

/**
 * Print text to a file.
 * @param jl: The library context.
 * @param fname: The name of the file to print to.
 * @param msg: The text to print.
**/
void jl_file_print(jl_t* jl, const char* fname, const char* msg) {
	// Write to the errf logfile
	if(jl->has.filesys && fname) jl_file_save_(jl, msg, fname, strlen(msg));
}

/**
 * Check whether a file or directory exists.
 * @param jl: The library context.
 * @param path: The path to the file to check.
 * @returns 0: If the file doesn't exist.
 * @returns 1: If the file does exist and is a directory.
 * @returns 2: If the file does exist and isn't a directory.
 * @returns 3: If the file exists and the user doesn't have permissions to open.
 * @returns 255: This should never happen.
**/
uint8_t jl_file_exist(jl_t* jl, const char* path) {
	DIR *dir;
	if ((dir = opendir (path)) == NULL) {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) //Not a directory - is a file
			return 2;
		else if(errsv == ENOENT) // Directory Doesn't Exist
			return 0;
		else if(errsv == EACCES) // Doesn't have permission
			return 3;
		else if((errsv == EMFILE) || (errsv == ENFILE) || (errsv == ENOMEM)) //Not enough memory!
			la_panic("jl_file_exist: Out of Memory!");
		else //Unknown error
			la_panic("jl_file_exist: Unknown Error!");
	}else{
		return 1; // Directory Does exist
	}
	return 255;
}

/**
 * Delete a file.
 * @param jl: The library context.
 * @param filename: The path of the file to delete.
**/
void jl_file_rm(jl_t* jl, const char* filename) {
	const char* converted_filename = jl_file_convert__(jl, filename);

	unlink(converted_filename);
}

/**
 * Save A File To The File System.  Save Data of "bytes" bytes in "file" to
 * file "name"
 * @param jl: Library Context
 * @param file: Data To Save To File
 * @param name: The Name Of The File to save to
 * @param bytes: Size of "File"
 */
void jl_file_save(jl_t* jl, const void *file, const char *name, uint32_t bytes){
	// delete file
	jl_file_rm(jl, name);
	// make file
	jl_file_save_(jl, file, name, bytes);
}

/**
 * Load a File from the file system.  Returns bytes loaded from "file_name"
 * @param jl: Library Context
 * @param load: Location to store loaded data.
 * @param file_name: file to load
 * @returns A readable "strt" containing the bytes from the file.
 */
void jl_file_load(jl_t* jl, data_t* load, const char* file_name) {
	jl_file_reset_cursor__(file_name);
	unsigned char *file = malloc(MAXFILELEN);
	const char* converted_filename = jl_file_convert__(jl, file_name);
	int fd = open(converted_filename, O_RDWR);
	
	//Open Block FLLD	
	if(fd <= 0) {
		int errsv = errno;

		la_print("jl_file_load/open: ");
		la_print("\tFailed to open file: \"%s\"", converted_filename);
		la_print("\tLoad failed because: %s", strerror(errsv));
		if(errsv != ENOENT)
			la_panic("jl_file_load can't load a directory.");
		jl->errf = JL_ERR_FIND;
		return;
	}
	int Read = read(fd, file, MAXFILELEN);
	jl->info = Read;

	la_print("jl_file_load(): read %d bytes", jl->info);
	close(fd);

	if(jl->info) jl_data_mkfrom_data(jl, load, jl->info, file);
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
char jl_file_pk_save(jl_t* jl, const char* packageFileName,
	const char* fileName, void *data, uint64_t dataSize)
{
	const char* converted = jl_file_convert__(jl, packageFileName);

	la_print("opening \"%s\"....", converted);
	struct zip *archive = zip_open(converted, ZIP_CREATE 
		| ZIP_CHECKCONS, NULL);
	if(archive == NULL) {
		return 1;
	}else{
		la_print("opened package, \"%d\".", converted);
	}

	struct zip_source *s;
	if ((s=zip_source_buffer(archive, (void *)data, dataSize, 0)) == NULL) {
		zip_source_free(s);
		la_panic("src null error[replace]: %s",
			(char *)zip_strerror(archive));
	}
//	la_print("%d,%d,%d\n",archive,sb.index,s);
	if(zip_file_add(archive, fileName, s, ZIP_FL_OVERWRITE)) {
		la_print("add/err: \"%s\"", zip_strerror(archive));
	}else{
		la_print("added \"%s\" to file sys.", fileName);
	}
	zip_close(archive);
	la_print("DONE!");
	return 0;
}

static void jl_file_pk_compress_iterate__(jl_t* jl, void* data) {
	char* name = la_file_temp;
	data_t read; jl_file_load(jl, &read, data);

	jl_file_pk_save(jl, name, data + strlen(name)-4, read.data, read.size);
	la_print("\t%s", data);
	jl_data_free(&read);
}

/**
 * Compress a folder in a package.
 * @param jl: The library context.
 * @param folderName: Name of the folder to compress.
 * @returns: Name of package, needs to be free'd.
**/
char* jl_file_pk_compress(jl_t* jl, const char* folderName) {
	// If last character is '/', then overwrite.
	uint32_t cursor = strlen(folderName);
	if(folderName[cursor - 1] == '/') cursor--;
	// Name+.zip\0
	char* pkName = jl_memi(jl, cursor + 5);
	jl_mem_copyto(folderName, pkName, cursor);
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
	jl_file_rm(jl, pkName);
	//
	la_file_temp = pkName;
	// Find Filse
	struct cl_list * filelist = jl_file_dir_ls(jl, folderName, 1);
	// Save Files Into Package
	jl_clump_list_iterate(jl, filelist, jl_file_pk_compress_iterate__);
	// Free 
	cl_list_destroy(filelist);
	return pkName;
}

/**
 * Load a zip package from memory.
 * @param jl: The library context.
 * @param rtn: An empty data_t structure to return to.  Needs to be freed.
 * @param data: The data that contains the zip file.
 * @param file_name: The name of the file to load.
**/
void jl_file_pk_load_fdata(jl_t* jl, data_t* rtn, data_t* data,
	const char* file_name)
{
	zip_error_t ze; ze.zip_err = ZIP_ER_OK;
	zip_source_t *file_data;
	int zerror = 0;

	file_data = zip_source_buffer_create(data->data, data->size, 0, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		la_print("couldn't make pckg buffer!");
		//zip_error_init_with_code(&ze, ze.zip_err);
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

//	struct zip *zipfile = zip_open(converted, ZIP_CHECKCONS, &zerror);
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
		jl->errf = JL_ERR_NONE;
		return;
	}
	la_print("opened file in package / reading opened file....");
	if((jl->info = zip_fread(file, fileToLoad, PKFMAX)) == -1)
		la_panic("file reading failed");
	if(jl->info == 0) {
		la_print("empty file, returning NULL.");
		return;
	}
	la_print("jl_file_pk_load: read %d bytes", jl->info);
	zip_close(zipfile);
	la_print("closed file.");
	// Make a data_t* from the data.
	if(jl->info) jl_data_mkfrom_data(jl, rtn, jl->info, fileToLoad);
	la_print("done.");
	jl->errf = JL_ERR_NERR;
}

/**
 * Load file "filename" in package "packageFileName" & Return contents
 * May return NULL.  If it does jl->errf will be set.
 * -ERR:
 *	-ERR_NERR:	File is empty.
 *	-ERR_NONE:	Can't find filename in packageFileName. [ DNE ]
 *	-ERR_FIND:	Can't find packageFileName. [ DNE ]
 * @param jl: Library Context
 * @param rtn: Data structure to return.
 * @param packageFileName: Package to load file from
 * @param filename: file within package to load
 * @returns: contents of file ( "filename" ) in package ( "packageFileName" )
*/
void jl_file_pk_load(jl_t* jl, data_t* rtn, const char *packageFileName,
	const char *filename)
{
	const char* converted = jl_file_convert__(jl, packageFileName);

	jl->errf = JL_ERR_NERR;

	la_print("loading package:\"%s\"...", converted);

	data_t data; jl_file_load(jl, &data, converted);
	la_print("error check 1.");
	if(jl->errf == JL_ERR_FIND) {
		la_print("!Package File doesn't exist!");
		return;
	}
	jl_file_pk_load_fdata(jl, rtn, &data, filename);
	if(jl->errf) la_panic("jl_file_pk_load_fdata failed!");
	return;
}

/**
 * Create a folder (directory)
 * @param jl: library context
 * @param pfilebase: name of directory to create
 * @returns NULL: Either made directory, or it exists already.
 * @returns error: If there was an error.
*/
const char* la_file_mkdir(const char* path) {
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

static int8_t jl_file_dirls__(jl_t* jl,const char* filename,uint8_t recursive,
	struct cl_list * filelist)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (filename)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(filename) +
				strlen(ent->d_name) + 3);
			element[0] = '!';
			memcpy(1 + element, filename, strlen(filename));
			element[1 + strlen(filename)] = '/';
			memcpy(1 + element + strlen(filename) + 1, ent->d_name,
				strlen(ent->d_name));
			element[1 + strlen(ent->d_name) + strlen(filename) + 1]
				= '\0';
			if(ent->d_name[0] == '.') continue;
			// Open And Read if directory
			if(jl_file_dirls__(jl, element+1, recursive, filelist)||
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
}

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
struct cl_list * jl_file_dir_ls(jl_t* jl,const char* dirname,uint8_t recursive){
	struct cl_list * filelist = cl_list_create();
	char* converted_dirname = jl_file_convert__(jl, dirname);
	char* end_character = &converted_dirname[strlen(converted_dirname) - 1];
	if(*end_character == '/') *end_character = '\0';

	if(jl_file_dirls__(jl, converted_dirname, recursive, filelist)) {
		cl_list_destroy(filelist);
		return NULL;
	}else{
		return filelist;
	}
}

/**
 * Get the designated location for a resource file. Resloc = Resource Location
 * @param jl: Library Context.
 * @param prg_folder: The name of the folder for all of the program's resources.
 *	For a company "PlopGrizzly" with game "Super Game":
 *		Pass: "PlopGrizzly_SG"
 *	For an individual game developer "Jeron Lau" with game "Cool Game":
 *		Pass: "JeronLau_CG"
 *	If prg_folder is NULL, uses the program name from jl_start.
 * @param fname: Name Of Resource Pack
 * @returns: The designated location for a resouce pack
*/
char* jl_file_get_resloc(jl_t* jl, const char* prg_folder, const char* fname) {
	data_t filesr; jl_data_mkfrom_str(&filesr, JL_FILE_SEPARATOR);
	data_t pfstrt; jl_data_mkfrom_str(&pfstrt, prg_folder);
	data_t fnstrt; jl_data_mkfrom_str(&fnstrt, fname);
	data_t resloc; jl_data_mkfrom_str(&resloc, jl->fl.paths.root);
	char * rtn = NULL;
	
	la_print("Getting Resource Location....");
	// Append 'prg_folder' onto 'resloc'
	jl_data_merg(jl, &resloc, &pfstrt);
	// Append 'filesr' onto 'resloc'
	jl_data_merg(jl, &resloc, &filesr);
	// Make 'prg_folder' if it doesn't already exist.
	const char* error = NULL;
	if( ( error = la_file_mkdir((char*) resloc.data) ) ) {
		la_print("jl_file_get_resloc: couldn't make \"%s\"",
			(char*) resloc.data);
		la_panic("mkdir: %s", error);
	}
	// Append 'fname' onto 'resloc'
	jl_data_merg(jl, &resloc, &fnstrt);
	// Set 'rtn' to 'resloc' and free 'resloc'
	rtn = jl_data_tostring(jl, &resloc);
	// Free pfstrt & fnstrt & filesr
	jl_data_free(&pfstrt),jl_data_free(&fnstrt),jl_data_free(&filesr);
	return rtn;
}

void jl_file_init_(jl_t * jl) {
	// Find out the native file separator.
	jl_data_mkfrom_str(&jl->fl.separator, "/");
	// Get ( and if need be, make ) the directory for everything.
	la_print("Get/Make directory for everything....");
	jl_file_get_root__(jl);
	la_print("Complete!");
	// Get ( and if need be, make ) the error file.
	la_print("Get/Make directory error logfile....");
	jl_file_get_errf__(jl);
	la_print("Complete!");
	//
	jl->has.filesys = 1;

	const char* pkfl = jl_file_get_resloc(jl, JL_MAIN_DIR, JL_MAIN_MEF);
	remove(pkfl);

	truncate(jl->fl.paths.errf, 0);
	la_print("Starting....");
	la_print("finished file init");
}
