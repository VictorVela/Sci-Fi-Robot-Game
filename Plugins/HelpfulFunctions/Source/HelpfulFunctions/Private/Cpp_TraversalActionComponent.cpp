


#include "Cpp_TraversalActionComponent.h"

//include gameFramework
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
//include math
#include "Kismet/KismetMathLibrary.h"
#include "Math/Vector.h"

#define KML UKismetMathLibrary
#define HLF UHelpfulFunctionsBPLibrary
#define CAP RefChar->GetCapsuleComponent()
#define WALKABLE RefChar->GetCharacterMovement()->IsWalkable
#define ToLOCAL UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix
#define ToWORLD UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix

FColor HexToColor(const FString& HexString)
{
	uint32 HexValue;
	if (FParse::HexNumber(*HexString))
	{
		HexValue = FParse::HexNumber(*HexString);
		return FColor(
			(HexValue >> 16) & 0xFF, // R
			(HexValue >> 8) & 0xFF,  // G
			HexValue & 0xFF,         // B
			(HexValue >> 24) & 0xFF  // A (optional, default to 0xFF)
		);
	}
	return FColor::Black; // Default color if parsing fails
}

FTransform SubtractTransforms(FTransform A, FTransform B)
{
	const FVector Position = A.GetLocation() - B.GetLocation();
	const FRotator Rotation = KML::NormalizedDeltaRotator(A.Rotator(), B.Rotator());
	return FTransform(Rotation, Position, FVector(1, 1, 1));
}


void UCpp_TraversalActionComponent::DrawSimpleDebugCircle(FVector Location, FRotator Rotation, float Radius, FColor Color, float DrawTime, float Thickness)
{
	FTransform T = FTransform::Identity;

	T.SetRotation(KML::Conv_RotatorToQuaternion(FRotator(90, Rotation.Yaw, 0)));
	T.SetLocation(Location);
	const FMatrix M = KML::Conv_TransformToMatrix(T);

	DrawDebugCircle(GetWorld(), M, Radius, 12, Color, false, DrawTime, 0, Thickness);
}

// Sets default values for this component's properties
UCpp_TraversalActionComponent::UCpp_TraversalActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCpp_TraversalActionComponent::BeginPlay()
{
	Super::BeginPlay();

	RefChar = Cast<ACharacter>(this->GetOwner());

	UpdateTraveralStatesData_Implementation(); // <- Convert Data Assets to TArray

}


// Called every frame
void UCpp_TraversalActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

float UCpp_TraversalActionComponent::GetShapesDrawTime()
{
	return ShapesDebugTime;
}

bool UCpp_TraversalActionComponent::MakeSureConditionIsRequired(int ConditionIndex, TArray<bool> ConditionsValidList)
{
	if (ConditionsValidList.Num() == 0) { return false; }
	if (ConditionsValidList.IsValidIndex(ConditionIndex) == false) { return false; }
	if (ConditionsValidList[ConditionIndex] == true) { return true; }
	return false;
}

/*
bool UCpp_TraversalActionComponent::CheckTraversalActionFast(FTraversalCollisionCheck& CheckingResult, FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel)
{
	return false;
}
*/

bool UCpp_TraversalActionComponent::CheckTraversalActionFast_Implementation(FTraversalCollisionCheck& CheckingResult, FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel)
{
	if (IsValid(RefChar) == false) { return false; }
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugTraceIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	//Step 0 (Initialize)
	FTraversalCollisionCheck RP; 
	FCALS_ComponentAndTransform FrontLedgePos, BackLedgePos;

	//Step 1
	const float TargetReachDistance = TraceSettings.ReachDistance + GetReachOffset_Implementation();
	const float VerticalCapsuleOffset = (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2;
	const float VerticalCapsuleHeight = (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2;
	UPrimitiveComponent* HitComponent = nullptr;

	//Step 2 - Try find object in front of capsule location
	FVector InitialTraceImpact, InitialTraceNormal, DownTraceLoc; //Init Main Vectors
	FVector TraceStart = (GetLocationFromBase(2.0) + GetPlayerMovementInput_Implementation() * -30.0) + FVector(0,0,VerticalCapsuleOffset);
	FVector TraceEnd = TraceStart + (GetPlayerMovementInput_Implementation() * TargetReachDistance);
	FHitResult VerticalHitResult;
	const bool VerticalHitValid = UKismetSystemLibrary::CapsuleTraceSingle(RefChar, TraceStart, TraceEnd, TraceSettings.ForwardTraceRadius, VerticalCapsuleHeight, TraceChannel, false, 
		ActorsToIgnoreList, TraceType, VerticalHitResult, true, FColor::Black, FColor::Blue, DrawDebugTime);
	if (VerticalHitValid == true && VerticalHitResult.bStartPenetrating == false && WALKABLE(VerticalHitResult) == false)
	{
		if (VerticalHitResult.GetComponent()->IsSimulatingPhysics(VerticalHitResult.BoneName) == true) //Check that detected component is simulating physic
		{
			if (VerticalHitResult.GetComponent()->GetComponentVelocity().Length() > 10) { return false; } //Make sure that detected object is not moving with high speed
		}
		InitialTraceImpact = VerticalHitResult.ImpactPoint; InitialTraceNormal = VerticalHitResult.ImpactNormal;
	}
	else
	{ return false; } //When trace is not detected collision just stop next calculations

	//Step 3 - Try find Ledge Point from detected object
	FHitResult LedgeHitResult;
	float ForwardOffset = -15.0; //Maybe For Change???
	for (int i = 0; i < TraceSettings.MaxForwarIterations; i++)
	{
		ForwardOffset = -15.0 + (float(i) * 8);
		TraceEnd = FVector(InitialTraceImpact.X, InitialTraceImpact.Y, GetLocationFromBase(5.0).Z + TraceSettings.DownwardTraceRadius) + (HLF::NormalToVector(InitialTraceNormal) * ForwardOffset);
		TraceStart = TraceEnd + FVector(0, 0, TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius);
		
		const bool LedgeHitValid = HLF::FindGapBySphereTrace(RefChar, TraceStart, TraceEnd, TraceSettings.DownwardTraceRadius, TraceChannel, false, ActorsToIgnoreList, DebugTraceIndex,
			LedgeHitResult, true, FColor::Cyan, FColor::Green, DrawDebugTime, 6);

		if (LedgeHitResult.bBlockingHit == true && WALKABLE(LedgeHitResult) == true)
		{
			HitComponent = LedgeHitResult.GetComponent(); DownTraceLoc = LedgeHitResult.ImpactPoint;
			break;
		}
	}
	if (LedgeHitResult.bBlockingHit == false || WALKABLE(LedgeHitResult) == false) { return false; }

	//Step 4 Normalized to origin of ledge and check free space for target capsule position
	NormalizeLedgeToObjectCenter(DownTraceLoc, HitComponent, DownTraceLoc, InitialTraceNormal, TraceSettings);

	TArray<AActor*> RoomCheckIgnore;
	if (HLF::CapsuleHaveRoomWithIgnore(RefChar, RefChar, ToCapsuleLocation(DownTraceLoc), RoomCheckIgnore, 0.98, 0.96, false) == false)
	{ return false; }
	FrontLedgePos.Transform = FTransform(KML::MakeRotFromX(HLF::NormalToVector(InitialTraceNormal)), DownTraceLoc, FVector(1, 1, 1));
	FrontLedgePos.Component = HitComponent;

	RP.HasFrontLedge = true;
	RP.FrontLedgeTransform = FrontLedgePos;
	RP.ObstacleHeight = abs(DownTraceLoc.Z - GetLocationFromBase(0.0).Z);

	//Step 5 - Try find back ledge
	FVector BackLedgeStart = DownTraceLoc - FVector(0, 0, 3);
	FVector BackTraceImpact, BackTraceNormal;

	FHitResult BackLedgeHit, WalkableLedgeHit;
	bool BackLedgeValid = UKismetSystemLibrary::SphereTraceSingle(RefChar, BackLedgeStart + (InitialTraceNormal * BackLedgeSearchLenght * -1.0), BackLedgeStart + (InitialTraceNormal * -5.0), 4.0, TraceChannel, 
		false, ActorsToIgnoreList, TraceType, BackLedgeHit, true, FColor::Black, FColor::Orange, DrawDebugTime);
	if (BackLedgeValid == true && BackLedgeHit.bStartPenetrating == false && WALKABLE(BackLedgeHit) == false)
	{
		if (BackLedgeHit.GetComponent()->IsSimulatingPhysics(BackLedgeHit.BoneName) == true) //Check that detected component is simulating physic
		{
			if (BackLedgeHit.GetComponent()->GetComponentVelocity().Length() < 10) //Make sure that detected object is not moving with high speed
			{
				BackTraceImpact = BackLedgeHit.ImpactPoint; BackTraceNormal = BackLedgeHit.ImpactNormal;
				
			}
		}
		else
		{
			BackTraceImpact = BackLedgeHit.ImpactPoint; BackTraceNormal = BackLedgeHit.ImpactNormal;
		}
	}
	BackLedgeStart = BackLedgeStart + (InitialTraceNormal * BackLedgeSearchLenght * -1.0);

	//Step 6 - Find Back Ledge Point
	if (BackTraceImpact.IsNearlyZero() == false && BackLedgeHit.bStartPenetrating == false)
	{
		BackLedgeValid = UKismetSystemLibrary::SphereTraceSingle(RefChar, BackTraceImpact + (HLF::NormalToVector(BackTraceNormal) * 8.0) + FVector(0, 0, 25.0), BackTraceImpact +
			(HLF::NormalToVector(BackTraceNormal) * 8.0) + FVector(0, 0, -25.0), 8.0, TraceChannel, false, ActorsToIgnoreList, TraceType, WalkableLedgeHit, true, FColor::Red, FColor::Yellow, DrawDebugTime);
		if (WalkableLedgeHit.bBlockingHit == true && WALKABLE(WalkableLedgeHit) == true && WalkableLedgeHit.bStartPenetrating == false)
		{
			if (HLF::CapsuleHaveRoomWithIgnore(RefChar, RefChar, ToCapsuleLocation(WalkableLedgeHit.ImpactPoint, 2.0, false), RoomCheckIgnore, 0.98, 0.96, false) == true)  // <-- Trace Disabled?
			{
				BackLedgePos.Component = WalkableLedgeHit.GetComponent();
				BackLedgePos.Transform = FTransform(KML::MakeRotFromX(HLF::NormalToVector(BackTraceNormal)), FVector(BackTraceImpact.X, BackTraceImpact.Y,
					WalkableLedgeHit.ImpactPoint.Z), FVector(1, 1, 1));
				RP.HasBackLedge = true;
				RP.BackLedgeTransform = BackLedgePos;
			}
		}
	}

	//Step 7
	FVector CalcDepth;
	if (RP.HasBackLedge == true)
	{
		CalcDepth = FrontLedgePos.Transform.GetLocation() - BackLedgePos.Transform.GetLocation(); CalcDepth.Z = 0.0;
		RP.ObstacleDepth = CalcDepth.Length();
	}
	else
	{
		CalcDepth = FrontLedgePos.Transform.GetLocation() - BackLedgeStart; CalcDepth.Z = 0.0;
		RP.ObstacleDepth = CalcDepth.Length();
	}
	//Step 8 - Find Land floor
	FHitResult FloorHitResult;
	if (RP.HasBackLedge == true)
	{
		const FVector FloorTraceStart = BackLedgePos.Transform.GetLocation() + (KML::GetForwardVector(BackLedgePos.Transform.Rotator()) * (RefChar->GetCapsuleComponent()->GetScaledCapsuleRadius() + 5) * -1);

		BackLedgeValid = UKismetSystemLibrary::SphereTraceSingle(RefChar, FloorTraceStart, FloorTraceStart - FVector(0, 0, RP.ObstacleHeight + 15), RefChar->GetCapsuleComponent()->GetScaledCapsuleRadius(), 
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnoreList, TraceType, FloorHitResult, true, HexToColor("112a5e"), HexToColor("4078f1"), DrawDebugTime);

		if (WALKABLE(FloorHitResult) == true)
		{
			RP.HasBackFloor = true;
			RP.BackFloorTransform.Transform.SetLocation(FloorHitResult.ImpactPoint);
			RP.BackFloorTransform.Transform.SetRotation(KML::Conv_RotatorToQuaternion(KML::MakeRotFromX(BackTraceNormal * -1.0)));
			RP.BackFloorTransform.Component = FloorHitResult.GetComponent();
			RP.BackLedgeHeight = abs(BackLedgePos.Transform.GetLocation().Z - FloorHitResult.ImpactPoint.Z);
		}
	}
	//Step 9 - Find State and draw debug shapes
	bool TraversalActionStateValid = false;
	TMap<int, FTraversalStateEvaluation> StatesData = TraversalActionStatesGrounded;

	if (RefChar->GetCharacterMovement()->IsFalling() == true) { StatesData = TraversalActionStatesInAir; }

	const int ChoosedStateIndex = ChooseTraversalActionState(TraversalActionStateValid, StatesData, RP);
	RP.ActionIndex = ChoosedStateIndex;

	CheckingResult = RP; //RETURN ALL CALCULATIONS IN ONE STRUCTURE

	if (DrawDebugShapes == true)
	{
		if(RP.HasFrontLedge == true) { DrawDebugSphere(GetWorld(), FrontLedgePos.Transform.GetLocation(), 8, 16, FColor::Green, false, GetShapesDrawTime(), 0, 1.5); }
		if (RP.HasBackLedge == true) { DrawDebugSphere(GetWorld(), BackLedgePos.Transform.GetLocation(), 6, 16, FColor::Orange, false, GetShapesDrawTime(), 0, 1.5); }
		if (RP.HasBackFloor == true) { DrawDebugSphere(GetWorld(), RP.BackFloorTransform.Transform.GetLocation(), 8, 16, FColor::Red, false, GetShapesDrawTime(), 0, 1.5); }
	}

	return true;
}



bool UCpp_TraversalActionComponent::CalculateLandPositionForJump_Implementation(FCALS_ComponentAndTransform& LandPosition, FCALS_ComponentAndTransform& BeginPosition, FVector& ReturnVelocity, FVector StartPositionOffset, 
	float ProjectileRadius, bool DrawDebug, float GroundDetectionRadius, float MaxVelocityZ, float AirControlBoostMultipler, float AirControlBoostVelocity, float VelocityBias, ETraceTypeQuery TraceChannel)
{
	TArray<AActor*> AllToIgnore = ActorsToIgnoreList;
	AllToIgnore.Add(RefChar);

	if (IsValid(RefChar) == false) { return false; }
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugTraceIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	//Step 1 - Capculate Jump Velocity
	FVector JumpVelocity = RefChar->GetVelocity(); JumpVelocity.Z = 0.0;
	FVector NormalizedVelocity = RefChar->GetVelocity(); NormalizedVelocity.Z = 0.0; NormalizedVelocity.Normalize();
	const float VelocityXY = JumpVelocity.Length();
	JumpVelocity = NormalizedVelocity * FMath::Clamp<float>(VelocityXY + VelocityBias, 40.0, 480.0) * AirControlBoostMultipler * 1.1;
	JumpVelocity = JumpVelocity + FVector(0, 0, abs(MaxVelocityZ / (VelocityXY - MaxVelocityZ)));
	//Step 2
	FPredictProjectilePathParams ProjectileParams;
	ProjectileParams.StartLocation = RefChar->GetActorLocation()+StartPositionOffset;
	ProjectileParams.LaunchVelocity = JumpVelocity;
	ProjectileParams.bTraceWithCollision = true;
	ProjectileParams.bTraceWithChannel = true;
	ProjectileParams.ProjectileRadius = ProjectileRadius;
	ProjectileParams.MaxSimTime = 2.0;
	ProjectileParams.TraceChannel = UCollisionProfile::Get()->ConvertToCollisionChannel(true, TraceChannel);
	ProjectileParams.SimFrequency = 15.0;
	ProjectileParams.DrawDebugType = TraceType;
	ProjectileParams.DrawDebugTime = DrawDebugTime;
	ProjectileParams.ActorsToIgnore = AllToIgnore;

	FPredictProjectilePathResult ProjectileResult;
	const bool ProjectileValid = UGameplayStatics::PredictProjectilePath(this, ProjectileParams, ProjectileResult);
	if (ProjectileValid == false) { return false; }

	FVector LastPoint = ProjectileResult.LastTraceDestination.Location;
	float TimeToReach = ProjectileResult.LastTraceDestination.Time;
	TArray<FPredictProjectilePathPointData> PathPoints = ProjectileResult.PathData;
	//Algo::Reverse(PathPoints); //Start From last point

	if (DrawDebugShapes == true)
	{
		DrawDebugSphere(GetWorld(), LastPoint, 10, 16, FColor::Blue, false, GetShapesDrawTime(), 0, 1.8);
	}
	//Step 3 Clamping Condition
	const float MappedScale = KML::MapRangeClamped(LastPoint.Z - GetLocationFromBase(0).Z + 10, -140, 0, 1.0, 0.0);
	FVector JumpVelocityXY = JumpVelocity; JumpVelocityXY.Z = 0;
	
	//Step 4
	FHitResult TraceResult;
	FVector FirstGroundImpact = FVector(0, 0, 0);
	FVector FirstGroundDirection = FVector(0, 0, 0);

	TArray<FCALS_ComponentAndTransform> Points;
	TArray<float> Weights;


	FVector VelocityDirection = RefChar->GetVelocity(); VelocityDirection.Z = 0; VelocityDirection.Normalize();

	for (int i = 0; i < PathPoints.Num(); i++)
	{
		FPredictProjectilePathPointData CurrentPoint = PathPoints[i];

		if (DrawDebugShapes == true) { DrawDebugLine(GetWorld(), CurrentPoint.Location, CurrentPoint.Location + CurrentPoint.Velocity, FColor::Blue, false, GetShapesDrawTime(), 0, 0.1); }

		if (i > (PathPoints.Num() - 1 )/ 2)
		{
			float OutScaleDown = FMath::Clamp<float>(pow(JumpVelocityXY.Length(), 2), 1, 700);
			OutScaleDown = OutScaleDown * MappedScale;
			OutScaleDown = FMath::Clamp<float>(KML::MapRangeClamped(OutScaleDown, 0, 1.1, 0, 1) * 10 * float(i), 0, 100); // <---------------------------- DO WERYFIKACJI ---------------

			const bool TraceValid = UKismetSystemLibrary::SphereTraceSingle(RefChar, CurrentPoint.Location + FVector(0,0,UpOffsetRangePerPoint.Y), CurrentPoint.Location + FVector(0, 0, UpOffsetRangePerPoint.X + OutScaleDown), 
				GroundDetectionRadius, TraceChannel, false, AllToIgnore, TraceType, TraceResult, true, HexToColor("00635e"), HexToColor("00f99a"), DrawDebugTime);

			if (TraceValid == true && WALKABLE(TraceResult) == true)
			{
				ACharacter* HitCharacter = Cast<ACharacter>(TraceResult.GetActor());
				if (IsValid(HitCharacter) == false)
				{
					FirstGroundImpact = TraceResult.ImpactPoint;
					FirstGroundDirection = KML::Cross_VectorVector(KML::GetForwardVector(KML::MakeRotFromX(TraceResult.Normal)), KML::Quat_RotateVector(KML::Conv_RotatorToQuaternion(FRotator(0,-90,0)),VelocityDirection));

					FHitResult CapTraceResult;
					UKismetSystemLibrary::CapsuleTraceSingle(RefChar, ToCapsuleLocation(FirstGroundImpact, 0), ToCapsuleLocation(FirstGroundImpact, 0), CAP->GetScaledCapsuleRadius() * 0.9, CAP->GetScaledCapsuleHalfHeight() * 0.87, 
						ETraceTypeQuery::TraceTypeQuery1, false, AllToIgnore, TraceType, CapTraceResult, true, FColor::Black, FColor::Red, DrawDebugTime);

					if (CapTraceResult.bBlockingHit == false && KML::Vector_Distance2D(GetLocationFromBase(0), FirstGroundImpact) > 5)
					{
						FCALS_ComponentAndTransform NewPoint;
						NewPoint.Component = TraceResult.GetComponent();
						NewPoint.Transform = FTransform(KML::MakeRotFromX(FirstGroundDirection), FirstGroundImpact, FVector(1, 1, 1));
						Points.Add(NewPoint);

						float Weight = KML::SafeDivide(10.0, KML::Vector_Distance2D(FirstGroundImpact, LastPoint));
						Weight = Weight - (GetLocationFromBase(0).Z - FirstGroundImpact.Z);
						Weights.Add(Weight);
					}
				}
			}

		}
	}
	if (Points.Num() == 0) { ReturnVelocity = JumpVelocity; return false; }

	FCALS_ComponentAndTransform Current;

	int MaxIndex = 0;
	float MaxValue = 0.0;

	KML::MaxOfFloatArray(Weights, MaxIndex, MaxValue);
	if (Points.IsValidIndex(MaxIndex) == true)
	{
		Current = Points[MaxIndex];
	}
	else
	{
		return false;
	}

	if(IsValid(Current.Component) == true)
	{
		if (HLF::CapsuleHaveRoomWithIgnore(RefChar, RefChar, ToCapsuleLocation(Current.Transform.GetLocation(), 0, false), AllToIgnore, 0.9, 0.8) == true)
		{
			FFindFloorResult FloorSweepResult;
			RefChar->GetCharacterMovement()->FindFloor(ToCapsuleLocation(Current.Transform.GetLocation(), -8, false), FloorSweepResult, false);
			FCALS_ComponentAndTransform OutLandPosition, OutBasePosition;
			OutLandPosition.Component = Current.Component;
			OutLandPosition.Transform = Current.Transform;
			if (FloorSweepResult.bWalkableFloor == true)
			{
				OutLandPosition.Transform.SetLocation(FVector(Current.Transform.GetLocation().X, Current.Transform.GetLocation().Y, FloorSweepResult.HitResult.ImpactPoint.Z));
			}

			LandPosition = OutLandPosition;
			ReturnVelocity = JumpVelocity;

			RefChar->GetCharacterMovement()->FindFloor(RefChar->GetActorLocation(), FloorSweepResult, false);
			OutBasePosition.Component = FloorSweepResult.HitResult.GetComponent();
			OutBasePosition.Transform = FTransform(RefChar->GetActorRotation(), GetLocationFromBase(0), FVector(1, 1, 1));

			BeginPosition = OutBasePosition;

			return true;
;			}
	}
	return false;
	
}

bool UCpp_TraversalActionComponent::TryActivatePredictableJump_Implementation(ETraceTypeQuery TraceChannel, bool& StartNormalJump)
{
	StartNormalJump = true;
	if (UsePredictableJumps == false) { return false; }

	bool DrawDebug = false;
	if (DebugTraceIndex > 0) { DrawDebug = true; }

	FVector StartGroundOrigin = FVector(0, 0, 0); FVector StartGroundOrginF = FVector(0, 0, 0);
	float StartGroundRadius = 0.0; float StartGroundRadiusF = 0.0;
	TArray<AActor*> FloorTypeToIgnore;
	FloorTypeToIgnore.Add(RefChar);

	//Step 1:
	HLF::CheckFloorTypeC(RefChar, StartGroundOrigin, StartGroundRadius, StartGroundOrginF, StartGroundRadiusF, GetLocationFromBase(0.0), RefChar->GetActorForwardVector(), 
		ETraceTypeQuery::TraceTypeQuery1, 2, 30.0, FloorTypeToIgnore, false, DebugTraceIndex);

	//Step 2:
	FTraversalJumpTrajectory LocalJumpProperties;
	FCALS_ComponentAndTransform LandPosition, BeginPosition;
	FVector JumpVelocity;

	const bool LandValid = CalculateLandPositionForJump(
		LandPosition, 
		BeginPosition, 
		JumpVelocity, 
		FVector(0, 0, 0), 
		10.0, 
		DrawDebug, 
		40.0, 
		DefJumpMaxZ, 
		DefAirControlBoostMultipler, 
		DefAirControlBoostVelocity, 
		20.0, 
		TraceChannel);

	if (LandValid == true)
	{
		LocalJumpProperties.IsValid = LandValid;
		LocalJumpProperties.StartPositionWS = BeginPosition;
		LocalJumpProperties.StartPositionLS = ToLOCAL(BeginPosition);
		LocalJumpProperties.LandPositionWS = LandPosition;
		LocalJumpProperties.LandPositionLS = ToLOCAL(LandPosition);
	}

	if (TryGetLadderComponent())
	{
		FCALS_ComponentAndTransform L_CapPosition;
		AActor* L_Actor;
		UPrimitiveComponent* L_RungComp;
		bool StartedLadderClimb = false;
		const bool CanStartLadderClimb = TryGetLadderComponent()->CheckCanStartClimbLadder_Implementation(true, false, RefChar->GetActorLocation(), RefChar->GetActorRotation(), L_CapPosition, L_Actor, L_RungComp);

		if (CanStartLadderClimb && IsValid(L_Actor) == true)
		{
			StartedLadderClimb = TryGetLadderComponent()->TryStartLadderClimb(L_CapPosition, L_Actor, L_RungComp);
			if (StartedLadderClimb == true)
			{
				StartNormalJump = false; return false;
			}
		}

	}
	if (LandValid == false)
	{
		StartNormalJump = true; return false;
	}
	//Step 3:
	FVector LandGroundOrigin = FVector(0, 0, 0);  FVector LandGroundOrginF = FVector(0,0,0);
	float LandGroundRadius = 0.0; float LandGroundRadiusF = 0.0;

	HLF::CheckFloorTypeC(RefChar, LandGroundOrigin, LandGroundRadius, LandGroundOrginF, LandGroundRadiusF, LocalJumpProperties.LandPositionWS.Transform.GetLocation(),
		KML::GetForwardVector(LocalJumpProperties.LandPositionWS.Transform.Rotator()), ETraceTypeQuery::TraceTypeQuery1, 2, 30.0, FloorTypeToIgnore, false, DebugTraceIndex);

	FTraversalGroundInfo StartGroundInfo, LandGroundInfo;
	StartGroundInfo.GlobalOrigin = StartGroundOrigin; StartGroundInfo.ForwardOffsetOrigin = StartGroundOrginF; StartGroundInfo.GlobalRadius = StartGroundRadius; StartGroundInfo.ForwardOffsetRadius = StartGroundRadiusF;
	LandGroundInfo.GlobalOrigin = LandGroundOrigin; LandGroundInfo.ForwardOffsetOrigin = LandGroundOrginF; LandGroundInfo.GlobalRadius = LandGroundRadius; LandGroundInfo.ForwardOffsetRadius = LandGroundRadiusF;

	LocalJumpProperties.StartGroundType = StartGroundInfo;
	LocalJumpProperties.LandGroundType = LandGroundInfo;

	JumpTrajectory = LocalJumpProperties;

	//Step 4:
	FVector2D DistancesWeight = FVector2D(0, 0);

	FVector TargetLandPosWS;
	FVector FixedStartPosWS;
	FRotator TargetLandRotWS;

	TargetLandPosWS = LocalJumpProperties.LandGroundType.GlobalOrigin;
	TargetLandPosWS.Z = LocalJumpProperties.LandPositionWS.Transform.GetLocation().Z;

	DistancesWeight.X = KML::Vector_Distance2D(TargetLandPosWS, LocalJumpProperties.StartPositionWS.Transform.GetLocation());
	DistancesWeight.Y = KML::Round((LocalJumpProperties.StartPositionWS.Transform.GetLocation().Z - LocalJumpProperties.LandPositionWS.Transform.GetLocation().Z) + 2.7) * 1.0;

	TargetLandRotWS = KML::FindLookAtRotation(LocalJumpProperties.StartPositionWS.Transform.GetLocation(), TargetLandPosWS);

	if (DrawDebugShapes)
	{
		DrawSimpleDebugCircle(TargetLandPosWS + FVector(0,0,2), TargetLandRotWS, LandGroundRadius, FColor::Emerald, GetShapesDrawTime() * 1.2, 2);
	}


	return true;

}

UCpp_LadderClimbingComponent* UCpp_TraversalActionComponent::TryGetLadderComponent()
{
	if (RefChar)
	{
		UCpp_LadderClimbingComponent* ClimbingComponent = Cast<UCpp_LadderClimbingComponent>(RefChar->GetComponentByClass(UCpp_LadderClimbingComponent::StaticClass()));
		return ClimbingComponent;
	}
	return nullptr;
}

UCpp_DynamicClimbingComponent* UCpp_TraversalActionComponent::TryGetClimbingComponent()
{
	if (RefChar)
	{
		UCpp_DynamicClimbingComponent* ClimbingComponent = Cast<UCpp_DynamicClimbingComponent>(RefChar->GetComponentByClass(UCpp_DynamicClimbingComponent::StaticClass()));
		return ClimbingComponent;
	}
	return nullptr;
}

bool UCpp_TraversalActionComponent::PredictableJumpStop_Implementation(int& LandAnimIndex)
{
	return false;
}

bool UCpp_TraversalActionComponent::UpdateTraveralStatesData_Implementation()
{
	ConvertStatesDataAssetToMap(TraversalActionsDataGrounded, TraversalActionsDataInAir);
	return true;
}

bool UCpp_TraversalActionComponent::CheckMantle_Implementation(float& MantleHeight, FCALS_ComponentAndTransform& MantleLedgeWS, FCALS_ComponentAndTransform& MantleCapsuleWS, FCALS_ComponentAndTransform& VaultLedgeWS, CALS_MantleType& MantleType,
	FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel, float CapHeightScale, float CapRadiusScale, bool CapSizeByDef, float VelocityOffsetScale, float LedgeFindingRadius, float DebugTime)
{
	if (IsValid(RefChar) == false) { return false; }
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugTraceIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	TArray<AActor*> AllToIgnore = ActorsToIgnoreList;
	MantleType = CALS_MantleType::None;

	//Step 1
	const float TargetReachDistance = TraceSettings.ReachDistance + (GetReachOffset_Implementation() * VelocityOffsetScale);
	const float VerticalCapsuleOffset = (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2;
	const float VerticalCapsuleHeight = (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2;
	UPrimitiveComponent* HitComponent = nullptr;

	//Step 2 - Try find object in front of capsule location
	FVector InitialTraceImpact, InitialTraceNormal, DownTraceLoc; //Init Main Vectors
	FVector TraceStart = (GetLocationFromBase(2.0) + GetPlayerMovementInput_Implementation() * -30.0) + FVector(0, 0, VerticalCapsuleOffset);
	FVector TraceEnd = TraceStart + (GetPlayerMovementInput_Implementation() * TargetReachDistance);

	FHitResult VerticalHitResult;
	const bool VerticalHitValid = UKismetSystemLibrary::CapsuleTraceSingle(RefChar, TraceStart, TraceEnd, TraceSettings.ForwardTraceRadius, VerticalCapsuleHeight, TraceChannel, false,
		ActorsToIgnoreList, TraceType, VerticalHitResult, true, FColor::Black, FColor::Blue, DrawDebugTime);

	if (VerticalHitValid == true && VerticalHitResult.bStartPenetrating == false && WALKABLE(VerticalHitResult) == false)
	{
		if (VerticalHitResult.GetComponent()->IsSimulatingPhysics(VerticalHitResult.BoneName) == true) //Check that detected component is simulating physic
		{
			if (VerticalHitResult.GetComponent()->GetComponentVelocity().Length() > 10) { return false; } //Make sure that detected object is not moving with high speed
		}
		InitialTraceImpact = VerticalHitResult.ImpactPoint; InitialTraceNormal = VerticalHitResult.ImpactNormal;
	}
	else
	{
		return false; //When trace is not detected collision just stop next calculations
	}

	//Step 3:
	FHitResult DownwardHitResult;

	for (int i = 0; i < TraceSettings.MaxForwarIterations; i++)
	{

		float ForwardOffset = -15.0 + (float(i) * 6);
		TraceStart = FVector(InitialTraceImpact.X, InitialTraceImpact.Y, GetLocationFromBase(2.0).Z) + (InitialTraceNormal * ForwardOffset);
		TraceEnd = TraceStart + FVector(0, 0, TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius);

		const bool DownwardHitValid = HLF::FindGapBySphereTrace(RefChar, TraceEnd, TraceStart, TraceSettings.DownwardTraceRadius, TraceChannel, false, AllToIgnore, TraceType, DownwardHitResult,
			true, FColor::Red, FColor::Orange, DebugTime);

		if (DownwardHitResult.bBlockingHit == true && WALKABLE(DownwardHitResult) == true)
		{
			HitComponent = DownwardHitResult.GetComponent(); DownTraceLoc = DownwardHitResult.ImpactPoint;
			break;
		}
	}
	if (DownwardHitResult.bBlockingHit == false || WALKABLE(DownwardHitResult) == false) { return false; }

	//Step 4:
	FTransform TargetTransform = FTransform(KML::MakeRotFromX(InitialTraceNormal * FVector(-1, -1, 0)), ToCapsuleLocation(DownTraceLoc, 2.0, false), FVector(1, 1, 1));
	MantleHeight = TargetTransform.GetLocation().Z - RefChar->GetActorLocation().Z;

	//Step 5: Vault  [ IMPLEMENTATION IN FUTURE]  _______________________________________________________________________________________________________________________________________

	//Step 7: Check Capsule have free space on taget transform
	if (HLF::CapsuleHaveRoomWithIgnore(RefChar, RefChar, ToCapsuleLocation(DownTraceLoc, 2.0, false), AllToIgnore, CapRadiusScale, CapHeightScale, false) == false)
	{
		return false;
	}

	FCALS_ComponentAndTransform OutCapsuleWS;
	OutCapsuleWS.Component = HitComponent;
	OutCapsuleWS.Transform = TargetTransform;
	MantleCapsuleWS = OutCapsuleWS;
	VaultLedgeWS = OutCapsuleWS;

	//Step 8: Calculate Ledge Position for Motion Warping
	FHitResult LedgeHitResult;

	TraceEnd = DownTraceLoc - FVector(0, 0, LedgeFindingRadius);
	TraceStart = TraceEnd + InitialTraceNormal * (KML::Vector_Distance(InitialTraceImpact, DownTraceLoc) + 10);
	FCALS_ComponentAndTransform OutLedgeWS;

	const bool LedgeHitValid = UKismetSystemLibrary::SphereTraceSingle(RefChar, TraceStart, TraceEnd, LedgeFindingRadius, TraceChannel, false, AllToIgnore, TraceType, LedgeHitResult, true, 
		FColor::Blue, FColor::Cyan, DebugTime);
	if (LedgeHitValid)
	{
		OutLedgeWS.Transform = FTransform(KML::MakeRotFromX(HLF::NormalToVector(LedgeHitResult.ImpactNormal)), FVector(LedgeHitResult.ImpactPoint.X, LedgeHitResult.ImpactPoint.Y, DownTraceLoc.Z), FVector(1, 1, 1));
		OutLedgeWS.Component = LedgeHitResult.GetComponent();
		MantleLedgeWS = OutLedgeWS;
	}
	else
	{
		MantleLedgeWS = OutCapsuleWS;
	}

	if (RefChar->GetCharacterMovement()->IsFalling() == true)
	{
		MantleType = CALS_MantleType::FallingCatch;
	}
	else
	{
		if (MantleHeight > 125) { MantleType = CALS_MantleType::HighMantle; }
		else { MantleType = CALS_MantleType::LowMantle; }
	}

	if (DrawDebugShapes)
	{
		DrawDebugLine(GetWorld(), FVector(InitialTraceImpact.X, InitialTraceImpact.Y, DownTraceLoc.Z), FVector(InitialTraceImpact.X, InitialTraceImpact.Y,
			DownTraceLoc.Z - MantleHeight), FColor::Emerald, false, DebugTime, 1, 1.5);
		DrawDebugLine(GetWorld(), FVector(InitialTraceImpact.X, InitialTraceImpact.Y, DownTraceLoc.Z), GetLocationFromBase(0.0), FColor::Cyan, false, DebugTime, 1, 1.5);
		DrawDebugSphere(GetWorld(), DownTraceLoc, 6, 12, FColor::Purple, false, DebugTime, 2, 1);
		DrawDebugSphere(GetWorld(), OutLedgeWS.Transform.GetLocation(), 5, 12, FColor::Cyan, false, DebugTime, 3, 1.5);
	}

	return IsValid(HitComponent);

}

bool UCpp_TraversalActionComponent::CheckAndStartMantle_Implementation(FTraversalTraceSettings InTraceSettings)
{
	return false;
}

bool UCpp_TraversalActionComponent::MantleStop_Implementation()
{
	return false;
}

bool UCpp_TraversalActionComponent::MantleStart_Implementation(float MantleHeight, FCALS_ComponentAndTransform MantleLedgeWS, FCALS_ComponentAndTransform VaultLedgeWS, FCALS_ComponentAndTransform MantleCapsuleWS, CALS_MantleType MantleType)
{
	return false;
}

float UCpp_TraversalActionComponent::GetReachOffset_Implementation()
{
	FVector VelocityXY = RefChar->GetVelocity();
	VelocityXY.Z = 0;
	return KML::MapRangeClamped(VelocityXY.Length(), 0, 500, 0, 75);
}

FVector UCpp_TraversalActionComponent::GetPlayerMovementInput_Implementation()
{
	FVector NormalizedAcc = RefChar->GetCharacterMovement()->GetCurrentAcceleration();
	const bool NormalizeOK = NormalizedAcc.Normalize();
	if (NormalizeOK == true)
	{ return NormalizedAcc; }
	else
	{ return FVector(0, 0, 0); }
}


void UCpp_TraversalActionComponent::ConvertStatesDataAssetToMap(const UTraversalActionsParamsData* DataGrounded, const UTraversalActionsParamsData* DataInAir)
{
	if (DataGrounded) { TraversalActionStatesGrounded = DataGrounded->ActionStates; }
	if (DataInAir) { TraversalActionStatesInAir = DataInAir->ActionStates; }
	return;
}


int UCpp_TraversalActionComponent::ChooseTraversalActionState(bool& StateValid, const TMap<int, FTraversalStateEvaluation> StatesData, FTraversalCollisionCheck CheckingResult)
{
	if (StatesData.Num() == 0) { StateValid = false; return 0; }
	
	bool BoolsConditionsResult = false;
	bool InRangesResult = false;

	for (const TPair<int, FTraversalStateEvaluation>& Current : StatesData)
	{
		bool CheckResult1 = false; bool CheckResult2 = false; bool CheckResult3 = false;
		int Key = Current.Key;
		const FTraversalStateEvaluation& Value = Current.Value;

		if (Value.HasFrontLedge == CheckingResult.HasFrontLedge || MakeSureConditionIsRequired(0,Value.ConditionToIgnore)) { CheckResult1 = true; }
		if (Value.HasBackLedge == CheckingResult.HasBackLedge || MakeSureConditionIsRequired(1, Value.ConditionToIgnore)) { CheckResult2 = true; }
		if (Value.HasBackFloor == CheckingResult.HasBackFloor || MakeSureConditionIsRequired(2, Value.ConditionToIgnore)) { CheckResult3 = true; }
		if (CheckResult1 ==true && CheckResult2 == true && CheckResult3 == true) { BoolsConditionsResult = true; }
		else { BoolsConditionsResult = false; }

		InRangesResult =
			KML::InRange_FloatFloat(CheckingResult.ObstacleHeight, Value.ObstacleHeightRange.X, Value.ObstacleHeightRange.Y, true, true) &&
			KML::InRange_FloatFloat(CheckingResult.ObstacleDepth, Value.ObstacleDepthRange.X, Value.ObstacleDepthRange.Y, true, true) &&
			KML::InRange_FloatFloat(CheckingResult.BackLedgeHeight, Value.BackLedgeHeightRange.X, Value.BackLedgeHeightRange.Y, true, true);

		if (BoolsConditionsResult == true && InRangesResult == true)
		{
			StateValid = true;
			return Key;
		}
	}
	StateValid = false; return 0;
}


bool UCpp_TraversalActionComponent::UpdateOffsetsForCharacter(FTransform& FrontLedgeOffset, FTransform& BackLedgeOffset, FTransform& BackFloorOffset, FTraversalCollisionCheck CurrentData, float MaxVectorSize)
{
	FrontLedgeOffset = SubtractTransforms(ToWORLD(CurrentData.FrontLedgeTransform).Transform, PrevTraversalData.FrontLedgeTransform.Transform);
	BackLedgeOffset = SubtractTransforms(ToWORLD(CurrentData.BackLedgeTransform).Transform, PrevTraversalData.BackLedgeTransform.Transform);
	BackFloorOffset = SubtractTransforms(ToWORLD(CurrentData.BackFloorTransform).Transform, PrevTraversalData.BackFloorTransform.Transform);
	// Save Cheched Values
	PrevTraversalData.FrontLedgeTransform = ToWORLD(CurrentData.FrontLedgeTransform);
	PrevTraversalData.BackLedgeTransform = ToWORLD(CurrentData.BackLedgeTransform);
	PrevTraversalData.BackFloorTransform = ToWORLD(CurrentData.BackFloorTransform);

	FrontLedgeOffset.SetLocation(KML::ClampVectorSize(FrontLedgeOffset.GetLocation(), -MaxVectorSize, MaxVectorSize));
	return true;
}

UCpp_TraversalActionComponent* UCpp_TraversalActionComponent::TryGetTraversalComponent(UObject* WorldContextObject, ACharacter* Target)
{
	if (Target)
	{
		UCpp_TraversalActionComponent* TargetComp = Cast< UCpp_TraversalActionComponent>(Target->GetComponentByClass(UCpp_TraversalActionComponent::StaticClass()));
		return TargetComp;
	}
	else
	{
		ACharacter* TargetChar = Cast<ACharacter>(WorldContextObject);
		if (TargetChar)
		{
			UCpp_TraversalActionComponent* TargetComp = Cast< UCpp_TraversalActionComponent>(TargetChar->GetComponentByClass(UCpp_TraversalActionComponent::StaticClass()));
			return TargetComp;
		}
		return nullptr;
	}
}

bool UCpp_TraversalActionComponent::TryActivateTraversalAction_Implementation(float& Duration, UAnimMontage*& SelectedMontage, FTraversalCollisionCheck InCheckingResult)
{
	if (InCheckingResult.ActionIndex == 0 || InCheckingResult.HasFrontLedge == false) { return false; }
	return false;
}

bool UCpp_TraversalActionComponent::TryActivateTraversalForClimb_Implementation(float& Duration, int& ActionIndex, FTraversalCollisionCheck InCheckingResult, FCALS_ComponentAndTransform LedgeL, 
	FCALS_ComponentAndTransform LedgeR, FCALS_ComponentAndTransform LedgeOrigin)
{
	return false;
}

void UCpp_TraversalActionComponent::PlayTraversalAnimAction_Implementation(FTraversalCollisionCheck OurParams)
{

}


bool UCpp_TraversalActionComponent::NormalizeLedgeToObjectCenter(FVector& NewPosition, UPrimitiveComponent* InComponent, FVector DownLocation, FVector InNormal, FTraversalTraceSettings InTraceSettings)
{
	NewPosition = DownLocation;

	if (IsValid(InComponent) == false) { return false; }
	
	const FVector OffsetDirection = KML::GetRightVector(KML::MakeRotFromX(HLF::NormalToVector(InNormal)));
	const FVector InitLocation = DownLocation + (HLF::NormalToVector(InNormal) * 3.0) - FVector(0, 0, 3);

	FCollisionQueryParams LineTraceParams;
	LineTraceParams.DefaultQueryParam;
	LineTraceParams.bTraceComplex = true;
	LineTraceParams.bTraceIntoSubComponents = true;

#if !(UE_BUILD_TEST || UE_BUILD_SHIPPING)
	LineTraceParams.bDebugQuery = DrawDebugShapes;
#endif

	FHitResult LeftTraceResult, RightTraceResult;
	FVector HitLocLeft, HitLocRight = FVector(0, 0, 0);

	const bool HitValid1 = InComponent->LineTraceComponent(LeftTraceResult, InitLocation + (OffsetDirection * InTraceSettings.NormalizationLenght * -1.0), InitLocation + (OffsetDirection * 2), LineTraceParams);
	if (HitValid1 == true) { HitLocLeft = LeftTraceResult.ImpactPoint; }
	else { HitLocLeft = InitLocation + (OffsetDirection * InTraceSettings.NormalizationLenght * -1.0); }

	const bool HitValid2 = InComponent->LineTraceComponent(RightTraceResult, InitLocation + (OffsetDirection * InTraceSettings.NormalizationLenght * 1.0), InitLocation + (OffsetDirection * -2), LineTraceParams);
	if (HitValid2 == true) { HitLocRight = RightTraceResult.ImpactPoint; }
	else { HitLocRight = InitLocation + (OffsetDirection * InTraceSettings.NormalizationLenght * 1.0); }

	if (HitLocLeft.IsNearlyZero() || HitLocRight.IsNearlyZero()) { NewPosition = DownLocation; return false; }

	if (DrawDebugShapes == true)
	{
		DrawDebugSphere(GetWorld(), HitLocLeft, 6, 8, FColor::Blue, false, GetShapesDrawTime(), 0, 0.8);
		DrawDebugSphere(GetWorld(), HitLocRight, 6, 8, FColor::Blue, false, GetShapesDrawTime(), 0, 0.8);
	}

	FVector OutLocation = (HitLocLeft + HitLocRight) / 2;
	OutLocation.Z = DownLocation.Z;
	OutLocation = OutLocation + (HLF::NormalToVector(InNormal) * -3.0);
	NewPosition = OutLocation;
	return true;

}

FVector UCpp_TraversalActionComponent::GetLocationFromBase(float ZOffset)
{
	return RefChar->GetActorLocation() - FVector(0, 0, RefChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

FVector UCpp_TraversalActionComponent::ToCapsuleLocation(FVector FloorLocation, float ZOffset, bool ByDef)
{
	float CapHeight = RefChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return FloorLocation + FVector(0, 0, CapHeight);
}







