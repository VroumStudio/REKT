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
//  IOBufferQueue.h
//  IntegrationDemo
//

#ifndef IntegrationDemo_IOBufferQueue_h
#define IntegrationDemo_IOBufferQueue_h

#include "Stdafx.h"

#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkLock.h>


#define AK_IO_BUFFER_COUNT 4

// NOTE: Currently only supports mono data.
// Sample size is equal to audio format byte size.
// The buffer is shared by Wwise thread and input plugin CoreAudio thread.
class IOBufferQueue 
{
public:
	IOBufferQueue();
	virtual ~IOBufferQueue();
	
public:
	bool AllocateAll(AkUInt16 sampleSize);
	bool DeAllocateAll();
	bool IsAllocated();
	
	AkUInt16 ReadFromProducer(const AkOSChar* inBuffer, AkUInt16 requestByteSize);
	AkUInt16 WriteToConsumer(AkOSChar* outBuffer, AkUInt16 requestByteSize);
	bool IsOverrun() const;
	bool IsUnderrun() const;
	void ResetOverrun();
	void ResetUnderrun();
	
private:
	enum BufferStatus
	{
		ReadyForProducer,
		ReadyForConsumer,
		NotAllocated
	};
	
private:
	void GoToNextProducerBuffer();
	void GoToNextConsumerBuffer();
	
private:
	AkOSChar* m_buffers[AK_IO_BUFFER_COUNT];
	BufferStatus m_bufferStatus[AK_IO_BUFFER_COUNT];
	AkUInt16 m_bufferByteSize;
	AkUInt16 m_currentConsumeStartPos;
	AkUInt16 m_producerBufferIndex;	
	AkUInt16 m_consumerBufferIndex;	
	bool m_isOverrun;
	bool m_isUnderrun;
	
	CAkLock m_lock;
};


#endif
