/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2018.1.4  Build: 6590
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

// GameRenderer.mm
/// \file 
/// Common implementation (iOS and Mac OS X) for drawing
/// Integration Demo text on screen.

#import "TextTexure2D.h"

@interface GameRenderer : NSObject<NSCacheDelegate> {
	GLuint		m_framebuffer;
    GLuint		m_renderbuffer;
	GLuint		m_viewWidth;
	GLuint		m_viewHeight;
	
	NSCache*	m_cache;	
}
-(void) drawTextOnScreenWithString:(char*) string AtPostionX:(int)in_iXPos Y:(int)in_iYPos withStyle:(DrawStyle) in_eDrawStyle;
-(void) doneDrawing;
-(void) beginDrawing;
-(void) freeTextureCache;
-(void) render;
@end
