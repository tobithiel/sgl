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
#include <string.h>

#include <sgl.h>
#include <sgl_linux_x11.h>

GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

int default_x11_event_mask = KeyPressMask | KeyReleaseMask | /*ButtonPressMask | ButtonReleaseMask | */ExposureMask;

sgl_env_x11_t *get_env_data(sgl_env_t *e) {
	return (sgl_env_x11_t *)e->impldata;
}

sgl_window_x11_t *get_window_data(sgl_window_t *w) {
	return (sgl_window_x11_t *)w->impldata;
}

sgl_window_t *get_sgl_window_from_x11(sgl_env_x11_t *edata, Window w) {
	int i;
	for (i = 0; i < edata->arr_used; i++) {
		if (edata->xwarr[i] == w)
			return edata->swarr[i];
	}
	printf("Could not find sgl window for window %lu!\n", w);
	return NULL;
}

void sgl_x11_enter_fullscreen(sgl_window_t *w) {
	printf("entering fullscreen\n");
	sgl_window_x11_t *wdata = get_window_data(w);
	XEvent xev;
	Atom wm_state = XInternAtom(wdata->dpy2, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(wdata->dpy2, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = wdata->w;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(wdata->dpy2, DefaultRootWindow(wdata->dpy2), False, SubstructureNotifyMask, &xev);
	w->settings->fullscreen = 1;
}

void sgl_x11_leave_fullscreen(sgl_window_t *w) {
	printf("leaving fullscreen\n");
	sgl_window_x11_t *wdata = get_window_data(w);
	XEvent xev;
	Atom wm_state = XInternAtom(wdata->dpy2, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(wdata->dpy2, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = wdata->w;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 0;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(wdata->dpy2, DefaultRootWindow(wdata->dpy2), False, SubstructureNotifyMask, &xev);
	w->settings->fullscreen = 0;
}

sgl_env_t *sgl_init(void) {
	// so we don't need to care about thread-safety of Xlib
	XInitThreads();

	sgl_env_t *e = calloc(1, sizeof(sgl_env_t));
	if(e == NULL) {
		printf("could not allocate memory for window structure.\n");
		return NULL;
	}
	e->eq = queue_create();

	sgl_env_x11_t *edata = calloc(1, sizeof(sgl_env_x11_t));
	if(edata == NULL) {
		printf("could not allocate memory for window structure.\n");
		return NULL;
	}

	edata->dpy = XOpenDisplay(NULL);
	if(edata->dpy == NULL) {
		printf("cannot connect to X server!\n");
		return NULL;
	}
	edata->arr_size = 10;
	edata->arr_used = 0;
	edata->xwarr = calloc(edata->arr_size, sizeof(Window));
	edata->swarr = calloc(edata->arr_size, sizeof(sgl_window_t *));
	if (edata->xwarr == NULL || edata->swarr == NULL) {
		printf("cannot create window arrays!\n");
		return NULL;
	}
	e->impldata = edata;
	return e;
}

uint8_t sgl_get_screens(sgl_env_t *e, sgl_screen_t **screens) {
	// TODO returns one big screen instead of two
	sgl_env_x11_t *edata = get_env_data(e);
	int i, num_screens = XScreenCount(edata->dpy);
	printf("num screens: %d\n", num_screens);
	*screens = calloc(num_screens, sizeof(sgl_screen_t));
	for (i = 0; i < num_screens; i++) {
		Screen *s = XScreenOfDisplay(edata->dpy, i);
		(*screens)[i].no = i;
		(*screens)[i].width = XWidthOfScreen(s);
		(*screens)[i].height = XHeightOfScreen(s);
		(*screens)[i].depth = XDefaultDepthOfScreen(s);
		(*screens)[i].description_len = 0;
		(*screens)[i].description = NULL;
	}
	return num_screens;
}

sgl_window_t *sgl_window_create(sgl_env_t *e, sgl_window_settings_t *ws) {
	sgl_env_x11_t *edata = get_env_data(e);
	if (edata->arr_size == edata->arr_used) {
		printf("too many windows. dynamic resize not implemented\n");
		return NULL;
	}

	sgl_window_t *w = calloc(1, sizeof(sgl_window_t));
	if(w == NULL) {
		printf("could not allocate memory for window structure.\n");
		return NULL;
	}
	sgl_window_x11_t *wdata = calloc(1, sizeof(sgl_window_x11_t));
	if(wdata == NULL) {
		printf("could not allocate memory for window structure.\n");
		return NULL;
	}
	sgl_window_settings_t *wscopy = calloc(1, sizeof(sgl_window_settings_t));
	if (wscopy == NULL)
		return NULL;
	memcpy(wscopy, ws, sizeof(sgl_window_settings_t));
	w->settings = wscopy;
	
	wdata->dpy2 = edata->dpy;
	Window root = XDefaultRootWindow(edata->dpy);
	wdata->vi = glXChooseVisual(edata->dpy, 0, att);
	if(wdata->vi == NULL) {
		printf("could not find visual with your parameters.\n");
		return NULL;
	}
	
	wdata->cmap = XCreateColormap(edata->dpy, root, wdata->vi->visual, AllocNone);
	
	XSetWindowAttributes swa;
	swa.colormap = wdata->cmap;
	swa.event_mask = default_x11_event_mask;
	
	wdata->w = XCreateWindow(edata->dpy, root, 0, 0, ws->width, ws->height, 0, wdata->vi->depth, InputOutput, wdata->vi->visual, CWColormap | CWEventMask, &swa);
	if(!(wdata->w)) {
		printf("failed to create window.\n");
		return NULL;
	}
	edata->xwarr[edata->arr_used] = wdata->w;
	edata->swarr[edata->arr_used] = w;
	printf("created window %lu\n", wdata->w);
	edata->arr_used++;
	
	wdata->wmDeleteMessage = XInternAtom(edata->dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(edata->dpy, wdata->w, &(wdata->wmDeleteMessage), 1);
	XSelectInput(edata->dpy, root, SubstructureNotifyMask);
	
	XStoreName(edata->dpy, wdata->w, ws->title);
	XMapWindow(edata->dpy, wdata->w);
	
	wdata->glc = glXCreateContext(edata->dpy, wdata->vi, NULL, GL_TRUE);
	if(wdata->glc == NULL) {
		printf("failed to create opengl context.\n");
		return NULL;
	}
	
	w->impldata = wdata;

	// needed so that window is really shown, in some cases	
	sgl_make_current(w);
	sgl_swap_buffers(w);

	return w;
}

sgl_window_settings_t *sgl_window_settings_get(sgl_window_t *w) {
	sgl_window_settings_t *wscopy = calloc(1, sizeof(sgl_window_settings_t));
	memcpy(wscopy, w->settings, sizeof(sgl_window_settings_t));
	return wscopy;
}

sgl_window_t *sgl_window_settings_change(sgl_window_t *w, sgl_window_settings_t *ws) {
	 // TODO other stuff
	if (w->settings->width != ws->width)
		return NULL;
	if (w->settings->height != ws->height)
		return NULL;
	if (w->settings->title != ws->title)
		return NULL;
	if (w->settings->fullscreen != ws->fullscreen) {
		if (!(w->settings->fullscreen) && ws->fullscreen) {
			sgl_x11_enter_fullscreen(w);
		} else if (w->settings->fullscreen && !(ws->fullscreen)) {
			sgl_x11_leave_fullscreen(w);
		}
	}
	return w;
}

int8_t sgl_translate_event(sgl_event_t *sex, XEvent *xe, sgl_env_t *e) {
	sgl_env_x11_t *edata = get_env_data(e);
	sgl_event_t *se = (sgl_event_t *)sex;
	switch(xe->type) {
		case ClientMessage:
			se->window = get_sgl_window_from_x11(edata, xe->xclient.window);
			//if((unsigned)xe->xclient.data.l[0] == wdata->wmDeleteMessage) {
				//se->type = SGL_WINDOW_CLOSE;
			//} else {
				return 0;
			//}
			break;
			
		case ConfigureNotify:
			se->window = get_sgl_window_from_x11(edata, xe->xconfigure.window);
			if (se->window == NULL)
				return 0;
			sgl_window_x11_t *wdata = get_window_data(se->window);
			if(wdata->width != xe->xconfigure.width || wdata->height != xe->xconfigure.height) {
				wdata->width = xe->xconfigure.width;
				wdata->height = xe->xconfigure.height;
				se->type = SGL_WINDOW_RESIZE;
			} else {
				return 0;
			}
			break;
			
		case DestroyNotify:
			se->window = get_sgl_window_from_x11(edata, xe->xdestroywindow.window);
			se->type = SGL_WINDOW_CLOSED;
			break;
			
		case Expose:
			se->window = get_sgl_window_from_x11(edata, xe->xexpose.window);
			se->type = SGL_WINDOW_EXPOSE;
			break;
			
		case KeyPress:
			se->window = get_sgl_window_from_x11(edata, xe->xkey.window);
			se->type = SGL_KEY_DOWN;
			if(0 == sgl_translate_key(&(se->key), &(xe->xkey)))
				return 0;
			break;
			
		case KeyRelease:
			se->window = get_sgl_window_from_x11(edata, xe->xkey.window);
			se->type = SGL_KEY_UP;
			if(0 == sgl_translate_key(&(se->key), &(xe->xkey)))
				return 0;
			break;
			
		// TODO mouse button stuff
		case ButtonPress:
			se->window = get_sgl_window_from_x11(edata, xe->xbutton.window);
			se->type = SGL_MOUSE_DOWN;
			break;
			
		case ButtonRelease:
			se->window = get_sgl_window_from_x11(edata, xe->xbutton.window);
			se->type = SGL_MOUSE_UP;
			break;
			
		case MotionNotify:
			se->window = get_sgl_window_from_x11(edata, xe->xmotion.window);
			se->type = SGL_MOUSE_MOVE;
			break;
			
		case EnterNotify:
			se->window = get_sgl_window_from_x11(edata, xe->xcrossing.window);
			se->type = SGL_MOUSE_ENTER;
			break;
			
		case LeaveNotify:
			se->window = get_sgl_window_from_x11(edata, xe->xcrossing.window);
			se->type = SGL_MOUSE_LEAVE;
			break;
			
		default:
			return 0;
	}
	
	return 1;
}

void sgl_check_new_events(sgl_env_t *e) {
	sgl_env_x11_t *edata = get_env_data(e);
	XEvent xe;
	sgl_event_t *ev;
	while(XPending(edata->dpy) > 0) {
		XNextEvent(edata->dpy, &xe);
		ev = malloc(sizeof(sgl_event_t));
		if(0 != sgl_translate_event(ev, &xe, e)) {
			queue_put(e->eq, ev);
		} else {
			free(ev);
		}
	}
}

void sgl_check_new_events_wait(sgl_env_t *e) {
	sgl_env_x11_t *edata = get_env_data(e);
	XEvent xe;
	sgl_event_t *ev;
	int new_events = 0;
	
	while(XPending(edata->dpy) > 0 || new_events == 0) {
		XNextEvent(edata->dpy, &xe);
		ev = malloc(sizeof(sgl_event_t));
		if(0 != sgl_translate_event(ev, &xe, e)) {
			queue_put(e->eq, ev);
			new_events++;
		} else {
			free(ev);
		}
	}
}

sgl_event_t *sgl_event_wait(sgl_env_t *e) {
	if(queue_empty(e->eq)) {
		sgl_check_new_events_wait(e);
	} else
		sgl_check_new_events(e);
	sgl_event_t *ev = NULL;
	queue_get(e->eq, (void **)&ev);
	return ev;
}


sgl_event_t *sgl_event_check(sgl_env_t *e) {
	sgl_check_new_events(e);
	sgl_event_t *ev = NULL;
	queue_get(e->eq, (void **)&ev);
	return ev;
}

void sgl_swap_buffers(sgl_window_t *w) {
	sgl_window_x11_t *wdata = get_window_data(w);
	glXSwapBuffers(wdata->dpy2, wdata->w);
}

void sgl_make_current(sgl_window_t *w) {
	sgl_window_x11_t *wdata = get_window_data(w);
	glXMakeCurrent(wdata->dpy2, wdata->w, wdata->glc);
}

void sgl_window_close(sgl_window_t *w) {
	sgl_window_x11_t *wdata = get_window_data(w);

	// first destroy window, so that waiting event threads, get the destroy msg, before the queue is destroyed
	glXMakeCurrent(wdata->dpy2, None, NULL); // release context
	glXDestroyContext(wdata->dpy2, wdata->glc);
	XDestroyWindow(wdata->dpy2, wdata->w);
	XFreeColormap(wdata->dpy2, wdata->cmap);
	XFree(wdata->vi);
	printf("destroyed window\n");
	
	free(w->settings);
	free(w->impldata);
	free(w);
}

void sgl_clean(sgl_env_t *e) {
	sgl_env_x11_t *edata = get_env_data(e);
	free(edata->xwarr);
	free(edata->swarr);
	// wait until now so that DestroyNotify is delivered
	XCloseDisplay(edata->dpy);
	free(edata);
	queue_destroy_complete(e->eq, NULL);
	free(e);
}

int8_t sgl_translate_key(sgl_event_key_t *ke, XKeyEvent *xk) {
	// check modifier
	if(xk->state & Mod1Mask)
		ke->modifier |= SGL_K_ALT;
	if(xk->state & Mod2Mask)
		ke->modifier |= SGL_K_NUMPAD;
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
		ke->key = SGL_K_ESC;
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
