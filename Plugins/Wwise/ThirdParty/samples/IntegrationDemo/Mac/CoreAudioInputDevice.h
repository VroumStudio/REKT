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

//
//  CoreAudioInputDevice.h
//  IntegrationDemo
//

#ifndef IntegrationDemo_CoreAudioInputDevice_h
#define IntegrationDemo_CoreAudioInputDevice_h

#include <CoreAudio/AudioHardware.h>
#include "Platform.h"

struct CoreAudioCallbackMessage
{
	AudioDeviceID deviceID;
#ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	AudioDeviceIOProcID ioProcID;
#endif // #ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	UInt32 inputStreamIndex;
	bool isInputOverrun;
	bool isInputFilled;
	
	CoreAudioCallbackMessage()
	: deviceID(0)
#ifdef COREAUDIO_MACOSX_V10_5_AND_LATER	
	, ioProcID(NULL)
#endif // #ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	, inputStreamIndex(0)
	, isInputOverrun(false) 
	, isInputFilled(false) {}
};


class CoreAudioInputDevice
{
public:
	CoreAudioInputDevice();	
	~CoreAudioInputDevice();
	
public:
	bool Initialize(UInt32 deviceIndex, UInt32 nDevices);
	
	AudioDeviceID GetDeviceID() const { return m_deviceID; }
	
	void SetPropertyScope(AudioObjectPropertyScope scope);
	
	bool GetPropertyData(AudioObjectPropertySelector selector, UInt32* outDataSize, void* outData = NULL);
	
	bool GetLastSelectedPropertyData(UInt32* outDataSize, void* outData = NULL);
	
	bool GetPropertyDataSize(AudioObjectPropertySelector selector, UInt32* outDataSize);
	
	bool SetPropertyData(AudioObjectPropertySelector selector, UInt32 inDataSize, void* outData = NULL);
	
	bool SetLastSelectedPropertyData(UInt32 inDataSize, void* outData = NULL);
	
	bool InstallSampleRateChangeMonitor(const AudioObjectPropertyAddress* externalProperty, void* outSampleRate);
	
	bool UninstallSampleRateChangeMonitor(const AudioObjectPropertyAddress* externalProperty, void* outSampleRate);
	
	static OSStatus SampleRateChangeMonitor(AudioObjectID inDevice, UInt32 nAddresses, const AudioObjectPropertyAddress properties[], void* ratePointer);
	
	bool AudioObjectHasSelectedProperty(AudioObjectPropertySelector selector);
	
	bool CreateIOProcess(AudioDeviceIOProc callbackFunc, void* clientData, CoreAudioCallbackMessage* callbackMessage);
	
	bool DestroyIOProcess(AudioDeviceIOProc callbackFunc, CoreAudioCallbackMessage* callbackMessage);
	
	bool InstallXrunMonitor(void* callbackMessage);
	
	bool UninstallXrunMonitor(void* callbackMessage);
	
	static OSStatus XrunMonitor(AudioObjectID inDevice,
								UInt32 nAddresses,
								const AudioObjectPropertyAddress properties[],
								void* inClientData );
	
	bool StartDevice(AudioDeviceIOProc callbackFunc);
	
	bool StopDevice(AudioDeviceIOProc callbackFunc);
	
private:
	bool m_isInitialized;
	AudioDeviceID m_deviceID;
	AudioObjectPropertyAddress m_property;
	
};

#endif
