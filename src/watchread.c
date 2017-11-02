#include <tizen.h>
#include <string.h>
#include <efl_extension.h>
#include "watchread.h"
#include <dlog.h>
#include <stdio.h>
#include <app_common.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *button;
	Evas_Object *button1;
	Evas_Object *text;
} appdata_s;
#define FONT "/home/nicolay/workspace/WatchRead/res/font"
#define TEXT_BUF_SIZE 512
const int TICKS = 4000;
int USER_SPEED = 300;
char * my_text[1000000];
int WIDTH, HEIGHT, CHR;
void * DATA;
int cnt = 0, id = 0, flag = 2, SZ = 560;

static void
button_clicked(void *data, Evas_Object *obj, void *event_info )
{

    flag = 2;

}

static void
clicked_cb1(void *user_data, Evas_Object *obj, void *event_info)
{

    flag = 1;
}

static int spritz_function(char* in, char* out, int width, int speed)
{
	int shift;
	int red_point = 0.35 * width;
	int len = (int)strlen(in);
	int num_of_words_in_s = speed / 60;
	int num_of_tics_in_s = 4000 / 60; //always default
	int num_of_tics = num_of_tics_in_s / (2 * num_of_words_in_s);
	// define shift of the word depending on his len
	if(len == 1 && in[0] != 'M')
	{
		shift = red_point - 0;
	}
	else if(len > 1 && len <= 5)
	{
		shift = red_point - 1;
	}
	else if (len > 5 && len <= 9)
	{
		shift = red_point - 2;
	}
	else if (len > 9 && len <= 13)
	{
		shift = red_point - 3;
	}
	else
	{
		shift = red_point - 4;
	}

	for(int i = 0; i < len; i++)
	{
		out[shift + i] = in[i];
	}
	//define the num of pin
	if(in[len - 1] == '.' && in[0] != 'M')
	{
		num_of_tics *= 5;
	}
	else if (len == 1)
	{
		num_of_tics *= 2;
	}
	else if (len > 1 && len <= 6)
	{
		num_of_tics *= 3;
	}
	else
	{
		num_of_tics *= 2;
	}

	return num_of_tics;

}

static void
update_watch(appdata_s *ad, int ambient)
{
	if (flag == 2)
	{
		evas_object_hide(ad->button);
		evas_object_hide(ad->button1);
		evas_object_show(ad->label);
		char watch_text[TEXT_BUF_SIZE];
		int width = WIDTH / CHR;
		char out[width];
		for (int i = 0; i < width; ++i)
		{
			out[i] = '#';
		}

		if (cnt == 0)
		{
			id++;
			id %= SZ;
			cnt = spritz_function(my_text[id], out, width, USER_SPEED);
			int pos = width * 0.35;
			int pref = 0, suff = 0;
			while(out[pref] == '#')
			{
				pref++;
			}
			while(out[pos + 1 + suff] != '#')
			{
				suff++;
			}

			//snprintf(watch_text, TEXT_BUF_SIZE, "<align=center>" "<color=#FF000000>" "%.*s" "</color>" "%.*s" "<color=#FF4500FF>" "%.*s" "</color>" "%.*s" "</align>", 1, out + pos, suff, out + pos + 1);
			snprintf(watch_text, TEXT_BUF_SIZE, "<align=center>" "<font=BreezeSans:style=condensed font_size=45>" "%.*s" "<color=#FF4500FF>" "%.*s" "</color>" "%.*s" "</font>"  "</align>",  pos - pref, out + pref, 1, out + pos, suff, out + pos + 1);
			//snprintf(watch_text, TEXT_BUF_SIZE, "<font = Sans>""<color=#FF4500FF>%s</color>%s<color=#FF4500FF>%c</color>%s", pref_empty, pref, v, suff);
			elm_object_text_set(ad->label, watch_text);
			cnt--;
		}
		else
		{
			//snprintf(watch_text, TEXT_BUF_SIZE, "<color=#FF4500FF>%s</color>%s<color=#FF4500FF>%c</color>%s", pref_empty, pref, v, suff);
			cnt--;
		}

	}
	if (flag == 1)
	{
		evas_object_hide(ad->button);
	}
	if (flag == 0)
	{
		evas_object_show(ad->button);
		evas_object_show(ad->button1);
	}
}

static void
create_base_gui(appdata_s *ad, int width, int height)
{

	evas_font_path_global_append("/home/nicolay/workspace/WatchRead/res/font/UbuntuMono-B.ttf");

	//ad = NULL;
    int ret;
	/* Window */
	ret = watch_app_get_elm_win(&ad->win);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, " failed to get window. err = %d", ret);
		return;
	}

	evas_object_resize(ad->win, width, height);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);
	/* Label*/
	ad->label = elm_label_add(ad->conform);
	evas_object_resize(ad->label, width, height / 3);
	evas_object_move(ad->label, 0, height / 2 - 40);
	update_watch(ad, 0);
	/* button */

	ad->button = elm_button_add(ad->conform);
	elm_object_text_set(ad->button, "Setting");
//		Evas_Smart_Cb func;
	evas_object_smart_callback_add(ad->button, "clicked", button_clicked, ad);

	evas_object_resize(ad->button, width, height / 4);
	evas_object_move(ad->button, 0, height / 3);
	//evas_object_show(ad->button);
	ad->button1 = elm_button_add(ad->conform);
	elm_object_text_set(ad->button1, "Last Book");
	evas_object_smart_callback_add(ad->button1, "clicked", clicked_cb1, ad);
	evas_object_resize(ad->button1, width, height / 4);
	evas_object_move(ad->button1, 0, height / 4 + height / 3);
	Evas *e;
	e = evas_object_evas_get(ad->win);

	evas_object_text_font_set(ad->label, "UbuntuMono-B.ttf", 20);

//		update_watch(ad, 0);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(int width, int height, void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	HEIGHT = height;
	WIDTH = width;
	DATA = data;
    CHR = width / 23;
	appdata_s *ad = data;
	int ret = watch_app_set_time_tick_frequency(TICKS, WATCH_APP_TIME_TICKS_PER_MINUTE);
	if (ret != 0) {
			dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
			return false;
		}

	create_base_gui(ad, width, height);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
app_time_tick(watch_time_h watch_time, void *data)
{
	/* Called at each second while your app is visible. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, 0);
}

static void
app_ambient_tick(watch_time_h watch_time, void *data)
{
	/* Called at each minute while the device is in ambient mode. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, 1);
}

static void
app_ambient_changed(bool ambient_mode, void *data)
{
	/* Update your watch UI to conform to the ambient mode */
}

static void
watch_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
watch_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

char* get_write_filepath(char *filename)
{

    char write_filepath[1000] = {0,};
    char *resource_path = app_get_data_path(); // get the application data directory path
    if(resource_path)
    {
        snprintf(write_filepath,1000,"%s%s",resource_path,filename);
        free(resource_path);
    }

    return write_filepath;
}

static char* write_file(const char* filepath, const char* buf)
{
    FILE *fp;
    fp = fopen(filepath,"w");
    fputs(buf,fp);
    fclose(fp);
}

int
main(int argc, char *argv[])
{
	//SZ = 0;
	/*if (fp)
	{
		while (fscanf(fp, " %1023s", x) == 1)
		{
			my_text[SZ++] = x;
		}
	}
	else
	{*/
		SZ = 3;
		my_text[0] = "aaaaaaaaaa";
		my_text[1] = "bbbbbbbbbb";
		my_text[2] = "cccccccccc";

	//}

	//write_file(get_write_filepath("file.txt") ,"asdf");
	appdata_s ad = {0,};
	int ret = 0;
    watch_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.time_tick = app_time_tick;
	event_callback.ambient_tick = app_ambient_tick;
	event_callback.ambient_changed = app_ambient_changed;

	//create_button(event_callback, "setting");
	//watch_app_set_time_tick_frequency(1, 0);
	watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, watch_app_lang_changed, &ad);
	watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, watch_app_region_changed, &ad);

	ret = watch_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
	}

	return ret;
}

