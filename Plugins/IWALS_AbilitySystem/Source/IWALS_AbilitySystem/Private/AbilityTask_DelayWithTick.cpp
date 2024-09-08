


#include "AbilityTask_DelayWithTick.h"

UAbilityTask_DelayWithTick* UAbilityTask_DelayWithTick::DelayTaskWithTick(UGameplayAbility* OwningAbility, FName TaskInstanceName, float InDuration)
{
    UAbilityTask_DelayWithTick* Task = NewAbilityTask<UAbilityTask_DelayWithTick>(OwningAbility, TaskInstanceName);

    Task->Duration = InDuration;
    Task->ElapsedTime = 0.0f;

    return Task;
}

void UAbilityTask_DelayWithTick::Activate()
{
    Super::Activate();
    bTickingTask = true;
}

void UAbilityTask_DelayWithTick::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);

    ElapsedTime = ElapsedTime + DeltaTime;

    float NormalizedTime = FMath::GetMappedRangeValueClamped(FVector2D(0, Duration), FVector2D(0, 1), ElapsedTime);

    Tick.Broadcast(ElapsedTime, NormalizedTime);

    if (ElapsedTime >= Duration)
    {
        Finished.Broadcast(ElapsedTime,1);
        bTickingTask = false;
        EndTask();
    }
}
