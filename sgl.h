#ifndef __SGL_H__
#define __SGL_H__

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

#include <stdlib.h>
#include <stdint.h>

#include <queue.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t no;
	uint16_t width;
	uint16_t height;
	uint16_t depth;
	uint8_t description_len;
	char *description;
} sgl_screen_t;

typedef struct {
	uint8_t fullscreen;
	uint8_t fullscreen_screen;
	uint8_t fullscreen_blanking;
	uint16_t width;
	uint16_t height;
	char *title;
} sgl_window_settings_t;

typedef struct {
	sgl_window_settings_t *settings;
	// implementation specific data
	void *impldata;
} sgl_window_t;

typedef enum {
	// window is made active, needs redraw
	SGL_WINDOW_EXPOSE = 0,
	// window is resized, creation is also a resize
	SGL_WINDOW_RESIZE = 1,
	// window should be closed (e.g. user pressed close button), cancelable
	SGL_WINDOW_CLOSE = 2,
	// window is closed
	SGL_WINDOW_CLOSED = 3,
	// key is pressed down
	SGL_KEY_DOWN = 4,
	// key is released
	SGL_KEY_UP = 5,
	// mouse button is pressed down
	SGL_MOUSE_DOWN = 6,
	// mouse button is released
	SGL_MOUSE_UP = 7,
	// mouse is being moved
	SGL_MOUSE_MOVE = 8,
	// mouse enters window/area
	SGL_MOUSE_ENTER = 9,
	// mouse leaves window/area
	SGL_MOUSE_LEAVE = 10
} sgl_event_types_t;

typedef enum {
	SGL_K_SHIFT = 1,
	SGL_K_CONTROL = 2,
	SGL_K_CAPSLOCK = 4,
	SGL_K_NUMPAD = 8,
	SGL_K_ALT = 16,
	SGL_K_ALTGR = 32,
	SGL_K_OS = 64 // also meta or super key
} sgl_keyboard_modifier_e;

typedef enum {
	SGL_K_SPACE,
	SGL_K_BACKSPACE,
	SGL_K_RETURN,
	SGL_K_DELETE,
	SGL_K_ESC,
	SGL_K_UP,
	SGL_K_DOWN,
	SGL_K_LEFT,
	SGL_K_RIGHT,
	SGL_K_0,
	SGL_K_1,
	SGL_K_2,
	SGL_K_3,
	SGL_K_4,
	SGL_K_5,
	SGL_K_6,
	SGL_K_7,
	SGL_K_8,
	SGL_K_9,
	SGL_K_A,
	SGL_K_B,
	SGL_K_C,
	SGL_K_D,
	SGL_K_E,
	SGL_K_F,
	SGL_K_G,
	SGL_K_H,
	SGL_K_I,
	SGL_K_J,
	SGL_K_K,
	SGL_K_L,
	SGL_K_M,
	SGL_K_N,
	SGL_K_O,
	SGL_K_P,
	SGL_K_Q,
	SGL_K_R,
	SGL_K_S,
	SGL_K_T,
	SGL_K_U,
	SGL_K_V,
	SGL_K_W,
	SGL_K_X,
	SGL_K_Y,
	SGL_K_Z
} sgl_keyboard_e;

typedef enum {
	SGL_MOUSE_LEFT,
	SGL_MOUSE_RIGHT
} sgl_mouse_button_e;

typedef struct {
	sgl_keyboard_e key;
	uint8_t modifier;
} sgl_event_key_t;

typedef struct {
	sgl_mouse_button_e button;
	uint8_t doubleclick;
	float x;
	float y;
} sgl_event_mouse_t;

typedef struct {
	sgl_event_types_t type;
	sgl_window_t *window;
	sgl_event_key_t key;
	sgl_event_mouse_t mouse;
} sgl_event_t;

#if defined(__APPLE__)
// Mac OS X windows
#include "sgl_macosx_cocoa.h"
#elif defined(linux) || defined(__linux)
// Linux windows
#include "sgl_linux_x11.h"
//#elif defined(_WIN32) || defined(__WIN32__)
// Windows windows
// TODO windows implementation
#else
#error "Unknown and unsupported operating system"
#endif

/*
 * initialize library
 * has to be called from the main thread!
 * not thread-safe, only call this once, before you begin
 */
void sgl_init(void);

/*
 * returns the number of screens in the system.
 * the argument will contain an array of this size.
 * the first entry will be the main screen.
 if the argument is NULL only the number of screens will be returned.
 */
uint8_t sgl_get_screens(sgl_screen_t **screens);

/*
 * creates a window with the given settings
 * returns NULL if error occured
 */
sgl_window_t *sgl_window_create(sgl_window_settings_t *);

/*
 * returns the settings of the given window
 */
sgl_window_settings_t *sgl_window_settings_get(sgl_window_t *);

/*
 * chnages the settings of a given window
 * not thread-safe
 */
sgl_window_t *sgl_window_settings_change(sgl_window_t *, sgl_window_settings_t *);

/*
 * blocks until an event occurs
 * you have to release the event when you are done with it using free
 * not thread-safe
 * TODO has/should be called from main thread
 * returns NULL if there is no event, otherwise an event
 */
sgl_event_t *sgl_event_wait(void);

/*
 * checks if an event occured
 * you have to release the event when you are done with it using free
 * not thread-safe
 * TODO has/should be called from main thread
 * returns NULL if there is no event, otherwise an event
 */
sgl_event_t *sgl_event_check(void);

/*
 * performs a buffer swap in the given window
 * not thread-safe for the same window
 */
void sgl_swap_buffers(sgl_window_t *);

/*
 * makes the OpenGL context of the window current in the thread from which is called
 */
void sgl_make_current(sgl_window_t *);

/*
 * the given window will be closed and its memory released
 * not thread-safe for the same window, only call this once, when you are done with the window
 */
void sgl_window_close(sgl_window_t *);

/*
 * wake threads which are waiting for events
 * starts termination of application
 * not necessary if you don't need to wake threads waiting at sgl_wait_event
 * the effect can only be used once
 */
void sgl_terminate(void);

/*
 * releases memory allocated in sgl_init
 * not thread-safe, only call this once, when you are done
 */
void sgl_clean(void);

#ifdef __cplusplus
}
#endif

#endif /* __SGL_H__ */
