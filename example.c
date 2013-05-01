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

#include <unistd.h>
#include <pthread.h>

#include "sgl.h"

typedef struct {
	uint8_t done;
	uint8_t n;
	sgl_env_t *e;
	sgl_window_t *w;
} state_t;

void *display(void *arg) {
	state_t *s = (state_t *)arg;
	
	sgl_window_settings_t ws;
	ws.fullscreen = 0;
	ws.width = 640;
	ws.height = 480;
	ws.title = "SGL Window";
	s->w = sgl_window_create(s->e, &ws);
	
	sgl_make_current(s->w);
	// Set color and depth clear value
	glClearDepth(1.f);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	
	// Enable Z-buffer read and write
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	
	int i = 0;
	while(s->done == 0) {
		// Setup a perspective projection
		glViewport(0, 0, 640, 480);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-2.0f, 2.0f, -2.0f, 2.0f, -5.0f, 5.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		//printf("glrendering ...\n");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//printf("... partially done\n");
		
		glBegin(GL_TRIANGLES);
		glVertex3f(-1.0f, -0.5f, -4.0f);    // lower left vertex
		glVertex3f( 1.0f, -0.5f, -4.0f);    // lower right vertex
		glVertex3f( 0.0f,  0.5f, -4.0f);    // upper vertex
		glEnd();
		//printf("... done\n");
		
		//printf("swapping buffers ...\n");
		sgl_swap_buffers(s->w);
		//printf("... done\n");
		
		usleep(10000);
		i += 1;
	}
	
	sgl_window_close(s->w);
	
	return NULL;
}

sgl_event_t *verbose_event_handling(sgl_env_t *e) {
	sgl_event_t *ev = sgl_event_check(e);
	if (ev == NULL)
		return NULL;
	if(ev->type == SGL_WINDOW_CLOSE) {
		printf("got window close\n");
	} else if(ev->type == SGL_WINDOW_CLOSED) {
		printf("got window closed\n");
	} else if(ev->type == SGL_WINDOW_RESIZE) {
		sgl_window_settings_t *ws = sgl_window_settings_get(ev->window);
		printf("got window resize (%d/%d)\n", ws->width, ws->height);
	} else if(ev->type == SGL_WINDOW_EXPOSE) {
		printf("got window expose\n");
	} else if(ev->type == SGL_MOUSE_DOWN) {
		printf("got mouse down\n");
	} else if(ev->type == SGL_MOUSE_UP) {
		printf("got mouse up\n");
	} else if(ev->type == SGL_MOUSE_MOVE) {
		printf("got mouse move\n");
	} else if(ev->type == SGL_MOUSE_ENTER) {
		printf("got mouse enter\n");
	} else if(ev->type == SGL_MOUSE_LEAVE) {
		printf("got mouse leave\n");
	} else if(ev->type == SGL_KEY_DOWN) {
		printf("got key press\n");
	} else if(ev->type == SGL_KEY_UP) {
		printf("got key release\n");
	} else {
		printf("unknown event\n");
	}
	
	return ev;
}

int main(int argc, char *argv[]) {
	state_t s;
	s.n = 1;
	s.done = 0;
	
	s.e = sgl_init();

	sgl_screen_t *screens;
	uint8_t num_screens = sgl_get_screens(s.e, &screens);
	printf("Screens:\n");
	int i;
	for (i = 0; i < num_screens; i++) {
		printf("\tScreen %d: %dx%d, %dbpp, %s\n", screens[i].no, screens[i].width, screens[i].height, screens[i].depth, screens[i].description);
	}
	printf("\n");
	
	pthread_t t;
	pthread_create(&t, NULL, display, &s);
	
	i = 0;
	while(s.done == 0) {
		sgl_event_t *e = verbose_event_handling(s.e);
		if (e != NULL && e->type == SGL_WINDOW_CLOSED) {
			s.done = 1;
		} else if (e != NULL && e->type == SGL_KEY_DOWN && (e->key.key == SGL_K_Q || e->key.key == SGL_K_ESC)) {
			s.done = 1;
		} else if (e != NULL && e->type == SGL_KEY_DOWN && e->key.key == SGL_K_F) {
			printf("toogling fullscreen\n");
			sgl_window_settings_t *ws = sgl_window_settings_get(s.w);
			ws->fullscreen = !(ws->fullscreen);
			ws->fullscreen_screen = 0;
			ws->fullscreen_blanking = 0;
			sgl_window_settings_change(s.w, ws);
			free(ws);
		}
		free(e);

		usleep(100);
		i++;
		//if(i == 150000) {
			//s.done = 1;
		//}
	}
	
	pthread_join(t, NULL);

	sgl_clean(s.e);
	
	return 0;
}
