#ifndef __DVBAPI_DEBUG_H__
#define __DVBAPI_DEBUG_H__

#define DVBAPI_DEBUG_AUDIO		0
#define DVBAPI_DEBUG_VIDEO		1
#define DVBAPI_DEBUG_DEMUX		2
#define DVBAPI_DEBUG_PLAYBACK	3
#define DVBAPI_DEBUG_CEC		4
#define DVBAPI_DEBUG_INIT		5
#define DVBAPI_DEBUG_CA			6
#define DVBAPI_DEBUG_RECORD		7
#define DVBAPI_DEBUG_ALL		((1<<8)-1)

extern int debuglevel;

void _dvbapi_debug(int facility, const void *, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void _dvbapi_info(int facility, const void *, const char *fmt, ...)  __attribute__((format(printf, 3, 4)));

void dvbapi_debug_init(void);
void dvbapi_set_threadname(const char *name);

#endif // __DVBAPI_DEBUG_H__
