

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_DelayWithTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTick, float, Time, float, NormalizedTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTickFinished, float, Time, float, NormalizedTime);


UCLASS()
class IWALS_ABILITYSYSTEM_API UAbilityTask_DelayWithTick : public UAbilityTask
{
	GENERATED_BODY()
	
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
        static UAbilityTask_DelayWithTick* DelayTaskWithTick(UGameplayAbility* OwningAbility, FName TaskInstanceName, float Duration);

    virtual void Activate() override;

    virtual void TickTask(float DeltaTime) override;

    UPROPERTY(BlueprintAssignable)
        FOnTick Tick;

    UPROPERTY(BlueprintAssignable)
        FOnTickFinished Finished;

protected:
    UPROPERTY()
        float Duration;

    UPROPERTY()
        float ElapsedTime;


};
