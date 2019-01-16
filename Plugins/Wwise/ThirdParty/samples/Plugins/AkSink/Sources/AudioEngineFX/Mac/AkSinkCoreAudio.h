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
// AkSink.h
//
// Platform dependent part
//
//////////////////////////////////////////////////////////////////////

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>

#import <AudioToolbox/AudioToolbox.h>

class CAkMixerLite
{
public:
	CAkMixerLite()
		: m_usMaxFrames( 0 )
		, m_fOneOverNumFrames( 0 )
	{
	}
	~CAkMixerLite() {}
	
	inline void Init( AkUInt16 in_uMaxFrames )
	{
		m_usMaxFrames = in_uMaxFrames;
		m_fOneOverNumFrames = (1.0f / ((AkReal32)in_uMaxFrames));
	}
	
	void ProcessVolume( AkAudioBuffer *	in_pInputBuffer, AkAudioBuffer * in_pOutputBuffer, AkRamp in_gain );
	
private:
	
	static void ApplyGainSIMD(
							  AkReal32*	in_pSourceData,
							  AkReal32*	in_pDestData,
							  AkReal32	in_fVolume,
							  AkReal32	in_fVolumeDelta,
							  AkUInt32	in_uNumSamples
							  );
private:
	AkUInt16 m_usMaxFrames;
	AkReal32 m_fOneOverNumFrames;
};

#pragma mark - class AkSinkCoreAudio
class AkSinkCoreAudio
	: public AK::IAkSinkPlugin
{
public:
	AkSinkCoreAudio();
	~AkSinkCoreAudio();

	//
	// AK::IAkSinkPlugin implementation.
	//
	virtual AKRESULT Init(
						  AK::IAkPluginMemAlloc *	in_pAllocator,				// Interface to memory allocator to be used by the effect.
						  AK::IAkSinkPluginContext*	in_pSinkPluginContext,		// Interface to sink plug-in's context.
						  AK::IAkPluginParam *		in_pParams,					// Interface to plug-in parameters.
						  AkAudioFormat &			in_rFormat					// Audio data format of the input signal.
	);
	virtual AKRESULT Term(
						  AK::IAkPluginMemAlloc * in_pAllocator 	// UNUSED interface to memory allocator to be used by the plug-in
	);
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo(
								   AkPluginInfo & out_rPluginInfo	// Reference to the plug-in information structure to be retrieved
	);
	virtual void Consume(
						 AkAudioBuffer *			in_pInputBuffer,		///< Input audio buffer data structure. Plugins should avoid processing data in-place.
						 AkRamp					in_gain					///< Volume gain to apply to this input (prev corresponds to the beginning, next corresponds to the end of the buffer).
	);
	virtual void OnFrameEnd();
	virtual bool IsStarved();
	virtual void ResetStarved();
	virtual AKRESULT IsDataNeeded( AkUInt32 & out_uBuffersNeeded );
	
	//
	// Sink helper
	//
protected:
	void AllocBuffer( AK::IAkPluginMemAlloc* in_pAllocator, AkUInt32  in_size );
	void DestroyBuffer( AK::IAkPluginMemAlloc* in_pAllocator );
	
	AkAudioBuffer& NextWriteBuffer();
	
	AkForceInline void* GetRefillPosition()
	{
		return m_pData + m_WriteHead;
	}
	
	void PassData();
	void PassSilence();
	
	//
	// CoreAudio specific implementation
	//
protected:
	OSStatus SetupStreamFormat();
	AkChannelConfig ProbeUserSpeakerSetup();
	AkUInt32 EstimateBufferSize( AkUInt32 in_targetRate );
	static OSStatus	RenderCallbackFunc(void *inRefCon,
									   AudioUnitRenderActionFlags* ioActionFlags,
									   const AudioTimeStamp* TimeStamp,
									   UInt32 inBusNumber,
									   UInt32 inNumberFrames,
									   AudioBufferList* ioData);
	
protected:
	AK::IAkSinkPluginContext* m_pSinkPluginContext;
    AkUInt32        m_uNumSamplesRefill;
	
	AkChannelConfig	m_speakerConfig;
	
	// status
	bool					m_bStarved;
	bool					m_bDataReady;
	
	// AkSink
	AkChannelConfig	m_SpeakersConfig;			// speakers config
	AkUInt32		m_ReadHead, m_WriteHead, m_BufferSize;
	AkInt32			m_SamplesReady;	// Samples that are ready to be consumed
	AkReal32*		m_pData;
	pthread_mutex_t	m_mutex;
	bool			m_bStarving;
	AkAudioBuffer m_MasterOut;
	
	// CAkSinkCoreAudioBase
	AUGraph m_Graph;
    AudioUnit m_OutputUnit;
	AUNode m_OutputNode;
	UInt32 m_OutputId;
};