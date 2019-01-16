// main.m
// Copyright (C) 2011 Audiokinetic Inc.
/// \file 
/// Application main

#import "IntegrationDemoAppDelegate.h"

int main(int argc, char *argv[])
{ 	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Force working directory to be application directory
	// Not the case when started from Finder
	NSString* bundlePath = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent]; 
	chdir([bundlePath UTF8String]);
	
	IntegrationDemoAppDelegate *controller = [[[IntegrationDemoAppDelegate alloc] init] autorelease];
	[[NSApplication sharedApplication] setDelegate:controller]; 
	int status = NSApplicationMain(argc, (const char **) argv);
	
	[pool drain];
	return status;
}
