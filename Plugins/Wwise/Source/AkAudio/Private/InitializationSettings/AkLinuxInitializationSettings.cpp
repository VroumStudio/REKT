// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "InitializationSettings/AkLinuxInitializationSettings.h"
#include "AkAudioDevice.h"


//////////////////////////////////////////////////////////////////////////
// UAkLinuxInitializationSettings

UAkLinuxInitializationSettings::UAkLinuxInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAkLinuxInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);
}
