#import "ViewHelper.h"
#import <UIKit/UIKit.h>

@implementation ViewHelper

+ (void) getScreenRegion:(CGRect&) appRect
{
	appRect = [UIScreen mainScreen].applicationFrame;
	if (SYSTEM_VERSION_LESS_THAN(@"8.0"))
	{
		// Need to hard flip width and height because
		// prior to iOS 8.0, the initial frame size is rotated to portrait after the app starts.
		CGFloat width = appRect.size.height;
		CGFloat height = appRect.size.width;
		
		CGSize sz = {width, height};
		appRect.size = sz; // CGRect.size.width/height are not directly writable.
	}
}
@end
