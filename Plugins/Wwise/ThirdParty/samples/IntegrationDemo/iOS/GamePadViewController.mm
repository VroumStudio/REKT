#import "GamePadViewController.h"
#import "ViewHelper.h"
#import <QuartzCore/QuartzCore.h> // needed by setCornerRadius... otherwise buttons will look ugly
#include "Platform.h"

extern UGBtnState g_btnState;
extern UGStickState g_sticksState[2];

@implementation GamePadViewController
@synthesize  backgroundView;


- (id)initWithButtonSize:(CGFloat) buttonSize marginSize:(CGFloat) marginSize
{
    self = [super init];
    if (self) {
		backgroundView = nil;
		m_buttonSize = buttonSize;
		m_marginSize = marginSize;
    }
    return self;
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
	CGRect appRect;
	[ViewHelper getScreenRegion: appRect];
	
	CGRect viewRect = CGRectMake(appRect.origin.x, appRect.origin.y, appRect.size.width, appRect.size.height);

	self.view =[[UIView alloc] initWithFrame:viewRect];
	self.view.alpha = 1.0;
	self.view.opaque = NO;
	self.view.autoresizesSubviews = false;
	self.view.autoresizingMask = false;
	
	// Do not instantiate backgroundView, it is assigned by caller.

	if( backgroundView != nil )
	{
		[self.view addSubview:backgroundView];
	}
	
	static const CGFloat alpha = 0.35;
	
	// Create the left stick
	m_leftStick = [[StickView alloc]initWithFrame:
					CGRectMake(m_marginSize, viewRect.size.height - 3*m_buttonSize - m_marginSize, 3*m_buttonSize, 3*m_buttonSize)];
	m_leftStick.alpha = alpha;
	
	[self.view addSubview:m_leftStick];

	// Create the right stick
	m_rightStick = [[StickView alloc]initWithFrame:
					CGRectMake(viewRect.size.width - 3*m_buttonSize - m_marginSize, viewRect.size.height - 3*m_buttonSize - m_marginSize, 3*m_buttonSize, 3*m_buttonSize)];
	m_rightStick.alpha = alpha;
	[self.view addSubview:m_rightStick];
	
	// Register for position changes
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(joystickPositionHasChanged:)
												 name:@"PositionChanged" object:m_rightStick];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(joystickPositionHasChanged:)
												 name:@"PositionChanged" object:m_leftStick];
	
	// Create all the buttons
	m_buttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_DPAD_UP], 
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_DPAD_DOWN],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_DPAD_LEFT],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_DPAD_RIGHT],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON1],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON2],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON3],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON4],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON7],
		[UIButton buttonWithType:(UIButtonType)UIButtonTypeCustom], [NSNumber numberWithInteger:UG_BUTTON8],
		nil];
	
	// Set the tag to store button mask
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_UP]]).tag		= UG_DPAD_UP;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_DOWN]]).tag		= UG_DPAD_DOWN;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_LEFT]]).tag		= UG_DPAD_LEFT;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_RIGHT]]).tag	= UG_DPAD_RIGHT;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON1]]).tag		= UG_BUTTON1;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON2]]).tag		= UG_BUTTON2;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON3]]).tag		= UG_BUTTON3;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON4]]).tag		= UG_BUTTON4;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON7]]).tag		= UG_BUTTON7;
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON8]]).tag		= UG_BUTTON8;
	
	// Set the title for each button
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_UP]]		setTitle:@" ↑" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_DOWN]]		setTitle:@" ↓" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_LEFT]]		setTitle:@"←" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_RIGHT]]	setTitle:@"→" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON1]]		setTitle:@"A" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON2]]		setTitle:@"B" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON3]]		setTitle:@"X" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON4]]		setTitle:@"Y" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON7]]		setTitle:@"start" forState:UIControlStateNormal];
	[[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON8]]		setTitle:@"select" forState:UIControlStateNormal];

	
	static const CGFloat s_buttonTextColor[]		= {JOYSTICK_COLOR};
	static const CGFloat s_buttonBackgroundColor[]	= {GAMEPAD_TEXT_COLOR};
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	
	CGColorRef colorButtonBackground = CGColorCreate(colorSpace, s_buttonBackgroundColor );
	CGColorRef colorButtonText = CGColorCreate(colorSpace, s_buttonTextColor );
	
	[m_buttons enumerateKeysAndObjectsWithOptions:NSEnumerationConcurrent
									   usingBlock:^(id key, id obj, BOOL *stop) 
	{
		@synchronized(m_buttons)
		{
			UIButton* it = ((UIButton*)obj);

			it.alpha = alpha;

		//	it.showsTouchWhenHighlighted =YES;

			it.titleLabel.font = [UIFont systemFontOfSize: 18];

			// Customize the buttons
			[[it layer] setCornerRadius:12.0f];
			[[it layer] setMasksToBounds:YES];
			[[it layer] setBorderWidth:1.0f];
			[it setBackgroundColor:[UIColor colorWithCGColor:colorButtonBackground]];
			[it setTitleColor:[UIColor colorWithCGColor:colorButtonText] forState:UIControlStateNormal];

			// Register to notifications
			[it addTarget:self action:@selector(buttonDown:) 
				forControlEvents:UIControlEventTouchDown];
			[it addTarget:self action:@selector(buttonUp:) 
				forControlEvents: UIControlEventTouchUpInside | UIControlEventTouchUpOutside | UIControlEventTouchCancel];

			[self.view addSubview:obj];
		}
	}];
	
	CGColorRelease(colorButtonBackground);
	CGColorRelease(colorButtonText);
	CGColorSpaceRelease(colorSpace);
	
	g_btnState = 0;
	memset(g_sticksState, 0, sizeof(g_sticksState));
	
	[self rotateToOrientation:self.interfaceOrientation];
}

- (void) rotateToOrientation:(UIInterfaceOrientation) orientation;
{
	CGRect appRect;
	[ViewHelper getScreenRegion: appRect];
	
	CGRect viewRect = CGRectMake(appRect.origin.x, appRect.origin.y, appRect.size.width, appRect.size.height);

	[m_leftStick setFrame:
				   CGRectMake(m_marginSize, viewRect.size.height - 3*m_buttonSize - m_marginSize, 3*m_buttonSize, 3*m_buttonSize)];
	
	[m_rightStick setFrame:
	 CGRectMake(viewRect.size.width - 3*m_buttonSize - m_marginSize, viewRect.size.height - 3*m_buttonSize - m_marginSize, 3*m_buttonSize, 3*m_buttonSize)];
	//[self.view addSubview:m_rightStick];
	
	// Set the position of the help button to the upper left frame
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON7]]).frame =
	CGRectMake(viewRect.size.width-100,0,100,30);
	
	// Set the position of the help button to the upper right frame
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON8]]).frame =
	CGRectMake(0,0,100,30);
	
	// Set the position for the direction pads
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_UP]]).frame =
	CGRectMake(m_buttonSize + m_marginSize, viewRect.size.height - 3*m_buttonSize - m_marginSize, m_buttonSize, m_buttonSize);
	
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_DOWN]]).frame =
	CGRectMake(m_buttonSize + m_marginSize, viewRect.size.height - m_buttonSize - m_marginSize, m_buttonSize, m_buttonSize);
	
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_LEFT]]).frame =
	CGRectMake(m_marginSize, viewRect.size.height - 2*m_buttonSize - m_marginSize, m_buttonSize, m_buttonSize);
    
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_DPAD_RIGHT]]).frame =
	CGRectMake(2*m_buttonSize + m_marginSize, viewRect.size.height - 2*m_buttonSize - m_marginSize, m_buttonSize, m_buttonSize);
	
	// Set the position for A B X Y buttons
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON1]]).frame = 
	CGRectMake(viewRect.size.width - 2*m_buttonSize - m_marginSize, viewRect.size.height - m_buttonSize - m_marginSize, m_buttonSize, m_buttonSize);	
	
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON2]]).frame = 
	CGRectMake(viewRect.size.width - m_buttonSize - m_marginSize, viewRect.size.height - (2*m_buttonSize) - m_marginSize, m_buttonSize, m_buttonSize);
	
    
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON3]]).frame = 
	CGRectMake(viewRect.size.width - (3*m_buttonSize) - m_marginSize, viewRect.size.height - (m_buttonSize*2) - m_marginSize, m_buttonSize, m_buttonSize);
	
	((UIButton*)[m_buttons objectForKey:[NSNumber numberWithInteger:UG_BUTTON4]]).frame = 
	CGRectMake(viewRect.size.width - 2*m_buttonSize - m_marginSize, viewRect.size.height - (3*m_buttonSize) - m_marginSize, m_buttonSize, m_buttonSize);
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	[[NSNotificationCenter defaultCenter] postNotificationName:@"UIInterfaceOrientationChanged" object:self];	
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	[UIView animateWithDuration:duration animations:^
		{
			[self rotateToOrientation:toInterfaceOrientation];
		
		}
	 ];
}

- (void)joystickPositionHasChanged:(NSNotification *)notification
{
	if([notification object] == m_leftStick)
	{
		g_sticksState[UG_STICKLEFT].x = m_leftStick.normalizedPostion.x;
		g_sticksState[UG_STICKLEFT].y = m_leftStick.normalizedPostion.y;
	}
	else if([notification object] == m_rightStick)
	{
		g_sticksState[UG_STICKRIGHT].x = m_rightStick.normalizedPostion.x;
		g_sticksState[UG_STICKRIGHT].y = m_rightStick.normalizedPostion.y;
	}
}

- (void)buttonDown:(id)sender
{
	UIButton* myButton = (UIButton*)sender;
	g_btnState |= myButton.tag;
}

- (void)buttonUp:(id)sender
{
	UIButton* myButton = (UIButton*)sender;
	g_btnState &= ~myButton.tag;
}


// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
#if defined(SUPPORTED_ORIENTATIONS)
    return SUPPORTED_ORIENTATIONS;
#else
	return YES;
#endif
}


- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
	
	[m_buttons enumerateKeysAndObjectsWithOptions:NSEnumerationConcurrent
									   usingBlock:^(id key, id obj, BOOL *stop) 
	 {
		 [obj removeTarget:self action:@selector(buttonDown:) forControlEvents:UIControlEventTouchDown];
		 [obj removeTarget:self action:@selector(buttonUp:) forControlEvents: UIControlEventTouchUpInside | 
		  UIControlEventTouchUpOutside | 
		  UIControlEventTouchCancel];
		 
		 [obj removeFromSuperview];
	 }];
	
	// Unregister to notifications
	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:@"PositionChanged" 
												  object:m_rightStick];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self
													name:@"PositionChanged" 
												  object:m_leftStick];
	
	[m_rightStick removeFromSuperview];
	[m_leftStick removeFromSuperview];
	if( backgroundView != nil )
	{	
		[backgroundView removeFromSuperview];
		backgroundView = nil;
	}
}


- (void)dealloc {
	[backgroundView removeFromSuperview];
	
}

// WG-25720: iOS 8 callback.
- (void)viewWillTransitionToSize:(CGSize)size
	   withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
	[super viewWillTransitionToSize: size withTransitionCoordinator: coordinator];
}

// WG-25720: iOS 8 callback.
- (void)viewWillLayoutSubviews
{
	[super viewWillLayoutSubviews];
}

- (NSUInteger)supportedInterfaceOrientations {
	return UIInterfaceOrientationMaskLandscapeLeft;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
	return UIInterfaceOrientationLandscapeLeft;
}

- (BOOL)shouldAutorotate {
	// WG-25720: MUST BE YES, otherwise we are stuck with Portrait on pre-iOS8 devices.
	return YES;
}
@end
