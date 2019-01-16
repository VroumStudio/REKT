// NSGLView.mm
// Copyright (C) 2011 Audiokinetic Inc.
/// \file 
/// NSView that holds the opengl view.

#import "NSGLView.h"
#import "GameRenderer.h"
#include "UniversalInput.h"

GameRenderer* g_pOpenGLRenderer;
extern UGBtnState g_btnState;

@interface NSGLView (PrivateMethods)
- (void) initGL;
- (void) drawView;
@end


@implementation NSGLView

-(void)keyUp:(NSEvent*)event
{
    NSNumber* btnBitMask = [m_keyMap objectForKey:[NSNumber numberWithUnsignedShort:[event keyCode]]];
	
	if( btnBitMask )
	{
		g_btnState &= ~[btnBitMask unsignedShortValue];
	}
	
	//NSLog(@"Key up: %@", event);
}

-(void)keyDown:(NSEvent*)event
{   
	NSNumber* btnBitMask = [m_keyMap objectForKey:[NSNumber numberWithUnsignedShort:[event keyCode]]];
	
	if( btnBitMask )
	{
		g_btnState |= [btnBitMask unsignedShortValue];
	}
	
	//NSLog(@"Key down: %@", event);
}

- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{ 
	// There is no autorelease pool when this method is called 
	// because it will be called from a background thread
	// It's important to create one or you will leak objects
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[self drawView];
	
	[pool release];
	return kCVReturnSuccess;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(NSGLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (id) initWithFrame:(NSRect)frameRect
{
	// Generate the keymap for user input
	m_keyMap = [[NSDictionary alloc] initWithObjectsAndKeys:
				// Value											// Key
				[NSNumber numberWithUnsignedShort:UG_DPAD_UP],		[NSNumber numberWithInt:126],	// Up
				[NSNumber numberWithUnsignedShort:UG_DPAD_DOWN],	[NSNumber numberWithInt:125],	// Down	
				[NSNumber numberWithUnsignedShort:UG_DPAD_LEFT],	[NSNumber numberWithInt:123],	// Left	
				[NSNumber numberWithUnsignedShort:UG_DPAD_RIGHT],	[NSNumber numberWithInt:124],	// Right
				[NSNumber numberWithUnsignedShort:UG_BUTTON1],		[NSNumber numberWithInt:36],	// Enter
				[NSNumber numberWithUnsignedShort:UG_BUTTON2],		[NSNumber numberWithInt:53],	// ESC
				[NSNumber numberWithUnsignedShort:UG_BUTTON3],		[NSNumber numberWithInt:12],	// Q
				[NSNumber numberWithUnsignedShort:UG_BUTTON4],		[NSNumber numberWithInt:14],	// E
				[NSNumber numberWithUnsignedShort:UG_BUTTON6],		[NSNumber numberWithInt:15],	// R
				[NSNumber numberWithUnsignedShort:UG_BUTTON7],		[NSNumber numberWithInt:122],	//F1	
				nil];
	
	NSOpenGLPixelFormatAttribute attributes [] = {
		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,	// double buffered
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
		(NSOpenGLPixelFormatAttribute)0
	};
	
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	
	if (!pf)
	{
		NSLog(@"No OpenGL pixel format");
	}
	
	if (self = [super initWithFrame:frameRect pixelFormat:[pf autorelease]])
	{
		// Make all the OpenGL calls to setup rendering
		[self initGL];
		
		// Create a display link capable of being used with all active displays
		CVDisplayLinkCreateWithActiveCGDisplays(&m_displayLink);
		
		// Set the renderer output callback function
		CVDisplayLinkSetOutputCallback(m_displayLink, &MyDisplayLinkCallback, self);
		
		// Set the display link for the current renderer
		CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
		CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
		CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(m_displayLink, cglContext, cglPixelFormat);
		
		// Activate the display link
		CVDisplayLinkStart(m_displayLink);
		
	}
	
	return self;
}

- (void) initGL
{
	// Make this openGL context current to the thread
	// (i.e. all openGL on this thread calls will go to this context)
	[[self openGLContext] makeCurrentContext];
	
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	// Init our renderer.  Use 0 for the defaultFBO which is appropriate for the mac (not iPhone)
	g_pOpenGLRenderer = [[GameRenderer alloc] init];
}

- (void) drawView
{
	[[self openGLContext] makeCurrentContext];
	
	// We draw on a secondary thread through the display link
	// When resizing the view, -reshape is called automatically on the main thread
	// Add a mutex around to avoid the threads accessing the context simultaneously	when resizing
	CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
	
	[g_pOpenGLRenderer render];
	
	CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
	CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void)removeFromSuperview
{
	CVDisplayLinkStop(m_displayLink);
	[super removeFromSuperview];
}

- (void) dealloc
{	
	[g_pOpenGLRenderer release];
	CVDisplayLinkRelease(m_displayLink);
	[m_keyMap release];
	[super dealloc];
}
@end
