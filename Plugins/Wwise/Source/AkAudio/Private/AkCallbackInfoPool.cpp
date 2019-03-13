// Copyright (c) 2019 Audiokinetic Inc. / All Rights Reserved
#include "AkCallbackInfoPool.h"

UAkCallbackInfo* AkCallbackInfoPool::internalAcquire(UClass* type)
{
	FScopeLock autoLock(&Mutex);

	auto& poolArray = Pool.FindOrAdd(type);

	if (poolArray.Num() > 0)
	{
		return poolArray.Pop();
	}
	else
	{
		auto* instance = NewObject<UAkCallbackInfo>((UObject*)GetTransientPackage(), type);
		instance->AddToRoot();

		return instance;
	}
}

void AkCallbackInfoPool::Release(UAkCallbackInfo* instance)
{
	FScopeLock autoLock(&Mutex);

	if (Pool.Contains(instance->GetClass()))
	{
		Pool[instance->GetClass()].Push(instance);
	}
}

void AkCallbackInfoPool::Teardown()
{
	for (auto& poolEntry : Pool)
	{
		for (auto* instance : poolEntry.Value)
		{
			if (instance)
			{
				instance->RemoveFromRoot();
			}
		}
	}
}
