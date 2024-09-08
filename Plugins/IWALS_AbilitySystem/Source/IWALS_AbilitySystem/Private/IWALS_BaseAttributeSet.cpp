
#include "IWALS_BaseAttributeSet.h"
#include "Net/UnrealNetwork.h"


UIWALS_BaseAttributeSet::UIWALS_BaseAttributeSet()
{
}

void UIWALS_BaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UIWALS_BaseAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIWALS_BaseAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIWALS_BaseAttributeSet, AttackMultiply, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UIWALS_BaseAttributeSet, Experience, COND_None, REPNOTIFY_Always);
}

void UIWALS_BaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIWALS_BaseAttributeSet, Health, OldHealth);
}

void UIWALS_BaseAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIWALS_BaseAttributeSet, Armor, OldArmor);
}

void UIWALS_BaseAttributeSet::OnRep_AttackMultiply(const FGameplayAttributeData& OldAttackMultiply)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIWALS_BaseAttributeSet, AttackMultiply, OldAttackMultiply);
}

void UIWALS_BaseAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIWALS_BaseAttributeSet, Experience, OldExperience);
}
