// Copyright Jakub W, All Rights Reserved. 

#include "Cpp_FallDamageAndSliding.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "HelpfulFunctionsBPLibrary.h"

// Sets default values for this component's properties
UCpp_FallDamageAndSliding::UCpp_FallDamageAndSliding()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCpp_FallDamageAndSliding::BeginPlay()
{
	Super::BeginPlay();
	CharC = Cast<ACharacter>(this->GetOwner());
	// ...
	
}


void UCpp_FallDamageAndSliding::SimpleTrajectoryC(bool& NotNull, TArray<FHitResult>& ReturnAllHits, TArray<float>& ReturnDistances, TArray<FHitResult>& ReturnWalkableHits, 
	ACharacter* Char, FVector StartLocation, FVector StartDirection, int Iterations, float OffsetScale, FVector Velocity, FVector Acceleration, bool ReturnOnlyValid)
{
	if (Velocity.Length() <= 5.0) //Check The Character Is Moving
	{ NotNull = false; ReturnAllHits = {}; return; }

	//Base Local Variables
	TArray<FHitResult> HitResultArray = {};
	TArray<float> DistancesArray = {};
	TArray<FHitResult> WalkableArray = {};
	FHitResult PredictHitResult = {};

	FVector PredictTraceStart, PredictTraceEnd = FVector(0, 0, 0);
	float NormalizedAccValue, BendingValue = 0.0;
	FVector VelocityDirection = FVector(0, 0, 0);
	FVector VelNormalized = Velocity;
	VelNormalized.Normalize(); //Normalize Velocity
	//Trace Config
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	TArray<AActor*> ToIgnore; ToIgnore.Add(Char);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }

	for (int i = 0; i <= Iterations-1; i++)
	{
		if (i == 0)
		{
			PredictTraceStart = StartLocation + (UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(StartDirection)) * 30.0);
			PredictTraceEnd = StartLocation + (UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(StartDirection)) * -40.0);
		}
		else
		{	
			NormalizedAccValue = UKismetMathLibrary::MapRangeClamped(FVector(Acceleration.X, Acceleration.Y, 0).Length(), 0.0, Char->GetCharacterMovement()->GetMaxAcceleration(), 0.0, 1.0);
			BendingValue = UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, Iterations - 1 * 1.0, 0.0, NormalizedAccValue);
			VelocityDirection = UKismetMathLibrary::SelectVector(VelNormalized, VelocityDirection, UKismetMathLibrary::EqualEqual_VectorVector(VelocityDirection,FVector(0,0,0),0.05));
			VelocityDirection = VelocityDirection + (FVector(Acceleration.X, Acceleration.Y, 0).GetSafeNormal() * BendingValue);
			VelocityDirection.Normalize();
			PredictTraceStart = PredictHitResult.Location + (VelocityDirection * OffsetScale) + 
			(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(PredictHitResult.Normal))) * 45.0);
			PredictTraceEnd = PredictHitResult.Location + (VelocityDirection * OffsetScale) +
			(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(PredictHitResult.Normal))) * -60.0);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, VelocityDirection.ToString());
		const bool HitValid = UKismetSystemLibrary::LineTraceSingle(Char, PredictTraceStart, PredictTraceEnd, Channel, false, ToIgnore, TraceType, PredictHitResult, true);
		if (HitValid == false)
		{
			PredictHitResult.Location = UKismetMathLibrary::VLerp(PredictTraceStart, PredictTraceEnd, 0.5);
			PredictHitResult.ImpactPoint = PredictHitResult.Location;
			PredictHitResult.Normal = FVector(0, 0, 1.0);
			PredictHitResult.ImpactNormal = FVector(0, 0, 1.0);
		}

		if (ReturnOnlyValid == true)
		{
			if (HitValid == true)
			{
				HitResultArray.Emplace(PredictHitResult);
				DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance(UKismetMathLibrary::VLerp(PredictTraceStart, PredictTraceEnd, 0.5), StartLocation));
				if (Char->GetCharacterMovement()->IsWalkable(PredictHitResult) == true)
				{ WalkableArray.Emplace(PredictHitResult); }
			}
		}
		else
		{
			HitResultArray.Emplace(PredictHitResult);
			DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance(UKismetMathLibrary::VLerp(PredictTraceStart, PredictTraceEnd, 0.5), StartLocation));
			if (Char->GetCharacterMovement()->IsWalkable(PredictHitResult) == true)
			{ WalkableArray.Emplace(PredictHitResult); }
		}
	}

	//Return
	NotNull = HitResultArray.Num() > 0;
	ReturnAllHits = HitResultArray;
	ReturnDistances = DistancesArray;
	ReturnWalkableHits = WalkableArray;
}

bool UCpp_FallDamageAndSliding::CheckCanStartSlidingC(FVector Acc, float SlopeDiffTollerance)
{
	FFindFloorResult FFR = {};
	bool SlopeDiffCorrect = true;

	SmoothAccelerationC = UKismetMathLibrary::VInterpTo(SmoothAccelerationC, Acc, UGameplayStatics::GetWorldDeltaSeconds(this), 15.0);
	if (SlidingConditionC() == true && StartSlidingC == false)
	{
		CharC->GetCharacterMovement()->FindFloor(CharC->GetActorLocation(), FFR, true); //Find Floor By Char Movement
		if (CharC->GetCharacterMovement()->CurrentFloor.bWalkableFloor == false && FFR.bBlockingHit == true)
		{
			bool TrajectoryNull = false; 
			TArray<FHitResult> HitResultArray = {};
			TArray<float> DistArr = {};
			TArray<FHitResult> WalkableArray = {};

			SimpleTrajectoryC(TrajectoryNull, HitResultArray, DistArr, WalkableArray, CharC, CharC->GetActorLocation() - FVector(0, 0, 
			CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), CharC->GetActorForwardVector(), 5, 22, CharC->GetVelocity(), SmoothAccelerationC, true);
			if (TrajectoryNull == false) { return false; } //Trejactory Is Valid
			if (HitResultArray.Num() <= 3 || WalkableArray.Num() > 0) { return false; } //Trajectory Is Valid

			int i = 0;
			for (const FHitResult& THit : HitResultArray)
			{
				if (HitResultArray.IsValidIndex(i + 1) == true)
				{
					if ((THit.ImpactPoint.Z - HitResultArray[i + 1].ImpactPoint.Z) <= SlopeDiffTollerance) 
					{ SlopeDiffCorrect = false; }
				}
				i = i + 1;
			}
			if (SlopeDiffCorrect == true)
			{
				PrepareToSlidingC(true); // Implement Start Action
				return true; 
			}
		}
		return false;
	}
	else
	{ return false; }
}

//Update Character Braking Friction Factor (Durning Sliding)
void UCpp_FallDamageAndSliding::UpdateCharFrictionDurningSliding(float& ReturnTheta, UCharacterMovementComponent* CMove, float InterpSpeed, float FrictionScale)
{
	FFindFloorResult FFR = {}; //Find Floor Result Structure
	float MaterialFriction = 1.0; //Default Material Friction
	float BFP1, BFP2 = 0.0; //Braking Friction Factor Update Steps
	float Theta = 45.0; //Angle Between Current Floor And Base Z Axis
	if (IsValid(CMove) == false) //Make Sure The Character Movement Is Valid
	{ ReturnTheta = 45.0; return; }
	CMove->FindFloor(CharC->GetActorLocation(), FFR, true); //Find Floor

	if (FFR.bBlockingHit == true) // ----> UPDATE BRAKING FRICTION FACTOR (Character Movement) <----
	{
		if (IsValid(FFR.HitResult.PhysMaterial.Get()) == true) //Get Material Friction (Scalar)
		{ MaterialFriction = FFR.HitResult.PhysMaterial.Get()->Friction; }
		Theta = UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(FFR.HitResult.Normal, FVector(0, 0, 1)));
		BFP1 = UKismetMathLibrary::MapRangeClamped(Theta, 25.0, 60.0, 1.3, 0.5);
		BFP2 = UKismetMathLibrary::MapRangeClamped(FFR.FloorDist, 0.2, 20.0, BFP1, SavedCharMoveValuesC.FrictionFactor);
		BFP2 = BFP2 * MaterialFriction * FrictionScale;
		CMove->BrakingFrictionFactor = UKismetMathLibrary::FInterpTo(CMove->BrakingFrictionFactor, BFP2, UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed);
	}
	else
	{	// ----> SET TO DEFAULT BRAKING FRICTION FACTOR (Character Movement) <----
		CMove->BrakingFrictionFactor = UKismetMathLibrary::FInterpTo(CMove->BrakingFrictionFactor, SavedCharMoveValuesC.FrictionFactor, UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed);
	}
	ReturnTheta = Theta;
	return;
}

//Update Character Walkable Floor Angle (Durning Sliding)
void UCpp_FallDamageAndSliding::UpdateCharWalkAngleDurningSliding(UCharacterMovementComponent* CMove, TArray<FHitResult> HitData, float ClampMin, float ClampMax, float InterpSpeed)
{
	if (IsValid(CMove) == false)
	{ return; }
	int AngleCount = 1;
	float AngleSum = 1.0;
	float AngleAvg = 0.0;
	TArray<FHitResult> ResizedHit = HitData;
	ResizedHit.SetNum(HitData.Num() / 2);
	for (const FHitResult& THit : ResizedHit)
	{
		if (THit.bBlockingHit == true)
		{
			AngleCount = AngleCount + 1;
			AngleSum = AngleSum + UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(THit.Normal, FVector(0, 0, 1)));
		}
	}
	AngleAvg = UKismetMathLibrary::FClamp(AngleSum / AngleCount - 20.0, ClampMin, ClampMax);
	CMove->SetWalkableFloorAngle(UKismetMathLibrary::FInterpTo(CMove->GetWalkableFloorAngle(), AngleAvg, UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed));
	return;
}

void UCpp_FallDamageAndSliding::UpdateAngleCorrectDurningSliding(float& ReturnDot, TArray<FHitResult> HitData, float Theta, float SlidingMinAngle, float InterpSpeed, bool CheckHitValid)
{
	int AngleCount = 1;
	float CrossCrossDotSum = 0.0;
	FVector CrossNormal = FVector(0, 0, 0);
	AngleConditionC = UKismetMathLibrary::FInterpTo(AngleConditionC, UKismetMathLibrary::SelectFloat(0.0, 1.0, SlidingMinAngle < Theta), UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed);
	for (const FHitResult& THit : HitData)
	{
		if (CheckHitValid == true)
		{
			if (THit.bBlockingHit == true)
			{
				AngleCount = AngleCount + 1;
				CrossNormal = UKismetMathLibrary::Cross_VectorVector(THit.Normal, CharC->GetActorRightVector());
				CrossCrossDotSum = CrossCrossDotSum + UKismetMathLibrary::Dot_VectorVector(CrossNormal, UKismetMathLibrary::Cross_VectorVector(HitData[0].Normal, CharC->GetActorRightVector()));
			}
		}
		else
		{
			AngleCount = AngleCount + 1;
			CrossNormal = UKismetMathLibrary::Cross_VectorVector(THit.Normal, CharC->GetActorRightVector());
			CrossCrossDotSum = CrossCrossDotSum + UKismetMathLibrary::Dot_VectorVector(CrossNormal, UKismetMathLibrary::Cross_VectorVector(HitData[0].Normal, CharC->GetActorRightVector()));
		}
	}
	ReturnDot = CrossCrossDotSum/AngleCount;
	return;
}

//FALL DAMAGE FUNCTION
void UCpp_FallDamageAndSliding::CheckCanMantleOnEdge(bool& CanStart, FVector& TargetCapLocation, FVector& SecondPoint, FVector& HitNormal1, FVector& HitNormal2,
	UPrimitiveComponent*& HitComponent, ECollisionChannel TChannel, int DrawDebugIndex, float RoomCheckCapScale)
{
	//Trace Config
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(TChannel);
	TArray<AActor*> ToIgnore; ToIgnore.Add(CharC);
	EDrawDebugTrace::Type TDebug = EDrawDebugTrace::None;
	if (DrawDebugIndex == 1)
	{ TDebug = EDrawDebugTrace::ForOneFrame; }
	else if(DrawDebugIndex == 2)
	{ TDebug = EDrawDebugTrace::ForDuration; }

	//Base Local Variables
	FVector TStart, TEnd = FVector(0, 0, 0);
	FTwoVectors SecondHitTransform = {};

	TStart = CharC->GetActorLocation() + FVector(0, 0, CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere());
	TEnd = CharC->GetActorLocation() + FVector(0, 0, CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight_WithoutHemisphere()*-1.025);
	FHitResult FTR = {}; //Run First Trace
	const bool HitValid1 = UKismetSystemLibrary::SphereTraceSingle(CharC, TStart, TEnd, CharC->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.025, Channel, false, ToIgnore, TDebug, FTR, 
	true, FLinearColor(0.09, 0, 0, 1), FLinearColor(0.6, 0, 0, 1), 2.5);

	if (HitValid1 == false) //If Trace Is Not Valid Finish Function
	{ CanStart = false; TargetCapLocation = CharC->GetActorLocation(); return; }

	TStart = FTR.ImpactPoint - FVector(0, 0, 10) + (UHelpfulFunctionsBPLibrary::NormalToVector(FTR.Normal) * -25.0);
	TEnd = FTR.ImpactPoint - FVector(0, 0, 10) + (UHelpfulFunctionsBPLibrary::NormalToVector(FTR.Normal) * 15.0);

	FHitResult STR = {}; //Run Second Trace
	const bool HitValid2 = UKismetSystemLibrary::SphereTraceSingle(CharC, TStart, TEnd, 5.0, Channel, false, ToIgnore, TDebug, STR,
	true, FLinearColor(0.13, 0, 0.11, 1), FLinearColor(1.0, 0.6, 0, 1), 3.0);

	if (HitValid2 == false || STR.bStartPenetrating==true ) //If Trace Is Not Valid Finish Function
	{ CanStart = false; TargetCapLocation = CharC->GetActorLocation(); return; }

	SecondHitTransform.v1 = STR.ImpactPoint;
	SecondHitTransform.v2 = STR.Normal;
	TStart = STR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STR.Normal) * 22.0) + FVector(0, 0, 30);
	TEnd = STR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STR.Normal) * 22.0) + FVector(0, 0, -20);

	FHitResult TTR = {};
	const bool HitValid3 = UKismetSystemLibrary::LineTraceSingle(CharC, TStart, TEnd, Channel, false, ToIgnore, TDebug, TTR, true, FLinearColor(1.0,0.11,0,1),FLinearColor(1,0,0.6,1),3.0);

	if (HitValid3 == false) //If Trace Is Not Valid Finish Function
	{ CanStart = false; TargetCapLocation = CharC->GetActorLocation(); return; }

	TargetCapLocation = TTR.ImpactPoint + FVector(0, 0, CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	TStart = SecondHitTransform.v1 + (UHelpfulFunctionsBPLibrary::NormalToVector(SecondHitTransform.v2) * CharC->GetCapsuleComponent()->GetScaledCapsuleRadius() * -1.0);
	SecondPoint = FVector(TStart.X, TStart.Y, TTR.ImpactPoint.Z - (CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.25));

	if (UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnore(this, CharC, TargetCapLocation, ToIgnore, RoomCheckCapScale, RoomCheckCapScale + 0.02, false) == true &&
		UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnore(this, CharC, SecondPoint, ToIgnore, RoomCheckCapScale, RoomCheckCapScale + 0.02, false) == true)
	{
		HitNormal1 = SecondHitTransform.v1;
		HitNormal2 = SecondHitTransform.v2;
		HitComponent = STR.GetComponent();
		CanStart = true;
		return;
	}
	CanStart = false; return;
}

// Called every frame
void UCpp_FallDamageAndSliding::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCpp_FallDamageAndSliding::PrepareToSlidingC_Implementation(bool CanStart)
{
	StartSlidingC = true; //Set Sliding State
	SavedCharMoveValuesC.WalkableFloorAngle = CharC->GetCharacterMovement()->GetWalkableFloorAngle(); //Save Char Movement Values
	SavedCharMoveValuesC.FrictionFactor = CharC->GetCharacterMovement()->BrakingFrictionFactor;
	SavedCharMoveValuesC.BrakingFriction = CharC->GetCharacterMovement()->BrakingFriction;
	SavedCharMoveValuesC.AirControl = CharC->GetCharacterMovement()->AirControl;

	CharC->UnCrouch(); //UnCrouch Character
	return;
}


bool UCpp_FallDamageAndSliding::SlidingConditionC_Implementation()
{
	return true;
}
