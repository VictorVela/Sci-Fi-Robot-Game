

#include "IWALS_GameplayAbilitySet.h"
#include "AbilitySystemComponent.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(UIWALS_GameplayAbilitySet)

UIWALS_GameplayAbilitySet::UIWALS_GameplayAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool UIWALS_GameplayAbilitySet::HasAbility(UAbilitySystemComponent* AbilitySystemComp, TSubclassOf<UGameplayAbility> AbilityClass) const
{
	TArray<FGameplayAbilitySpec> Abilitiess = AbilitySystemComp->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : Abilitiess)
	{
		if (Spec.Ability->GetClass() == AbilityClass)
		{
			// Zdolnoœæ zosta³a ju¿ przydzielona
			return true;
		}
	}
	// Zdolnoœæ nie zosta³a jeszcze przydzielona
	return false;
}

void UIWALS_GameplayAbilitySet::GiveAbilities(UAbilitySystemComponent* AbilitySystemComponent, AActor* TargetActor) const
{
	if (TargetActor)
	{
		for (const FGameplayAbilityBindIWALS& BindInfo : Abilities)
		{
			if (BindInfo.GameplayAbilityClass && !HasAbility(AbilitySystemComponent, BindInfo.GameplayAbilityClass))
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(BindInfo.GameplayAbilityClass, 1, (int32)BindInfo.Command, TargetActor));
			}
		}
	}
}