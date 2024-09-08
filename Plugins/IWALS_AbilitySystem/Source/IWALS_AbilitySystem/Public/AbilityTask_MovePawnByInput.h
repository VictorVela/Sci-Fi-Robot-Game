//JakubW

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_MovePawnByInput.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDurningMove, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLocationReached, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFailed, float, Time);

UCLASS()
class IWALS_ABILITYSYSTEM_API UAbilityTask_MovePawnByInput : public UAbilityTask
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAbilityTask_MovePawnByInput* MovePawnByInput(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector TargetLocation, float MaxDuration = 1.0, float DistanceTolerance = 10.0);

    virtual void Activate() override;

    virtual void TickTask(float DeltaTime) override;

    UPROPERTY(BlueprintAssignable)
    FOnDurningMove DurningMove;

    UPROPERTY(BlueprintAssignable)
    FOnTargetLocationReached TargetLocationReached;

    UPROPERTY(BlueprintAssignable)
    FOnFailed Failed;

protected:
    UPROPERTY()
    FVector TargetLocation;

    UPROPERTY()
    float MaxDuration;

    UPROPERTY()
    float DistanceTolerance;

    UPROPERTY()
    float ElapsedTime;
};