#ifndef __SGL_LINUX_X11_H__
#define __SGL_LINUX_X11_H__

/**
  * Copyright (C) 2011 by Tobias Thiel
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.
  */

#include <stdio.h>
#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

typedef struct {
	Display *dpy;
	// window array
	uint8_t arr_size;
	uint8_t arr_used;
	Window *xwarr;
	sgl_window_t **swarr;
} sgl_env_x11_t;

typedef struct {
	Display *dpy2;
	Window w;
	uint16_t width;
	uint16_t height;
	XVisualInfo *vi;
	Colormap cmap;
	Atom wmDeleteMessage;
	GLXContext glc;
} sgl_window_x11_t;

sgl_env_x11_t *get_env_data(sgl_env_t *);
sgl_window_x11_t *get_window_data(sgl_window_t *);
sgl_window_t *get_sgl_window_from_x11(sgl_env_x11_t *edata, Window w);
void sgl_check_new_events(sgl_env_t *w);
void sgl_check_new_events_wait(sgl_env_t *w);
int8_t sgl_translate_event(sgl_event_t *se, XEvent *xe, sgl_env_t *e);
int8_t sgl_translate_key(sgl_event_key_t *ke, XKeyEvent *ks);

#endif /* __SGL_LINUX_X11_H__ */
