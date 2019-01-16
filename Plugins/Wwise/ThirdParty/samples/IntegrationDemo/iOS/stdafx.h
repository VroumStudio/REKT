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

// stdafx.h
/// \file 
/// Precompiled Header File for the Integration Demo application.

#pragma once

////////////////////////////////////////////
// Helper Includes
////////////////////////////////////////////

#include <cmath>
#include <cstdio>
#include <cassert>

#include <AvailabilityMacros.h>
//#incluce <CoreFoundation/CoreFoundation.h>
//#incluce <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>

#include <CoreAudio/CoreAudioTypes.h>
//#include <CAHostTimeBase.h>

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

#ifdef AK_SOUNDINPUT_DEBUG
	#undef AK_OPTIMIZED

	#define LOG_ERROR(msg) printf("Error: %s, %d: %s\n", __FILE__, __LINE__, msg)
#else
	#define LOG_ERROR(msg)
#endif //ifdef AK_SOUNDINPUT_DEBUG


#define AK_MALLOC(nBytes) malloc(nBytes)
#define AK_MALLOC_ARRAY(arraySize, dataType) malloc(arraySize*sizeof(dataType))
#define AK_SAFE_FREE(pointer) free(pointer); pointer = NULL

#define AK_NEW(type) new type
#define AK_SAFE_DELETE(obj) delete obj; obj = NULL 

#define AK_MEMSET_ZERO(array, arraySize, dataType) memset(array, 0, arraySize*sizeof(dataType))
#define AK_MEMCPY(destArray, srcArray, nArrayBytes) memcpy(destArray, srcArray, nArrayBytes) 



#define DEFAULT_MESSAGE_SIZE 256
#define ERROR_NO_ERROR "No Error"

#define CHANNEL_INDEX_MONO 0

typedef AkReal32 AkDefaultSampleFormat;

