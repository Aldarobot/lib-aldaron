/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_COLLECTOR
#define LA_COLLECTOR

typedef void(*la_iterator_fn_t)(void* context, void* data);
typedef struct cl_list la_list_t;

void la_list_alphabetize(la_list_t *list);
void la_list_iterate(void* context, la_list_t *list, la_iterator_fn_t fn);

#endif
