

#include "AbilityTask_MovePawnByInput.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UAbilityTask_MovePawnByInput* UAbilityTask_MovePawnByInput::MovePawnByInput(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector InTargetLocation, float InMaxDuration, float InDistanceTolerance)
{
    UAbilityTask_MovePawnByInput* Task = NewAbilityTask<UAbilityTask_MovePawnByInput>(OwningAbility, TaskInstanceName);

    Task->TargetLocation = InTargetLocation;
    Task->MaxDuration = InMaxDuration;
    Task->DistanceTolerance = InDistanceTolerance;
    Task->ElapsedTime = 0.0f;

    return Task;
}

void UAbilityTask_MovePawnByInput::Activate()
{
    Super::Activate();
    bTickingTask = true;
}

void UAbilityTask_MovePawnByInput::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);

    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!IsValid(Pawn))
    {
        return;
    }

    ElapsedTime += DeltaTime;

    FVector CurrentLocation = Pawn->GetActorLocation();
    FRotator DirectionToTarget = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
    FVector WorldDirection = DirectionToTarget.Vector();

    Pawn->AddMovementInput(WorldDirection);

    DurningMove.Broadcast(ElapsedTime);

    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    if (DistanceToTarget <= DistanceTolerance)
    {
        TargetLocationReached.Broadcast(ElapsedTime);
        bTickingTask = false;
        EndTask();
    }
    else if (ElapsedTime >= MaxDuration)
    {
        Failed.Broadcast(ElapsedTime);
        bTickingTask = false;
        EndTask();
    }
}