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

#include "stdafx.h"

#include "SoundInput.h"
#include "SoundInputMgr.h"

// The sample rate
#define AUDIO_INPUT_SAMPLE_RATE 48000.0
#define USB_MIC_NAME_KEYWORD "USB"
#define INVALID_DEVICE_INDEX -1

SoundInput SoundInput::ms_Instance;

OSStatus CoreAudioCallbackFunction( AudioDeviceID inDevice,
								   const AudioTimeStamp* inNow,
								   const AudioBufferList* inInputData,
								   const AudioTimeStamp* inInputTime,
								   AudioBufferList* outOutputData,
								   const AudioTimeStamp* inOutputTime, 
								   void* userData );

//---------------------------------------------------------------------------
SoundInput::SoundInput()
: m_callbackMessage(NULL)
, m_streamStatus(InStream_Closed)
{
	UnsetError();
	
	m_audioFormat.uSampleRate = 48000;
	m_audioFormat.channelConfig.SetStandard( AK_SPEAKER_SETUP_MONO );
	m_audioFormat.uBitsPerSample = 16;
	m_audioFormat.uBlockAlign = 2;
	m_audioFormat.uTypeID = AK_FLOAT;
	m_audioFormat.uInterleaveID = AK_NONINTERLEAVED;
}

//---------------------------------------------------------------------------
SoundInput::~SoundInput() 
{
    InputOff();
	
	usleep(100);
	
}

//---------------------------------------------------------------------------
bool SoundInput::InputOn( MacUInt in_DevNumber )
{	
	const char* errorLocation = "SoundInput::InputOn()";

	const MacInt UsbMicDeviceIndex = FindFirstInputDevice();
	if (UsbMicDeviceIndex == INVALID_DEVICE_INDEX) 
	{
		SetError(errorLocation, "Failed to find any USB audio input device.");
		return false;
	}
	
	if ( ! OpenStream(UsbMicDeviceIndex) ) 
	{
		return false;
	}
	
	if ( ! StartStream() ) 
	{
		return false;
	}
	
	if ( ! SoundInputMgr::Instance().RegisterInput(this, 0) ) 
	{
		return false;
	}
	
	return true;
}


bool SoundInput::OpenStream(MacUInt deviceIndex)
{	
	const char* errorLocation = "SoundInput::OpenStream()";
	
	if ( m_streamStatus != InStream_Closed)
	{
		SetError(errorLocation, "Error opening input stream, one stream already open.");
		return false;
	}
		
	UInt32 nDevices = GetDeviceCount();
	if (HasError()) 
	{
		return false;
	}
	
	if (deviceIndex >= nDevices) 
	{
		SetError(errorLocation, "Invalid device index exceeds device count!");
		return false;
	}
	
	if ( ! m_device.Initialize(deviceIndex, nDevices) )
	{
		SetError(errorLocation, "Error getting device IDs.");
		return false;
	}

	UInt32 dataSize = 0;
	m_device.SetPropertyScope(kAudioDevicePropertyScopeInput);
	if ( ! m_device.GetPropertyDataSize(kAudioDevicePropertyStreamConfiguration, &dataSize) ) 
	{
		SetError(errorLocation, "Error getting stream configuration info for device.");
		return false;
	}

	// Find a hardware stream for input
	AudioBufferList* deviceStreams = NULL;
	deviceStreams = (AudioBufferList *) AK_MALLOC( dataSize );
	if (deviceStreams == NULL) 
	{
		SetError(errorLocation, "Memory error allocating AudioBufferList.");
		return false;
	}
	
	if ( ! m_device.GetLastSelectedPropertyData(&dataSize, (void*)deviceStreams) || dataSize == 0)
	{
		SetError(errorLocation, "Error getting stream configuration for device");
		return false;
	}
	
	// Find a hardware stream that meets request.
	const UInt32 RequestChannelCount = 1;
	const MacInt InvalidStreamIndex = -1;
	MacInt inputStreamIndex = InvalidStreamIndex;
	UInt32 nStreams = deviceStreams->mNumberBuffers;
	for (MacInt s=0; s<nStreams; s++) 
	{
		UInt32 nStreamChannels = deviceStreams->mBuffers[s].mNumberChannels;
		if (nStreamChannels >= RequestChannelCount) 
		{
			inputStreamIndex = s;
			break;
		}		
	}
	
	bool foundStream = inputStreamIndex != -1;
	if ( ! foundStream ) 
	{
		SetError(errorLocation, "Found no stream that supports requested channel count.");
		return false;
	}
	
	AK_SAFE_FREE(deviceStreams);
	
	// Determine buffer size.
	AudioValueRange	bufferRange;
	dataSize = sizeof( AudioValueRange );
	if ( ! m_device.GetPropertyData(kAudioDevicePropertyBufferFrameSizeRange, &dataSize, &bufferRange) ) 
	{
		SetError(errorLocation, "Error getting buffer size range for device.");
		return false;
	}
	
	// NOTE: Reduce requestBufferSampleSize to reduce latency.
	UInt32 requestBufferSampleSize = g_uSamplesPerFrame;
	// Clamp buffer size if necessary
	if (requestBufferSampleSize < bufferRange.mMinimum) 
	{
		requestBufferSampleSize = bufferRange.mMinimum;
	}
	else if (requestBufferSampleSize > bufferRange.mMaximum)
	{
		requestBufferSampleSize = bufferRange.mMaximum;
	}
	
	// Set stream buffer size
	UInt32 resultBufferSize = requestBufferSampleSize;
	dataSize = sizeof(UInt32);
	if ( ! m_device.SetPropertyData(kAudioDevicePropertyBufferFrameSize, dataSize, &resultBufferSize) ) 
	{
		SetError(errorLocation, "Error setting device stream buffer size.");
		return false;
	}
	//
	// NOTE: In RtAudio here we set stream object bufferSize and bufferCount (1)
	//
	
	// Set stream samplerate
	Float64 defaultSampleRate;
	dataSize = sizeof(Float64);
	if ( ! m_device.GetPropertyData(kAudioDevicePropertyNominalSampleRate, &dataSize, &defaultSampleRate) ) 
	{
		SetError(errorLocation, "Error getting current sample rate.");
		return false;
	}
	
	const Float64 RequestSampleRate = AUDIO_INPUT_SAMPLE_RATE;
	const Float64 TooMuchDiff = 1.0;
	bool needToUpdateDeviceSampleRate = fabs(RequestSampleRate - defaultSampleRate) > TooMuchDiff;
	if (needToUpdateDeviceSampleRate)
	{
		//Set a listener to monitor and block until sample rate is updated.
		Float64 reportedSampleRate = 0.0;
		AudioObjectPropertyAddress tmpProperty = 
		{ 
			kAudioDevicePropertyNominalSampleRate, 
			kAudioObjectPropertyScopeGlobal, 
			kAudioObjectPropertyElementMaster 
		};
		if ( ! m_device.InstallSampleRateChangeMonitor(&tmpProperty, (void *) &reportedSampleRate ) )
		{
			SetError(errorLocation, "Error setting sample rate property listener for device.");
			return false;
		}
		
		// NOTE: We nominate the new samplerate by assigning it to defaultSampleRate first.
		// it will be reset to a value different from the requested one only if CoreAudio's
		// setProperty call failed, and it could take some time to finish.
		defaultSampleRate = RequestSampleRate;		 
		if ( ! m_device.SetLastSelectedPropertyData(sizeof(Float64), (void*) &defaultSampleRate) )
		{
			SetError(errorLocation, "Error setting sample rate for device.");
			return false;
		}
		
		UInt32 timer = 0;
		const UInt32 OneSec = 1000;
		UInt32 deltatimeMs = 5*OneSec;
		const UInt32 MaxWaitCycle = 1000;
		bool isUpdateSampleRateTimeOut = false;
		while (reportedSampleRate != defaultSampleRate) 
		{
			++timer;
			
			isUpdateSampleRateTimeOut = timer > MaxWaitCycle;
			if (isUpdateSampleRateTimeOut)
			{
				break;
			}
			
			usleep(deltatimeMs);
		}
		
		m_device.UninstallSampleRateChangeMonitor(&tmpProperty, (void*) &reportedSampleRate);
		if (isUpdateSampleRateTimeOut) 
		{
			SetError(errorLocation, "Timeout while waiting for sample rate update for device.");
			return false;
		}
		
	}
	
	printf("Sampling rate: %.1f (Hz)\n", RequestSampleRate);
	
	// Set audio format for the stream.
	AudioStreamBasicDescription streamDescription;
	dataSize = sizeof(AudioStreamBasicDescription);
	if ( ! m_device.GetPropertyData(kAudioStreamPropertyVirtualFormat, &dataSize, &streamDescription) ) 
	{
		SetError(errorLocation, "Error getting stream format for device");
		return false;
	}

	bool needToUpdateStreamSampleRate = fabs(RequestSampleRate - streamDescription.mSampleRate) > TooMuchDiff;
	if (needToUpdateStreamSampleRate) 
	{
		streamDescription.mSampleRate = RequestSampleRate;
	}
	
	const UInt32 RequestAudioFormatID = kAudioFormatLinearPCM;
	bool needToUpdateStreamAudioFormat = streamDescription.mFormatID != RequestAudioFormatID;
	if (needToUpdateStreamAudioFormat) 
	{
		streamDescription.mFormatID = RequestAudioFormatID;
	}
	
	if (needToUpdateStreamSampleRate || needToUpdateStreamAudioFormat) 
	{
		if ( ! m_device.SetLastSelectedPropertyData(dataSize, &streamDescription) )
		{
			SetError(errorLocation, "Error setting sample rate or audio format for device.");
			return false;
		}
	}
	
	// Set ASBD.
	AudioStreamBasicDescription	physicalDescription;
	physicalDescription.mSampleRate = AUDIO_INPUT_SAMPLE_RATE;
	physicalDescription.mFormatID = kAudioFormatLinearPCM;
	physicalDescription.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved;
	physicalDescription.mBytesPerPacket = 4;
	physicalDescription.mFramesPerPacket = 1;
	physicalDescription.mBytesPerFrame = 4;
	physicalDescription.mChannelsPerFrame = 1;
	physicalDescription.mBitsPerChannel = 32;
	physicalDescription.mReserved = 0;
	
	dataSize = sizeof(AudioStreamBasicDescription);
	
	if ( ! m_device.SetPropertyData(kAudioStreamPropertyPhysicalFormat, dataSize, &physicalDescription) )
	{
		SetError(errorLocation, "Error setting stream physical format for device.");
		return false;
	}
	
	//
	// CoreAudio requires no byteswap, always native endianess.
	//
	//bool isBigEndian = physicalDescription.mFormatFlags & kAudioFormatFlagIsBigEndian;
	
	//
	// It's useless making queries here.
	// Query will return the updated info only after the pipeline starts.
	// We should just trust the above settings now that the set-call is successful.
	//
	m_audioFormat.uSampleRate = RequestSampleRate;
	m_audioFormat.channelConfig.SetStandard( AK_SPEAKER_SETUP_MONO );
	m_audioFormat.uBlockAlign = sizeof(CoreAudioNativeFormat);
	const UInt8 BitsPerByte = 8;
	m_audioFormat.uBitsPerSample = BitsPerByte * m_audioFormat.uBlockAlign;
	m_audioFormat.uTypeID = AK_FLOAT;
	m_audioFormat.uInterleaveID = AK_NONINTERLEAVED;
	
	// Get device latency
	UInt32 deviceLatency = 0;
	dataSize = sizeof(UInt32);
	if (m_device.AudioObjectHasSelectedProperty(kAudioDevicePropertyLatency)) 
	{
		if ( ! m_device.GetLastSelectedPropertyData(&dataSize, (void*) &deviceLatency) )
		{
			SetError(errorLocation, "Error getting device latency for device.");
		}
	}
	
	UInt32 streamLatency = 0;
	dataSize = sizeof(UInt32);
	if (m_device.AudioObjectHasSelectedProperty(kAudioStreamPropertyLatency)) 
	{
		if ( ! m_device.GetLastSelectedPropertyData(&dataSize, (void*) &streamLatency) )
		{
			SetError(errorLocation, "Error getting device latency for device.");		
		}
	}
	
	UInt32 deviceSafeOffsetLatency = 0;
	dataSize = sizeof(UInt32);
	if (m_device.AudioObjectHasSelectedProperty(kAudioDevicePropertySafetyOffset)) 
	{
		if ( ! m_device.GetLastSelectedPropertyData(&dataSize, (void*) &deviceSafeOffsetLatency) )
		{
			SetError(errorLocation, "Error getting device latency for device.");			
		}
	}
	
	UInt32 hardwareLatencySamples = deviceLatency + streamLatency + deviceSafeOffsetLatency;
	printf("Hardware latency: %u (samples)\n\n", (unsigned int)hardwareLatencySamples);
	
	// Allocate callback message
	bool isCallbackMessageAllocated = m_callbackMessage != NULL;
	if ( ! isCallbackMessageAllocated ) 
	{
		m_callbackMessage = AK_NEW(CoreAudioCallbackMessage);
		if (m_callbackMessage == NULL)
		{
			SetError(errorLocation, "Error allocating callback message.");
			DeAllocateAll();
			return false;
		}
	}
	
	m_callbackMessage->deviceID = m_device.GetDeviceID();
	m_callbackMessage->inputStreamIndex = inputStreamIndex;
	
	// Allocate user stream buffer
	
	if ( ! m_iobufferQueue.AllocateAll(requestBufferSampleSize*sizeof(CoreAudioNativeFormat)) )
	{
		SetError(errorLocation, "Error allocating user buffer for HAL input data.");
		DeAllocateAll();
		return false;
	}
	
	// Start IO process and assign callback.
    if ( ! m_device.CreateIOProcess(CoreAudioCallbackFunction, (void *) this, m_callbackMessage) ) 
	{
		SetError(errorLocation, "Error assigning callback for device.");
		DeAllocateAll();
		return false;
    }	
	
	// Install Xrun listener
	if ( ! m_device.InstallXrunMonitor( (void*) m_callbackMessage ) ) 
	{
		SetError(errorLocation, "Error setting Xrun monitor callback.");
		DeAllocateAll();
		return false;
	}
	
	m_streamLock.Lock();
	m_streamStatus = InStream_OpenAndStopped;
	m_streamLock.Unlock();
	
	return true;
}

//---------------------------------------------------------------------------
void SoundInput::DeAllocateAll()
{
	if (m_callbackMessage) 
	{
		AK_SAFE_DELETE(m_callbackMessage);	
	}
	
	m_iobufferQueue.DeAllocateAll();
}

//---------------------------------------------------------------------------
bool SoundInput::StartStream()
{
	const char* errorLocation = "SoundInput::StartStream()";
	m_streamLock.Lock();
	if (m_streamStatus != InStream_OpenAndStopped) 
	{
		SetError(errorLocation, "Error starting a closed or running stream.");
		m_streamLock.Unlock();
		return false;
	}
	m_streamLock.Unlock();
	
	if ( ! m_device.StartDevice(CoreAudioCallbackFunction)) 
	{
		SetError(errorLocation, "Error starting callback for device.");
		return false;
	}

	m_streamLock.Lock();	
	m_streamStatus = InStream_Started;	
	m_streamLock.Unlock();
	return true;
}


//---------------------------------------------------------------------------
bool SoundInput::StopStream()
{
	const char* errorLocation = "SoundInput::StopStream()";
	
	//m_streamLock.Lock();
	if (m_streamStatus != InStream_Started) 
	{
		SetError(errorLocation, "Error trying to stop a stream that has not started yet.");
		//m_streamLock.Unlock();
		return false;
	}	
	//m_streamLock.Unlock();
	
	if ( ! m_device.StopDevice(CoreAudioCallbackFunction) )  
	{
		SetError(errorLocation, "Error trying to stop the device.");
		return false;
	}
	
	//m_streamLock.Lock();
	m_streamStatus = InStream_OpenAndStopped;
	//m_streamLock.Unlock();
	
	return true;
}

//---------------------------------------------------------------------------
AkUInt8 SoundInput::GetDeviceCount()
{	
	const char* errorLocation = "SoundInput::GetDeviceCount()";
	
	UInt32 dataSize;
	AudioObjectPropertyAddress propertyAddress = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	OSStatus result = AudioObjectGetPropertyDataSize( kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize );
	if ( result != noErr ) {
		SetError(errorLocation, "Error getting device info!");
		return 0;
	}
	
	return dataSize / sizeof( AudioDeviceID );
}

//---------------------------------------------------------------------------
MacInt SoundInput::FindFirstInputDevice(bool isUseUSBMicOnly /*= false*/)
{
	const char* errorLocation = "SoundInput::FindFirstInputDevice()";
	
	MacUInt nDevices = GetDeviceCount();
	if ( nDevices == 0 ) {
		SetError(errorLocation, "No devices found!");		
		return INVALID_DEVICE_INDEX;
	}
	
	AudioDeviceID deviceList[ nDevices ];
	UInt32 dataSize = sizeof( AudioDeviceID ) * nDevices;
	AudioObjectPropertyAddress property = { kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster };
	OSStatus result = AudioObjectGetPropertyData( kAudioObjectSystemObject, &property,
												 0, NULL, &dataSize, (void *) &deviceList );
	if ( result != noErr ) {
		SetError(errorLocation, "Error getting device IDs!");		
		return INVALID_DEVICE_INDEX;
	}
	
	MacInt deviceIndex = INVALID_DEVICE_INDEX;
	
	for (MacInt d=0; d<nDevices; d++) 
	{
		AudioDeviceID id = deviceList[d];
		
		// Get the output stream "configuration".
		AudioBufferList	*bufferList = nil;
		property.mSelector = kAudioDevicePropertyStreamConfiguration;
		property.mScope = kAudioDevicePropertyScopeInput;
		//  property.mElement = kAudioObjectPropertyElementWildcard;
		dataSize = 0;
		result = AudioObjectGetPropertyDataSize( id, &property, 0, NULL, &dataSize );
		if ( result != noErr || dataSize == 0 ) {
			SetError(errorLocation, "Error getting output stream configuration info for device.");
			return INVALID_DEVICE_INDEX;
		}
		
		// Allocate the AudioBufferList.
		bufferList = (AudioBufferList *) AK_MALLOC( dataSize );
		if ( bufferList == NULL ) {
			SetError(errorLocation, "Memory error allocating output AudioBufferList.");		
			return INVALID_DEVICE_INDEX;
		}
		
		result = AudioObjectGetPropertyData( id, &property, 0, NULL, &dataSize, bufferList );
		if ( result != noErr || dataSize == 0 ) {
			AK_SAFE_FREE(bufferList);
			SetError(errorLocation, "Error getting input stream configuration info for device.");
			return INVALID_DEVICE_INDEX;
		}
		
		// Get the input stream "configuration".		
		result = AudioObjectGetPropertyDataSize( id, &property, 0, NULL, &dataSize );
		if ( result != noErr || dataSize == 0 ) {
			SetError(errorLocation, "Error getting input stream configuration info for device.");
			return INVALID_DEVICE_INDEX;
		}
		
		// Allocate the AudioBufferList.
		bufferList = (AudioBufferList *) AK_MALLOC( dataSize );
		if ( bufferList == NULL ) {
			SetError(errorLocation, "Memory error allocating input AudioBufferList.");
			return INVALID_DEVICE_INDEX;
		}
		
		result = AudioObjectGetPropertyData( id, &property, 0, NULL, &dataSize, bufferList );
		if (result != noErr || dataSize == 0) {
			AK_SAFE_FREE(bufferList);
			SetError(errorLocation, "Error getting input stream configuration for device.");
			return INVALID_DEVICE_INDEX;
		}
		
		// Get input channel information.
		UInt32 nInputChannels = 0;
		
		UInt32 nStreams = bufferList->mNumberBuffers;
		for (UInt32 s=0; s<nStreams; s++ )
			nInputChannels += bufferList->mBuffers[s].mNumberChannels;
		AK_SAFE_FREE(bufferList);
		
		bool isInputDevice = nInputChannels > 0;
		if (isInputDevice) 
		{
			CFStringRef cfName;
			dataSize = sizeof( CFStringRef );
			property.mSelector = kAudioObjectPropertyManufacturer;
			result = AudioObjectGetPropertyData( id, &property, 0, NULL, &dataSize, &cfName );
			if ( result != noErr ) {
				SetError(errorLocation, "Error getting device manufacturer.");
				return INVALID_DEVICE_INDEX;
			}
			
			char manufactureName[DEFAULT_MESSAGE_SIZE];				
			MacInt length = (MacInt) CFStringGetLength(cfName);
			CFStringGetCString(cfName, manufactureName, 3*length + 1, CFStringGetSystemEncoding());
			
			property.mSelector = kAudioObjectPropertyName;
			result = AudioObjectGetPropertyData( id, &property, 0, NULL, &dataSize, &cfName );
			if ( result != noErr ) {
				SetError(errorLocation, "Error getting device name.");
				return INVALID_DEVICE_INDEX;
			}
			
			char deviceName[DEFAULT_MESSAGE_SIZE];			
			length = (MacInt) CFStringGetLength(cfName);
			CFStringGetCString(cfName, deviceName, 3*length + 1, CFStringGetSystemEncoding());
			
			deviceIndex = d;
			
			// NOTE: We are only interested in USB mic here.
			if (isUseUSBMicOnly)
			{
				// Reset deviceIndex
				deviceIndex = INVALID_DEVICE_INDEX;
				
				bool foundUSBmic = strstr(deviceName, USB_MIC_NAME_KEYWORD);
				if (foundUSBmic) 
				{
					deviceIndex = d;
					
					printf("Found first available USB input device:\nAudio IO Device Index: %u\nName: %s\nManufacturer:%s\nInput channels: %u \n", (unsigned int) deviceIndex, deviceName, manufactureName, (unsigned int) nInputChannels);
					
					CFRelease( cfName );
					break;
				}
			}
			
			CFRelease( cfName );
		}
	}
	
	return deviceIndex;
	
}


//---------------------------------------------------------------------------
bool SoundInput::InputOff()
{
	const char* errorLocation = "SoundInput::InputOff()";
	
	SoundInputMgr::Instance().UnregisterInput( this, 0 );
			
	if ( ! StopStream() ) 
	{
		SetError(errorLocation, "Failed to stop input stream.");
		return false;
	}
	
	if ( ! CloseStream() ) 
	{
		SetError(errorLocation, "Failed to close input stream.");		
		return false;
	}
	
	return true;
}

//---------------------------------------------------------------------------
bool SoundInput::CloseStream()
{
	const char* errorLocation = "SoundInput::CloseStream()";

	if (m_streamStatus != InStream_OpenAndStopped) 
	{
		SetError(errorLocation, "Error closing an running or closed stream for device.");
		return false;
	}
	
	if ( ! m_device.UninstallXrunMonitor( (void*) m_callbackMessage ) ) 
	{
		SetError(errorLocation, "Error setting Xrun monitor callback.");
		DeAllocateAll();
		return false;
	}
	
	
	if ( ! m_device.DestroyIOProcess(CoreAudioCallbackFunction, m_callbackMessage) )
	{
		SetError(errorLocation, "Error stopping device.");
		DeAllocateAll();
		return false;
	}
	
	
	
	DeAllocateAll();
	
	//m_streamLock.Lock();
	m_streamStatus = InStream_Closed;
	//m_streamLock.Unlock();
	
	return true;
}

//---------------------------------------------------------------------------
void SoundInput::GetFormatCallback(AkAudioFormat&  io_AudioFormat)
{
    io_AudioFormat.SetAll(
						  m_audioFormat.uSampleRate,
						  m_audioFormat.channelConfig,
						  m_audioFormat.uBitsPerSample,
						  m_audioFormat.uBlockAlign,
						  m_audioFormat.uTypeID,
						  m_audioFormat.uInterleaveID
						  );
}


//---------------------------------------------------------------------------
void SoundInput::Execute(AkAudioBuffer*	io_pBufferOut)
{
	if ( ! m_iobufferQueue.IsAllocated() ) 
	{
		io_pBufferOut->eState = AK_Fail;
		io_pBufferOut->uValidFrames = 0;
		
		return;
	}
	
	AkUInt32 uMaxFrames = io_pBufferOut->MaxFrames();
	
	AkOSChar* AK_RESTRICT outBufferWritePos = (AkOSChar*)( io_pBufferOut->GetChannel(0) );	
	assert(io_pBufferOut->uValidFrames == 0);
	
	while (io_pBufferOut->uValidFrames != uMaxFrames) 
	{
		AkUInt16 framesLeftToWrite = (AkUInt16)uMaxFrames - io_pBufferOut->uValidFrames;
		
		AkUInt16 nTransferredBytes = m_iobufferQueue.WriteToConsumer( (AkOSChar*) outBufferWritePos, framesLeftToWrite*m_audioFormat.uBlockAlign);
		
		// Update consumer write pointer and size
		outBufferWritePos += nTransferredBytes;
		io_pBufferOut->uValidFrames += nTransferredBytes/m_audioFormat.uBlockAlign;
		
		if ( m_iobufferQueue.IsUnderrun() ) 
		{			
			break;
		}
		
	}

}

//---------------------------------------------------------------------------
OSStatus CoreAudioCallbackFunction( AudioDeviceID inDevice,
								   const AudioTimeStamp* inNow,
								   const AudioBufferList* inInputData,
								   const AudioTimeStamp* inInputTime,
								   AudioBufferList* outOutputData,
								   const AudioTimeStamp* inOutputTime, 
								   void* userData )
{	
	SoundInput* thisInput = (SoundInput*) userData;
	
	if ( ! thisInput->CallbackWorker(inDevice, inInputData, inInputTime) )
		return kAudioHardwareUnspecifiedError;
	
	return kAudioHardwareNoError;
}

//---------------------------------------------------------------------------
bool SoundInput::CallbackWorker(AudioDeviceID inDevice, 
								const AudioBufferList *inBufferList, 
								const AudioTimeStamp* inInputTime)
{	
	if (m_streamStatus == InStream_OpenAndStopped) 
	{
		return true;
	}
	
	if (m_streamStatus == InStream_Closed) 
	{
		return false;
	}		
	
	//state may change during context switching
	if (m_streamStatus == InStream_OpenAndStopped) 
	{
		m_streamLock.Unlock();
		return true;
	}

	// Skip unrelated device.
	if (m_callbackMessage && m_callbackMessage->deviceID != inDevice)
	{
		m_streamLock.Unlock();	
		return true;
	}
	
	// Refresh Xrun status
	if (m_callbackMessage && m_callbackMessage->isInputOverrun)
	{
		// NOTE: Add HAL Xrun handling here.
		// Reset HAL Xrun flag to prepare for next notification.
		m_callbackMessage->isInputOverrun = false;
	}
	
	// copy to user buffer
	UInt32 streamIndex = m_callbackMessage->inputStreamIndex;
	AkUInt16 bytesIn = m_iobufferQueue.ReadFromProducer( (AkOSChar*) inBufferList->mBuffers[streamIndex].mData, inBufferList->mBuffers[streamIndex].mDataByteSize);
	if ( bytesIn != inBufferList->mBuffers[streamIndex].mDataByteSize )
	{
		m_callbackMessage->isInputFilled = false;
		return false;
	}
	else
	{
		m_callbackMessage->isInputFilled = true;
	}
	
	return true;
}

//---------------------------------------------------------------------------
bool SoundInput::HasError() const
{
#ifdef AK_SOUNDINPUT_DEBUG
	return m_hasError; 
#else
	return false;
#endif // #ifdef AK_SOUNDINPUT_DEBUG
}

//---------------------------------------------------------------------------
std::string SoundInput::GetError() const
{
#ifdef AK_SOUNDINPUT_DEBUG
	return m_error;
#else
	return std::string("Error unavailable because preprocessor AK_SOUNDINPUT_DEBUG is not defined.");
#endif // #ifdef AK_SOUNDINPUT_DEBUG
}



//---------------------------------------------------------------------------
void SoundInput::SetError(const char* location, const char* error)
{
#ifdef AK_SOUNDINPUT_DEBUG
	char errorMsg[DEFAULT_MESSAGE_SIZE];
	sprintf(errorMsg, "%s: (MacOSX) %s\n", location, error);
	m_error = errorMsg;
	m_hasError = true;
#endif // #ifdef AK_SOUNDINPUT_DEBUG
}

//---------------------------------------------------------------------------
void SoundInput::UnsetError() 
{
#ifdef AK_SOUNDINPUT_DEBUG
	m_error = ERROR_NO_ERROR;
	m_hasError = false;
#endif // #ifdef AK_SOUNDINPUT_DEBUG
}

