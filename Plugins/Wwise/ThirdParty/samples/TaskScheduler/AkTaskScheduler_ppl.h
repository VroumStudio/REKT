/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Version: v2018.1.4  Build: 6807
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#pragma once

/// \file 
/// Integration of the Wwise Task Scheduler interface with the Windows Parallel Patterns library.

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <ppl.h>

namespace AkTaskScheduler
{
	static void ParallelForFunc(
		void * in_pData,
		AkUInt32 in_uIdxBegin,
		AkUInt32 in_uIdxEnd,
		AkUInt32 in_uTileSize,
		AkParallelForFunc in_func,
		void * in_pUserData,
		const char * /*in_szDebugName*/)
	{
		concurrency::parallel_for(in_uIdxBegin, in_uIdxEnd,
			[&](AkUInt32 n)
			{
				AkTaskContext ctx;
				ctx.uIdxThread = 0;
				in_func(in_pData, n, n + 1, ctx, in_pUserData);
			}
		);
	}
	
	inline void InitDesc(AkTaskSchedulerDesc & io_desc)
	{
		io_desc.uNumSchedulerWorkerThreads = ::Concurrency::details::_CurrentScheduler::_GetNumberOfVirtualProcessors();
		io_desc.fcnParallelFor = ParallelForFunc;
	}
}
