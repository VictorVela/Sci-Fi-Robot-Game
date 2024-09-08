

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TimerFunction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExecuteTickDelegate);

UCLASS()
class IWALS_ABILITYSYSTEM_API UAbilityTask_TimerFunction : public UAbilityTask
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
        FExecuteTickDelegate OnTick;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
        static UAbilityTask_TimerFunction* RunTimerTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, float TimeBetweenTicks = 0.05);

    virtual void Activate() override;

protected:
    FTimerHandle TimerHandle;

    float TimeBetweenTicks;

    void ExecuteTickFunction();
	
};
