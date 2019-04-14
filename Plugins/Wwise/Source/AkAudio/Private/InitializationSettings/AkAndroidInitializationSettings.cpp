// Copyright (c) 2006-2018 Audiokinetic Inc. / All Rights Reserved

#include "InitializationSettings/AkAndroidInitializationSettings.h"
#include "AkAudioDevice.h"

#if PLATFORM_ANDROID && !PLATFORM_LUMIN
#include "Android/AndroidApplication.h"
#endif


//////////////////////////////////////////////////////////////////////////
// FAkAndroidAdvancedInitializationSettings

void FAkAndroidAdvancedInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	Super::FillInitializationStructure(InitializationStructure);

#if PLATFORM_ANDROID && !PLATFORM_LUMIN
	InitializationStructure.PlatformInitSettings.bRoundFrameSizeToHWSize = RoundFrameSizeToHardwareSize;
#endif
}


//////////////////////////////////////////////////////////////////////////
// UAkAndroidInitializationSettings

UAkAndroidInitializationSettings::UAkAndroidInitializationSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CommonSettings.DefaultPoolSize = 16 * 1024 * 1024;
	CommonSettings.MainOutputSettings.PanningRule = EAkPanningRule::Headphones;
	CommonSettings.MainOutputSettings.ChannelConfigType = EAkChannelConfigType::Standard;
	CommonSettings.MainOutputSettings.ChannelMask = AK_SPEAKER_SETUP_STEREO;
}

void UAkAndroidInitializationSettings::FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const
{
	CommonSettings.FillInitializationStructure(InitializationStructure);
	CommunicationSettings.FillInitializationStructure(InitializationStructure);
	AdvancedSettings.FillInitializationStructure(InitializationStructure);

#if PLATFORM_ANDROID && !PLATFORM_LUMIN
	extern JavaVM* GJavaVM;
	InitializationStructure.PlatformInitSettings.pJavaVM = GJavaVM;
	InitializationStructure.PlatformInitSettings.jNativeActivity = FAndroidApplication::GetGameActivityThis();
#endif
}
