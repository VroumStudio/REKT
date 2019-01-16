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

// Platform.h
/// \file 
/// Contains platform-specific helpers for the iOS platform.

#pragma once

#include <MacTypes.h>

#ifndef __OBJC__
#include <AK/SoundEngine/Common/AkTypes.h>
#else
typedef char AkOSChar;
#endif

#define SYSTEM_VERSION_EQUAL_TO(v)                  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedSame)
#define SYSTEM_VERSION_GREATER_THAN(v)              ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedDescending)
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)
#define SYSTEM_VERSION_LESS_THAN(v)                 ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedAscending)
#define SYSTEM_VERSION_LESS_THAN_OR_EQUAL_TO(v)     ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedDescending)

#define SOUND_BANK_PATH g_szBasePath
#define DOCUMENTS_PATH g_szDocumentsPath
#define DEMO_DEFAULT_POOL_SIZE 2*1024*1024
#define DEMO_LENGINE_DEFAULT_POOL_SIZE 2*1024*1024
#define AK_IOS_MAX_BANK_PATH 1024
#define AK_IOS_MAX_DOCUMENTS_PATH 1024
extern char g_szBasePath[AK_IOS_MAX_BANK_PATH];
extern char g_szDocumentsPath[AK_IOS_MAX_DOCUMENTS_PATH];

// Defines for button name strings
#define UG_BUTTON1_NAME  "A"
#define UG_BUTTON2_NAME  "B"
#define UG_BUTTON3_NAME  "X"
#define UG_BUTTON4_NAME  "Y"
#define UG_BUTTON5_NAME  "UNDEFINED"
#define UG_BUTTON6_NAME  "UNDEFINED"
#define UG_BUTTON7_NAME  "Start"
#define UG_BUTTON8_NAME  "Select"
#define UG_BUTTON9_NAME  "UNDEFINED"
#define UG_BUTTON10_NAME "UNDEFINED"  
#define UG_BUTTON11_NAME "UNDEFINED"
#define OS_BUTTON_NAME   "UNDEFINED"
#define UG_RIGHT_STICK	 "Right"
#define UG_LEFT_STICK	 "Left"
#define DIRECTIONAL_TYPE "Dpad"
#define HELP_BUTTON   UG_BUTTON7_NAME
#define ACTION_BUTTON UG_BUTTON1_NAME
#define BACK_BUTTON   UG_BUTTON2_NAME

#define INTEGRATIONDEMO_SCREEN_CHAR_WIDTH	95

#define __AK_OSCHAR_SNPRINTF snprintf

// Defines specific to iOS UI
// Interface colors

//									Red			Green		Blue		Alpha
#define JOYSTICK_BACKGROUND_COLOR	0.04296875,	0.37890625,	0.640625,	1
#define JOYSTICK_COLOR				0,			0.703125,	0.328125,	1
#define	GAMEPAD_TEXT_COLOR			0.140625,	0.09765625,	0.6953125,	1

//									Red			Green		Blue		Alpha
#define DEFAULT_DRAW_COLOUR			1,			1,			1,			1	// 0xFFFFFFFF	 
#define SELECTED_DRAW_COLOUR		1,			1,			0,			1	// Yellow
#define TITLE_DRAW_COLOUR			1,			1,			1,			1	// White
#define ERROR_DRAW_COLOUR			1,			0,			0,			1	// Red

// Number of string texture that view will cache
#define TEXTURE_CACHE_COUNT 40

#define CODECTYPE_STANDARD	AKCODECID_ADPCM

#if ! TARGET_IPHONE_SIMULATOR
	#define INTEGRATIONDEMO_MICROPHONE
#endif // #if ! TARGET_IPHONE_SIMULATOR

extern UInt32 g_uSamplesPerFrame;

extern bool g_bEnableMicDemo;
