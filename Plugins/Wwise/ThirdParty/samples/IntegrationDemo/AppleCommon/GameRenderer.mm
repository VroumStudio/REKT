// GameRenderer.mm
// Copyright (C) 2011 Audiokinetic Inc 
/// \file 
/// Common implementation (iOS and Mac OS X) for drawing
/// Integration Demo text on screen.

#import "GameRenderer.h"
#include "IntegrationDemo.h"
#include "Drawing.h"
#include "Helpers.h"

#ifndef AK_IOS
	#include <AppKit/NSApplication.h>
#endif

#ifdef AK_IOS
	#define glOrtho glOrthof
#endif

@interface GameRenderer (PrivateMethods)
- (void) render;
-(TextTexure2D*) TextTexure2DWithString:(char*)string withStyle:(DrawStyle) in_eDrawStyle;
@end

@implementation GameRenderer

- (id)init;
{
	if (self = [super init])
	{
		m_viewWidth		= IntegrationDemo::Instance().GetWindowWidth();
		m_viewHeight	= IntegrationDemo::Instance().GetWindowHeight();
		
		glViewport(0, 0, m_viewWidth, m_viewHeight);
		glEnable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);
		
		// Initialize the projection matrix.
		const float maxX = m_viewWidth;
		const float maxY = m_viewHeight;
		
		glOrtho(-maxX/2, +maxX/2, -maxY/2, +maxY/2, -1, 1);
		
		glMatrixMode(GL_MODELVIEW);
		
		m_cache = [[NSCache alloc] init];
		[m_cache setCountLimit:TEXTURE_CACHE_COUNT]; 
		[m_cache setDelegate:self];
	}
	
	return self;
}


-(void) render
{
	// Records the starting time of the current frame
	IntegrationDemo::Instance().StartFrame();
	
	// Process the current frame, quit if Update() is false.
	if ( ! IntegrationDemo::Instance().Update() )
	{
		
#ifdef AK_IOS
		// There is no API provided for gracefully terminating an iPhone application. 
		// Under the iPhone OS, the user presses the Home button to close applications.
		//http://developer.apple.com/library/ios/#qa/qa2008/qa1561.html
		
		// Note _terminateWithStatus is probably a private API
		// so you should not use this code in an application that will be submitted
		// to the app store!
		
		[[NSRunLoop mainRunLoop] performSelector:@selector(_terminateWithStatus:) 
											 target:[UIApplication sharedApplication] 
										   argument:nil 
											  order:100 
											  modes: [NSArray arrayWithObject:NSDefaultRunLoopMode]];
		
#else
		[[NSRunLoop mainRunLoop]	performSelector:@selector(terminate:) 
											 target:NSApp
										   argument:self 
											  order:100 
											  modes: [NSArray arrayWithObject:NSDefaultRunLoopMode]];
#endif
		return;
	}
	IntegrationDemo::Instance().Render();
	
	// Ends the frame and regulates the application's framerate
	IntegrationDemo::Instance().EndFrame();
}

-(void) drawTextOnScreenWithString:(char*) string AtPostionX:(int)in_iXPos Y:(int)in_iYPos withStyle:(DrawStyle) in_eDrawStyle
{
	// Draw using the font and colour based on the desired DrawStyle
	switch ( in_eDrawStyle )
	{
		case DrawStyle_Title:
			glColor4f(TITLE_DRAW_COLOUR);
			break;
		case DrawStyle_Selected:
			glColor4f(SELECTED_DRAW_COLOUR);
			break;
		case DrawStyle_Error:
			glColor4f(ERROR_DRAW_COLOUR);
			break;
		case DrawStyle_Text:
			glColor4f(DEFAULT_DRAW_COLOUR);
			break;
		case DrawStyle_Control:
			glColor4f(DEFAULT_DRAW_COLOUR);
			break;
	}
	
	TextTexure2D *textTex = [self TextTexure2DWithString:string withStyle:in_eDrawStyle];
	CGFloat windowWidth = m_viewWidth;
	CGFloat windowHeight = m_viewHeight;
	
	
	CGPoint point = CGPointMake(in_iXPos - (windowWidth / 2.f), -in_iYPos + (windowHeight / 2.f));
	
	[textTex drawAtPoint:point];
}


-(void) doneDrawing
{
	glPopMatrix();
	glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

-(void) beginDrawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
    glEnableClientState(GL_VERTEX_ARRAY);
	
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glLoadIdentity();
	glPushMatrix();
}

- (TextTexure2D*) TextTexure2DWithString:(char*)string withStyle:(DrawStyle)in_eDrawStyle;
{	
	NSString* keyValue = [[NSString alloc] initWithFormat:@"%@_%d", [NSString stringWithUTF8String:string],in_eDrawStyle]; 
	
	id object = [m_cache objectForKey:keyValue];
	
	if( object != nil)
	{	
		#if defined (AK_MAC_OS_X)
		[keyValue release];
        #endif
		return (TextTexure2D*)object; 
	}
	
    TextTexure2D* texture =[[TextTexure2D alloc] initWithString:string withStyle:in_eDrawStyle];
	
	// The texture will be own by the NSCache
	// The cache will send a release message to the texture
	// when no longer needed
	if (texture != nil)
	{
		[m_cache setObject:texture forKey:keyValue];
    }
    
	#if defined (AK_MAC_OS_X)
	[keyValue release];
    return [texture autorelease];
    #else
     return texture;
	#endif
    
	
}

-(void) freeTextureCache
{
	// Do this when needed
	[m_cache removeAllObjects];
}

@end
