#include "ft.h"
#include "ft_receiver.h"
#include "ft_progressbar.h"
#include <Ecore.h>
#include <unistd.h>

double TIMEOUT_1 = 1.0; // interval for timer1

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *naviframe;
	Evas_Object *conform;
	Evas_Object *content;
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *icon;
	Evas_Object *layout;
} appdata_s;

char* my_text[1000000];
int SZ = 0;
int flag = 0;
Ecore_Timer *timer_p;

//////////////////// timer ////////////////////////
static double _initial_time = 0;

static double _get_current_time(void) {
	return ecore_time_get() - _initial_time;
}

static Eina_Bool _timer1_cb(void *data EINA_UNUSED) {
	dlog_print(DLOG_DEBUG, TAG, "Timer1 expired after %0.3f seconds.",
			_get_current_time());

	// write your ui changing code here

	return ECORE_CALLBACK_RENEW;
}

////////// end timer ///////

static appdata_s *global_ad;

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static Eina_Bool _naviframe_pop_cb(void *data, Elm_Object_Item *it) {
	ui_app_exit();
	return EINA_TRUE;
}

static void _reject_cb(void *data, Evas_Object *obj, void *event_info) {
	if (!obj)
		return;
	evas_object_del(data);
	reject_file();
}

static void _accept_cb(void *data, Evas_Object *obj, void *event_info) {
	if (!obj)
		return;
	evas_object_del(data);
	accept_file();
}

static void _popup_hide_cb(void *data, Evas_Object *obj, void *event_info) {
	if (!obj)
		return;
	elm_popup_dismiss(obj);
}

static void _popup_hide_finished_cb(void *data, Evas_Object *obj,
		void *event_info) {
	if (!obj)
		return;
	evas_object_del(obj);
}

static void popup_title_text_check_button(void *data, Evas_Object *obj,
		void *event_info) {
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

void show_file_req_popup(void) {
	popup_title_text_check_button(global_ad->naviframe, NULL, NULL);
}
void hide_file_req_popup(void) {
	elm_popup_dismiss(global_ad->popup);
}
static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad) {
	Elm_Object_Item *nf_it = NULL;

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
	nf_it = elm_naviframe_item_push(ad->naviframe, "Book Receiver", NULL, NULL,
			ad->content, NULL);
	elm_naviframe_item_pop_cb_set(nf_it, _naviframe_pop_cb, ad->win);

	global_ad = ad;

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool app_create(void *data) {
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);
	initialize_sap();

	// create timer
	timer_p = ecore_timer_add(TIMEOUT_1, _timer1_cb, NULL);
	if (!timer_p) {
		dlog_print(DLOG_ERROR, TAG, "timer creation FAILED");
	}

	return true;
}

static void app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
	if (timer_p) {
		ecore_timer_freeze(timer_p);
	}
}

static void app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */
	if (timer_p) {
		ecore_timer_thaw(timer_p);
	}
}

static void app_terminate(void *data) {
	/* Release all resources. */
	dlog_print(DLOG_INFO, TAG, "free %d strings in my_text", SZ);
	for (int i = 0; i != SZ; ++i) {
		free(my_text[i]);
	}
	if (timer_p) {
		ecore_timer_del(timer_p);
	}
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
			&locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

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
