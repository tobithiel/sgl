// OS X: gcc -Wall -Wextra -Wno-unused-parameter -framework Cocoa -framework OpenGL -m64 -g -o example -x objective-c example.c sgl_macosx_cocoa.m ../queue/queue.c

#include <unistd.h>
#include <pthread.h>

#include "sgl.h"

typedef struct {
	uint8_t done;
	uint8_t n;
	sgl_window_t *w;
} state_t;

void *display(void *arg) {
	state_t *s = (state_t *)arg;
	
	sgl_window_settings_t ws;
	ws.width = 640;
	ws.height = 480;
	ws.title = "SGL Window";
	s->w = sgl_create_window(&ws);
	
	sgl_make_current(s->w);
	// Set color and depth clear value
	glClearDepth(1.f);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	
	// Enable Z-buffer read and write
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
		
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
	}
	
	sgl_close_window(s->w);
	
	return NULL;
}

int verbose_event_handling(void) {
	int close = 0;
	sgl_event_t *e = sgl_wait_event();
	if(e->type == SGL_WINDOW_CLOSE) {
		printf("got window close\n");
		close = 1;
	} else if(e->type == SGL_WINDOW_CLOSED) {
		printf("got window closed\n");
		close = 1;
	} else if(e->type == SGL_WINDOW_RESIZE) {
		printf("got window resize\n");
	} else if(e->type == SGL_WINDOW_EXPOSE) {
		printf("got window expose\n");
	} else if(e->type == SGL_MOUSE_DOWN) {
		printf("got mouse down\n");
	} else if(e->type == SGL_MOUSE_UP) {
		printf("got mouse up\n");
	} else if(e->type == SGL_MOUSE_MOVE) {
		printf("got mouse move\n");
	} else if(e->type == SGL_MOUSE_ENTER) {
		printf("got mouse enter\n");
	} else if(e->type == SGL_MOUSE_LEAVE) {
		printf("got mouse leave\n");
	} else if(e->type == SGL_KEY_DOWN) {
		printf("got key press\n");
	} else if(e->type == SGL_KEY_UP) {
		printf("got key release\n");
	} else {
		printf("unknown event\n");
	}
	free(e);
	
	return close;
}

int main(int argc, char *argv[]) {
	state_t s;
	s.n = 1;
	s.done = 0;
	
	state_t s2;
	s.n = 2;
	s2.done = 0;
	
	sgl_init();
	
	pthread_t t;
	pthread_create(&t, NULL, display, &s);
	pthread_t t2;
	pthread_create(&t2, NULL, display, &s2);
	
	int i = 0;
	while(s.done == 0 || s2.done == 0) {
		int close = verbose_event_handling();
		if (close == 1) {
			s.done = 1;
			s2.done = 1;
		}
		
		usleep(10000);
		i++;
		/*if(i == 15) {
			s.done = 1;
		} else if(i == 35) {
			s2.done = 1;
			usleep(20000);
		}*/
	}
	
	sgl_clean();
	
	return 0;
}
