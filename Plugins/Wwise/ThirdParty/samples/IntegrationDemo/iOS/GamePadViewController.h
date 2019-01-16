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
#import "StickView.h"
#import "InputMgr.h"
#import "GLView.h"

// Used by both IntegratoinDemo and GameSim
@interface GamePadViewController : UIViewController {
	NSDictionary *m_buttons;
	StickView* m_leftStick;
	StickView* m_rightStick;
	UIView* backgroundView; // Inserted view from caller, can be IntegrationDemo or GameSim main view.
	CGFloat m_buttonSize;
	CGFloat m_marginSize;
}

@property (nonatomic, strong) UIView *backgroundView;

- (id)initWithButtonSize:(CGFloat) buttonSize marginSize:(CGFloat) marginSize;
- (void) rotateToOrientation:(UIInterfaceOrientation) orientation;
@end

