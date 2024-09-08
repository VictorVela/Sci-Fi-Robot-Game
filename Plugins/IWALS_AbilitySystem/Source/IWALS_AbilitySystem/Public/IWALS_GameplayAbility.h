

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "IWALS_AbilityInterface.h"
#include "GAS_MainCharacterCpp.h"
#include "IWALS_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class IWALS_ABILITYSYSTEM_API UIWALS_GameplayAbility : public UGameplayAbility, public IIWALS_AbilityInterface
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "IWALS Ability System", meta = (DisplayName = "Send To Character Spec Handle"))
		virtual void SendToCharacterSpecHandle();
	
};
