// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "InitializationSettings/AkMacInitializationSettings.h"
#include "AkAudioDevice.h"


//////////////////////////////////////////////////////////////////////////
// UAkMacInitializationSettings

UAkMacInitializationSettings::UAkMacInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkMacInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_MAC
	// From FRunnableThreadMac
	InitializationStructure.DeviceSettings.threadProperties.uStackSize = 4 * 1024 * 1024;
#endif
}
