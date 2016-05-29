/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLfiles.c
 * 	This allows you to modify the file system.  It uses libzip.
 */
#include "jl_pr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// LIBZIP
#define ZIP_DISABLE_DEPRECATED //Don't allow the old functions.
#include "zip.h"

#define PKFMAX 10000000
#define JL_FL_PERMISSIONS ( S_IRWXU | S_IRWXG | S_IRWXO )

#if JL_PLAT == JL_PLAT_PHONE
	extern char* JL_FL_BASE;
#endif

/** @cond **/
// Static Functions

// This function converts linux filenames to native filnames
str_t jl_file_convert__(jl_t* jl, str_t filename) {
	jvct_t * _jl = jl->_jl;

	data_t* src = jl_data_mkfrom_str(filename);
	data_t* converted = jl_data_make(0);

	if(jl_data_test_next(src, "!")) {
		src->curs++; // ignore
	}else{
		src->curs++; // ignore
		jl_data_merg(_jl->jl, converted, _jl->fl.separator);
	}
	while(1) {
		data_t* append = jl_data_read_upto(jl, src, '/', 300); 
		if(append->data[0] == '\0') break;
		jl_data_merg(_jl->jl, converted, append);
		if(jl_data_byte(src) == '/')
			jl_data_merg(_jl->jl,converted,_jl->fl.separator);
		src->curs++; // Skip '/'
		jl_data_free(append);
	}
	jl_data_free(src);
	return jl_data_tostring(jl, converted);
}

static int jl_file_save_(jl_t* jl, const void *file_data, const char *file_name,
	uint32_t bytes)
{
	int errsv;
	ssize_t n_bytes;
	int fd;
	
	if(file_name == NULL) {
		jl_print(jl, "Save[file_name]: is Null");
		exit(-1);
	}else if(strlen(file_name) == 0) {
		jl_print(jl, "Save[strlen]: file_name is Empty String");
		exit(-1);
	}else if(!file_data) {
		jl_print(jl, "Save[file_data]: file_data is NULL");
		exit(-1);
	}

	str_t converted_filename = jl_file_convert__(jl, file_name);
	fd = open(converted_filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS);

	if(fd <= 0) {
		errsv = errno;

		jl_print(jl, "Save/Open: ");
		jl_print(jl, "\tFailed to open file: \"%s\"",
			converted_filename);
		jl_print(jl, "\tWrite failed: %s", strerror(errsv));
		exit(-1);
	}
	int at = lseek(fd, 0, SEEK_END);
	n_bytes = write(fd, file_data, bytes);
	if(n_bytes <= 0) {
		errsv = errno;
		close(fd);
		jl_print(jl, ":Save[write]: Write to \"%s\" failed:");
		jl_print(jl, "\"%s\"", strerror(errsv));
		exit(-1);
	}
	close(fd);
	return at;
}

static inline void jl_file_reset_cursor__(str_t file_name) {
	int fd = open(file_name, O_RDWR);
	lseek(fd, 0, SEEK_SET);
	close(fd);
}

static inline void jl_file_get_root__(jvct_t * _jl) {
	data_t* root_path;

#if JL_PLAT == JL_PLAT_PHONE
	data_t* root_dir;

	JL_PRINT_DEBUG(_jl->jl, "Get external storage directory.");
	root_path = jl_data_mkfrom_str(JL_FL_BASE);
	JL_PRINT_DEBUG(_jl->jl, "Append JL_ROOT_DIR.");
	root_dir = jl_data_mkfrom_str(JL_ROOT_DIR);
	JL_PRINT_DEBUG(_jl->jl, "Merging root_path and root_dir.");
	jl_data_merg(_jl->jl, root_path, root_dir);
	JL_PRINT_DEBUG(_jl->jl, "Free root_dir.");
	jl_data_free(root_dir);
#elif JL_PLAT_RPI
	data_t* root_dir;

	JL_PRINT_DEBUG(_jl->jl, "Get external storage directory.");
	root_path = jl_data_mkfrom_str("/home/pi/.local/share/");
	JL_PRINT_DEBUG(_jl->jl, "Append JL_ROOT_DIR.");
	root_dir = jl_data_mkfrom_str(JL_ROOT_DIR);
	JL_PRINT_DEBUG(_jl->jl, "Merging root_path and root_dir.");
	jl_data_merg(_jl->jl, root_path, root_dir);
	JL_PRINT_DEBUG(_jl->jl, "Free root_dir.");
	jl_data_free(root_dir);
#else
	// Get the operating systems prefered path
	m_str_t pref_path = SDL_GetPrefPath(JL_ROOT_DIRNAME, "\0");

	if(!pref_path) {
		jl_print(_jl->jl, "This platform has no pref path!");
		exit(-1);
	}
	// Erase extra non-needed '/'s
	pref_path[strlen(pref_path) - 1] = '\0';
	// Set root path to pref path
	root_path = jl_data_mkfrom_str(pref_path);
	// Free the pointer to pref path
	SDL_free(pref_path);
#endif
	// Make "-- JL_ROOT_DIR"
	if(jl_file_mkdir(_jl->jl, (str_t) root_path->data) == 2) {
		jl_print(_jl->jl, (str_t) root_path->data);
		jl_print(_jl->jl, ": mkdir : Permission Denied");
		exit(-1);
	}
	// Set paths.root & free root_path
	_jl->fl.paths.root = jl_data_tostring(_jl->jl, root_path);
	JL_PRINT_DEBUG(_jl->jl, "Root Path=\"%s\"", _jl->fl.paths.root);
}

static inline void jl_file_get_errf__(jvct_t * _jl) {
	data_t* fname = jl_data_mkfrom_str("errf.txt");
	// Add the root path
	data_t* errfs = jl_data_mkfrom_str(_jl->fl.paths.root);

	// Add the file name
	jl_data_merg(_jl->jl, errfs, fname);
	// Free fname
	jl_data_free(fname);
	// Set paths.errf & free errfs
	_jl->fl.paths.errf = jl_data_tostring(_jl->jl, errfs);
}

// NON-STATIC Library Dependent Functions

/** @endcond **/

/**
 * Print text to a file.
 * @param jl: The library context.
 * @param fname: The name of the file to print to.
 * @param msg: The text to print.
**/
void jl_file_print(jl_t* jl, str_t fname, str_t msg) {
	jvct_t * _jl = jl->_jl;

	// Write to the errf file
	if(_jl->has.filesys && fname)
		jl_file_save_(_jl->jl, msg, fname, strlen(msg));
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
u8_t jl_file_exist(jl_t* jl, str_t path) {
	DIR *dir;
	if ((dir = opendir (path)) == NULL) {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) { //Not a directory - is a file
			return 2;
		}else if(errsv == ENOENT) { // Directory Doesn't Exist
			return 0;
		}else if(errsv == EACCES) { // Doesn't have permission
			return 3;
		}else if((errsv == EMFILE) || (errsv == ENFILE) ||
			(errsv == ENOMEM)) //Not enough memory!
		{
			jl_print(jl, "jl_file_exist: Out of Memory!");
			exit(-1);
		}else{ //Unknown error
			jl_print(jl, "jl_file_exist: Unknown Error!");
			exit(-1);
		}
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
void jl_file_rm(jl_t* jl, str_t filename) {
	str_t converted_filename = jl_file_convert__(jl, filename);

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
void jl_file_save(jl_t* jl, const void *file, const char *name, uint32_t bytes) {
	// delete file
	jl_file_rm(jl, name);
	// make file
	jl_file_save_(jl, file, name, bytes);
}

/**
 * Load a File from the file system.  Returns bytes loaded from "file_name"
 * @param jl: Library Context
 * @param file_name: file to load
 * @returns A readable "strt" containing the bytes from the file.
 */
data_t* jl_file_load(jl_t* jl, str_t file_name) {
	jl_file_reset_cursor__(file_name);
	unsigned char *file = malloc(MAXFILELEN);
	str_t converted_filename = jl_file_convert__(jl, file_name);
	int fd = open(converted_filename, O_RDWR);
	
	//Open Block FLLD
	jl_print_function(jl, "FL_Load");
	
	if(fd <= 0) {
		int errsv = errno;

		jl_print(jl, "jl_file_load/open: ");
		jl_print(jl, "\tFailed to open file: \"%s\"", file_name);
		jl_print(jl, "\tLoad failed because: %s", strerror(errsv));
		if(errsv == ENOENT) {
			// Doesn't exist
			jl_print_return(jl, "FL_Load");
		}else{
			// Is a Directory
			exit(-1);
		}
		return NULL;
	}
	int Read = read(fd, file, MAXFILELEN);
	jl->info = Read;

	jl_print(jl, "jl_file_load(): read %d bytes", jl->info);
	close(fd);

	data_t* rtn = jl->info ? jl_data_mkfrom_data(jl, jl->info, file) : NULL;
	
	jl_print_return(jl, "FL_Load"); //Close Block "FLLD"
	return rtn;
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
char jl_file_pk_save(jl_t* jl, str_t packageFileName, str_t fileName,
	void *data, uint64_t dataSize)
{
	str_t converted = jl_file_convert__(jl, packageFileName);

	jl_print_function(jl, "FL_PkSave");
	jl_print(jl, "opening \"%s\"....", converted);
	struct zip *archive = zip_open(converted, ZIP_CREATE 
		| ZIP_CHECKCONS, NULL);
	if(archive == NULL) {
		jl_print_return(jl, "FL_PkSave");
		return 1;
	}else{
		jl_print(jl, "opened package, \"%d\".", converted);
	}

	struct zip_source *s;
	if ((s=zip_source_buffer(archive, (void *)data, dataSize, 0)) == NULL) {
		zip_source_free(s);
		jl_print(jl, "[JL_FL_PK_SAVE] src null error[replace]: %s",
			(char *)zip_strerror(archive));
		jl_print_return(jl, "FL_PkSave");
		exit(-1);
	}
//	JL_PRINT("%d,%d,%d\n",archive,sb.index,s);
	if(zip_file_add(archive, fileName, s, ZIP_FL_OVERWRITE)) {
		jl_print(jl, "add/err: \"%s\"", zip_strerror(archive));
	}else{
		jl_print(jl, "added \"%s\" to file sys.", fileName);
	}
	zip_close(archive);
	jl_print(jl, "DONE!");
	jl_print_return(jl, "FL_PkSave");
	return 0;
}

static void _jl_file_pk_load_quit(jl_t* jl) {
	jl_print_return(jl, "FL_PkLd"); //Close Block "FL_PkLd"
}

/**
 * Load a zip package from memory.
 * @param jl: The library context.
 * @param data: The data that contains the zip file.
 * @param file_name: The name of the file to load.
**/
data_t* jl_file_pk_load_fdata(jl_t* jl, data_t* data, str_t file_name) {
	data_t* rtn;
	zip_error_t ze; ze.zip_err = ZIP_ER_OK;
	zip_source_t *file_data;
	int zerror = 0;

	file_data = zip_source_buffer_create(data->data, data->size, 0, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		jl_print(jl, "couldn't make pckg buffer!");
		//zip_error_init_with_code(&ze, ze.zip_err);
		jl_print(jl, "because: \"%s\"", zip_error_strerror(&ze));
		exit(-1);
	}

	JL_PRINT_DEBUG(jl, "error check 2.");
	struct zip *zipfile = zip_open_from_source(file_data,
		ZIP_CHECKCONS | ZIP_RDONLY, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		zip_error_init_with_code(&ze, ze.zip_err);
		jl_print(jl, "couldn't load pckg file");
		jl_print(jl, "because: \"%s\"", zip_error_strerror(&ze));
		char name[3]; name[0] = data->data[0]; name[1] = '\0';
		jl_print(jl, "First character = %1s", name);
		exit(-1);
	}

//	struct zip *zipfile = zip_open(converted, ZIP_CHECKCONS, &zerror);
	JL_PRINT_DEBUG(jl, "error check 3.");
	if(zipfile == NULL) {
		jl_print(jl, "couldn't load zip because:");
		if(zerror == ZIP_ER_INCONS) {
			jl_print(jl, "\tcorrupt file");
		}else if(zerror == ZIP_ER_NOZIP) {
			jl_print(jl, "\tnot a zip file");
		}else{
			jl_print(jl, "\tunknown error");
		}
		_jl_file_pk_load_quit(jl);
		exit(-1);
	}
	JL_PRINT_DEBUG(jl, "error check 4.");
	JL_PRINT_DEBUG(jl, (char *)zip_strerror(zipfile));
	JL_PRINT_DEBUG(jl, "loaded package.");
	unsigned char *fileToLoad = malloc(PKFMAX);
	JL_PRINT_DEBUG(jl, "opening file in package....");
	struct zip_file *file = zip_fopen(zipfile, file_name, ZIP_FL_UNCHANGED);
	JL_PRINT_DEBUG(jl, "call pass.");
	if(file == NULL) {
		jl_print(jl, "couldn't open up file: \"%s\" in package:",
			file_name);
		jl_print(jl, "because: %s", (void *)zip_strerror(zipfile));
		jl->errf = JL_ERR_NONE;
		_jl_file_pk_load_quit(jl);
		return NULL;
	}
	JL_PRINT_DEBUG(jl, "opened file in package / reading opened file....");
	if((jl->info = zip_fread(file, fileToLoad, PKFMAX)) == -1) {
		jl_print(jl, "file reading failed");
		_jl_file_pk_load_quit(jl);
		exit(-1);
	}
	if(jl->info == 0) {
		JL_PRINT_DEBUG(jl, "empty file, returning NULL.");
		_jl_file_pk_load_quit(jl);
		return NULL;
	}
	JL_PRINT_DEBUG(jl, "jl_file_pk_load: read %d bytes", jl->info);
	zip_close(zipfile);
	JL_PRINT_DEBUG(jl, "closed file.");
	// Make a data_t* from the data.
	rtn = jl->info ? jl_data_mkfrom_data(jl, jl->info, fileToLoad) : NULL;
	JL_PRINT_DEBUG(jl, "done.");
	jl->errf = JL_ERR_NERR;
	_jl_file_pk_load_quit(jl);
	return rtn;
}

/**
 * Load file "filename" in package "packageFileName" & Return contents
 * May return NULL.  If it does jl->errf will be set.
 * -ERR:
 *	-ERR_NERR:	File is empty.
 *	-ERR_NONE:	Can't find filename in packageFileName. [ DNE ]
 *	-ERR_FIND:	Can't find packageFileName. [ DNE ]
 * @param jl: Library Context
 * @param packageFileName: Package to load file from
 * @param filename: file within package to load
 * @returns: contents of file ( "filename" ) in package ( "packageFileName" )
*/
data_t* jl_file_pk_load(jl_t* jl, const char *packageFileName,
	const char *filename)
{
	str_t converted = jl_file_convert__(jl, packageFileName);

	jl->errf = JL_ERR_NERR;
	jl_print_function(jl, "FL_PkLd");

	JL_PRINT_DEBUG(jl, "loading package:\"%s\"...", converted);

	data_t* data = jl_file_load(jl, converted);
	JL_PRINT_DEBUG(jl, "error check 1.");
	if(data == NULL) {
		JL_PRINT_DEBUG(jl, "!Package File doesn't exist!");
		jl->errf = JL_ERR_FIND;
		_jl_file_pk_load_quit(jl);
		return NULL;
	}
	return jl_file_pk_load_fdata(jl, data, filename);
}

/**
 * Create a folder (directory)
 * @param jl: library context
 * @param pfilebase: name of directory to create
 * @returns 0: Success
 * @returns 1: If the directory already exists.
 * @returns 2: Permission Error
 * @returns 255: Never.
*/
u8_t jl_file_mkdir(jl_t* jl, str_t path) {
	m_u8_t rtn = 255;

	jl_print_function(jl, "FL_MkDir");
	if(mkdir(path, JL_FL_PERMISSIONS)) {
		int errsv = errno;
		if(errsv == EEXIST) {
			rtn = 1;
		}else if((errsv == EACCES) || (errsv == EROFS)) {
			rtn = 2;
		}else{
			jl_print(jl, "couldn't mkdir:%s", strerror(errsv));
			exit(-1);
		}
	}else{
		rtn = 0;
	}
	jl_print_return(jl, "FL_MkDir");
	// Return
	return rtn;
}

/**
 * Create the media package file & load a file from it.
 * @param jl: library context
 * @param pfilebase: name of file to load from package
 * @param pzipfile: the name of the zipfile to create.
 * @param contents: the contents to put in the file
 * @param size: the size (in bytes) of the contents.
 * @return x: the data contents of the file.
*/
data_t* jl_file_mkfile(jl_t* jl, str_t pzipfile, str_t pfilebase,
	char *contents, uint32_t size)
{
//	if(!pfilebase) { return; }
	data_t* rtn;

	//Create Block "MKFL"
	jl_print_function(jl, "FL_MkFl");

	JL_PRINT_DEBUG(jl, "Creating File....");
	jl_file_save(jl, contents, pzipfile, size);
	JL_PRINT_DEBUG(jl, "Try loading....");
	if(
		((rtn = jl_file_pk_load(jl, pzipfile, pfilebase))== NULL) &&
		(jl->errf == JL_ERR_FIND) )//Package still doesn't exist!!
	{
		JL_PRINT_DEBUG(jl, "Failed To Create file");
		jl_print_return(jl, "FL_MkFl");
		exit(-1);
	}
	JL_PRINT_DEBUG(jl, "Good loading! / File Made!");
	//Close Block "MKFL"
	jl_print_return(jl, "FL_MkFl");
	return rtn;
}

/**
 * Load media package, create it if it doesn't exist.
 * @param jl: Library Context
 * @param Fname: File in Media Package to load.
 * @param pzipfile: Where to make the package.
 * @param pdata: Media Package Data to save if it doesn't exist.
 * @param psize: Size of "pdata" 
*/
data_t* jl_file_media(jl_t* jl, str_t Fname, str_t pzipfile, void *pdata,
	uint64_t psize)
{
	// Try to load package	
	data_t* rtn = jl_file_pk_load(jl, pzipfile, Fname);
	JL_PRINT_DEBUG(jl, "JL_FL_MEDIA Returning");
	//If Package doesn't exist!! - create
	if( (rtn == NULL) && (jl->errf == JL_ERR_FIND) )
		return jl_file_mkfile(jl, pzipfile, Fname, pdata, psize);
	else
		return rtn;
}

/**
 * Load 
**/

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
str_t jl_file_get_resloc(jl_t* jl, str_t prg_folder, str_t fname) {
	jvct_t * _jl = jl->_jl;
	data_t* filesr = jl_data_mkfrom_str(JL_FILE_SEPARATOR);
	data_t* pfstrt = jl_data_mkfrom_str(prg_folder);
	data_t* fnstrt = jl_data_mkfrom_str(fname);
	data_t* resloc = jl_data_mkfrom_str(_jl->fl.paths.root);
	str_t rtn = NULL;
	
	//Open Block "FLBS"
	jl_print_function(jl, "FL_Base");
	
	JL_PRINT_DEBUG(jl, "Getting Resource Location....");
	// Append 'prg_folder' onto 'resloc'
	jl_data_merg(jl, resloc, pfstrt);
	// Append 'filesr' onto 'resloc'
	jl_data_merg(jl, resloc, filesr);
	// Make 'prg_folder' if it doesn't already exist.
	if( jl_file_mkdir(jl, (str_t) resloc->data) == 2 ) {
		jl_print(jl, "jl_file_get_resloc: couldn't make \"%s\"",
			(str_t) resloc->data);
		jl_print(jl, "mkdir : Permission Denied");
		exit(-1);
	}
	// Append 'fname' onto 'resloc'
	jl_data_merg(jl, resloc, fnstrt);
	// Set 'rtn' to 'resloc' and free 'resloc'
	rtn = jl_data_tostring(jl, resloc);
	// Free pfstrt & fnstrt & filesr
	jl_data_free(pfstrt),jl_data_free(fnstrt),jl_data_free(filesr);
	// Close Block "FLBS"
	jl_print_return(jl, "FL_Base");
	//jl_print(jl, "finished resloc w/ \"%s\"", rtn); 
	return rtn;
}

void jl_file_kill__(jvct_t * _jl) {
	if(_jl->has.fileviewer) {
		JL_PRINT_DEBUG(_jl->jl, "killing fl....");
		cl_list_destroy(_jl->fl.filelist);
		JL_PRINT_DEBUG(_jl->jl, "killed fl!");
	}
}

void jl_file_init__(jvct_t * _jl) {
	jl_print_function(_jl->jl, "FL_Init");
	// Find out the native file separator.
	_jl->fl.separator = jl_data_mkfrom_str("/");
	// Get ( and if need be, make ) the directory for everything.
	JL_PRINT_DEBUG(_jl->jl, "Get/Make directory for everything....");
	jl_file_get_root__(_jl);
	JL_PRINT_DEBUG(_jl->jl, "Complete!");
	// Get ( and if need be, make ) the error file.
	JL_PRINT_DEBUG(_jl->jl, "Get/Make directory error logfile....");
	jl_file_get_errf__(_jl);
	JL_PRINT_DEBUG(_jl->jl, "Complete!");
	//
	_jl->has.filesys = 1;

	str_t pkfl = jl_file_get_resloc(_jl->jl, JL_MAIN_DIR, JL_MAIN_MEF);
	remove(pkfl);

	truncate(_jl->fl.paths.errf, 0);
	JL_PRINT_DEBUG(_jl->jl, "Starting....");
	JL_PRINT_DEBUG(_jl->jl, "finished file init");
	jl_print_return(_jl->jl, "FL_Init");
}
