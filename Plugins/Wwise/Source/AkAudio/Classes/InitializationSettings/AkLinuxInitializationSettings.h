#pragma once

#include "AkInclude.h"
#include "AkInitializationSettings.h"
#include "AkLinuxInitializationSettings.generated.h"

UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkLinuxInitializationSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettingsWithSampleRate CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettingsWithSystemInitialization CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkAdvancedInitializationSettingsWithMultiCoreRendering AdvancedSettings;
};
