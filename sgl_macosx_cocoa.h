#ifndef __SGL_MACOSX_COCOA_H__
#define __SGL_MACOSX_COCOA_H__

#include <stdio.h>
#include <stdlib.h>

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <Carbon/Carbon.h> // only for the keyCodes

#import "../queue/queue.h"

#define COCOA_FROM_CMD 1

typedef struct sgl_window_s sgl_window_t;

@interface SGLApplicationDelegate : NSObject <NSApplicationDelegate> {
	Queue *m_eq;
	uint8_t m_sentTermination;
}
- (void)terminate;
- (Queue *)eventQueue;
- (NSString *)applicationName;
- (void)populateApplicationMenu:(NSMenu *)aMenu;
- (void)populateWindowMenu:(NSMenu *)aMenu;
- (void)populateHelpMenu:(NSMenu *)aMenu;
@end

@interface SGLWindow : NSWindow <NSWindowDelegate> {
	SGLApplicationDelegate *m_ad;
}
@end

@interface SGLView : NSOpenGLView {
	SGLApplicationDelegate *m_ad;
	NSTrackingArea *m_ta;
}
@end

struct sgl_window_s {
	SGLApplicationDelegate *ad;
	SGLWindow *w;
	SGLView *v;
};

int8_t sgl_translate_event(sgl_event_t *se, NSEvent *ne, SGLView *v);
int8_t sgl_translate_key(sgl_event_key_t *ke, unsigned short kc);
void sgl_translate_mouse_location(sgl_event_mouse_t *me, NSPoint eventLocation, SGLView *v);

#endif /* __SGL_MACOSX_COCOA_H__ */
