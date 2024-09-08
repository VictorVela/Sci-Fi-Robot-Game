


#include "AsyncAnimInstance.h"

UAsyncAnimInstance::UAsyncAnimInstance()
{
	TimeSinceLastUpdate = 0.0f;
}

void UAsyncAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (AnimationUpdateInterval <= 0.0)
    {
        CustomTick(DeltaSeconds);
    }
    else
    {
        TimeSinceLastUpdate += DeltaSeconds;

        if (TimeSinceLastUpdate >= AnimationUpdateInterval)
        {
            CustomTick(AnimationUpdateInterval);

            TimeSinceLastUpdate = 0.0f;
        }
    }

}

void UAsyncAnimInstance::PreUpdateLinkedInstances(float DeltaSeconds)
{
    //bNeedsUpdate = false;
}


void UAsyncAnimInstance::SetNewCurveValue(FName CurveName, float Value)
{
    OverrideCurveValue(CurveName, Value);
}

void UAsyncAnimInstance::SetNeedsUpdate(bool Update)
{
    bNeedsUpdate = Update;
}
