#import <UIKit/UIKit.h>
#include "Platform.h"



namespace AK
{
void * AllocHook( size_t in_size )
{
	return malloc( in_size );
}
void FreeHook( void * in_ptr )
{
	free( in_ptr );
}
}
	
int main(int argc, char *argv[]) {
    
    @autoreleasepool {
    
	// Set the bank base path
		NSString* bankPath = [NSString stringWithFormat:@"%@/iOS/", [[NSBundle mainBundle] resourcePath]];
		strncpy(g_szBasePath, [bankPath UTF8String], AK_IOS_MAX_BANK_PATH);
		g_szBasePath[AK_IOS_MAX_BANK_PATH-1] = '\0';

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsPath = [paths objectAtIndex:0];
		strncpy(g_szDocumentsPath, [documentsPath UTF8String], AK_IOS_MAX_DOCUMENTS_PATH);
		g_szDocumentsPath[AK_IOS_MAX_DOCUMENTS_PATH-1] = '\0';
		
		int retVal = UIApplicationMain(argc, argv, nil, @"IntegrationDemoAppDelegate");
		return retVal;
    }
}
