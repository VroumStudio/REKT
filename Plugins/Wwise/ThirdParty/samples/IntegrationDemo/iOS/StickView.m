#import "StickView.h"
#include "Platform.h"

@implementation StickView

@synthesize normalizedPostion;

- (id)initWithFrame:(CGRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
	{
        // Initialization code.
		m_bIsMoving = false;
		m_stickRect = CGRectMake(frame.size.width / 3, frame.size.width / 3, frame.size.width / 3, frame.size.width / 3);
		
		self.opaque = NO;
    }
    return self;
}


- (void)drawRect:(CGRect)rect 
{
	CGContextRef myContext = UIGraphicsGetCurrentContext();
	
	static const CGFloat s_joystickBagkGroundColor[] = {JOYSTICK_BACKGROUND_COLOR};
	
	// Clear the context
	CGContextClearRect(myContext, CGRectMake (0, 0, self.frame.size.width, self.frame.size.height));
	
	// Draw the background circle
	CGContextSetFillColor(myContext, s_joystickBagkGroundColor);
	
	CGContextFillEllipseInRect(myContext,rect);

	static const CGFloat s_joystickColor[] = {JOYSTICK_COLOR};
	
	CGContextSetFillColor (myContext, s_joystickColor);
	CGContextFillEllipseInRect(myContext,m_stickRect);
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event 
{
	UITouch *touch = [touches anyObject];
	CGPoint touchPoint = [touch locationInView:self];
	
	// See if the point touched is within these rectangular bounds
	if (CGRectContainsPoint(m_stickRect, touchPoint))
	{
		// begin moving
		m_bIsMoving = YES;
		[self moveStickToPosition:touchPoint];
	} 
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event 
{
	
	if( m_bIsMoving )
	{
		// We only support single touches, so anyObject retrieves just that touch from touches
		UITouch *touch = [touches anyObject];
	
		CGPoint touchPoint = [touch locationInView:self];

		[self moveStickToPosition:touchPoint];
	}
}


- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event 
{
	if( m_bIsMoving )
	{
		m_bIsMoving = NO;
		[self moveStickToPosition:CGPointMake(self.frame.size.width / 2, self.frame.size.height / 2)];
	}
}

-(void) moveStickToPosition:(CGPoint) position
{	
	CGSize realSize = CGSizeMake((self.bounds.size.width - m_stickRect.size.width), (self.bounds.size.height - m_stickRect.size.height));
		
	// Calculate the normalized postion
	normalizedPostion.x = (2*position.x - self.bounds.size.width) / (self.bounds.size.width - m_stickRect.size.width);
	normalizedPostion.y = -(2*position.y - self.bounds.size.height) / (self.bounds.size.height - m_stickRect.size.height);
	
	float cSquare = normalizedPostion.x * normalizedPostion.x + normalizedPostion.y * normalizedPostion.y;

	// Check if outside bounds
	if( cSquare > 1)
	{
		// Modify the position to it is inside the bounds of the joystick variable
		float c =  sqrt(cSquare);
		normalizedPostion.x = normalizedPostion.x / c;
		normalizedPostion.y = normalizedPostion.y / c;
	}
	
	// Convert to view coordinates
	m_stickRect = CGRectMake((normalizedPostion.x/2 + 1) * realSize.width  - m_stickRect.size.width, (-normalizedPostion.y/2 + 1) * realSize.height  - m_stickRect.size.height, m_stickRect.size.width, m_stickRect.size.height);
	
	// Update the view
	[self setNeedsDisplay];
	
	// Trigger notifications the registered objects
	[[NSNotificationCenter defaultCenter] postNotificationName:@"PositionChanged" object:self];
}


@end
