#ifndef __UTILS_H__
#define __UTILS_H__

#if !defined(TAG)
#define TAG "FTRceiver"
#endif

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.filetransferreceiver"
#endif

void reader_start(const char* path);

int serialize_last_book(void);
int deserialize_last_book(void);

#endif /* __UTILS_H__ */
