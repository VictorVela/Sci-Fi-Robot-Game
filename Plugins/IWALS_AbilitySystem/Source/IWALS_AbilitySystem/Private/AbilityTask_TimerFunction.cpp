


#include "AbilityTask_TimerFunction.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UAbilityTask_TimerFunction* UAbilityTask_TimerFunction::RunTimerTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, float InTimeBetweenTicks)
{
    UAbilityTask_TimerFunction* MyObj = NewAbilityTask<UAbilityTask_TimerFunction>(OwningAbility, TaskInstanceName);
    MyObj->TimeBetweenTicks = InTimeBetweenTicks;
    return MyObj;
}

void UAbilityTask_TimerFunction::Activate()
{
    Super::Activate();

    if (AbilitySystemComponent.Get())
    {
        AbilitySystemComponent.Get()->GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_TimerFunction::ExecuteTickFunction, TimeBetweenTicks, true);
    }
}

void UAbilityTask_TimerFunction::ExecuteTickFunction()
{
    OnTick.Broadcast();
}
