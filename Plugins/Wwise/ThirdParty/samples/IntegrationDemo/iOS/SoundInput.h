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

#ifndef IntegrationDemo_SoundInput_h
#define IntegrationDemo_SoundInput_h

#pragma warning( push )
#pragma warning( disable: 4201 )
#pragma warning( pop )

#include "../Common/SoundInputBase.h"
#include "../Common/SoundInputMgrBase.h"
#include "../AppleCommon/IOBufferQueue.h"

#include <AK/SoundEngine/Platforms/iOS/AkiOSSoundEngine.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/Tools/Common/AkLock.h>
#include <string>

// The window size of the sample input

#ifdef AK_LOW_LATENCY
#define AUDIO_INPUT_NUM_BUFFERS 8
#define NUM_TARGET_FREE_BUFFERS 5
#else
#define AUDIO_INPUT_NUM_BUFFERS 8
#define NUM_TARGET_FREE_BUFFERS (AUDIO_INPUT_NUM_BUFFERS/2)
#endif

class SoundInput : public SoundInputBase
{
public:
	SoundInput();
    ~SoundInput();
	
	static SoundInput& Instance()
	{
		return ms_Instance;
	}
	
	virtual bool InputOn( AkUInt32 in_DevNumber = 0 );		// Start input recording
	virtual bool InputOff();
	
	virtual void Execute( AkAudioBuffer* io_pBufferOut );
	virtual void GetFormatCallback( AkAudioFormat&  io_AudioFormat );

	bool EnqueueAudioData( const AudioBufferList* inBufferList );
	
	static AKRESULT AudioInputCallback(AudioBufferList* io_Data, void* in_pCookie);
	
private:
	AkUInt8 GetDeviceCount();
	bool OpenStream(AkUInt32 deviceIndex);
	bool CloseStream();
	
private:
	AkInt32 FindFirstInputDevice();
	void DeAllocateAll();
	
private:
	static SoundInput ms_Instance;
	
	SoundInputDevID m_deviceID;
	
	AkAudioFormat m_audioFormat;
	
	IOBufferQueue m_ioBufferQueue;

	bool m_isInputOn;
	
	CAkLock m_streamLock;

#ifdef AK_SOUNDINPUT_DEBUG
	std::string m_error;
	bool m_hasError;
#endif//#ifdef AK_SOUNDINPUT_DEBUG

};

#endif
