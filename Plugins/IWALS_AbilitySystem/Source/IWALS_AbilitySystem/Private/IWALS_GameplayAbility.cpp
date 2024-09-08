


#include "IWALS_GameplayAbility.h"

void UIWALS_GameplayAbility::SendToCharacterSpecHandle()
{
	FGameplayAbilitySpecHandle CurrentHandle = CurrentSpecHandle;
	AGAS_MainCharacterCpp* TargetChar = Cast<AGAS_MainCharacterCpp>(GetAvatarActorFromActorInfo());
	if (TargetChar)
	{
		TargetChar->AbilityHandle = CurrentHandle;
	}
	return;
}
