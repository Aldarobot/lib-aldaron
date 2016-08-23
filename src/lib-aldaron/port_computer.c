#include "la.h"

#ifdef LA_COMPUTER

#include "port.h"
#include <stdio.h>

void la_print(const char* format, ...) {
	char temp[256];
	va_list arglist;

	// Write to temp
	va_start( arglist, format );
	vsprintf( temp, format, arglist );
	va_end( arglist );

//	la_file_append(LA_FILE_LOG, temp, strlen(temp)); // To File
	printf("%s\n", temp); // To Terminal
}

#endif
