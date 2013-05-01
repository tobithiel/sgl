#ifndef __SGL_MACOSX_COCOA_H__
#define __SGL_MACOSX_COCOA_H__

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

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <Carbon/Carbon.h> // only for the keyCodes, no linking necessary

// TODO dynamic checking whether cmd program or not
#define COCOA_FROM_CMD 1

@interface SGLApplicationDelegate : NSObject <NSApplicationDelegate> {
	uint8_t m_sentTermination;
}
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
