


#include "Cpp_LadderClimbingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UCpp_LadderClimbingComponent::UCpp_LadderClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCpp_LadderClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterC = Cast<ACharacter>(this->GetOwner());
	if (CharacterC)
	{
		DefCapsuleSizeC = FVector2D(CharacterC->GetCapsuleComponent()->GetScaledCapsuleRadius(), 
									CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}

	InitComponent();
	
}


// Called every frame
void UCpp_LadderClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UCpp_LadderClimbingComponent::InitComponent_Implementation()
{
	return false;
}

bool UCpp_LadderClimbingComponent::TryFinishLadderClimb_Implementation(bool IgnoreCondition)
{
	return false;
}

bool UCpp_LadderClimbingComponent::TryStartLadderClimb_Implementation(FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp)
{
	return false;
}

bool UCpp_LadderClimbingComponent::TryStartLadderClimbWhenHoldLedge_Implementation(FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp)
{
	return false;
}

bool UCpp_LadderClimbingComponent::CheckCanStartClimbLadder_Implementation(bool CheckMainCondition, bool Param2, FVector InLocation, FRotator InRotation, 
	FCALS_ComponentAndTransform& TargetCapPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp)
{
	return false;
}

bool UCpp_LadderClimbingComponent::CheckCanStartClimbLadderIteration_Implementation(bool CheckMainCondition, bool FromClimb, FVector InStartLocation, FVector InEndLocation, FRotator InRotation, 
	int Iterations, FCALS_ComponentAndTransform& TargetCapPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp)
{
	float Alpha = 0.0;

	for (int i = 0; i <= Iterations; i++)
	{
		Alpha = FMath::GetMappedRangeValueClamped(FVector2D(0.0, Iterations * 1.0), FVector2D(0.25, 1.0), i * 1.0);
		FVector TargetLoc = FMath::Lerp<FVector>(InEndLocation, InStartLocation, Alpha);

		const bool DetectedLadder = CheckCanStartClimbLadder(CheckMainCondition, FromClimb, TargetLoc, InRotation, TargetCapPositionWS, LadderActor, RungComp);
		if (DetectedLadder && IsValid(RungComp))
		{
			return DetectedLadder;
		}
	}
	return false;
}

bool UCpp_LadderClimbingComponent::GetMainValues_Implementation(bool& StartedLadderClimb, FCALS_ComponentAndTransform& TargetPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp)
{
	return false;
}

bool UCpp_LadderClimbingComponent::SetMainValues_Implementation(bool StartedLadderClimb, FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp)
{
	return false;
}

int UCpp_LadderClimbingComponent::ChooseMainPosesIndex_Implementation(FVector InitLocation, FRotator InitRotation)
{
	return 0;
}
