#pragma once

#include "AkInclude.h"
#include "AkInitializationSettings.h"
#include "AkSwitchInitializationSettings.generated.h"

UCLASS(config = Game, defaultconfig)
class AKAUDIO_API UAkSwitchInitializationSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void FillInitializationStructure(FAkInitializationStructure& InitializationStructure) const;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommonInitializationSettingsWithSampleRate CommonSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization")
	FAkCommunicationSettings CommunicationSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Initialization", AdvancedDisplay)
	FAkAdvancedInitializationSettingsWithMultiCoreRendering AdvancedSettings;
};
