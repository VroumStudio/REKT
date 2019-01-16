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
//////////////////////////////////////////////////////////////////////
//
// AkSinkCoreAudio.cpp
//
// Platform dependent part
//
//////////////////////////////////////////////////////////////////////

#include "AkSinkCoreAudio.h"
#include "SinkPluginParams.h"
#include <AK/SoundEngine/Common/AkSimd.h>

#pragma mark - Global Parameter Macros
#define AK_PLUGIN_CORE_SAMPLERATE 48000
#define ONE_OVER_SQRT_OF_TWO (0.70710678118654752440084436210485f)

#define AK_PLUGIN_NUM_REFILL_VOICES 4

#pragma mark - Helper Macros
#define RETURN_ON_ERROR(result)\
if( result != noErr )\
{\
return AK_Fail;\
}\

#define SET_ZERO(obj) memset( &obj, 0, sizeof( obj) )

#pragma mark - AkSink Macros
// Number of floats in vectors. Implementations that use v2f32 (2 floats) also use this value
// as they are unrolled twice as much.
static const AkUInt32	ulVectorSize = 4;
AK_ALIGN_SIMD( static ) AkReal32 aVolumes[4];
#define BUILD_VOLUME_VECTOR( VECT, VOLUME, VOLUME_DELTA ) \
aVolumes[0] = VOLUME; \
aVolumes[1] = VOLUME + VOLUME_DELTA; \
aVolumes[2] = VOLUME + 2.f * VOLUME_DELTA; \
aVolumes[3] = VOLUME + 3.f * VOLUME_DELTA; \
AKSIMD_V4F32 VECT = AKSIMD_LOAD_V4F32( aVolumes );


#pragma mark - Plugin Factory
AK::IAkPlugin* CreateSink( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AKASSERT( in_pAllocator != NULL );
	return AK_PLUGIN_NEW( in_pAllocator, AkSinkCoreAudio() );
}

AK::IAkPluginParam* CreateSinkParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	AKASSERT(in_pAllocator != NULL);
	return AK_PLUGIN_NEW( in_pAllocator, SinkPluginParams() );
}

AK::PluginRegistration AkSinkRegistration(AkPluginTypeSink, AKCOMPANYID_AUDIOKINETIC, 152, CreateSink, CreateSinkParams);

#pragma mark - Constructor and Destructor
AkSinkCoreAudio::AkSinkCoreAudio()
: m_pSinkPluginContext( NULL )
, m_uNumSamplesRefill( 0 )
// AkSink
, m_ReadHead( 0 )
, m_SamplesReady( 0 )
, m_WriteHead( 0 )
, m_BufferSize( 0 )
, m_pData( NULL )
// CAkSinkCoreAudioBase
, m_Graph( NULL )
, m_OutputUnit( NULL )
, m_OutputNode( NULL )
, m_OutputId( NULL )
// status
, m_bStarving( false )
, m_bDataReady( false )
{
	// Create the mutex for the buffer
	memset( &m_mutex, 0, sizeof( pthread_mutex_t) );
}

AkSinkCoreAudio::~AkSinkCoreAudio()
{
	AKVERIFY(!pthread_mutex_destroy( &m_mutex ));
}

#pragma mark - IAkSinkPlugin Interfaces
AKRESULT AkSinkCoreAudio::Init(
			AK::IAkPluginMemAlloc *	in_pAllocator,	// Interface to memory allocator to be used by the effect.
			AK::IAkSinkPluginContext * in_pSinkPluginContext, // Interface to sink plug-in's context.
			AK::IAkPluginParam *		/*in_pParams*/,		// Interface to plug-in parameters.
			AkAudioFormat &	io_rFormat // Audio data format of the input signal.
		)
{
	m_pSinkPluginContext = in_pSinkPluginContext;
    m_uNumSamplesRefill = in_pSinkPluginContext->GlobalContext()->GetMaxBufferLength();
    
    
	// Create the mutex for the buffer
    pthread_mutexattr_t mutex_attr;
	AKVERIFY(!pthread_mutexattr_init(&mutex_attr));
	AKVERIFY(!pthread_mutex_init( &m_mutex, &mutex_attr ));
	AKVERIFY(!pthread_mutexattr_destroy(&mutex_attr));
	
	// Init speaker config.
	if( !m_SpeakersConfig.IsValid() )
	{
		m_SpeakersConfig = ProbeUserSpeakerSetup();
	}
	
	m_MasterOut.AttachInterleavedData( NULL, 0, 0, m_SpeakersConfig );
	m_MasterOut.Clear();
		
	// Make the	Graph
	OSStatus result = noErr;
	
	// Init processing graph
	result = NewAUGraph(&m_Graph);
	RETURN_ON_ERROR(result);
	
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_6
	AudioComponentDescription outDesc;
#else
	ComponentDescription outDesc;
#endif
	
	SET_ZERO(outDesc);
	
	// Description for the default output device
	outDesc.componentType			= kAudioUnitType_Output;
	outDesc.componentSubType		= kAudioUnitSubType_HALOutput;
	outDesc.componentManufacturer	= kAudioUnitManufacturer_Apple;
	outDesc.componentFlags			= 0;
	outDesc.componentFlagsMask		= 0;
	
	result = AUGraphAddNode(m_Graph, &outDesc, &m_OutputNode);
	RETURN_ON_ERROR(result);
	
	// Description for the default mixer device
	AudioComponentDescription mixerDesc;
    SET_ZERO(mixerDesc);
	mixerDesc.componentType			= kAudioUnitType_Mixer;
	mixerDesc.componentSubType		= kAudioUnitSubType_MatrixMixer;
	mixerDesc.componentManufacturer	= kAudioUnitManufacturer_Apple;
	mixerDesc.componentFlags			= 0;
	mixerDesc.componentFlagsMask		= 0;
	
	result = AUGraphOpen(m_Graph);
	RETURN_ON_ERROR(result);
	
	
	// Get the Audio Units out of the nodes
	result = AUGraphNodeInfo(m_Graph, m_OutputNode, NULL, &m_OutputUnit);
	RETURN_ON_ERROR(result);
	
	// Get the outputID of the device
	// not possible on iPhone
	UInt32 size = sizeof(AudioDeviceID);
	
	AudioObjectPropertyAddress thePropertyAddress
	= { kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster };
	
	result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &thePropertyAddress, 0, NULL, &size, &m_OutputId);
	RETURN_ON_ERROR(result);
	
	// Tell core audio that there are no application run loop and use its own
	// Need to this to keep audio running after headphone jack is pluged/unplugged or when sample rate is changed
	CFRunLoopRef theRunLoop =  NULL;
	thePropertyAddress.mSelector = kAudioHardwarePropertyRunLoop;
	result = AudioObjectSetPropertyData(kAudioObjectSystemObject,
										&thePropertyAddress,
										0,
										NULL,
										sizeof(CFRunLoopRef),
										&theRunLoop);
	RETURN_ON_ERROR(result);
	
	// Try to set the sample rate of the audio output
	Float64 sampleRate = AK_PLUGIN_CORE_SAMPLERATE;
	thePropertyAddress.mSelector = kAudioDevicePropertyNominalSampleRate;
	// This is not that bad to get an error here, CoreAudio should handle the case and provide a resampler
    AudioObjectPropertyAddress deviceAddress = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    
	AudioObjectSetPropertyData(	m_OutputId,
							   &deviceAddress,
							   0,
							   NULL,
							   sizeof(sampleRate),
							   &sampleRate);
	
	// Assigned the current device to the output audio unit
    result = AudioUnitSetProperty(	m_OutputUnit,
								  kAudioOutputUnitProperty_CurrentDevice,
								  kAudioUnitScope_Global,
								  0,
								  &m_OutputId,
								  sizeof(m_OutputId));
	RETURN_ON_ERROR(result);
	
	result = SetupStreamFormat();
	RETURN_ON_ERROR(result);
	
	result = AUGraphInitialize(m_Graph);
	RETURN_ON_ERROR(result);
    
    // Set the callback function
	AURenderCallbackStruct inputCallbackProc;
	SET_ZERO(inputCallbackProc);
	inputCallbackProc.inputProc = RenderCallbackFunc;
	inputCallbackProc.inputProcRefCon = this;
    // Attach to output node if there is no matrix mixer node.
	//result = AUGraphSetNodeInputCallback(m_Graph, m_matrixMixerNode, 0, &inputCallbackProc );
    result = AUGraphSetNodeInputCallback(m_Graph, m_OutputNode, 0, &inputCallbackProc );
	RETURN_ON_ERROR(result);
	
	// Alloc buffers
	AllocBuffer(in_pAllocator, EstimateBufferSize( AK_PLUGIN_CORE_SAMPLERATE ));
	
	if( m_pData == NULL )
	{
		return AK_InsufficientMemory;
	}
    
	return AK_Success;
}

AKRESULT AkSinkCoreAudio::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	// AUGraphStop will wait that the InputProc has returned
	AKVERIFY( AUGraphStop( m_Graph ) == noErr );
	
	if( m_OutputUnit )
	{
		AKVERIFY( AudioOutputUnitStop(m_OutputUnit) == noErr );
		AKVERIFY( AudioUnitUninitialize(m_OutputUnit) == noErr );
	}
	m_OutputUnit = NULL;
	
	if( m_Graph )
	{
		AKVERIFY( AUGraphClose( m_Graph ) == noErr );
		AKVERIFY( DisposeAUGraph(m_Graph ) == noErr );
	}
	m_Graph = NULL;
	
	DestroyBuffer(in_pAllocator);
	
	AK_PLUGIN_DELETE( in_pAllocator, this );
	
	return AK_Success;
}

AKRESULT AkSinkCoreAudio::Reset()
{
	// Start processing
	OSStatus result = AUGraphStart(m_Graph);
	if( result != noErr )
	{
		return AK_Fail;
	}
	
	return AK_Success;
}

bool AkSinkCoreAudio::IsStarved()
{
	return m_bStarving;
}

void AkSinkCoreAudio::ResetStarved()
{
	m_bStarving = false;
}

AKRESULT AkSinkCoreAudio::IsDataNeeded( AkUInt32 & out_uBuffersNeeded )
{
	AKVERIFY(!pthread_mutex_lock(&m_mutex));
	out_uBuffersNeeded = (m_BufferSize - m_SamplesReady) / m_uNumSamplesRefill;
	AKVERIFY(!pthread_mutex_unlock(&m_mutex));
	return AK_Success;
}

AKRESULT AkSinkCoreAudio::GetPluginInfo(
			 AkPluginInfo & out_rPluginInfo	///< Reference to the plug-in information structure to be retrieved
		)
{
	out_rPluginInfo.eType			= AkPluginTypeSink;
	out_rPluginInfo.bIsInPlace		= false;
	return AK_Success;
}

void AkSinkCoreAudio::Consume(
	   AkAudioBuffer *	in_pInputBuffer, ///< Input audio buffer data structure. Plugins should avoid processing data in-place.
	   AkRamp in_gain ///< Volume gain to apply to this input (prev corresponds to the beginning, next corresponds to the end of the buffer).
)
{
	if ( in_pInputBuffer->uValidFrames > 0 )
	{
		// Do final pass on data (interleave, format conversion, and the like) before presenting to sink.
		AkAudioBuffer& rBuffer = NextWriteBuffer();
		if ( rBuffer.HasData() )
		{
			CAkMixerLite m_Mixer;
			m_Mixer.Init( in_pInputBuffer->MaxFrames() );
			m_Mixer.ProcessVolume( in_pInputBuffer, &rBuffer, in_gain );
			
			rBuffer.uValidFrames = in_pInputBuffer->uValidFrames;
		}
		
		m_bDataReady = true;
	}
}

void AkSinkCoreAudio::OnFrameEnd()
{
	if ( m_bDataReady )
		PassData();
	else
	{
		// Perform NextWriteBuffer since it was not done. This is a NOP on some platforms/sinks,
		// but not on those that perform in-place final mix onto their output ring buffer.
		NextWriteBuffer();
		PassSilence();
	}
	
	// Prepare for next frame.
	
	m_bDataReady = false;
}

#pragma mark - AkSink Helpers
void AkSinkCoreAudio::AllocBuffer( AK::IAkPluginMemAlloc* in_pAllocator, AkUInt32  in_size )
{
	if( in_size <= m_BufferSize )
	{
		return;
	}
	
	DestroyBuffer(in_pAllocator);
	
	AKVERIFY(!pthread_mutex_lock(&m_mutex));
	// Expect a lot of glicth if not a mutliple of AK_NUM_VOICE_REFILL_FRAMES
	AKASSERT( in_size % m_uNumSamplesRefill == 0 );
	
	AkUInt32 numChannels = m_SpeakersConfig.uNumChannels;
	m_BufferSize = in_size;
	
	m_pData = (AkReal32*)AK_PLUGIN_ALLOC( in_pAllocator, sizeof(AkReal32) * in_size * numChannels );
	AKVERIFY(!pthread_mutex_unlock(&m_mutex));
	
	// Will reset the data
	if( m_pData == NULL )
	{
		DestroyBuffer(in_pAllocator);
	}
	else
	{
		memset( m_pData, 0, sizeof(AkReal32) * in_size * numChannels );
	}
	
}

void AkSinkCoreAudio::DestroyBuffer( AK::IAkPluginMemAlloc* in_pAllocator )
{
	AKVERIFY(!pthread_mutex_lock(&m_mutex));
	if( m_pData )
	{
		AK_PLUGIN_FREE(in_pAllocator, m_pData);
		m_pData = NULL;
	}
	
	m_BufferSize = 0;
	m_ReadHead = m_SamplesReady = m_WriteHead = 0;
	AKVERIFY(!pthread_mutex_unlock(&m_mutex));
}

AkAudioBuffer& AkSinkCoreAudio::NextWriteBuffer()
{
	m_MasterOut.AttachContiguousDeinterleavedData(
												  GetRefillPosition(),
												  m_BufferSize,
												  0,
												  m_SpeakersConfig );
	return m_MasterOut;
}


void CAkMixerLite::ProcessVolume(
							 AkAudioBuffer *	in_pInputBuffer,
							 AkAudioBuffer *	in_pOutputBuffer,
							 AkRamp			in_gain
							 )
{
	unsigned int uChannel = 0;
	unsigned int uNumChannels = in_pInputBuffer->NumChannels();
	
    AkReal32 fVolume	= in_gain.fPrev;
    AkReal32 fDelta		= (in_gain.fNext - fVolume) * m_fOneOverNumFrames;
    
	do
	{
		AkReal32* AK_RESTRICT pInSample = in_pInputBuffer->GetChannel( uChannel );
		
		ApplyGainSIMD( pInSample, in_pOutputBuffer->GetChannel( uChannel ), fVolume, fDelta, m_usMaxFrames );
		
	} while(++uChannel < uNumChannels );
	
}


void CAkMixerLite::ApplyGainSIMD(
							 AkReal32*	in_pSourceData,
							 AkReal32*	in_pDestData,
							 AkReal32	in_fVolume,
							 AkReal32	in_fVolumeDelta,
							 AkUInt32	in_uNumSamples
							 )
{
	AkReal32* AK_RESTRICT pSourceData = in_pSourceData;
	AkReal32* AK_RESTRICT pDestData = in_pDestData;
	AkReal32* AK_RESTRICT pSourceEnd = pSourceData + in_uNumSamples;
	
	if ( in_fVolumeDelta == 0.0f )
	{
#ifdef AKSIMD_V2F32_SUPPORTED
		
		AKSIMD_BUILD_V2F32( const AKSIMD_V2F32 vVolumes, in_fVolume, in_fVolume );
		
		do
		{
			AKASSERT( !( in_uNumSamples % 4 ) );
			// get four samples
			AKSIMD_V2F32 vIn1 = AKSIMD_LOAD_V2F32_OFFSET( pSourceData, 0 );
			AKSIMD_V2F32 vIn2 = AKSIMD_LOAD_V2F32_OFFSET( pSourceData, 8 );
			
			pSourceData += ulVectorSize;
			
			// apply volume
			AKSIMD_V2F32 vOut1 = AKSIMD_MUL_V2F32( vIn1, vVolumes );
			AKSIMD_V2F32 vOut2 = AKSIMD_MUL_V2F32( vIn2, vVolumes );
			
			// store the result
			AKSIMD_STORE_V2F32_OFFSET( pDestData, 0, vOut1 );
			AKSIMD_STORE_V2F32_OFFSET( pDestData, 8, vOut2 );
			
			pDestData += ulVectorSize;
			
		}
		while ( pSourceData < pSourceEnd );
#else
		
		AKASSERT( !( in_uNumSamples % 8 ) );
		BUILD_VOLUME_VECTOR( vVolumes, in_fVolume, 0.0f );
		
		do
		{
			// get eight samples
			AKSIMD_V4F32 vSrc1 = AKSIMD_LOAD_V4F32( pSourceData );
			AKSIMD_V4F32 vSrc2 = AKSIMD_LOAD_V4F32( pSourceData + ulVectorSize );
			pSourceData += ulVectorSize * 2;
			
			// apply volume
			AKSIMD_V4F32 vOut1 = AKSIMD_MUL_V4F32( vSrc1, vVolumes );
			AKSIMD_V4F32 vOut2 = AKSIMD_MUL_V4F32( vSrc2, vVolumes );
			
			// store the result
			AKSIMD_STORE_V4F32( pDestData, vOut1 );
			AKSIMD_STORE_V4F32( pDestData + ulVectorSize, vOut2 );
			pDestData += ulVectorSize * 2;
		}
		while ( pSourceData < pSourceEnd );
#endif
	}
	
	else // has volume delta
	{
#ifdef AKSIMD_V2F32_SUPPORTED
		AKASSERT( !( in_uNumSamples % 4 ) );
		AkReal32 fVolumesDelta = in_fVolumeDelta * 2;
		AKSIMD_V2F32 vVolumesDelta = AKSIMD_SET_V2F32( fVolumesDelta );
		AKSIMD_BUILD_V2F32( AKSIMD_V2F32 vVolumes1, in_fVolume, in_fVolume + in_fVolumeDelta );
		AKSIMD_V2F32 vVolumes2 = AKSIMD_ADD_V2F32( vVolumes1, vVolumesDelta );
		
		// multiply volumes delta by 2 because the loop is unrolled by 2.
		AKSIMD_BUILD_V2F32( const AKSIMD_V2F32 vUnrollFactor, 2.f, 2.f );
		vVolumesDelta = AKSIMD_MUL_V2F32( vVolumesDelta, vUnrollFactor );
		
		do
		{
			// get eight samples
			AKSIMD_V2F32 vIn1 = AKSIMD_LOAD_V2F32_OFFSET( pSourceData, 0 );
			AKSIMD_V2F32 vIn2 = AKSIMD_LOAD_V2F32_OFFSET( pSourceData, 8 );
			
			pSourceData += ulVectorSize;
			
			// apply volume
			AKSIMD_V2F32 vOut1 = AKSIMD_MUL_V2F32( vIn1, vVolumes1 );
			AKSIMD_V2F32 vOut2 = AKSIMD_MUL_V2F32( vIn2, vVolumes2 );
			
			// store the result
			AKSIMD_STORE_V2F32_OFFSET( pDestData, 0, vOut1 );
			AKSIMD_STORE_V2F32_OFFSET( pDestData, 8, vOut2 );
			
			pDestData += ulVectorSize;
			
			// in_fVolume += in_fVolumeDelta;
			vVolumes1 = AKSIMD_ADD_V2F32( vVolumes1, vVolumesDelta );
			vVolumes2 = AKSIMD_ADD_V2F32( vVolumes2, vVolumesDelta );
		}
		while ( pSourceData < pSourceEnd );
		
#else
		AKASSERT( !( in_uNumSamples % 8 ) );
		AkReal32 fVolumesDelta = in_fVolumeDelta * 4;
		AKSIMD_V4F32 vVolumesDelta = AKSIMD_LOAD1_V4F32( fVolumesDelta );
		
		BUILD_VOLUME_VECTOR( vVolumes1, in_fVolume, in_fVolumeDelta );
		AKSIMD_V4F32 vVolumes2 = AKSIMD_ADD_V4F32( vVolumes1, vVolumesDelta );
		
		// multiply volumes delta by 2 because the loop is unrolled by 2.
		vVolumesDelta = AKSIMD_ADD_V4F32( vVolumesDelta, vVolumesDelta );
		
		do
		{
			// get eight samples
			AKSIMD_V4F32 vSrc1 = AKSIMD_LOAD_V4F32( pSourceData );
			AKSIMD_V4F32 vSrc2 = AKSIMD_LOAD_V4F32( pSourceData + ulVectorSize );
			pSourceData += ulVectorSize * 2;
			
			// apply volume
			AKSIMD_V4F32 vOutput1 = AKSIMD_MUL_V4F32( vSrc1, vVolumes1 );
			AKSIMD_V4F32 vOutput2 = AKSIMD_MUL_V4F32( vSrc2, vVolumes2 );
			
			// store the result
			AKSIMD_STORE_V4F32( pDestData, vOutput1 );
			AKSIMD_STORE_V4F32( pDestData + ulVectorSize, vOutput2 );
			pDestData += ulVectorSize * 2;
			
			// in_fVolume += in_fVolumeDelta;
			vVolumes1 = AKSIMD_ADD_V4F32( vVolumes1, vVolumesDelta );
			vVolumes2 = AKSIMD_ADD_V4F32( vVolumes2, vVolumesDelta );
		}
		while ( pSourceData < pSourceEnd );
		
#endif
		
	}
}


void AkSinkCoreAudio::PassData()
{
	if( m_pData == NULL )
		return;
	
	void*	pvFrom = GetRefillPosition();
	
	AKVERIFY(!pthread_mutex_lock(&m_mutex));
    m_WriteHead = (m_WriteHead + m_uNumSamplesRefill) % m_BufferSize;
	m_SamplesReady += m_uNumSamplesRefill;
	AKASSERT( m_SamplesReady <= m_BufferSize );
	AKVERIFY(!pthread_mutex_unlock(&m_mutex));
	
//#ifndef AK_OPTIMIZED
//	UpdateProfileData( (float *) pvFrom, AK_NUM_VOICE_REFILL_FRAMES * m_MasterOut.NumChannels() );
//#endif
}


void AkSinkCoreAudio::PassSilence()
{
	if( m_pData == NULL )
		return;
	
	AkReal32* pFrom = (AkReal32*)GetRefillPosition();
	AkUInt32 nChannels = m_SpeakersConfig.uNumChannels;
	size_t channelOffset = 0;
	for (AkUInt32 c=0; c<nChannels; ++c)
	{
		memset(pFrom + channelOffset, 0, m_uNumSamplesRefill * sizeof(AkReal32));
		channelOffset += m_BufferSize;
	}
	
	PassData();
}

OSStatus AkSinkCoreAudio::SetupStreamFormat()
{
	OSStatus result;
    UInt32 propertySize;
    
    AkUInt32 uNumChannels = m_speakerConfig.uNumChannels;

    if ( !m_speakerConfig.IsValid() )
    {
        // Query channel config if it wasn't done before.
        UInt32 asbdSize = sizeof(AudioStreamBasicDescription);
        AudioStreamBasicDescription asbd;// = {0};
        AudioUnitGetProperty(m_OutputUnit,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Output,
                             0,
                             &asbd,
                             &asbdSize);
        
        AKASSERT( result == noErr );
        uNumChannels = asbd.mChannelsPerFrame;
        m_speakerConfig.SetAnonymous(uNumChannels);
    }
    
    // Setup the desired stream
    AudioStreamBasicDescription desiredStream =
    {
		AK_PLUGIN_CORE_SAMPLERATE,			//Float64 mSampleRate;
        kAudioFormatLinearPCM,		//UInt32  mFormatID;
        kAudioFormatFlagIsPacked |	//UInt32  mFormatFlags;
        kAudioFormatFlagIsFloat |
        kAudioFormatFlagIsNonInterleaved,
        4,							//UInt32  mBytesPerPacket;
        1,							//UInt32  mFramesPerPacket;
        4,							//UInt32  mBytesPerFrame;
        uNumChannels,        		//UInt32  mChannelsPerFrame;
        32,							//UInt32  mBitsPerChannel;
        0							//UInt32  mReserved;
    };
	
    propertySize = sizeof(desiredStream);
	
	// Set the input stream format of the sound card
	result = AudioUnitSetProperty(m_OutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &desiredStream, propertySize );
	AKASSERT( result == noErr );
	
    return result;
}

AkUInt32 AkSinkCoreAudio::EstimateBufferSize( AkUInt32 in_targetRate )
{
	return AK_PLUGIN_NUM_REFILL_VOICES * m_uNumSamplesRefill;
}


AkChannelConfig AkSinkCoreAudio::ProbeUserSpeakerSetup()
{
    OSStatus result = noErr;
	
    AkChannelConfig speakerSetup;   // Leave invalid if could not query default setup.
    UInt32 outputId;
	
    // Get the outputID of the default device (set in Audi Midi Setup)
	UInt32 size = sizeof(AudioDeviceID);
	
	AudioObjectPropertyAddress outputDeviceID = {   kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster };
    
	result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &outputDeviceID, 0, NULL, &size, &outputId);
	AKASSERT(result == noErr);
    
    if(result != noErr)
    {
        return speakerSetup;
    }
    
    AudioObjectPropertyAddress chLayoutAddress = {      kAudioDevicePropertyPreferredChannelLayout,
		kAudioDevicePropertyScopeOutput,
		kAudioObjectPropertyElementMaster };
	
	UInt32 outSize	= 0;
	result = AudioObjectGetPropertyDataSize(outputId,
											&chLayoutAddress,
											0,
											NULL,
											&outSize);
	
	//Get the users speaker configuration
	if (result == noErr)
	{
		AudioChannelLayout    *layout = (AudioChannelLayout *) AkAlloca(outSize);
		result = AudioObjectGetPropertyData(outputId,
											&chLayoutAddress,
											0,
											NULL,
											&outSize,
											layout);
		
		AKASSERT( result == noErr );
		
		if ( layout != NULL )
        {
            if ( layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions )
            {
                // no channel layout tag is returned,
                //so walk through the channel descriptions and count
                // the channels that are associated with a speaker
                if (layout->mNumberChannelDescriptions == 2)
                {
                    speakerSetup.SetStandard( AK_SPEAKER_SETUP_STEREO );    // there is no channel info for stereo
                }
                else if (layout->mNumberChannelDescriptions <= 6)
                {
                    AkChannelMask uMask = 0;
                    for (UInt32 i = 0; i < layout->mNumberChannelDescriptions; i++)
                    {
                        switch( layout->mChannelDescriptions[i].mChannelLabel )
                        {
                            case kAudioChannelLabel_Left:
                                uMask |= AK_SPEAKER_FRONT_LEFT;
                                
                                break;
                            case kAudioChannelLabel_Right:
                                uMask |= AK_SPEAKER_FRONT_RIGHT;
                                
                                break;
                            case kAudioChannelLabel_Center:
                                uMask |= AK_SPEAKER_FRONT_CENTER;
                                
                                break;
                            case kAudioChannelLabel_LFEScreen:
                                uMask |= AK_SPEAKER_LOW_FREQUENCY;
                                
                                break;
                            case kAudioChannelLabel_LeftSurround:
                                uMask |= AK_SPEAKER_BACK_LEFT;
                                
                                break;
                            case kAudioChannelLabel_RightSurround:
                                uMask |= AK_SPEAKER_BACK_RIGHT;
                                break;
                            default:
                                break;
                        }
                        speakerSetup.SetStandard( uMask );
                    }
                }
                else
                {
                    speakerSetup.SetAnonymous( layout->mNumberChannelDescriptions );
                }
            }
            else
            {
                if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_AudioUnit_5_1 )
                    speakerSetup.SetStandard( AK_SPEAKER_SETUP_5POINT1 );
                else
                {
                    AkUInt32 uNumChannels = AudioChannelLayoutTag_GetNumberOfChannels( layout->mChannelLayoutTag );
                    if ( uNumChannels > 2 )
                        speakerSetup.SetAnonymous( uNumChannels );
                }
            }
        }
	}
	
    return speakerSetup;
}


OSStatus AkSinkCoreAudio::RenderCallbackFunc(void *inRefCon,
												  AudioUnitRenderActionFlags *ioActionFlags,
												  const AudioTimeStamp *TimeStamp,
												  UInt32 inBusNumber,
												  UInt32 inNumberFrames,
												  AudioBufferList * ioData)
{
	AkSinkCoreAudio* This = (AkSinkCoreAudio*)inRefCon;
	
	AKVERIFY(!pthread_mutex_lock(&This->m_mutex));
	
	AkInt32 starvingSamples	= inNumberFrames - This->m_SamplesReady;
	AkInt32 sampleToProcess	= AkMin(inNumberFrames, This->m_SamplesReady);
	AkInt32 tailSamples		= AkMin( This->m_BufferSize - This->m_ReadHead, sampleToProcess );
	AkInt32 headSamples		= sampleToProcess - tailSamples;
    AkUInt32 numChannels    = This->m_SpeakersConfig.uNumChannels;
    
	if( tailSamples > 0 )
	{
		size_t tailSize = tailSamples * sizeof(float);
		size_t channelOffset = 0;
		// Copy each channel into core audio buffer
		for( int b = 0; b < numChannels ; b++)
		{
            memcpy( ioData->mBuffers[b].mData, This->m_pData + channelOffset + This->m_ReadHead , tailSize);
			channelOffset += This->m_BufferSize;
		}
	}
	
	if( headSamples > 0)
	{
		size_t headSize = headSamples * sizeof(float);
		size_t channelOffset = 0;
		// Copy each channel into core audio buffer
		for( int b = 0; b < numChannels ; b++)
		{
            memcpy( ((float*)ioData->mBuffers[b].mData + tailSamples), This->m_pData + channelOffset, headSize );
			channelOffset += This->m_BufferSize;
		}
	}
    
	// Starvation
	if( starvingSamples > 0 )
	{
		This->m_bStarving = true;
		
		// If we are starving the zero the samples that should not have been there
		AkUInt32 offset	= inNumberFrames - starvingSamples;
		for( int b = 0; b < numChannels; b++)
		{
            memset( (AkReal32*)(ioData->mBuffers[b].mData) + offset , 0 , starvingSamples * sizeof(AkReal32));
		}
		
		// Reset the read head and samples ready to zero...
		// If we are starving, better start with an empty stomach too
		// Resetting the readhead is mandatory in case where
		// the user has change the sample rate while the engine
		// is on
		This->m_ReadHead		= 0;
		This->m_SamplesReady	= 0;
	}
	
	else
	{
		This->m_SamplesReady -= inNumberFrames;
		AKASSERT( This->m_SamplesReady >= 0);
		This->m_ReadHead += inNumberFrames;
		This->m_ReadHead %= This->m_BufferSize;
	}
	
	
	AKVERIFY(!pthread_mutex_unlock(&This->m_mutex));
	
	// Tell the audio manager that we need a buffer
	This->m_pSinkPluginContext->SignalAudioThread(); // Voice still not 'full': trigger buffer generation now.
	
	return noErr;

}