#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <glib.h>
#include <watch_app.h>
#include <watch_app_efl.h>

#ifndef __FT_RECEIVER_H__
#define __FT_RECEIVER_H__

#define TAG "FTRceiver"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.filetransferreceiver"
#endif

void show_file_req_popup(void);
void hide_file_req_popup(void);

#endif /* __FT_RECEIVER_H__ */
