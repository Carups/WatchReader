#include <stdio.h>
#include <dlog.h>
#include "utils.h"
#include <stdlib.h>
#include <string.h>

extern char* my_text[1000000];
extern int SZ;
extern int flag;

//static void draw_word(const char* str)
//{
//	dlog_print(DLOG_INFO, TAG, "# read: %s", str);
//	// put your drawing code here
//}

static void check(){
	dlog_print(DLOG_INFO, TAG, "# saved strings are:");
	for(int i = 0; i != SZ; ++i){
		dlog_print(DLOG_INFO, TAG, "# string: \'%s\'", my_text[i]);
	}
}

void reader_start(const char* path)
{
	dlog_print(DLOG_INFO, TAG, "# reader_start called with path=%s", path);

	const int BUFSIZE = 128; // word size
	char str[BUFSIZE];
	FILE* fp = fopen(path, "r");
	if(fp){
		SZ = 0;
		while (fscanf(fp, "%s", str) != EOF){
			dlog_print(DLOG_INFO, TAG, "# got string: %s", str);
			my_text[SZ] = (char *) malloc(strlen(str) + 1);
			dlog_print(DLOG_INFO, TAG, "# allocated %lu bytes for %s", strlen(str) + 1, str);
			strcpy(my_text[SZ], str);
			++SZ;
		}
		fclose(fp);
		dlog_print(DLOG_INFO, TAG, "# reader_start get %d lines in my_text", SZ);
	}else{
		dlog_print(DLOG_INFO, TAG, "# can't open file %s", path);
	}

	check();

	// set flag to 2
	flag = 2;
}
