#import "GLView.h"
#import <OpenGLES/ES2/gl.h> // <-- for GL_RENDERBUFFER only

#import "GameRenderer.h"

GameRenderer* g_pOpenGLRenderer;

@implementation GLView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame 
{
    if (self = [super initWithFrame:frame]) {
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*) super.layer;
        eaglLayer.opaque = YES;

		
        EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES1;
        m_context = [[EAGLContext alloc] initWithAPI:api];
		
        if (!m_context || ![EAGLContext setCurrentContext:m_context]) {
            return nil;
        }
		
		// Create & bind the color buffer so that the caller can allocate its space.
		glGenRenderbuffersOES(1, &m_renderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_renderbuffer);
			
		[m_context	renderbufferStorage:GL_RENDERBUFFER fromDrawable: eaglLayer];
        
		// Create the framebuffer object and attach the color buffer.
		glGenFramebuffersOES(1, &m_framebuffer);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_framebuffer);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,GL_RENDERBUFFER_OES, m_renderbuffer);
		
		g_pOpenGLRenderer = [[GameRenderer alloc] init];	
	}
    return self;
}

- (void)stopRendering
{
	[m_displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)startRendering
{
	m_displayLink = [CADisplayLink displayLinkWithTarget:self
												selector:@selector(renderFrame:)];
	
	[m_displayLink addToRunLoop:[NSRunLoop currentRunLoop]
						forMode:NSDefaultRunLoopMode];
}
	
- (void)renderFrame: (CADisplayLink*) displayLink;
{
	[g_pOpenGLRenderer render];
	[m_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)dealloc 
{
	[self stopRendering];
}


@end
