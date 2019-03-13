// Copyright (c) 2019 Audiokinetic Inc. / All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"
#include "AkGameplayTypes.h"

class AkCallbackInfoPool
{
public:
	template<typename CallbackType>
	CallbackType* Acquire()
	{
		return static_cast<CallbackType*>(internalAcquire(CallbackType::StaticClass()));
	}

	void Release(UAkCallbackInfo* instance);

	void Teardown();

private:
	UAkCallbackInfo* internalAcquire(UClass* type);

private:
	FCriticalSection Mutex;
	TMap<UClass*, TArray<UAkCallbackInfo*>> Pool;
};