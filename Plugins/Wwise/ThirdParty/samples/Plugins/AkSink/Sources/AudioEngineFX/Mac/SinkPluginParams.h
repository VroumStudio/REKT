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
// SinkPluginParams.h
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

// Parameters IDs for the Wwise or RTPC.
enum SinkPluginParamIDs
{
	SINK_SAMPLE_PROP_0 = 0,
	SINK_SAMPLE_MAX_PARAMS	// Keep last
};

//-----------------------------------------------------------------------------
// Structures.
//-----------------------------------------------------------------------------

// Structure of Sink Effect Stub parameters (on bus)
struct SinkPluginParamStruct
{
	float fSampleProp;
};

//-----------------------------------------------------------------------------
// Name: class SceAudio3dSinkEffectParams
// Desc: Shared parameters implementation.
//-----------------------------------------------------------------------------
class SinkPluginParams : public AK::IAkPluginParam
{
public:
	// Constructor/destructor.
	SinkPluginParams();
	~SinkPluginParams();
	SinkPluginParams(const SinkPluginParams & in_rCopy);

	// Create duplicate.
	IAkPluginParam * Clone(AK::IAkPluginMemAlloc * in_pAllocator);

	// Init/Term.
	AKRESULT Init(AK::IAkPluginMemAlloc *	in_pAllocator,
		const void *			in_pParamsBlock,
		AkUInt32				in_ulBlockSize
		);
	AKRESULT Term(AK::IAkPluginMemAlloc * in_pAllocator);

	// Blob set.
	AKRESULT SetParamsBlock(const void * in_pParamsBlock,
		AkUInt32 in_ulBlockSize
		);

	// Update one parameter.
	AKRESULT SetParam(AkPluginParamID in_ParamID,
		const void * in_pValue,
		AkUInt32 in_ulParamSize
		);

private:
	SinkPluginParamStruct m_params;	// Parameter structure.
};

