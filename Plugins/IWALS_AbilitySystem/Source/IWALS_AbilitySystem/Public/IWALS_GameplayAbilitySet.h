

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "Abilities/GameplayAbility.h"
#include "IWALS_GameplayAbilitySet.generated.h"


class UAbilitySystemComponent;

/**
 *	This is an example input binding enum for GameplayAbilities. Your project may want to create its own.
 *	The MetaData default bind keys (LMB, RMB, Q, E, etc) are a convenience for designers setting up abilities sets
 *	or whatever other data you have that gives an ability with a default key binding. Actual key binding is up to DefaultInput.ini
 *
 *	E.g., "Ability1" is the command string that is bound to AbilitySystemComponent::ActivateAbility(1). The Meta data only *suggests*
 *	that you are binding "Ability1" to LMB by default in your projects DefaultInput.ini.
 */
UENUM(BlueprintType)
namespace EIWALS_AbilityInputBinds
{
	enum Type : int
	{
		None0					UMETA(DisplayName = "None"),
		JumpAction				UMETA(DisplayName = "JumpAction (Space)"),
		StanceAction			UMETA(DisplayName = "StanceAction (Alt)"),
		SprintAction			UMETA(DisplayName = "SprintAction (Shift)"),
		WalkAction				UMETA(DisplayName = "WalkAction (Ctrl)"),
		AimAction				UMETA(DisplayName = "AimAction (RMB)"),
		Interaction				UMETA(DisplayName = "Interaction (E)"),
		Shot					UMETA(DisplayName = "Shot (LMB)"),
		Reload					UMETA(DisplayName = "Reload (R)"),
		AimActionType_2			UMETA(DisplayName = "AimActionType_2 (MMB)"),
	};
}

/**
 *	Example struct that pairs a enum input command to a GameplayAbilityClass.6
 */
USTRUCT()
struct FGameplayAbilityBindIWALS
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = BindInfo)
		TEnumAsByte<EIWALS_AbilityInputBinds::Type>	Command = EIWALS_AbilityInputBinds::None0;;

	UPROPERTY(EditAnywhere, Category = BindInfo)
		TSubclassOf<UGameplayAbility>	GameplayAbilityClass;
};

UCLASS()
class IWALS_ABILITYSYSTEM_API UIWALS_GameplayAbilitySet : public UDataAsset
{
	GENERATED_UCLASS_BODY()

	UIWALS_GameplayAbilitySet();

	UPROPERTY(EditAnywhere, Category = AbilitySet)
		TArray<FGameplayAbilityBindIWALS>	Abilities;

	void GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, AActor* TargetActor) const;

	bool HasAbility(UAbilitySystemComponent* AbilitySystemComp, TSubclassOf<UGameplayAbility> AbilityClass) const;
};
