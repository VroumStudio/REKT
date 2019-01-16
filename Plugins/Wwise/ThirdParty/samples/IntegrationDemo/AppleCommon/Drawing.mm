// Drawing.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
// Implements the MenuSystem/Drawing.h functions for the Windows platform

#include "Platform.h"
#include "Drawing.h"
#include <assert.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#import "GameRenderer.h"
#import "IntegrationDemoAppDelegate.h"

extern GameRenderer* g_pOpenGLRenderer;

static AkUInt32 g_iWidth;
static AkUInt32 g_iHeight;

AkUInt32 GetWindowWidth()
{
	return g_iWidth;
}

AkUInt32 GetWindowHeight()
{
	return g_iHeight;
}
/////////////////////////
//  FUNCTIONS
/////////////////////////

void BeginDrawing()
{
    assert(g_pOpenGLRenderer);
	
	[g_pOpenGLRenderer beginDrawing];
}

void DoneDrawing()
{
	assert(g_pOpenGLRenderer);
	
	[g_pOpenGLRenderer doneDrawing];
}


bool InitDrawing( void* in_pParam, AkOSChar* in_szErrorBuffer, unsigned int in_unErrorBufferCharCount,AkUInt32 in_windowWidth, AkUInt32 in_windowHeight)
{
	g_iWidth = in_windowWidth;
	g_iHeight = in_windowHeight;
	return true;
}


void DrawTextOnScreen( const char* in_szText, int in_iXPos, int in_iYPos, DrawStyle in_eDrawStyle )
{
	assert(g_pOpenGLRenderer);

	assert(in_szText);
	
	if(!in_szText || strlen(in_szText) == 0)
	{
		return;
	}	
	
	string buffer = in_szText;
	
	// Replace tags in the text
	ReplaceTags( buffer );

	char tmpStr[INTEGRATIONDEMO_MAX_MESSAGE];
	char* token;
	
	AKPLATFORM::SafeStrCpy(tmpStr, buffer.c_str(), INTEGRATIONDEMO_MAX_MESSAGE); 
	
	token = strtok( tmpStr, "\n" );
	
	while ( token )
	{
		[g_pOpenGLRenderer drawTextOnScreenWithString:token AtPostionX:in_iXPos Y:in_iYPos withStyle:in_eDrawStyle];	
		in_iYPos += GetLineHeight( in_eDrawStyle );
		token = strtok( NULL, "\n" );
	}
}

void TermDrawing() {}

int GetLineHeight( DrawStyle in_eDrawStyle )
{
#ifdef AK_IOS	
	if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad)
		return 20;
	else
		return 12;	
#else
	return 16;
#endif	
}
