#ifndef __SGL_LINUX_X11_H__
#define __SGL_LINUX_X11_H__

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "../queue/queue.h"

typedef struct {
	Queue *eq;
	Display *dpy;
	Window w;
	uint16_t width;
	uint16_t height;
	XVisualInfo *vi;
	Colormap cmap;
	Atom wmDeleteMessage;
	GLXContext glc;
	pthread_mutex_t *mtx;
} sgl_window_t;

#include "sgl.h"

void sgl_check_new_events(sgl_window_t *w);
void sgl_check_new_events_wait(sgl_window_t *w);
int8_t sgl_translate_event(sgl_event_t *se, XEvent *xe, sgl_window_t *w);
int8_t sgl_translate_key(sgl_event_key_t *ke, XKeyEvent *ks);

#endif /* __SGL_LINUX_X11_H__ */
