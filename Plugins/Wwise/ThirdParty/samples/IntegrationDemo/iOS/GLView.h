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

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <OpenGLES/ES1/glext.h>

@interface GLView : UIView {
	EAGLContext* m_context;
	GLuint m_framebuffer;
    GLuint m_renderbuffer;
	CADisplayLink* m_displayLink;

}

- (void)startRendering;
- (void)stopRendering;
- (void)renderFrame: (CADisplayLink*) displayLink;
@end
