#ifndef __SGL_MACOSX_COCOA_H__
#define __SGL_MACOSX_COCOA_H__

#include <stdio.h>
#include <stdlib.h>

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <Carbon/Carbon.h> // only for the keyCodes

#define COCOA_FROM_CMD 1

@interface SGLApplicationDelegate : NSObject <NSApplicationDelegate> {
	queue_t *m_eq;
	uint8_t m_sentTermination;
}
- (void)terminate;
- (queue_t *)eventQueue;
- (NSString *)applicationName;
- (void)populateApplicationMenu:(NSMenu *)aMenu;
- (void)populateWindowMenu:(NSMenu *)aMenu;
- (void)populateHelpMenu:(NSMenu *)aMenu;
@end

@interface SGLWindow : NSWindow <NSWindowDelegate> {
	SGLApplicationDelegate *m_ad;
	sgl_window_t *m_w;
}
- (sgl_window_t *)sglwindow;
- (void)setSglWindow:(sgl_window_t *)theW;
@end

@interface SGLView : NSOpenGLView {
	SGLApplicationDelegate *m_ad;
	NSTrackingArea *m_ta;
	sgl_window_t *m_w;
}
- (sgl_window_t *)sglwindow;
- (void)setSglWindow:(sgl_window_t *)theW;
@end

typedef struct {
	SGLApplicationDelegate *ad;
	SGLWindow *w;
	SGLView *v;
	uint8_t fullscreen_transition;
} sgl_window_cocoa_t;

int8_t sgl_translate_event(sgl_event_t *se, NSEvent *ne, sgl_window_t *w);
uint8_t sgl_translate_modifier(NSUInteger modifierFlags);
int8_t sgl_translate_key(sgl_event_key_t *ke, unsigned short kc);
void sgl_translate_mouse_location(sgl_event_mouse_t *me, NSPoint eventLocation, SGLView *v);

#endif /* __SGL_MACOSX_COCOA_H__ */
