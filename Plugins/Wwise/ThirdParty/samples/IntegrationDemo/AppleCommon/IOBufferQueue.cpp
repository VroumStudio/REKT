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
//  IOBufferQueue.cpp
//  IntegrationDemo
//

#include "IOBufferQueue.h"

#include <cmath>
#include <algorithm>

// Main thread
// NOTE: For simplicity, each user buffer size is the same
// as the HAL incoming buffer size. So that we don't do 
// readpointer management in producer cycle.
IOBufferQueue::IOBufferQueue()
: m_bufferByteSize(0)
, m_currentConsumeStartPos(0)
, m_producerBufferIndex(0)
, m_consumerBufferIndex(0) // make consumer wait in the beginning
, m_isOverrun(false)
, m_isUnderrun(false)
{
	for (AkUInt8 b=0; b<AK_IO_BUFFER_COUNT; b++) 
	{
		m_buffers[b] = NULL;
		m_bufferStatus[b] = NotAllocated;
	}
}


IOBufferQueue::~IOBufferQueue()
{
	DeAllocateAll();
}


bool IOBufferQueue::AllocateAll(AkUInt16 byteSize)
{
	m_lock.Lock();
	
	m_bufferByteSize = byteSize;
	
	bool isAllSuccess = true;
	for (AkUInt8 b=0; b<AK_IO_BUFFER_COUNT; b++) 
	{
		// NOTE: We do not try to dealloc first.
		//       Client should manage deallocation.
		if (m_bufferStatus[b] != NotAllocated) 
		{
			isAllSuccess = false;
		}
		
		if (m_buffers[b] == NULL) 
		{
			m_buffers[b] = (AkOSChar*) malloc(byteSize);
			m_bufferStatus[b] = ReadyForProducer;
		}
		
	}
	
	m_lock.Unlock();
	
	return isAllSuccess;
}


bool IOBufferQueue::DeAllocateAll()
{
	m_lock.Lock();
	
	bool isAllSuccess = true;
	for (AkUInt8 b=0; b<AK_IO_BUFFER_COUNT; b++) 
	{
		if (m_bufferStatus[b] == NotAllocated) 
		{
			isAllSuccess = false;
		}
		
		if (m_buffers[b] != NULL) 
		{
			free(m_buffers[b]);
			m_buffers[b] = NULL;
			m_bufferStatus[b] = NotAllocated;
		}
	}
	
	m_lock.Unlock();
	
	return isAllSuccess;
}

bool IOBufferQueue::IsAllocated()
{
	m_lock.Lock();
	
	for (AkUInt8 b=0; b<AK_IO_BUFFER_COUNT; b++) 
	{
		if (m_bufferStatus[b] != NotAllocated) 
		{
			m_lock.Unlock();
			return true;
		}		
	}
	
	m_lock.Unlock();
	
	return false;
}

// HAL thread: Import data from HAL incoming buffer
AkUInt16 IOBufferQueue::ReadFromProducer(const AkOSChar* inBuffer, AkUInt16 requestByteSize /*= AUDIO_INPUT_BUFFER_SIZE*/)
{	
	m_isOverrun = m_bufferStatus[m_producerBufferIndex] != ReadyForProducer;
	
	// Clamp if necessary
	AkUInt16 actualInByteSize = requestByteSize <= m_bufferByteSize ?
		requestByteSize : m_bufferByteSize;	
	
	m_lock.Lock();
	memcpy(m_buffers[m_producerBufferIndex], inBuffer, actualInByteSize);
	
	m_bufferStatus[m_producerBufferIndex] = ReadyForConsumer;
	m_lock.Unlock();
	
	GoToNextProducerBuffer();
	
	return actualInByteSize;
}


//
// Wwise plugin thread
//
// NOTE: Only tries to pull data from current available buffer in queue.
// We don't try to go through buffer queue to assemble data if request size is 
// larger than free space in current buffer. Client code has to plan for
// multiple calls with size deduction based on returned size.
AkUInt16 IOBufferQueue::WriteToConsumer(AkOSChar* outBuffer, AkUInt16 requestByteSize)
{	
	m_isUnderrun = m_bufferStatus[m_consumerBufferIndex] != ReadyForConsumer;
	const AkUInt16 NothingOut = 0;
	if (m_isUnderrun) 
	{
		return NothingOut;
	}
	
	AkUInt16 bytesLeft = m_bufferByteSize - m_currentConsumeStartPos;
	bool isNothingLeftToConsume = bytesLeft == 0;
	if (isNothingLeftToConsume) 
	{
		GoToNextConsumerBuffer();	
		return NothingOut;
	}	
	
	AkUInt16 actualOutByteSize = std::min(requestByteSize, bytesLeft);

	m_lock.Lock();
	memcpy(outBuffer, m_buffers[m_consumerBufferIndex]+m_currentConsumeStartPos, actualOutByteSize);
	m_lock.Unlock();
	
	// Update read pointer
	m_currentConsumeStartPos += actualOutByteSize;
	
	bool isFullyConsumed = actualOutByteSize == bytesLeft;
	if (isFullyConsumed) 
	{
		GoToNextConsumerBuffer();		
	}
	
	return actualOutByteSize;
}


//HAL thread
bool IOBufferQueue::IsOverrun() const	
{
	return m_isOverrun;
}

//Wwise thread
bool IOBufferQueue::IsUnderrun() const
{
	return m_isUnderrun;
}

//HAL thread
void IOBufferQueue::ResetOverrun()
{
	m_isOverrun = false;
}

//Wwise thread
void IOBufferQueue::ResetUnderrun()
{
	m_isUnderrun = false;
}


// NOTE: It's client's job to reset Xrun flags.
void IOBufferQueue::GoToNextProducerBuffer()
{
	m_producerBufferIndex = (m_producerBufferIndex+1) % AK_IO_BUFFER_COUNT;	
}


void IOBufferQueue::GoToNextConsumerBuffer()
{
	m_lock.Lock();
	m_bufferStatus[m_consumerBufferIndex] = ReadyForProducer;
	m_lock.Unlock();
	
	m_consumerBufferIndex = (m_consumerBufferIndex+1) % AK_IO_BUFFER_COUNT;
	m_currentConsumeStartPos = 0;
}
