

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_MoveByInputWithRot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDurningMove2, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLocationReached2, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFailed2, float, Time);


UCLASS()
class IWALS_ABILITYSYSTEM_API UAbilityTask_MoveByInputWithRot : public UAbilityTask
{
	GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
        static UAbilityTask_MoveByInputWithRot* MovePawnByInputWithRotation(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector TargetLocation, FRotator TargetRotation,
            float MaxDuration = 1.0, float DistanceTolerance = 10.0, float RotUpdateStartTime = 0.5, float RotationInterpSpeed = 10.0, bool UseLocationFixAtEnd = false);

    virtual void Activate() override;

    virtual void TickTask(float DeltaTime) override;

    UPROPERTY(BlueprintAssignable)
        FOnDurningMove2 DurningMove;

    UPROPERTY(BlueprintAssignable)
        FOnTargetLocationReached2 TargetLocationReached;

    UPROPERTY(BlueprintAssignable)
        FOnFailed2 Failed;

protected:
    UPROPERTY()
        FVector TargetLocation;

    UPROPERTY()
        float MaxDuration;

    UPROPERTY()
        float DistanceTolerance;

    UPROPERTY()
        FRotator TargetRotation;

    UPROPERTY()
        float RotUpdateStartTime;

    UPROPERTY()
        float RotationInterpSpeed;

    UPROPERTY()
        bool UseLocationFixAtEnd;

    UPROPERTY()
        float ElapsedTime;
	
};
