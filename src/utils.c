#include <stdio.h>
#include <dlog.h>
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <app_common.h>

extern char* my_text[1000000];
extern int SZ;
extern int id;
extern int flag;

///////////
#define OK 1
#define NO 0

//static void check() {
//	dlog_print(DLOG_INFO, TAG, "# saved strings are:");
//	for (int i = 0; i != SZ; ++i) {
//		dlog_print(DLOG_INFO, TAG, "# string: \'%s\'", my_text[i]);
//	}
//}

// return 1 if file exist, else return 0
int my_text_generate(const char* path) {
	const int BUFSIZE = 128; // word size
	char str[BUFSIZE];
	FILE* fp = fopen(path, "r");

	if (fp) {
		SZ = 0;
		while (fscanf(fp, "%s", str) != EOF) {
			my_text[SZ] = (char *) malloc(strlen(str) + 1);
			strcpy(my_text[SZ], str);
			++SZ;
		}
		fclose(fp);
		dlog_print(DLOG_INFO, TAG, "# reader_start get %d lines in my_text",
				SZ);
		return OK; // OK
	} else {
		dlog_print(DLOG_INFO, TAG, "# can't open file %s", path);
		return NO;
	}
}

void reader_start(const char* path) {
	dlog_print(DLOG_INFO, TAG, "# reader_start called with path=%s", path);

	int rc = my_text_generate(path);
	// check();

	id = 0;	  // start pos
	if (rc)
		flag = 2; // watch_update will run reading mode =)
}

///////////////////////////////////////////////////////////////////////////
static const char* prefix =
		"b5dbed42e8a420d365cef38a5a26a86af7bbdd337d13d2231f27d51fa2cc3acf_";
static const char* last_book_filename = "last_book";
static const char* cur_ix_filename = "cur_ix";

// save book and cur_pos if cur_pos exist
// don't check anything, just save
// return 1 of ok, else return 0
int serialize_last_book() {
	char* data_path;

	// save to current array
	if (SZ > 0) { // not empty
		data_path = app_get_data_path();

		char last_book_path[128];
		sprintf(last_book_path, "%s%s%s", data_path, prefix,
				last_book_filename); // generate filename here
		FILE* fp_last_book = fopen(last_book_path, "w");
		if (!fp_last_book) {
			free(data_path);
			dlog_print(DLOG_ERROR, TAG, "memory error in serialize_last_book");
			return NO;
		}
		for (int i = 0; i != SZ; ++i) {
			fprintf(fp_last_book, "%s ", my_text[i]);
		}
		fclose(fp_last_book);

		char cur_pos_file_path[128];
		sprintf(cur_pos_file_path, "%s%s%s", data_path, prefix,
				cur_ix_filename); // generate filename here
		FILE* fp_pos = fopen(cur_pos_file_path, "w");
		if (!fp_pos) {
			free(data_path);
			dlog_print(DLOG_ERROR, TAG, "memory error in serialize_last_book");
			return NO;
		}
		fprintf(fp_pos, "%d", id);
		fclose(fp_pos);

		free(data_path);
	} else {
		dlog_print(DLOG_INFO, TAG, "book not loaded");
		return NO;
	}

	return OK;
}

// return 1, if book was saved, else return 0
int deserialize_last_book() {
	// read file with last book (if it exist)
	char* data_path;
	data_path = app_get_data_path();

	char last_book_path[128];
	sprintf(last_book_path, "%s%s%s", data_path, prefix, last_book_filename); // generate filename here
	int rc = my_text_generate(last_book_path);
	if (!rc) {
		free(data_path);
		dlog_print(DLOG_ERROR, TAG, "no last book");
		return NO;
	}

	// read last pos from file (if it exist)
	char cur_pos_file_path[128];
	sprintf(cur_pos_file_path, "%s%s%s", data_path, prefix, cur_ix_filename); // generate filename here
	FILE* fp_pos = fopen(cur_pos_file_path, "r");
	if (!fp_pos) {
		free(data_path);
		dlog_print(DLOG_ERROR, TAG, "no last pos");
		return NO;
	}
	fscanf(fp_pos, "%d", &id);
	fclose(fp_pos);

	free(data_path);

	return OK;
}

// deserialize book

