#include <stdio.h>
#include <dlog.h>
#include "utils.h"


static void draw_word(const char* str)
{
	dlog_print(DLOG_INFO, TAG, "# read: %s", str);
	// put your drawing code here
}

void reader_start(const char* path)
{
	dlog_print(DLOG_INFO, TAG, "# reader_start called with path=%s", path);

	const int BUFSIZE = 128; // word size
	char str[BUFSIZE];
	FILE* fp = fopen(path, "r");
	if(fp){
		while (fscanf(fp, "%s", str)!=EOF){
			draw_word(str);
		}
		fclose(fp);
	}else{
		dlog_print(DLOG_INFO, TAG, "# can't open file %s", path);
	}

}
