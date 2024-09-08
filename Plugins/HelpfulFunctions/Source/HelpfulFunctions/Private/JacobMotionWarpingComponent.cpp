// Copyright Jakub W, All Rights Reserved. 

#include "JacobMotionWarpingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
//#include "Engine/Public/TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UJacobMotionWarpingComponent::UJacobMotionWarpingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UJacobMotionWarpingComponent::BeginPlay()
{
	Super::BeginPlay();
	//Set References
	CharacterC = Cast<ACharacter>(this->GetOwner());
	SetComponentTickEnabled(false);
}

//GET CURVE VALUE BY INDEX
float UJacobMotionWarpingComponent::GetCurveValueByIndex(int ArrIndex)
{
	if (IsValid(CharacterC) == true && CurvesName.IsValidIndex(ArrIndex))
	{
		return CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(CurvesName[ArrIndex]);
	}
	return -1.0;
}

//GET ORIGIN POSITION
FVector UJacobMotionWarpingComponent::GetOriginPosition()
{
	return WarpingTarget->GetActorLocation() +
		UKismetMathLibrary::GetForwardVector(WarpingTarget->GetActorRotation()) * ConstOffset.X +
		UKismetMathLibrary::GetRightVector(WarpingTarget->GetActorRotation()) * ConstOffset.Y +
		UKismetMathLibrary::GetUpVector(WarpingTarget->GetActorRotation()) * ConstOffset.Z;
}

//GET TARGET AXIS
void UJacobMotionWarpingComponent::GetTargetAxis(FVector& Forward, FVector& Right, FVector& Up, float ScaleXY)
{
	Forward = UKismetMathLibrary::GetForwardVector(SavedWarpActorRot) * ScaleXY;
	Right = UKismetMathLibrary::GetRightVector(SavedWarpActorRot) * ScaleXY*-1.0;
	Up = WarpingTarget->GetActorUpVector();
}

//GET WARP ACTOR INFO
FVector UJacobMotionWarpingComponent::GetWarpActorInfo(AActor* InActor, FVector Offset, FRotator Rotation)
{
	if (IsValid(InActor) == true)
	{
		return InActor->GetActorLocation() + UKismetMathLibrary::GetForwardVector(Rotation) * Offset.X +
		UKismetMathLibrary::GetRightVector(Rotation) * Offset.Y + UKismetMathLibrary::GetUpVector(Rotation) * Offset.Z;
	}
	return FVector(0, 0, 0);
}

//GET WARP TARGET CURVE VALUE
void UJacobMotionWarpingComponent::GetWarpTagetCurveValue(float& X, float& Y, float& Z)
{
	if (IsValid(WarpingTarget) == true)
	{
		ACharacter* AsCharacter = Cast<ACharacter>(WarpingTarget);
		X = AsCharacter->GetMesh()->GetAnimInstance()->GetCurveValue(CurvesName[0]);
		Y = AsCharacter->GetMesh()->GetAnimInstance()->GetCurveValue(CurvesName[1]);
		Z = AsCharacter->GetMesh()->GetAnimInstance()->GetCurveValue(CurvesName[2]);
		return;
	}
	return;
}

//TRY FIX Z LOCATION
FVector UJacobMotionWarpingComponent::TryFixZPosition(bool Use, FVector CharLocation)
{
	FVector LocVector = CharLocation;
	if (Use == true)
	{
		LocVector.Z = CharacterC->GetActorLocation().Z;
		return LocVector;
	}
	else
	{ return CharLocation; }
}

//TRY CONVERT TO RELATIVE
FCALS_ComponentAndTransform UJacobMotionWarpingComponent::TryConvertToRelative(FVector VectorWS)
{
	FCALS_ComponentAndTransform AsStruct = {};
	AsStruct.Transform = FTransform(FRotator(0, 0, 0), VectorWS, FVector(1, 1, 1));
	AsStruct.Component = FloorComponent;

	if (IsValid(FloorComponent) == true)
	{
		return UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(AsStruct);
	}
	else
	{
		ACharacter* AsCharacter = Cast<ACharacter>(WarpingTarget);
		if (IsValid(AsCharacter) == true)
		{
			AsStruct.Component = AsCharacter->GetCapsuleComponent();
			return UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(AsStruct);
		}
		else
		{
			AsStruct.Component = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetCapsuleComponent();
			return UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(AsStruct);
		}
	}
}

//TIMER FUNCTION
void UJacobMotionWarpingComponent::TimerFunction1()
{ return; }

//TIMELINE BLEND IN
void UJacobMotionWarpingComponent::TimelineBlendIn(float Duration)
{
	BlendingDuration = Duration;
 	GetWorld()->GetTimerManager().SetTimer(TimerHandle1, this, &UJacobMotionWarpingComponent::TimerFunction1, Duration, false);
	SetComponentTickEnabled(true);
}

//PREDICTED SINGLE MOTION
void UJacobMotionWarpingComponent::PredictedSingleMotionPosition(bool& Valid, bool& HaveRoom, FVector& ReturnLocation, ACharacter* WarpTargetActor, FVector ConstantOffset,
float NormalizedTime, UCurveVector* RootMotionChar, UCurveVector* RootMotionWarpRot, bool ExcludeWarpActorRoot, bool UseUpAxis, FRotator ApplyCustomRotator)
{
	FVector RootMotionA, RootMotionB = FVector(0, 0, 0);
	FRotator WarpActorRotation = FRotator(0, 0, 0);
	float TS, TE = 0.0;
	RootMotionChar->GetTimeRange(TS, TE);
	RootMotionA = RootMotionChar->GetVectorValue(UKismetMathLibrary::MapRangeClamped(NormalizedTime, 0.0, 1.0, TS, TE));
	RootMotionWarpRot->GetTimeRange(TS, TE);
	RootMotionB = RootMotionWarpRot->GetVectorValue(UKismetMathLibrary::MapRangeClamped(NormalizedTime, 0.0, 1.0, TS, TE));

	WarpActorRotation = UKismetMathLibrary::SelectRotator(ApplyCustomRotator, WarpTargetActor->GetActorRotation(), ApplyCustomRotator.Yaw != 0.0);
	FVector ExcludeRoot, LocalTargetActorLoc = FVector(0, 0, 0);
	if (ExcludeWarpActorRoot == true)
	{
		ExcludeRoot = UKismetMathLibrary::GetForwardVector(WarpActorRotation) * 1.0 * RootMotionB.X +
					  UKismetMathLibrary::GetRightVector(WarpActorRotation) * -1.0 * RootMotionB.Y +
					  UKismetMathLibrary::GetUpVector(WarpActorRotation) * 1.0 * RootMotionB.Z;
	}
	LocalTargetActorLoc = GetWarpActorInfo(WarpTargetActor, ConstantOffset, WarpActorRotation) - ExcludeRoot;
	FVector NextLocation = LocalTargetActorLoc + UKismetMathLibrary::GetForwardVector(WarpActorRotation) * -1.0 * RootMotionA.X +
	UKismetMathLibrary::GetRightVector(WarpActorRotation) * 1.0 * RootMotionA.Y + UKismetMathLibrary::GetUpVector(WarpActorRotation) * 1.0 * RootMotionA.Z;
	if (DevMode == true)
	{
		DrawDebugPoint(GetWorld(), NextLocation, 25.0, FColor(255, 124, 0), false, 1.5);
		DrawDebugDirectionalArrow(GetWorld(), WarpTargetActor->GetActorLocation(), WarpTargetActor->GetActorLocation() + 
		(UKismetMathLibrary::GetForwardVector(WarpActorRotation) * 55.0), 35.0, FColor(70, 255, 0), false, 2.5, 0, 2.0);
		DrawDebugDirectionalArrow(GetWorld(), WarpTargetActor->GetActorLocation(), WarpTargetActor->GetActorLocation() +
		(WarpTargetActor->GetActorForwardVector() * 51.0), 30.0, FColor(255, 193, 0), false, 2.0, 0, 2.0);
	}
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(WarpTargetActor);
	Valid = true;
	HaveRoom = UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnoreTransform(this, WarpTargetActor, FTransform(WarpTargetActor->GetActorRotation(), 
	NextLocation, FVector(1, 1, 1)), ToIgnore, 0.92, 0.88, false);
	ReturnLocation = NextLocation;
	return;
}

void UJacobMotionWarpingComponent::PredictedSingleMotionPositionAuto(bool& Valid, bool& HaveRoom, FVector& ReturnLocation, ACharacter* WarpTargetActor, UAnimSequence* AnimSeqAtt, 
	UAnimSequence* AnimSeqVic, FVector WarpingOrigin, FVector ConstantOffset, float NormalizedTime, bool ExcludeWarpActorRoot, bool UseUpAxis, FRotator ApplyCustomRotator, bool InvertAxis)
{
	FVector RootMotionA, RootMotionB = FVector(0, 0, 0);
	FRotator WarpActorRotation = FRotator(0, 0, 0);
	float TE = 0.0;
	TE = AnimSeqAtt->GetPlayLength();
	RootMotionA = UHelpfulFunctionsBPLibrary::GetRootMotionValueAtTime(CharacterC, AnimSeqAtt, UKismetMathLibrary::MapRangeClamped(NormalizedTime, 0.0, 1.0, 0.0, TE));
	TE = AnimSeqVic->GetPlayLength();
	RootMotionB = UHelpfulFunctionsBPLibrary::GetRootMotionValueAtTime(CharacterC, AnimSeqVic, UKismetMathLibrary::MapRangeClamped(NormalizedTime, 0.0, 1.0, 0.0, TE));

	if (InvertAxis == true)
	{
		RootMotionA = FVector(RootMotionA.Y, RootMotionA.X, RootMotionA.Z);
		RootMotionB = FVector(RootMotionB.Y, RootMotionB.X, RootMotionB.Z);
	}
	WarpActorRotation = UKismetMathLibrary::SelectRotator(ApplyCustomRotator, WarpTargetActor->GetActorRotation(), ApplyCustomRotator.Yaw != 0.0);
	FVector ExcludeRoot, LocalTargetActorLoc = FVector(0, 0, 0);
	if (ExcludeWarpActorRoot == true)
	{
		ExcludeRoot = UKismetMathLibrary::GetForwardVector(WarpActorRotation) * 1.0 * RootMotionB.X +
			UKismetMathLibrary::GetRightVector(WarpActorRotation) * -1.0 * RootMotionB.Y +
			UKismetMathLibrary::GetUpVector(WarpActorRotation) * 1.0 * RootMotionB.Z;
	}
	LocalTargetActorLoc = WarpingOrigin + (UKismetMathLibrary::GetForwardVector(WarpActorRotation) * ConstantOffset.X + 
	UKismetMathLibrary::GetRightVector(WarpActorRotation) * ConstantOffset.Y + UKismetMathLibrary::GetUpVector(WarpActorRotation) * ConstantOffset.Z) - ExcludeRoot;

	FVector NextLocation = LocalTargetActorLoc + UKismetMathLibrary::GetForwardVector(WarpActorRotation) * -1.0 * RootMotionA.X +
		UKismetMathLibrary::GetRightVector(WarpActorRotation) * 1.0 * RootMotionA.Y + UKismetMathLibrary::GetUpVector(WarpActorRotation) * 1.0 * RootMotionA.Z;
	if (DevMode == true)
	{
		DrawDebugPoint(GetWorld(), NextLocation, 25.0, FColor(255, 124, 0), false, 1.5);
		DrawDebugDirectionalArrow(GetWorld(), WarpTargetActor->GetActorLocation(), WarpTargetActor->GetActorLocation() +
			(UKismetMathLibrary::GetForwardVector(WarpActorRotation) * 55.0), 35.0, FColor(70, 255, 0), false, 2.5, 0, 2.0);
		DrawDebugDirectionalArrow(GetWorld(), WarpTargetActor->GetActorLocation(), WarpTargetActor->GetActorLocation() +
			(WarpTargetActor->GetActorForwardVector() * 51.0), 30.0, FColor(255, 193, 0), false, 2.0, 0, 2.0);
	}
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(WarpTargetActor);
	Valid = true;
	HaveRoom = UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnoreTransform(this, WarpTargetActor, FTransform(WarpTargetActor->GetActorRotation(),
		NextLocation, FVector(1, 1, 1)), ToIgnore, 0.92, 0.88, false);
	ReturnLocation = NextLocation;
	return;
}

//RETURN VALUES
void UJacobMotionWarpingComponent::ResetValues()
{
	LockWarpingActor = false;
	ExcludedRootValue = FVector(0, 0, 0);
}

// >>>> SET UP MOTION WARPING <<<< 
void UJacobMotionWarpingComponent::SetMotionWarpingTarget(FName WarpTargetName, AActor* TargetActor, FVector ConstantOffset, FRotator ConstRotOffset, bool ExcludeRootMotion)
{
	WarpingTarget = TargetActor;
	ConstOffset = ConstantOffset;
	ExcludeRootMotionV = ExcludeRootMotion;
	LockWarpingActor = false;
	ExcludedRootValue = FVector(0, 0, 0);
	RotationOffset = ConstRotOffset;
	AlphaByTimeline = 1.0;
	FloorComponent = nullptr;

	if (WarpKeys.Find(WarpTargetName) != -1)
	{
		WarpKeys.Add(WarpTargetName);
		if (IsValid(WarpingTarget) == true)
		{
			SavedWarpActorRot = UKismetMathLibrary::NormalizedDeltaRotator(WarpingTarget->GetActorRotation(), RotationOffset);
		}
	}
}

// >>>> SET UP MOTION WARPING <<<< 
void UJacobMotionWarpingComponent::StopUpdatingWarpPoint(bool StopUpdating)
{
	if (StopUpdating == false)
	{
		ExcludeRootMotionV = false; return;
	}
	LockWarpingActor = true; ExcludeRootMotionV = false;
	if (IsValid(WarpingTarget) == false) { return; }
	//Config Trace
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);
	ToIgnore.Add(WarpingTarget);
	FHitResult TraceResult = {};
	const bool TraceValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, WarpingTarget->GetActorLocation() + FVector(0, 0, 5), WarpingTarget->GetActorLocation()
		+ FVector(0, 0, -150), 3.0, Channel, false, ToIgnore, TraceType, TraceResult, true);
	if (TraceValid == true)
	{
		FloorComponent = TraceResult.GetComponent();
		TargetActorLocationLS = TryConvertToRelative(TargetActorLocation);
		return;
	}
	return;
}

// >>>> MAIN FUNCTION - TICK <<<< 
void UJacobMotionWarpingComponent::WarpingUpdate(FName X, FName Y, FName Z, FName R, bool UseUpAxis, bool WithSweep, bool WarpRotation, float StrengthAlpha, FName ConstAlphaCurve)
{
	CurvesName = { X,Y,Z,R };
	if (IsValid(WarpingTarget) == false)
	{
		return;
	}
	//Step 1)
	float NormalizeByAlpha = 1;
	if (ConstAlphaCurve != "")
	{
		NormalizeByAlpha = UKismetMathLibrary::FClamp(CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(ConstAlphaCurve), 0.05, 1.0);
	}
	if (LockWarpingActor == false)
	{
		SavedWarpActorRot = UKismetMathLibrary::NormalizedDeltaRotator(WarpingTarget->GetActorRotation(), RotationOffset);
	}
	//Step 2)
	FVector FV, RV, ZV = FVector(0, 0, 0);
	float FCV, RCV, ZCV = 1.0;
	if (ExcludeRootMotionV == true)
	{
		GetTargetAxis(FV, RV, ZV, 1.0);
		FV = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(RotationOffset), FV);
		RV = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(RotationOffset), RV);
		ZV = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(RotationOffset), ZV);
		GetWarpTagetCurveValue(FCV, RCV, ZCV);
		FV = FV * FCV;
		RV = RV * RCV;
		ZV = ZV * ZCV;
		ExcludedRootValue = FV + RV + ZV;
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, ExcludedRootValue.ToString());
	}
	else
	{
		ExcludedRootValue = ExcludedRootValue;
	}
	if (LockWarpingActor == false)
	{
		TargetActorLocation = GetOriginPosition() - ExcludedRootValue;
	}
	//Step 3)
	FVector NextLocation, DeltaOffsetValue = FVector(0, 0, 0);
	FVector ActorLocWithGroudOffset = UKismetMathLibrary::SelectVector(UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(TargetActorLocationLS).Transform.GetLocation(),
		TargetActorLocation, IsValid(FloorComponent) && LockWarpingActor);
	//Step 4)
	GetTargetAxis(FV, RV, ZV, -1.0);
	FV = FV * GetCurveValueByIndex(0) / NormalizeByAlpha;
	RV = RV * GetCurveValueByIndex(1) / NormalizeByAlpha;
	ZV = ZV * GetCurveValueByIndex(2) / NormalizeByAlpha;
	NextLocation = TryFixZPosition(UseUpAxis, ActorLocWithGroudOffset + FV + RV + ZV);
	DeltaOffsetValue = (NextLocation - CharacterC->GetActorLocation()) * StrengthAlpha * pow(AlphaByTimeline, 2);
	//Step 5)
	FHitResult* SweepResult = {};
	FRotator DeltaRotation = FRotator();

	CharacterC->AddActorWorldOffset(DeltaOffsetValue, WithSweep, SweepResult, ETeleportType::None); //Move Character

	if (WarpRotation == true)
	{
		DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			UKismetMathLibrary::NormalizedDeltaRotator(
				UKismetMathLibrary::MakeRotFromX(
					UKismetMathLibrary::GetForwardVector(SavedWarpActorRot) * -1.0), FRotator(0, GetCurveValueByIndex(3), 0)), CharacterC->GetActorRotation());

		CharacterC->AddActorWorldRotation(DeltaRotation * StrengthAlpha, false, nullptr, ETeleportType::None); //Rotate Character
	}
	//Step 6)
	CharacterC->GetCharacterMovement()->Velocity = FVector(0, 0, 0);
	ACharacter* WarpAsCharacter = Cast<ACharacter>(WarpingTarget);
	if (IsValid(WarpAsCharacter) == true)
	{
		WarpAsCharacter->GetCharacterMovement()->Velocity = FVector(0, 0, 0);
	}
	//Step 7) DEBUG
	if (DevMode == true)
	{
		APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (IsValid(Camera) == false)
		{
			return;
		}
		DrawDebugSphere(GetWorld(), TargetActorLocation, 15.0, 12, FColor(60.0f, 204.0f, 0.0f), false, 0.0, 0, 0.25);
		DrawDebugCapsule(GetWorld(), NextLocation, 90.0, 25.0, UKismetMathLibrary::Conv_RotatorToQuaternion(SavedWarpActorRot), FColor(248.0f, 255.0f, 48.0f), false, 0.0, 0, 0.1);
		DrawDebugString(GetWorld(), CharacterC->GetActorLocation() + (Camera->GetActorRightVector() * 40.0 + Camera->GetActorUpVector() * 40), TEXT("LockWarpingActor = ")
			+ UKismetMathLibrary::SelectString("True", "False", LockWarpingActor), nullptr, FColor(255, 76, 45), 0.0, false, 1.0);
		DrawDebugString(GetWorld(), CharacterC->GetActorLocation() + (Camera->GetActorRightVector() * 40.0 + Camera->GetActorUpVector() * 32), TEXT("ExcludeRootMotion = ")
			+ UKismetMathLibrary::SelectString("True", "False", ExcludeRootMotionV), nullptr, FColor(255, 76, 45), 0.0, false, 1.0);
	}
	return;
}

// Called every frame
void UJacobMotionWarpingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
	if (IsValid(WarpingTarget) == true)
	{
		if (GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle1) != -1.0)
		{
			AlphaByTimeline = UKismetMathLibrary::MapRangeClamped(GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle1), 0.0, BlendingDuration, 1.0, 0.0);
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FVector(AlphaByTimeline,0,0).ToString());
		}
		else
		{
			AlphaByTimeline = 1.0;
			SetComponentTickEnabled(false);
		}
	}
}

