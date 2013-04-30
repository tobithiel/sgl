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

#include <sgl.h>
#include <sgl_linux_x11.h>

GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

int default_x11_event_mask = KeyPressMask | KeyReleaseMask | /*ButtonPressMask | ButtonReleaseMask | */ExposureMask;

sgl_window_t *sgl_init(sgl_window_settings_t *ws) {
	// so we don't need to care about thread-safety of Xlib
	int s = XInitThreads();
	if(s == 0) {
		printf("could not enable Xlib multi-threading.\n");
		return NULL;
	}
	
	sgl_window_t *w = calloc(1, sizeof(sgl_window_t));
	if(w == NULL) {
		printf("could not allocate memory for window structure.\n");
		return NULL;
	}
	w->eq = queue_create();
	
	w->mtx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(w->mtx, NULL);
	
	w->dpy = XOpenDisplay(NULL);
	if(w->dpy == NULL) {
		printf("cannot connect to X server!\n");
		return NULL;
	}
	
	Window root = XDefaultRootWindow(w->dpy);
	w->vi = glXChooseVisual(w->dpy, 0, att);
	if(w->vi == NULL) {
		printf("could not find visual with your parameters.\n");
		return NULL;
	}
	
	w->cmap = XCreateColormap(w->dpy, root, w->vi->visual, AllocNone);
	
	XSetWindowAttributes swa;
	swa.colormap = w->cmap;
	swa.event_mask = default_x11_event_mask;
	
	w->w = XCreateWindow(w->dpy, root, 0, 0, ws->width, ws->height, 0, w->vi->depth, InputOutput, w->vi->visual, CWColormap | CWEventMask, &swa);
	if(!(w->w)) {
		printf("failed to create window.\n");
		return NULL;
	}
	
	w->wmDeleteMessage = XInternAtom(w->dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(w->dpy, w->w, &(w->wmDeleteMessage), 1);
	XSelectInput(w->dpy, root, SubstructureNotifyMask);
	
	XStoreName(w->dpy, w->w, ws->title);
	XMapWindow(w->dpy, w->w);
	
	w->glc = glXCreateContext(w->dpy, w->vi, NULL, GL_TRUE);
	if(w->glc == NULL) {
		printf("failed to create opengl context.\n");
		return NULL;
	}
	glXMakeCurrent(w->dpy, w->w, w->glc);
	
	sgl_swap_buffers(w); // needed so that window is really shown, in some cases
	
	return w;
}

int8_t sgl_translate_event(sgl_event_t *sex, XEvent *xe, sgl_window_t *w) {
	sgl_event_t *se = (sgl_event_t *)sex;
	switch(xe->type) {
		case ClientMessage:
			if(xe->xclient.window == w->w && (unsigned)xe->xclient.data.l[0] == w->wmDeleteMessage) {
				se->type = SGL_WINDOW_CLOSE;
			} else {
				return 0;
			}
			break;
			
		case ConfigureNotify:
			if(xe->xconfigure.window == w->w && (w->width != xe->xconfigure.width || w->height != xe->xconfigure.height)) {
				w->width = xe->xconfigure.width;
				w->height = xe->xconfigure.height;
				se->type = SGL_WINDOW_RESIZE;
			} else {
				return 0;
			}
			break;
			
		case DestroyNotify:
			if(xe->xdestroywindow.window == w->w)
				se->type = SGL_WINDOW_CLOSED;
			else
				return 0;
			break;
			
		case Expose:
			se->type = SGL_WINDOW_EXPOSE;
			break;
			
		case KeyPress:
			se->type = SGL_KEY_PRESS;
			if(0 == sgl_translate_key(&(se->key), &(xe->xkey)))
				return 0;
			break;
			
		case KeyRelease:
			se->type = SGL_KEY_RELEASE;
			if(0 == sgl_translate_key(&(se->key), &(xe->xkey)))
				return 0;
			break;
			
		case ButtonPress:
			se->type = SGL_MOUSE_PRESS;
			break;
			
		case ButtonRelease:
			se->type = SGL_MOUSE_RELEASE;
			break;
			
		case MotionNotify:
			se->type = SGL_MOUSE_MOVE;
			break;
			
		case EnterNotify:
			se->type = SGL_MOUSE_ENTER;
			break;
			
		case LeaveNotify:
			se->type = SGL_MOUSE_LEAVE;
			break;
			
		default:
			return 0;
	}
	
	return 1;
}

void sgl_check_new_events(sgl_window_t *w) {
	XEvent xe;
	sgl_event_t *e;
	while(XPending(w->dpy) > 0) {
		XNextEvent(w->dpy, &xe);
		e = malloc(sizeof(sgl_event_t));
		if(0 != sgl_translate_event(e, &xe, w)) {
			queue_put(w->eq, e);
		} else {
			free(e);
		}
	}
}

void sgl_check_new_events_wait(sgl_window_t *w) {
	XEvent xe;
	sgl_event_t *e;
	int new_events = 0;
	
	while(XPending(w->dpy) > 0 || new_events == 0) {
		XNextEvent(w->dpy, &xe);
		e = malloc(sizeof(sgl_event_t));
		if(0 != sgl_translate_event(e, &xe, w)) {
			queue_put(w->eq, e);
			new_events++;
		} else {
			free(e);
		}
	}
}

sgl_event_t *sgl_wait_event(sgl_window_t *w) {
	if(0 != pthread_mutex_lock(w->mtx)) {
		return NULL;
	}
	if(queue_empty(w->eq)) {
		sgl_check_new_events_wait(w);
	} else
		sgl_check_new_events(w);
	sgl_event_t *e = NULL;
	queue_get(w->eq, (void **)&e);
	pthread_mutex_unlock(w->mtx);
	return e;
}


sgl_event_t *sgl_check_event(sgl_window_t *w) {
	if(0 != pthread_mutex_lock(w->mtx)) {
		return NULL;
	}
	sgl_check_new_events(w);
	sgl_event_t *e = NULL;
	queue_get(w->eq, (void **)&e);
	pthread_mutex_unlock(w->mtx);
	return e;
}

void sgl_swap_buffers(sgl_window_t *w) {
	glXSwapBuffers(w->dpy, w->w);
}

void sgl_clean(sgl_window_t *w) {
	// first destroy window, so that waiting event threads, get the destory msg, before the queue is destroyed
	glXMakeCurrent(w->dpy, None, NULL); // release context
	glXDestroyContext(w->dpy, w->glc);
	XDestroyWindow(w->dpy, w->w);
	XFreeColormap(w->dpy, w->cmap);
	XFree(w->vi);
	printf("destroyed window\n");
	
	// finally destroy lock and queue
	while(EBUSY == pthread_mutex_destroy(w->mtx))
		usleep(100 * 1000);
	free(w->mtx);
	queue_destroy_complete(w->eq, NULL);
	
	// wait until now so that DestroyNotify is delivered
	XCloseDisplay(w->dpy);
	
	free(w);
}

int8_t sgl_translate_key(sgl_event_key_t *ke, XKeyEvent *xk) {
	// check modifier
	if(xk->state & Mod1Mask)
		ke->modifier |= SGL_K_ALT;
	if(xk->state & Mod2Mask)
		ke->modifier |= SGL_K_NUMLOCK;
	if(xk->state & Mod4Mask)
		ke->modifier |= SGL_K_OS;
	if(xk->state & Mod5Mask)
		ke->modifier |= SGL_K_ALTGR;
	if(xk->state & ShiftMask)
		ke->modifier |= SGL_K_SHIFT;
	if(xk->state & ControlMask)
		ke->modifier |= SGL_K_CONTROL;
	if(xk->state & LockMask)
		ke->modifier |= SGL_K_CAPSLOCK;
		
	// check pressed key
	KeySym ks = XLookupKeysym(xk, 0);
	
	// special keys
	if(ks == XK_space)
		ke->key = SGL_K_SPACE;
	else if(ks == XK_BackSpace)
		ke->key = SGL_K_BACKSPACE;
	else if(ks == XK_Return)
		ke->key = SGL_K_RETURN;
	else if(ks == XK_Escape)
		ke->key = SGL_K_ESCAPE;
	else if(ks == XK_Delete)
		ke->key = SGL_K_DELETE;
	// direction keys
	else if(ks == XK_Up)
		ke->key = SGL_K_UP;
	else if(ks == XK_Down)
		ke->key = SGL_K_DOWN;
	else if(ks == XK_Left)
		ke->key = SGL_K_LEFT;
	else if(ks == XK_Right)
		ke->key = SGL_K_RIGHT;
	// numbers, we don't differentiate between NumLock and Normal
	else if(ks == XK_KP_0 || ks == XK_0)
		ke->key = SGL_K_0;
	else if(ks == XK_KP_1 || ks == XK_1)
		ke->key = SGL_K_1;
	else if(ks == XK_KP_2 || ks == XK_2)
		ke->key = SGL_K_2;
	else if(ks == XK_KP_3 || ks == XK_3)
		ke->key = SGL_K_3;
	else if(ks == XK_KP_4 || ks == XK_4)
		ke->key = SGL_K_4;
	else if(ks == XK_KP_5 || ks == XK_5)
		ke->key = SGL_K_5;
	else if(ks == XK_KP_6 || ks == XK_6)
		ke->key = SGL_K_6;
	else if(ks == XK_KP_7 || ks == XK_7)
		ke->key = SGL_K_7;
	else if(ks == XK_KP_8 || ks == XK_8)
		ke->key = SGL_K_8;
	else if(ks == XK_KP_9 || ks == XK_9)
		ke->key = SGL_K_9;
	// chars, capital or not can be determined through modifiers
	else if(ks == XK_A || ks == XK_a)
		ke->key = SGL_K_A;
	else if(ks == XK_B || ks == XK_b)
		ke->key = SGL_K_B;
	else if(ks == XK_C || ks == XK_c)
		ke->key = SGL_K_C;
	else if(ks == XK_D || ks == XK_d)
		ke->key = SGL_K_D;
	else if(ks == XK_E || ks == XK_e)
		ke->key = SGL_K_E;
	else if(ks == XK_F || ks == XK_f)
		ke->key = SGL_K_F;
	else if(ks == XK_G || ks == XK_g)
		ke->key = SGL_K_G;
	else if(ks == XK_H || ks == XK_h)
		ke->key = SGL_K_H;
	else if(ks == XK_I || ks == XK_i)
		ke->key = SGL_K_I;
	else if(ks == XK_J || ks == XK_j)
		ke->key = SGL_K_J;
	else if(ks == XK_K || ks == XK_k)
		ke->key = SGL_K_K;
	else if(ks == XK_L || ks == XK_l)
		ke->key = SGL_K_L;
	else if(ks == XK_M || ks == XK_m)
		ke->key = SGL_K_M;
	else if(ks == XK_N || ks == XK_n)
		ke->key = SGL_K_N;
	else if(ks == XK_O || ks == XK_o)
		ke->key = SGL_K_O;
	else if(ks == XK_P || ks == XK_p)
		ke->key = SGL_K_P;
	else if(ks == XK_Q || ks == XK_q)
		ke->key = SGL_K_Q;
	else if(ks == XK_R || ks == XK_r)
		ke->key = SGL_K_R;
	else if(ks == XK_S || ks == XK_s)
		ke->key = SGL_K_S;
	else if(ks == XK_T || ks == XK_t)
		ke->key = SGL_K_T;
	else if(ks == XK_U || ks == XK_u)
		ke->key = SGL_K_U;
	else if(ks == XK_V || ks == XK_v)
		ke->key = SGL_K_V;
	else if(ks == XK_W || ks == XK_w)
		ke->key = SGL_K_W;
	else if(ks == XK_X || ks == XK_x)
		ke->key = SGL_K_X;
	else if(ks == XK_Y || ks == XK_y)
		ke->key = SGL_K_Y;
	else if(ks == XK_Z || ks == XK_z)
		ke->key = SGL_K_Z;
	else
		return 0;
		
	return 1;
}
