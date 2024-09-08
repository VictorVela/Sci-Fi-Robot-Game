


#include "AbilityTask_MoveByInputWithRot.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"


UAbilityTask_MoveByInputWithRot* UAbilityTask_MoveByInputWithRot::MovePawnByInputWithRotation(UGameplayAbility* OwningAbility, FName TaskInstanceName, FVector InTargetLocation, 
    FRotator InTargetRotation, float InMaxDuration, float InDistanceTolerance, float InRotUpdateStartTime, float InRotationInterpSpeed, bool InUseLocationFixAtEnd)
{
    UAbilityTask_MoveByInputWithRot* Task = NewAbilityTask<UAbilityTask_MoveByInputWithRot>(OwningAbility, TaskInstanceName);

    Task->TargetLocation = InTargetLocation;
    Task->MaxDuration = InMaxDuration;
    Task->DistanceTolerance = InDistanceTolerance;
    Task->TargetRotation = InTargetRotation;
    Task->RotUpdateStartTime = InRotUpdateStartTime;
    Task->RotationInterpSpeed = InRotationInterpSpeed;
    Task->UseLocationFixAtEnd = InUseLocationFixAtEnd;
    Task->ElapsedTime = 0.0f;

    return Task;
}

void UAbilityTask_MoveByInputWithRot::Activate()
{
    Super::Activate();
    bTickingTask = true;
}

void UAbilityTask_MoveByInputWithRot::TickTask(float DeltaTime)
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

    if (ElapsedTime >= RotUpdateStartTime)
    {
        Pawn->SetActorRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(UKismetMathLibrary::RInterpTo(Pawn->GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed)));
    }

    DurningMove.Broadcast(ElapsedTime);

    float DistanceToTarget = UKismetMathLibrary::Vector_Distance2D(CurrentLocation, TargetLocation);

    if (DistanceToTarget <= DistanceTolerance)
    {
        if (UseLocationFixAtEnd == true)
        {
            Pawn->SetActorLocation(TargetLocation, true);
        }

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
