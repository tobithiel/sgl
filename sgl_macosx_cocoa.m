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

#include <sgl.h>

#include <sgl_macosx_cocoa.h>

sgl_window_cocoa_t *get_window_data(sgl_window_t *w) {
	return (sgl_window_cocoa_t *)w->impldata;
}

@implementation SGLApplicationDelegate

- (id)init {
	if([super init]) {
		m_sentTermination = 0;
		m_eq = queue_create();
	}
	
	return self;
}

- (void)dealloc {
	queue_destroy_complete(m_eq, free);
	[super dealloc];
}

- (queue_t *)eventQueue {
	return m_eq;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification {
	NSApplication *app = [NSApplication sharedApplication];
	NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
	
	NSMenuItem *menuItem;
	NSMenu *submenu;
	
	// The titles of the menu items are for identification purposes only and shouldn't be localized.
	// The strings in the menu bar come from the submenu titles,
	// except for the application menu, whose title is ignored at runtime.
	menuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:@"Apple"];
	[app performSelector:@selector(setAppleMenu:) withObject:submenu];
	[self populateApplicationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	menuItem = [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", @"The Window menu")];
	[self populateWindowMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	[app setWindowsMenu:submenu];
	
	menuItem = [mainMenu addItemWithTitle:@"Help" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Help", @"The Help menu")];
	[self populateHelpMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	[app setMainMenu:mainMenu];
}

- (NSString *)applicationName {
	static NSString * applicationName = nil;
	
	if(applicationName == nil) {
		applicationName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
		if(applicationName == nil) {
			//NSLog(@"[[NSBundle mainBundle] objectForInfoDictionaryKey:@\"CFBundleName\"] == nil");
			applicationName = NSLocalizedString(@"SGLApp", @"The name of this application");
		}
	}
	
	return applicationName;
}

- (void)populateApplicationMenu:(NSMenu *)aMenu {
	NSString * applicationName = [self applicationName];
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), applicationName]
								action:@selector(orderFrontStandardAboutPanel:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Preferences...", nil)
								action:NULL
						 keyEquivalent:@","];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", nil), applicationName]
								action:@selector(hide:)
						 keyEquivalent:@"h"];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
								action:@selector(hideOtherApplications:)
						 keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Show All", nil)
								action:@selector(unhideAllApplications:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), applicationName]
								action:@selector(terminate:)
						 keyEquivalent:@"q"];
	[menuItem setTarget:NSApp];
}

- (void)populateWindowMenu:(NSMenu *)aMenu {
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Minimize", nil)
								action:@selector(performMinimize:)
						 keyEquivalent:@"m"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Zoom", nil)
								action:@selector(performZoom:)
						 keyEquivalent:@""];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Bring All to Front", nil)
								action:@selector(arrangeInFront:)
						 keyEquivalent:@""];
}

- (void)populateHelpMenu:(NSMenu *)aMenu {
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", [self applicationName], NSLocalizedString(@"Help", nil)]
				action:@selector(showHelp:)
				keyEquivalent:@"?"];
	[menuItem setTarget:NSApp];
}

@end

@implementation SGLWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation {
	if([super initWithContentRect:contentRect styleMask:windowStyle backing:bufferingType defer:deferCreation]) {
		m_ad = (SGLApplicationDelegate *)[[NSApplication sharedApplication] delegate];
	}
	
	return self;
}

- (sgl_window_t *)sglwindow {
	return m_w;
}

- (void)setSglWindow:(sgl_window_t *)theW {
	m_w = theW;
}

- (BOOL)windowShouldClose:(id)sender {
	sgl_window_cocoa_t *wdata = get_window_data(m_w);
	if (!(wdata->fullscreen_transition)) {
		sgl_event_t *e = malloc(sizeof(sgl_event_t));
		e->type = SGL_WINDOW_CLOSE;
		e->window = m_w;
		queue_put([m_ad eventQueue], e);
	}
	return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
	sgl_window_cocoa_t *wdata = get_window_data(m_w);
	if (!(wdata->fullscreen_transition)) {
		sgl_event_t *e = malloc(sizeof(sgl_event_t));
		e->type = SGL_WINDOW_CLOSED;
		e->window = m_w;
		queue_put([m_ad eventQueue], e);
	}
}

- (void)windowDidResize:(NSNotification *)notification {
	sgl_event_t *e = malloc(sizeof(sgl_event_t));
	e->type = SGL_WINDOW_RESIZE;
	e->window = m_w;
	queue_put([m_ad eventQueue], e);
}

- (void)windowDidExpose:(NSNotification *)notification {
	sgl_event_t *e = malloc(sizeof(sgl_event_t));
	e->type = SGL_WINDOW_EXPOSE;
	e->window = m_w;
	queue_put([m_ad eventQueue], e);
}

- (BOOL)canBecomeKeyWindow {
	return YES;
}

@end

@implementation SGLView

- (id)initWithFrame:(NSRect)frameRect {
	if([super initWithFrame:frameRect]) {
		m_ad = (SGLApplicationDelegate *)[[NSApplication sharedApplication] delegate];
	}
	
	return self;
}

- (sgl_window_t *)sglwindow {
	return m_w;
}

- (void)setSglWindow:(sgl_window_t *)theW {
	m_w = theW;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)becomeFirstResponder {
	return YES;
}

- (BOOL)resignFirstResponder {
	return YES;
}

- (void)putEventInQueue:(NSEvent *)theEvent {
	sgl_event_t *e = malloc(sizeof(sgl_event_t));
	sgl_translate_event(e, theEvent, m_w);
	queue_put([m_ad eventQueue], e);
}

- (void)keyDown:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)mouseEntered:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)mouseExited:(NSEvent *)theEvent {
	[self putEventInQueue:theEvent];
}

- (void)updateTrackingAreas {
	if(m_ta != nil) {
		[self removeTrackingArea:m_ta];
		[m_ta release];
	}
	m_ta = [[NSTrackingArea alloc] initWithRect:[self frame]
			options: (NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow)
			owner:self userInfo:nil];
	[self addTrackingArea:m_ta];
}

- (void)dealloc {
	[self removeTrackingArea:m_ta];
	[m_ta release];
	[super dealloc];
}

@end

void sgl_init(void) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	NSApplication *app = [NSApplication sharedApplication];
	SGLApplicationDelegate *ad = [[SGLApplicationDelegate alloc] init]; // TODO leaks?
	[app setDelegate:ad];
	
#ifdef COCOA_FROM_CMD
	// we promote our background app to a full Cocoa Application
	// Set the process as a normal application so it can get focus.
	ProcessSerialNumber psn;
	if (!GetCurrentProcess(&psn)) {
		TransformProcessType(&psn, kProcessTransformToForegroundApplication);
		SetFrontProcess(&psn);
	}
#endif
	
	// application launched
	[app finishLaunching];
	[arp release];
}

uint8_t sgl_get_screens(sgl_screen_t **screens) {
	NSArray *tmp = [NSScreen screens];
	uint8_t num_screens = [tmp count];
	if (screens != NULL) {
		*screens = calloc(num_screens, sizeof(sgl_screen_t));

		int i;
		for (i = 0; i < num_screens; i++) {
			NSScreen *s = [tmp objectAtIndex:i];
			(*screens)[i].no = i;
			(*screens)[i].width = s.frame.size.width;
			(*screens)[i].height = s.frame.size.height;
			(*screens)[i].depth = s.depth;
			(*screens)[i].description_len = 0;
			(*screens)[i].description = NULL;
		}
	}

	return num_screens;
}

void sgl_window_fullscreen_enter(sgl_window_t *w, sgl_window_settings_t *newws) {
	if (!(w->settings->fullscreen)) {
		sgl_window_cocoa_t *wdata = get_window_data(w);
		wdata->fullscreen_transition = 1;
		[wdata->w close];
		NSApplicationPresentationOptions options = NSApplicationPresentationHideDock + NSApplicationPresentationHideMenuBar;
		NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
			[NSNumber numberWithUnsignedLong:options], @"NSFullScreenModeApplicationPresentationOptions",
			[NSNumber numberWithBool:(newws->fullscreen_blanking)], @"NSFullScreenModeAllScreens",
			nil];
		[wdata->v enterFullScreenMode:[[NSScreen screens] objectAtIndex:newws->fullscreen_screen]
			withOptions:dict];
		w->settings->fullscreen = 1;
		w->settings->fullscreen_screen = newws->fullscreen_screen;
		w->settings->fullscreen_blanking = newws->fullscreen_blanking;
		wdata->fullscreen_transition = 0;
	}
}

void sgl_window_fullscreen_exit(sgl_window_t *w) {
	if (w->settings->fullscreen) {
		sgl_window_cocoa_t *wdata = get_window_data(w);
		NSApplicationPresentationOptions options = NSApplicationPresentationDefault;
		NSDictionary *dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedLong:options] forKey:@"NSFullScreenModeApplicationPresentationOptions"];
		[wdata->v exitFullScreenModeWithOptions:dict];
		// needed to get focus and key events again
		[[wdata->v window] makeKeyAndOrderFront:wdata->ad];
		[[wdata->v window] makeFirstResponder:wdata->v];
		w->settings->fullscreen = 0;
	}
}

sgl_window_t *sgl_window_create(sgl_window_settings_t *ws) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	sgl_window_t *w = calloc(1, sizeof(sgl_window_t));
	if (w == NULL)
		return NULL;
	sgl_window_cocoa_t *wdata = calloc(1, sizeof(sgl_window_cocoa_t));
	if (wdata == NULL)
		return NULL;
	sgl_window_settings_t *wscopy = calloc(1, sizeof(sgl_window_settings_t));
	if (wscopy == NULL)
		return NULL;
	memcpy(wscopy, ws, sizeof(sgl_window_settings_t));
	w->settings = wscopy;
	
	wdata->ad = [[NSApplication sharedApplication] delegate];
	
	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFANoRecovery,
		0 // very important ...
	};
	NSOpenGLPixelFormat *glpf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
	NSRect viewBounds = NSMakeRect(0, 0, ws->width, ws->height);
	NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
	wdata->w = [[SGLWindow alloc]
			initWithContentRect:viewBounds
			styleMask:styleMask
			backing:NSBackingStoreBuffered
			defer:NO];
	[wdata->w setReleasedWhenClosed:NO];
	wdata->v = [[SGLView alloc] initWithFrame:viewBounds];
	[wdata->v setPixelFormat:glpf];
	[glpf release];
	[wdata->w setSglWindow:w];
	[wdata->v setSglWindow:w];
	
	[wdata->w setContentView:wdata->v];
	[wdata->w setDelegate:wdata->w];
	[wdata->w setTitle:[NSString stringWithCString:ws->title encoding:NSASCIIStringEncoding]];
	[wdata->w setLevel: NSNormalWindowLevel];
	[wdata->w setInitialFirstResponder:wdata->v];
	if (ws->fullscreen)
		sgl_window_fullscreen_enter(w, ws);
	[wdata->w makeKeyAndOrderFront:wdata->ad];
	[wdata->v updateTrackingAreas];
	[arp release];
	
	w->impldata = wdata;
	return w;
}

sgl_window_settings_t *sgl_window_settings_get(sgl_window_t *w) {
	sgl_window_settings_t *wscopy = calloc(1, sizeof(sgl_window_settings_t));
	memcpy(wscopy, w->settings, sizeof(sgl_window_settings_t));
	return wscopy;
}

sgl_window_t *sgl_window_settings_change(sgl_window_t *w, sgl_window_settings_t *ws) {
	// TODO implement other attributes
	if (w->settings->width != ws->width)
		return NULL;
	if (w->settings->height != ws->height)
		return NULL;
	if (w->settings->title != ws->title)
		return NULL;
	if (w->settings->fullscreen != ws->fullscreen) {
		if (!(w->settings->fullscreen) && ws->fullscreen) {
			sgl_window_fullscreen_enter(w, ws);
		} else if (w->settings->fullscreen && !(ws->fullscreen)) {
			sgl_window_fullscreen_exit(w);
		}
	}
	return w;
}

sgl_event_t *sgl_event_wait(void) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	NSApplication *app = [NSApplication sharedApplication];
	SGLApplicationDelegate *ad = (SGLApplicationDelegate *)[app delegate];
	NSDate *past = [NSDate distantPast];
	NSDate *future = [NSDate distantFuture];
	queue_t *q = [ad eventQueue];
	
	// cocoa event loop - get all available events, wait if none
	NSEvent *event = nil;
	while(nil != (event = [app nextEventMatchingMask:NSAnyEventMask untilDate:((queue_empty(q) != 0) ? future : past) inMode:NSDefaultRunLoopMode dequeue:YES]))
		[app sendEvent:event];
	[arp release];
	
	// get a event for the application
	sgl_event_t *e = NULL;
	queue_get(q, (void **)&e);

	return e;
}

sgl_event_t *sgl_event_check(void) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	NSApplication *app = [NSApplication sharedApplication];
	SGLApplicationDelegate *ad = (SGLApplicationDelegate *)[app delegate];
	NSDate *past = [NSDate distantPast];
	
	// cocoa event queue - get all available events
	NSEvent *event = nil;
	while(nil != (event = [app nextEventMatchingMask:NSAnyEventMask untilDate:past inMode:NSDefaultRunLoopMode dequeue:YES]))
		[app sendEvent:event];
	[arp release];
	
	// get a event for the application
	queue_t *q = [ad eventQueue];
	sgl_event_t *e = NULL;
	queue_get(q, (void **)&e);
	
	return e;
}

void sgl_swap_buffers(sgl_window_t *w) {
	//NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	sgl_window_cocoa_t *wdata = get_window_data(w);
	[[wdata->v openGLContext] flushBuffer];
	//[arp release];
}

void sgl_make_current(sgl_window_t *w) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	sgl_window_cocoa_t *wdata = get_window_data(w);
	[[wdata->v openGLContext] makeCurrentContext];
	[arp release];
}

void sgl_window_close(sgl_window_t *w) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	sgl_window_cocoa_t *wdata = get_window_data(w);
	if ([wdata->w isVisible] || [wdata->v isInFullScreenMode]) {
		printf("closing before destroy\n");
		if (w->settings->fullscreen) {
			printf("exiting fullscreen before close\n");
			sgl_window_fullscreen_exit(w);
			usleep(500);
		}
		[wdata->w close];
	}
	[wdata->v release];
	[wdata->w release];
	[arp release];
	free(w->settings);
	free(w);
}

void sgl_clean(void) {
	NSAutoreleasePool *arp = [[NSAutoreleasePool alloc] init];
	SGLApplicationDelegate *ad = [[NSApplication sharedApplication] delegate];
	[[NSApplication sharedApplication] terminate:nil];
	[ad release]; // must be available for [NSApplication terminate:]
	[arp release];
}

int8_t sgl_translate_event(sgl_event_t *se, NSEvent *ne, sgl_window_t *w) {
	se->window = w;
	sgl_window_cocoa_t *wdata = get_window_data(w);
	switch([ne type]) {
		case NSLeftMouseDown:
			se->type = SGL_MOUSE_DOWN;
			se->mouse.button = SGL_MOUSE_LEFT;
			se->mouse.doubleclick = ([ne clickCount] > 0 && [ne clickCount] % 2 == 0);
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSRightMouseDown:
			se->type = SGL_MOUSE_DOWN;
			se->mouse.button = SGL_MOUSE_RIGHT;
			se->mouse.doubleclick = ([ne clickCount] > 0 && [ne clickCount] % 2 == 0);
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSLeftMouseUp:
			se->type = SGL_MOUSE_UP;
			se->mouse.button = SGL_MOUSE_LEFT;
			se->mouse.doubleclick = ([ne clickCount] > 0 && [ne clickCount] % 2 == 0);
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSRightMouseUp:
			se->type = SGL_MOUSE_UP;
			se->mouse.button = SGL_MOUSE_RIGHT;
			se->mouse.doubleclick = ([ne clickCount] > 0 && [ne clickCount] % 2 == 0);
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSMouseMoved:
			se->type = SGL_MOUSE_MOVE;
			se->mouse.doubleclick = 0;
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSMouseEntered:
			se->type = SGL_MOUSE_ENTER;
			se->mouse.doubleclick = 0;
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		case NSMouseExited:
			se->type = SGL_MOUSE_LEAVE;
			se->mouse.doubleclick = 0;
			sgl_translate_mouse_location(&(se->mouse), [ne locationInWindow], wdata->v);
			break;
			
		//case NSLeftMouseDragged:
		//case NSRightMouseDragged:
		//case NSScrollWheel:
		case NSKeyDown:
			se->type = SGL_KEY_DOWN;
			if(0 == sgl_translate_key(&(se->key), [ne keyCode]))
				return 0;
			se->key.modifier = sgl_translate_modifier([ne modifierFlags]);
			break;
			
		case NSKeyUp:
			se->type = SGL_KEY_UP;
			if(0 == sgl_translate_key(&(se->key), [ne keyCode]))
				return 0;
			se->key.modifier = sgl_translate_modifier([ne modifierFlags]);
			break;
			
		//case NSFlagsChanged: // modifier flags changed
		default:
			return 0;
	}
	return 1;
}

void sgl_translate_mouse_location(sgl_event_mouse_t *me, NSPoint eventLocation, SGLView *v) {
	eventLocation = [v convertPoint:eventLocation fromView:nil];
	me->x = eventLocation.x;
	me->y = eventLocation.y;
}

uint8_t sgl_translate_modifier(NSUInteger modifierFlags) {
	uint8_t mods = 0;
	if (modifierFlags & NSAlphaShiftKeyMask)
		mods |= SGL_K_CAPSLOCK;
	if (modifierFlags & NSShiftKeyMask)
		mods |= SGL_K_SHIFT;
	if (modifierFlags & NSControlKeyMask)
		mods |= SGL_K_CONTROL;
	if (modifierFlags & NSAlternateKeyMask)
		mods |= SGL_K_ALT;
	if (modifierFlags & NSCommandKeyMask)
		mods |= SGL_K_OS;
	if (modifierFlags & NSNumericPadKeyMask)
		mods |= SGL_K_NUMPAD;
	return mods;
}

int8_t sgl_translate_key(sgl_event_key_t *ke, unsigned short kc) {
	// special keys
	if(kc == kVK_Space)
		ke->key = SGL_K_SPACE;
	else if(kc == kVK_Delete)
		ke->key = SGL_K_BACKSPACE;
	else if(kc == kVK_Return)
		ke->key = SGL_K_RETURN;
	else if(kc == kVK_Escape)
		ke->key = SGL_K_ESC;
	else if(kc == kVK_ForwardDelete)
		ke->key = SGL_K_DELETE;
	// direction keys
	else if(kc == kVK_UpArrow)
		ke->key = SGL_K_UP;
	else if(kc == kVK_DownArrow)
		ke->key = SGL_K_DOWN;
	else if(kc == kVK_LeftArrow)
		ke->key = SGL_K_LEFT;
	else if(kc == kVK_RightArrow)
		ke->key = SGL_K_RIGHT;
	// numbers, we don't differentiate between NumLock and Normal (modifiers)
	else if(kc == kVK_ANSI_Keypad0 || kc == kVK_ANSI_0)
		ke->key = SGL_K_0;
	else if(kc == kVK_ANSI_Keypad1 || kc == kVK_ANSI_1)
		ke->key = SGL_K_1;
	else if(kc == kVK_ANSI_Keypad2 || kc == kVK_ANSI_2)
		ke->key = SGL_K_2;
	else if(kc == kVK_ANSI_Keypad3 || kc == kVK_ANSI_3)
		ke->key = SGL_K_3;
	else if(kc == kVK_ANSI_Keypad4 || kc == kVK_ANSI_4)
		ke->key = SGL_K_4;
	else if(kc == kVK_ANSI_Keypad5 || kc == kVK_ANSI_5)
		ke->key = SGL_K_5;
	else if(kc == kVK_ANSI_Keypad6 || kc == kVK_ANSI_6)
		ke->key = SGL_K_6;
	else if(kc == kVK_ANSI_Keypad7 || kc == kVK_ANSI_7)
		ke->key = SGL_K_7;
	else if(kc == kVK_ANSI_Keypad8 || kc == kVK_ANSI_8)
		ke->key = SGL_K_8;
	else if(kc == kVK_ANSI_Keypad9 || kc == kVK_ANSI_9)
		ke->key = SGL_K_9;
	// chars, capital or not can be determined through modifiers
	else if(kc == kVK_ANSI_A)
		ke->key = SGL_K_A;
	else if(kc == kVK_ANSI_B)
		ke->key = SGL_K_B;
	else if(kc == kVK_ANSI_C)
		ke->key = SGL_K_C;
	else if(kc == kVK_ANSI_D)
		ke->key = SGL_K_D;
	else if(kc == kVK_ANSI_E)
		ke->key = SGL_K_E;
	else if(kc == kVK_ANSI_F)
		ke->key = SGL_K_F;
	else if(kc == kVK_ANSI_G)
		ke->key = SGL_K_G;
	else if(kc == kVK_ANSI_H)
		ke->key = SGL_K_H;
	else if(kc == kVK_ANSI_I)
		ke->key = SGL_K_I;
	else if(kc == kVK_ANSI_J)
		ke->key = SGL_K_J;
	else if(kc == kVK_ANSI_K)
		ke->key = SGL_K_K;
	else if(kc == kVK_ANSI_L)
		ke->key = SGL_K_L;
	else if(kc == kVK_ANSI_M)
		ke->key = SGL_K_M;
	else if(kc == kVK_ANSI_N)
		ke->key = SGL_K_N;
	else if(kc == kVK_ANSI_O)
		ke->key = SGL_K_O;
	else if(kc == kVK_ANSI_P)
		ke->key = SGL_K_P;
	else if(kc == kVK_ANSI_Q)
		ke->key = SGL_K_Q;
	else if(kc == kVK_ANSI_R)
		ke->key = SGL_K_R;
	else if(kc == kVK_ANSI_S)
		ke->key = SGL_K_S;
	else if(kc == kVK_ANSI_T)
		ke->key = SGL_K_T;
	else if(kc == kVK_ANSI_U)
		ke->key = SGL_K_U;
	else if(kc == kVK_ANSI_V)
		ke->key = SGL_K_V;
	else if(kc == kVK_ANSI_W)
		ke->key = SGL_K_W;
	else if(kc == kVK_ANSI_X)
		ke->key = SGL_K_X;
	else if(kc == kVK_ANSI_Y)
		ke->key = SGL_K_Y;
	else if(kc == kVK_ANSI_Z)
		ke->key = SGL_K_Z;
	else
		return 0;
	
	return 1;
}
