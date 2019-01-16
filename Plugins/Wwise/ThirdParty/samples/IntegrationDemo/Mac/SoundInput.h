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

#include "stdafx.h"
#include "../Common/SoundInputBase.h"
#include "CoreAudioInputDevice.h"
#include "IOBufferQueue.h"

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
	
	virtual bool InputOn( MacUInt in_DevNumber = 0 );		// Start input recording
	virtual bool InputOff();
	
	virtual void Execute( AkAudioBuffer* io_pBufferOut );
	virtual void GetFormatCallback( AkAudioFormat&  io_AudioFormat );

	virtual bool HasError() const;
	virtual std::string GetError() const;

	bool CallbackWorker( AudioDeviceID inDevice, 
						const AudioBufferList* inBufferList,
						const AudioTimeStamp* inInputTime
						);
	
public:
	enum InputStreamStatus
	{
		InStream_OpenAndStopped,
		InStream_Started,
		InStream_Closed
	};

private:
	AkUInt8 GetDeviceCount();
	bool OpenStream(MacUInt deviceIndex);
	bool StartStream();
	bool StopStream();
	bool CloseStream();
	
private:
	void SetError(const char* location, const char* error);
	void UnsetError();

	// New Mac machines can't drive regular line-in mics. So we opted to set USB mic as default supported.
	// Set isUseUSBMicOnly to false if you have a compatible line-in mic that Mac can drive.
	MacInt FindFirstInputDevice(bool isUseUSBMicOnly = true);
	void DeAllocateAll();
	
private:
	static SoundInput ms_Instance;	
	CoreAudioCallbackMessage* m_callbackMessage;
	CoreAudioInputDevice m_device;
	AkAudioFormat m_audioFormat;	
	
	IOBufferQueue m_iobufferQueue;

	CAkLock m_streamLock;
	InputStreamStatus m_streamStatus;

#ifdef AK_SOUNDINPUT_DEBUG
	std::string m_error;
	bool m_hasError;
#endif//#ifdef AK_SOUNDINPUT_DEBUG
};

#endif
