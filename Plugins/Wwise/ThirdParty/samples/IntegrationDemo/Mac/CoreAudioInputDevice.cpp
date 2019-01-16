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
//  CoreAudioInputDevice.cpp
//  IntegrationDemo
//

#include "CoreAudioInputDevice.h"


CoreAudioInputDevice::CoreAudioInputDevice()
: m_isInitialized(false)
, m_deviceID(0)
{
#if defined( COREAUDIO_MACOSX_V10_6_AND_LATER )
	// Undocumented but requirement starting with OS-X 10.6.  
	// If not called, queries and updates to various audio device properties 
	// are not handled correctly.
	CFRunLoopRef theRunLoop = NULL;
	AudioObjectPropertyAddress property = { kAudioHardwarePropertyRunLoop,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster };
	AudioObjectSetPropertyData( kAudioObjectSystemObject, &property, 0, NULL, sizeof(CFRunLoopRef), &theRunLoop);
#endif	// #if defined( COREAUDIO_MACOSX_V10_6_AND_LATER )	
	m_property.mSelector = kAudioHardwarePropertyDevices;
	m_property.mScope = kAudioObjectPropertyScopeGlobal;
	m_property.mElement = kAudioObjectPropertyElementMaster;
}

CoreAudioInputDevice::~CoreAudioInputDevice() {}


bool CoreAudioInputDevice::Initialize(UInt32 deviceIndex, UInt32 nDevices)
{
	m_property.mSelector = kAudioHardwarePropertyDevices;
	m_property.mScope = kAudioObjectPropertyScopeGlobal;
	m_property.mElement = kAudioObjectPropertyElementMaster;
	
	AudioDeviceID deviceList[ nDevices ];
	UInt32 dataSize = sizeof( AudioDeviceID ) * nDevices;
	OSStatus result = AudioObjectGetPropertyData( kAudioObjectSystemObject, &m_property, 0, NULL, &dataSize, (void *) &deviceList );
	if (result != noErr) 
	{
		return false;
	}
	
	m_deviceID = deviceList[ deviceIndex ];
	m_isInitialized = true;
	
	return true;
}

bool CoreAudioInputDevice::GetPropertyData(AudioObjectPropertySelector selector, UInt32* outDataSize, void* outData /*= NULL*/)
{
	if ( ! m_isInitialized )
		return false;
	
	m_property.mSelector = selector;
	OSStatus result = AudioObjectGetPropertyData( m_deviceID, &m_property, 0, NULL, outDataSize, outData);
	
	return result == noErr;
}

bool CoreAudioInputDevice::GetLastSelectedPropertyData(UInt32* outDataSize, void* outData /*= NULL*/)
{
	if ( ! m_isInitialized )
		return false;
	
	// Use current selector
	OSStatus result = AudioObjectGetPropertyData( m_deviceID, &m_property, 0, NULL, outDataSize, outData);
	
	return result == noErr;
}

bool CoreAudioInputDevice::GetPropertyDataSize(AudioObjectPropertySelector selector, UInt32* outDataSize)
{
	if ( ! m_isInitialized )
		return false;
	
	m_property.mSelector = selector;			
	OSStatus result = AudioObjectGetPropertyDataSize( m_deviceID, &m_property, 0, NULL, outDataSize);
	
	return result == noErr && *outDataSize != 0;
}

bool CoreAudioInputDevice::SetPropertyData(AudioObjectPropertySelector selector, UInt32 inDataSize, void* outData /*= NULL*/)
{
	if ( ! m_isInitialized )
		return false;
	
	m_property.mSelector = selector;
	OSStatus result = AudioObjectSetPropertyData( m_deviceID, &m_property, 0, NULL, inDataSize, outData);
	
	return result == noErr;
}

bool CoreAudioInputDevice::SetLastSelectedPropertyData(UInt32 inDataSize, void* outData /*= NULL*/)
{
	if ( ! m_isInitialized )
		return false;
	
	// Use current selector
	OSStatus result = AudioObjectSetPropertyData( m_deviceID, &m_property, 0, NULL, inDataSize, outData);
	
	return result == noErr;
}

bool CoreAudioInputDevice::InstallSampleRateChangeMonitor(const AudioObjectPropertyAddress* externalProperty, void* outSampleRate)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = AudioObjectAddPropertyListener(m_deviceID, externalProperty, SampleRateChangeMonitor, outSampleRate);
	
	return result == noErr;
}

bool CoreAudioInputDevice::UninstallSampleRateChangeMonitor(const AudioObjectPropertyAddress* externalProperty, void* outSampleRate)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = AudioObjectRemovePropertyListener(m_deviceID, externalProperty, SampleRateChangeMonitor, outSampleRate);
	
	return result == noErr;
}

OSStatus CoreAudioInputDevice::SampleRateChangeMonitor(AudioObjectID inDevice, UInt32 nAddresses, const AudioObjectPropertyAddress properties[], void* ratePointer)
{
	Float64 *rate = (Float64 *) ratePointer;
	UInt32 dataSize = sizeof( Float64 );
	AudioObjectPropertyAddress property = 
	{ 
		kAudioDevicePropertyNominalSampleRate,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster 
	};
	AudioObjectGetPropertyData( inDevice, &property, 0, NULL, &dataSize, rate );
	
	return kAudioHardwareNoError;
}

bool CoreAudioInputDevice::AudioObjectHasSelectedProperty(AudioObjectPropertySelector selector)
{
	if ( ! m_isInitialized )
		return false;
	
	m_property.mSelector = selector;
	return AudioObjectHasProperty(m_deviceID, &m_property);
}

void CoreAudioInputDevice::SetPropertyScope(AudioObjectPropertyScope scope) 
{
	m_property.mScope = scope;
}


bool CoreAudioInputDevice::CreateIOProcess(AudioDeviceIOProc callbackFunc, void* clientData, CoreAudioCallbackMessage* callbackMessage)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = noErr;
#ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	result = AudioDeviceCreateIOProcID(m_deviceID, callbackFunc, clientData, &(callbackMessage->ioProcID));
#else
	result = AudioDeviceAddIOProc(m_deviceID, callbackFunc, clientData);
#endif // #ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	return result == noErr;
}


bool CoreAudioInputDevice::DestroyIOProcess(AudioDeviceIOProc callbackFunc, CoreAudioCallbackMessage* callbackMessage)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = noErr;
#ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	result = AudioDeviceDestroyIOProcID(m_deviceID, callbackMessage->ioProcID);
#else
	result = AudioDeviceRemoveIOProc(m_deviceID, callbackFunc);
#endif // #ifdef COREAUDIO_MACOSX_V10_5_AND_LATER
	return result == noErr;
}


bool CoreAudioInputDevice::InstallXrunMonitor(void* callbackMessage)
{
	if ( ! m_isInitialized )
		return false;

	m_property.mSelector = kAudioDeviceProcessorOverload;
	OSStatus result = AudioObjectAddPropertyListener(m_deviceID, &m_property, XrunMonitor, callbackMessage);
	
	return result == noErr;
}

bool CoreAudioInputDevice::UninstallXrunMonitor(void* callbackMessage)
{
	if ( ! m_isInitialized )
		return false;
	
	m_property.mSelector = kAudioDeviceProcessorOverload;
	OSStatus result = AudioObjectRemovePropertyListener(m_deviceID, &m_property, XrunMonitor, callbackMessage);
	
	return result == noErr;
}

OSStatus CoreAudioInputDevice::XrunMonitor(AudioObjectID inDevice, UInt32 nAddress, const AudioObjectPropertyAddress properties[], void* inClientData )
{
	return kAudioHardwareNoError;
	
	//CoreAudioCallbackMessage* message = (CoreAudioCallbackMessage*) inClientData;
	//for (UInt32 p=0; p<nAddress; p++) 
	//{
	//	if (properties[p].mSelector == kAudioDeviceProcessorOverload) 
	//	{
	//		if (properties[p].mScope == kAudioDevicePropertyScopeInput) 
	//		{
	//			message->isInputOverrun = true;
	//		}
	//	}
	//}
	//
	//return kAudioHardwareNoError;
}


bool CoreAudioInputDevice::StartDevice(AudioDeviceIOProc callbackFunc)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = AudioDeviceStart(m_deviceID, callbackFunc);
	
	return result == noErr;
}

bool CoreAudioInputDevice::StopDevice(AudioDeviceIOProc callbackFunc)
{
	if ( ! m_isInitialized )
		return false;
	
	OSStatus result = AudioDeviceStop(m_deviceID, callbackFunc);
	
	return result == noErr;
}
