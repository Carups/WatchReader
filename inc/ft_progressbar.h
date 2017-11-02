#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <glib.h>

#ifndef __FT_PROGRESSBAR_H__
#define __FT_PROGRESSBAR_H__

#define TAG "FTRceiver"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.filetransferreceiver"
#endif

Evas_Object *create_content(Evas_Object *);
void         show_progress_bar(void);
void         hide_progress_bar(void);
void         set_progress_bar_value(float percentage);

void hide_progress_bar(void);
void show_progress_bar(void);
void show_message_popup(char *message);


#endif /* __FT_PROGRESSBAR_H__ */
