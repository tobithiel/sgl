// OS X: gcc -Wall -Wextra -Wno-unused-parameter -framework Cocoa -framework OpenGL -m64 -g -o example -x objective-c example.c sgl_macosx_cocoa.m ../queue/queue.c

// gcc -Wall -Wextra -Wno-unused-parameter -framework Cocoa -framework OpenGL -I../ -I../../queue/ -L./lib/queue/ -lqueue_static -m64 -g -o example -x objective-c ../example.c ../sgl_macosx_cocoa.m ../sgl_internal.c

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
	ws.fullscreen = 0;
	ws.width = 640;
	ws.height = 480;
	ws.title = "SGL Window";
	s->w = sgl_window_create(&ws);
	
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
		
		usleep(100);
		i += 1;
	}
	
	sgl_window_close(s->w);
	
	return NULL;
}

sgl_event_t *verbose_event_handling(void) {
	int close = 0;
	sgl_event_t *e = sgl_event_check();
	if (e == NULL)
		return NULL;
	if(e->type == SGL_WINDOW_CLOSE) {
		printf("got window close\n");
		close = 1;
	} else if(e->type == SGL_WINDOW_CLOSED) {
		printf("got window closed\n");
		close = 1;
	} else if(e->type == SGL_WINDOW_RESIZE) {
		sgl_window_settings_t *ws = sgl_window_settings_get(e->window);
		printf("got window resize (%d/%d)\n", ws->width, ws->height);
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
	
	return e;
}

int main(int argc, char *argv[]) {
	state_t s;
	s.n = 1;
	s.done = 0;
	
	sgl_init();

	sgl_screen_t *screens;
	uint8_t num_screens = sgl_get_screens(&screens);
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
		sgl_event_t *e = verbose_event_handling();
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
		}
		free(e);

		usleep(1000);
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
