/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

typedef struct {
	void* loop;
	void* init;
	void* kill;
}la_mode_t;

void la_mode_init(void* context, la_mode_t* mode, la_mode_t newmode);
void la_mode_change(void* context, la_mode_t* mode, la_mode_t newmode);
void la_mode_run(void* context, la_mode_t mode);
