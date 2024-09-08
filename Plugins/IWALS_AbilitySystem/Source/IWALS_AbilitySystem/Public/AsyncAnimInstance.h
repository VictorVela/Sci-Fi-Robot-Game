

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AsyncAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class IWALS_ABILITYSYSTEM_API UAsyncAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tick Interval")
    float AnimationUpdateInterval = 0.0165;

protected:
    float TimeSinceLastUpdate;

public:
    UAsyncAnimInstance();

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    virtual void PreUpdateLinkedInstances(float DeltaSeconds) override;

    UFUNCTION(BlueprintImplementableEvent)
    void CustomTick(float DeltaSecond);
	
    UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe))
    void SetNewCurveValue(FName CurveName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe))
    void SetNeedsUpdate(bool Update);

};
