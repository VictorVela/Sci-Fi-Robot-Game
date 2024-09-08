// JakubW

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IWALS_AbilityInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIWALS_AbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IWALS_ABILITYSYSTEM_API IIWALS_AbilityInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "IWALS Ability System")
		void GASBPI_CallToCurrentActivatedAbility(int EventID);

};
