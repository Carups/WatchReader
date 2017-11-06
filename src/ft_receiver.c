#include "ft.h"
#include "ft_receiver.h"
#include "ft_progressbar.h"
#include "utils.h"
#include <Ecore.h>
#include <unistd.h>

double TIMEOUT_1 = 0.01; // interval for timer1

#define READ_MODE 2
#define SETTINGS_MODE 1
#define MENU_MODE 0

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *naviframe;
	Evas_Object *conform;
	Evas_Object *content;
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *icon;
	Evas_Object *layout;
	// added
	Evas_Object *label;
	Evas_Object *button;
	Evas_Object *button1;
	Evas_Object *faster;
	Evas_Object *slower;
	Evas_Object *text;
	Evas_Object *back;
} appdata_s;

char *my_text[1000000] = {NULL};
int SZ = 0;
int USER_SPEED = 300;
int flag = MENU_MODE, cnt = 0, id = 0;
const int TEXT_BUF_SIZE = 256, WIDTH = 400, HEGHT = 400;

Ecore_Timer *timer_p;
appdata_s *DATA;

void pause_timer()
{
	if (timer_p) {
		ecore_timer_freeze(timer_p);
	}
}

void resume_timer()
{
	if (timer_p) {
		ecore_timer_thaw(timer_p);
	}
}

void stop_reading()
{
	pause_timer();
}

void start_reading()
{
	resume_timer();
}


//////////////////// timer ////////////////////////
//static double _initial_time = 0;
//static double _get_current_time(void) {
//	return ecore_time_get() - _initial_time;
//}

static void update_watch(appdata_s *);

static void draw_menu()
{
	appdata_s *ad = DATA;
	evas_object_show(ad->button);
	evas_object_show(ad->button1);
	evas_object_hide(ad->back);
	evas_object_hide(ad->label);
	evas_object_hide(ad->slower);
	evas_object_hide(ad->faster);
}

static void draw_settings()
{
	appdata_s *ad = DATA;
	evas_object_hide(ad->button);
	evas_object_hide(ad->button1);
	evas_object_show(ad->faster);
	evas_object_show(ad->slower);
	evas_object_show(ad->back);
	evas_object_show(ad->label);
	char watch_text[TEXT_BUF_SIZE];
	snprintf(watch_text, TEXT_BUF_SIZE, "<align=center>%d<align>", USER_SPEED);
	elm_object_text_set(ad->label, watch_text);
}

// timer callback - draw screen
static Eina_Bool _timer1_cb(void *data EINA_UNUSED)
{
//	dlog_print(DLOG_DEBUG, TAG, "Timer1 expired after %0.3f seconds.",
//			_get_current_time());

	if (DATA && flag == READ_MODE) {
		update_watch(DATA); // update screen
	}

	return ECORE_CALLBACK_RENEW;
}

////////// end timer ///////

static appdata_s *global_ad;

static void win_delete_request_cb(void *data, Evas_Object *obj,
								  void *event_info)
{
	ui_app_exit();
}

//static Eina_Bool _naviframe_pop_cb(void *data, Elm_Object_Item *it) {
//	ui_app_exit();
//	return EINA_TRUE;
//}
static void button_setting(void *data, Evas_Object *obj, void *event_info)
{
	// flag = SETTINGS_MODE;
	// changed
	draw_settings();
}

static void button_last_book(void *user_data, Evas_Object *obj,
							 void *event_info)
{

	// restore last book, start reading mode
	if (SZ > 0) { // was readed
		start_reading();
	} else {
		int rc = deserialize_last_book();
		if (rc) {
			// flag = READ_MODE;
			// changed
			start_reading();
		} else {
			dlog_print(DLOG_ERROR, TAG, "# last book not loaded");
		}
	}
}

static void button_slower(void *data, Evas_Object *obj, void *event_info)
{
	USER_SPEED -= 50;
	if (USER_SPEED <= 100) {
		USER_SPEED = 100;
	}
	draw_settings();

}

static void button_fast(void *data, Evas_Object *obj, void *event_info)
{
	USER_SPEED += 50;
	draw_settings();
}

static void button_back(void *data, Evas_Object *obj, void *event_info)
{
	// flag = MENU_MODE;
	draw_menu();
}

static int spritz_function(char *in, char *out, int width, int speed)
{
	int shift;
	int red_point = 0.35 * width;
	int len = (int) strlen(in);
	int num_of_tics = (60 / TIMEOUT_1) / (speed);
	// define shift of the word depending on his len
	if (len == 1 && in[0] != 'M') {
		shift = red_point - 0;
	} else if (len > 1 && len <= 5) {
		shift = red_point - 1;
	} else if (len > 5 && len <= 9) {
		shift = red_point - 2;
	} else if (len > 9 && len <= 13) {
		shift = red_point - 3;
	} else {
		shift = red_point - 4;
	}

	for (int i = 0; i < len; i++) {
		out[shift + i] = in[i];
	}
	//define the num of pin
	if (in[len - 1] == '.' && in[0] != 'M') {
		num_of_tics *= 5;
	} else if (len == 1) {
		num_of_tics *= 2;
	} else if (len > 1 && len <= 6) {
		num_of_tics *= 3;
	} else {
		num_of_tics *= 2;
	}

	return num_of_tics;

}

static void update_watch(appdata_s *ad)
{
//	if (flag == READ_MODE) {
	evas_object_hide(ad->button);
	evas_object_hide(ad->button1);
	evas_object_show(ad->label);

	char watch_text[TEXT_BUF_SIZE];
	int width = WIDTH;
	char out[width];
	for (int i = 0; i < width; ++i) {
		out[i] = '#';
	}

	if (cnt == 0) {

		cnt = spritz_function(my_text[id], out, width, USER_SPEED);
		int pos = width * 0.35;
		int pref = 0, suff = 0;
		while (out[pref] == '#') {
			pref++;
		}
		while (out[pos + 1 + suff] != '#') {
			suff++;
		}

		snprintf(watch_text, TEXT_BUF_SIZE,
				 "<align=center>" "<font=BreezeSans:style=condensed font_size=45>"
					 "%.*s" "<color=#FF4500FF>" "%.*s" "</color>" "%.*s" "</font>" "</align>",
				 pos - pref, out + pref, 1, out + pos, suff, out + pos + 1);
		elm_object_text_set(ad->label, watch_text);
		cnt--;
	} else {
		cnt--;
	}
	if (cnt <= 0) {
		cnt = 0;
		id++;
		id %= SZ;
	}
//	}
//	if (flag == SETTINGS_MODE) {
//		draw_settings();
//	}
//
//	if (flag == MENU_MODE) {
//		draw_menu();
//	}
}

static void _reject_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj)
		return;
	evas_object_del(data);
	reject_file();
}

static void _accept_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj)
		return;
	evas_object_del(data);
	accept_file();
}

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj)
		return;
	elm_popup_dismiss(obj);
}

static void _popup_hide_finished_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	if (!obj)
		return;
	evas_object_del(obj);
}

static void popup_title_text_check_button(void *data, Evas_Object *obj,
										  void *event_info)
{
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *icon;
	Evas_Object *layout;
	struct appdata *ad = (struct appdata *) data;

	popup = elm_popup_add(ad->naviframe);
	elm_object_style_set(popup, "circle");
	//uxt_popup_set_rotary_event_enabled(popup, EINA_TRUE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_hide_cb,
								   NULL);
	evas_object_smart_callback_add(popup, "dismissed", _popup_hide_finished_cb,
								   NULL);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle/buttons2");
	elm_object_part_text_set(layout, "elm.text.title", "");

	elm_object_part_text_set(layout, "elm.text",
							 "Do you want to receive file?");
	elm_object_content_set(popup, layout);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup/circle/left");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _accept_cb, popup);

	icon = elm_image_add(btn);
	elm_image_file_set(icon,
					   "/opt/usr/apps/org.tizen.filetransferreceiver/res/images/tw_ic_popup_btn_check.png",
					   NULL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(btn, "elm.swallow.content", icon);
	evas_object_show(icon);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup/circle/right");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", _reject_cb, popup);

	icon = elm_image_add(btn);
	elm_image_file_set(icon,
					   "/opt/usr/apps/org.tizen.filetransferreceiver/res/images/tw_ic_popup_btn_delete.png",
					   NULL);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(btn, "elm.swallow.content", icon);
	evas_object_show(icon);

	evas_object_show(popup);
	global_ad->icon = icon;
	global_ad->layout = layout;
	global_ad->popup = popup;
	global_ad->btn = btn;
}

void show_file_req_popup(void)
{
	popup_title_text_check_button(global_ad->naviframe, NULL, NULL);
}
void hide_file_req_popup(void)
{
	elm_popup_dismiss(global_ad->popup);
}
static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	evas_object_smart_callback_add(ad->win, "delete,request",
								   win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
								   ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
									 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	ad->naviframe = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, ad->naviframe);

	ad->content = create_content(ad->naviframe);

	global_ad = ad;

	int width = 300;
	int height = 400;
	/* Label*/
	ad->label = elm_label_add(ad->conform);

	evas_object_resize(ad->label, 200, 200);
	evas_object_move(ad->label, 70, 150);

	/* button */
	ad->button = elm_button_add(ad->conform);
	elm_object_text_set(ad->button, "Setting");
	evas_object_smart_callback_add(ad->button, "clicked", button_setting, ad);
	evas_object_resize(ad->button, width, height / 4);
	evas_object_move(ad->button, width / 10, height / 3);

	ad->button1 = elm_button_add(ad->conform);
	elm_object_text_set(ad->button1, "Last Book");
	evas_object_smart_callback_add(ad->button1, "clicked", button_last_book,
								   ad);
	evas_object_resize(ad->button1, width, height / 4);
	evas_object_move(ad->button1, width / 10, height / 4 + height / 3);
	/* settings */
	ad->faster = elm_button_add(ad->conform);
	elm_object_text_set(ad->faster, "i");
	evas_object_smart_callback_add(ad->faster, "clicked", button_fast, ad);
	evas_object_resize(ad->faster, width / 5, height / 4);
	evas_object_move(ad->faster, 4 * width / 5 + width / 10, height / 2 - 40);

	ad->slower = elm_button_add(ad->conform);
	elm_object_text_set(ad->slower, "r");
	evas_object_smart_callback_add(ad->slower, "clicked", button_slower, ad);
	evas_object_resize(ad->slower, width / 5, height / 4);
	evas_object_move(ad->slower, width / 10, height / 2 - 40);

	ad->back = elm_button_add(ad->conform);
	elm_object_text_set(ad->back, "back");
	evas_object_smart_callback_add(ad->back, "clicked", button_back, ad);
	evas_object_resize(ad->back, width, height / 4);
	evas_object_move(ad->back, width / 10, height / 3 + height / 4);
	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	// update_watch(ad);
	draw_menu();
}

static bool app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;

	DATA = ad;

	create_base_gui(ad);
	initialize_sap();

	// create timer
	timer_p = ecore_timer_add(TIMEOUT_1, _timer1_cb, NULL);
	if (!timer_p) {
		dlog_print(DLOG_ERROR, TAG, "timer creation FAILED");
	}
	stop_reading();

	return true;
}

static void app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
	dlog_print(DLOG_DEBUG, TAG, "app_pause() called");
//	if (timer_p) {
//		ecore_timer_freeze(timer_p);
//	}
//
//	flag = MENU_MODE;
	stop_reading();
	id = max(id - WORDS_BACK,  0);
	// draw_menu();

//	int is_saved = serialize_last_book();
//	if (!is_saved) {
//		dlog_print(DLOG_ERROR, TAG, "# last book not saved");
//	}
}

static void app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
	dlog_print(DLOG_DEBUG, TAG, "app_resume() called");
	draw_menu();
}

static void app_terminate(void *data)
{
	if (timer_p) {
		ecore_timer_del(timer_p);
	}

	/* Release all resources. */
	dlog_print(DLOG_DEBUG, TAG, "app_terminate() called");
	dlog_print(DLOG_INFO, TAG, "free %d strings in my_text", SZ);

	for (int i = 0; i != SZ; ++i) {
		free(my_text[i]);
	}

	int is_saved = serialize_last_book();
	if (!is_saved) {
		dlog_print(DLOG_ERROR, TAG, "# last book not saved");
	}
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
									 &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL,};

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
							 APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
							 APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
							 APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
							 APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
							 APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, TAG, "app_main() is failed. err = %d", ret);
	} else {
		dlog_print(DLOG_DEBUG, TAG, "app_main() is not failed");
	}

	return ret;
}
