// Copyright Jakub W, All Rights Reserved.

#include "HelpfulFunctionsBPLibrary.h"
#include "HelpfulFunctions.h"


//include components
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"

//include gameFramework
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
//include math
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Math/Vector.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"

//include engine
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"

//Enums Collision
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimSequenceDecompressionContext.h"
#include "Animation/AnimCompressionTypes.h"

//Include Navigation
#include "NavigationSystem.h"

//Include Interface
#include "ALS_BaseComponentsInterfaceCpp.h"

#include <algorithm>

UHelpfulFunctionsBPLibrary::UHelpfulFunctionsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}


float UHelpfulFunctionsBPLibrary::HelpfulFunctionsSampleFunction(float Param)
{
	return 1;
}


FVector UHelpfulFunctionsBPLibrary::GetPlayerCapsuleStartLocation(const UObject* WorldContextObject, const ACharacter* TargetChar)
{
	if (!IsValid(TargetChar)) { return FVector(0, 0, 0); }
	FVector Loc;
	FVector Offset;
	UCapsuleComponent* cap = TargetChar->GetCapsuleComponent();

	Loc = TargetChar->GetActorLocation();
	Offset.Z = cap->GetScaledCapsuleHalfHeight();

	return Loc-Offset;

}

void UHelpfulFunctionsBPLibrary::ForceDestroyComponent(UActorComponent* ActorComponent)
{
	if (ActorComponent && ActorComponent->IsValidLowLevel())
	{
		ActorComponent->DestroyComponent();
	}
}

// Simple Find Capsule have Room By Using Single Sphere Trace By Channel (Rotation From Capsule Component)
bool UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnore(const UObject* WorldContextObject, const ACharacter* TargetChar, FVector Location, 
															TArray<AActor*> ToIgnore, float ScaleRadius, float ScaleHeight, const bool DrawTrace)
{
	if (!IsValid(TargetChar)) { return false; }
	UCapsuleComponent* cap = TargetChar->GetCapsuleComponent();
	FVector TStart = Location;
	FVector TEnd = Location;
	TStart = TStart + (cap->GetUpVector() * (cap->GetScaledCapsuleHalfHeight_WithoutHemisphere()*ScaleHeight));
	TEnd = TEnd - (cap->GetUpVector() * (cap->GetScaledCapsuleHalfHeight_WithoutHemisphere() * ScaleHeight));
	float TRadius = cap->GetScaledCapsuleRadius() * ScaleRadius;

	EDrawDebugTrace::Type trace;
	if (DrawTrace == true)
	{
		trace=EDrawDebugTrace::ForOneFrame;
	}
	else
	{
		trace=EDrawDebugTrace::None;
	};

	FHitResult HitResult;
	const bool ReturnHit = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, TStart, TEnd, TRadius, ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, trace, HitResult, true);

	return !(HitResult.bBlockingHit || HitResult.bStartPenetrating);
}

// Simple Find Capsule have Room By Using Single Sphere Trace By Channel (Rotation From Target Transform [Parameter])
bool UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnoreTransform(const UObject* WorldContextObject, const ACharacter* TargetChar, FTransform TargetTransform, TArray<AActor*> ToIgnore, float ScaleRadius, float ScaleHeight, const bool DrawTrace)
{
	if (!IsValid(TargetChar)) { return false; }
	UCapsuleComponent* cap = TargetChar->GetCapsuleComponent(); 
	FVector TStart = TargetTransform.GetLocation();
	FVector TEnd = TargetTransform.GetLocation();

	TStart = TStart + (UKismetMathLibrary::Quat_VectorUp(TargetTransform.GetRotation()) * (cap->GetScaledCapsuleHalfHeight_WithoutHemisphere() * ScaleHeight));
	TEnd = TEnd - (UKismetMathLibrary::Quat_VectorUp(TargetTransform.GetRotation()) * (cap->GetScaledCapsuleHalfHeight_WithoutHemisphere() * ScaleHeight));
	float TRadius = cap->GetScaledCapsuleRadius() * ScaleRadius;

	EDrawDebugTrace::Type trace;
	if (DrawTrace == true)
	{
		trace = EDrawDebugTrace::ForOneFrame;
	}
	else
	{
		trace = EDrawDebugTrace::None;
	};

	FHitResult HitResult;
	const bool ReturnHit = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, TStart, TEnd, TRadius, ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, trace, HitResult, true);

	return !(HitResult.bBlockingHit || HitResult.bStartPenetrating);
}
// |||||||||||||||||||||||||||||||| C L I M B I N G    F O O T S    I K    F U N C T I O N |||||||||||||||||||||||||||||||| 
void UHelpfulFunctionsBPLibrary::ClimbingFootIK(const UObject* WorldContextObject, bool& ReturnHit, FVector& ReturnLocation, FVector& ReturnNormal, 
	const ACharacter* TargetChar, FVector FootLocation, FVector Direction, float UpOffset, float TraceRadius, FVector2D TraceLength, bool UseTwoBoneIK, 
	FName RootBoneName, FName JoinBoneName, FName EndBoneName, float FootUpOffsetGround, int DebugIndex)
{
	EDrawDebugTrace::Type TraceType;
	TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (DebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}

	if (!IsValid(TargetChar))
	{
		ReturnHit = false;
		ReturnLocation = FootLocation;
		ReturnNormal = Direction;
		return;
	}

	FVector TraceStart = (FootLocation + FVector(0.0, 0.0, UpOffset)) + (Direction * TraceLength.X);
	FVector TraceEnd = (FootLocation + FVector(0.0, 0.0, UpOffset)) + (Direction * TraceLength.Y);

	TArray<AActor*> ToIgnore;
	ToIgnore.Add(TargetChar->GetOwner());

	FHitResult HitResult1;
	const bool ReturnHit1 = UKismetSystemLibrary::SphereTraceSingle(TargetChar, TraceStart, TraceEnd, 
			   TraceRadius, ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, TraceType, HitResult1, true);

	if (ReturnHit1 == false)
	{
		ReturnHit = false;
		ReturnLocation = FootLocation;
		ReturnNormal = Direction;
		return;
	}

	//Get Trace Start/End From Hit Result
	TraceStart = HitResult1.TraceStart;
	TraceEnd = HitResult1.TraceEnd;

	FTransform ImpactTransform = FTransform(UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd), HitResult1.ImpactPoint, FVector(1, 1, 1));
	FTransform RelativeT = FTransform(UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd), UKismetMathLibrary::VLerp(TraceStart,TraceEnd,0.5), FVector(1, 1, 1));
	FTransform ConvertToRelative = UKismetMathLibrary::MakeRelativeTransform(ImpactTransform, RelativeT);
	FVector LookAtstart = UKismetMathLibrary::VLerp(TraceStart, TraceEnd, 0.5)+ (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd))*ConvertToRelative.GetLocation().X);

	FVector ImpactWithOffset;
	float inverter = 1.0;
	float RelativeDistance=0;
	if (DistancePointToLine(TraceStart, TraceEnd, HitResult1.ImpactPoint) > 0.5)
	{
		RelativeDistance= UKismetMathLibrary::FClamp(DistancePointToLine(TraceStart, TraceEnd, HitResult1.ImpactPoint) * 10, 2, 5);
	}
	//Setup Line Trace
	FHitResult HitResult2Line;
	bool ReturnHit2Line;
	FHitResult HitResult3Floor;

	for (int i = 0; i < 2; i++)
	{
		if (i > 0)
		{ inverter = -1.0; }
		ImpactWithOffset = HitResult1.ImpactPoint + (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(LookAtstart, HitResult1.ImpactPoint))*RelativeDistance*inverter);

		ReturnHit2Line = UKismetSystemLibrary::LineTraceSingle(TargetChar, (ImpactWithOffset + Direction * -12), (ImpactWithOffset + Direction * 6), ETraceTypeQuery::TraceTypeQuery1, 
						false, ToIgnore, TraceType, HitResult2Line, true);
		
		if (UKismetMathLibrary::Dot_VectorVector(HitResult2Line.Normal, FVector(0, 0, 1)) > 0.85 && HitResult2Line.ImpactPoint.Z<FootLocation.Z)
		{
			const bool ReturnHit3 = UKismetSystemLibrary::LineTraceSingle(TargetChar, (HitResult2Line.ImpactPoint+FVector(0,0,10)), (HitResult2Line.ImpactPoint + FVector(0, 0, -10)), 
				ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, TraceType, HitResult2Line, true, FLinearColor::Blue,FLinearColor::Yellow, 0.1f);

			if (ReturnHit3 == true && HitResult3Floor.ImpactPoint != HitResult3Floor.TraceStart && HitResult3Floor.ImpactPoint != HitResult3Floor.TraceEnd)
			{
				ReturnHit = true;
				ReturnLocation = HitResult3Floor.ImpactPoint + (Direction * -5);
				ReturnNormal = UKismetMathLibrary::VLerp(Direction, FVector(0, 0, -1), 0.5);
				return;
			}
		}
		else
		{
			if (UKismetMathLibrary::Dot_VectorVector(HitResult2Line.Normal, Direction) < -0.65)
			{
				ReturnHit = true;
				ReturnLocation = HitResult2Line.ImpactPoint;
				ReturnNormal = HitResult2Line.Normal;
				return;
			}
		}
	}
	ReturnHit = false;
	ReturnLocation = FootLocation;
	ReturnNormal = Direction;
	return;

}

// Distance Point To Line Function (Oparte na transformacjach)
float UHelpfulFunctionsBPLibrary::DistancePointToLine(FVector LineV1, FVector LineV2, FVector PointVector)
{
	FTransform t = UKismetMathLibrary::MakeRelativeTransform(FTransform(UKismetMathLibrary::FindLookAtRotation(LineV1, LineV2), PointVector, 
				   FVector(1, 1, 1)), FTransform(UKismetMathLibrary::FindLookAtRotation(LineV1, LineV2),UKismetMathLibrary::VLerp(LineV1,LineV2,0.5), FVector(1, 1, 1)));
	float dis = abs(t.GetLocation().Y)+ abs(t.GetLocation().Z);
	return dis;
}

// :::::::::::::::::::::::::::::::  L E D G E    P O I N T    F I N D I N G  :::::::::::::::::::::::::::::::
void UHelpfulFunctionsBPLibrary::FindLedgePoint(const UObject* WorldContextObject, bool& ReturnValid, FVector& ReturnLocation, FVector& ReturnNormal, FTransform& ReturnActorT, 
	UPrimitiveComponent*& ReturnComponent, const ACharacter* TargetChar, FVector StartLocation, FVector ForwardDirection, TArray<UClass*> NotThisClass, float ForwardScale, float UpOffset, bool preValid,
	FVector preLocation, FVector preNormal, bool IsFirst, bool ForLeftSide, int DebugIndex, ECollisionChannel TraceChannel)
{


	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1) 
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	if (!IsValid(TargetChar))
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	TArray<AActor*> ToIgnore;
	ToIgnore.Add(TargetChar->GetOwner());

	ETraceTypeQuery MyTraceQuery= UEngineTypes::ConvertToTraceType(TraceChannel);

	FVector FirstTraceOrigin = StartLocation + FVector(0,0,UpOffset);
	FHitResult CapsuleHit;

	float CapTraceHeight = 30;
	if (preValid == true && !IsFirst == true)
	{ CapTraceHeight = 20; }

	const bool CapsuleReturnHit = UKismetSystemLibrary::CapsuleTraceSingle(TargetChar, FirstTraceOrigin, FirstTraceOrigin + (ForwardDirection * ForwardScale), 10.0, 
			   CapTraceHeight, ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, TraceType, CapsuleHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.2f);

	if (CapsuleReturnHit == false)
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	if (UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.Location, CapsuleHit.TraceStart, 0.05) == true && 
		UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.Location, CapsuleHit.TraceEnd, 0.05) == true && 
		UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.ImpactPoint, CapsuleHit.TraceEnd, 0.05) == true && 
		UKismetMathLibrary::Dot_VectorVector(ForwardDirection, CapsuleHit.ImpactNormal) < 0.5)
	{
		CapTraceHeight = CapTraceHeight + 0.5;
	}
	else
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	FVector SphereTraceOrigin = CapsuleHit.ImpactPoint + (CapsuleHit.Normal * 0.3);
	SphereTraceOrigin = FVector(SphereTraceOrigin.X, SphereTraceOrigin.Y, CapsuleHit.Location.Z);

	FHitResult SphereHit;
	const bool SphereReturnHit = UKismetSystemLibrary::SphereTraceSingle(TargetChar, SphereTraceOrigin+FVector(0,0,15), SphereTraceOrigin+ FVector(0,0,-15),10,
								 MyTraceQuery, false, ToIgnore, TraceType, SphereHit, true, FLinearColor(0.35f, 0.0f, 0.48f, 1.0f), FLinearColor::Yellow, 0.1f);
	if (SphereReturnHit == false)
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	FHitResult LineHit;
	bool VerticalHitReturn=false;
	bool UseImpactAsStart=false;
	FVector LineImpactPoint;
	FVector LineTraceOrigin;
	FLinearColor LineTraceColor = FLinearColor(0, 0.05f, 0.5f, 1.0f);

	for (int i = 0; i < 2; i++)
	{
		LineTraceOrigin = CapsuleHit.ImpactPoint;
		LineTraceOrigin = FVector(LineTraceOrigin.X, LineTraceOrigin.Y, SphereHit.ImpactPoint.Z + 3);
		LineTraceOrigin = LineTraceOrigin + (NormalToVector(CapsuleHit.Normal)*3);
		// SelectVector(Wartosc jezeli Prawda, Wartosc jezeli Falsz, Warunek Bool)

		VerticalHitReturn = UKismetSystemLibrary::LineTraceSingle(TargetChar, UKismetMathLibrary::SelectVector(LineImpactPoint - FVector(0, 0, 7), LineTraceOrigin+FVector(0, 0, 16), UseImpactAsStart),
							LineTraceOrigin + FVector(0, 0, -15), MyTraceQuery, false, ToIgnore, TraceType, LineHit, true, LineTraceColor, FLinearColor(0, 0.5f, 1.0f, 1.0f), 0.1f);


		if (VerticalHitReturn == false)
		{ ReturnValid = false; ReturnLocation = StartLocation; return; }

		if (!ClassToIgnore(LineHit.GetActor()->GetClass(), NotThisClass))
		{ ReturnValid = false; ReturnLocation = StartLocation; return; }

		if (UKismetMathLibrary::EqualEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceStart, 0.8) == true)
		{   LineImpactPoint = LineHit.ImpactPoint;
			UseImpactAsStart = true; 
			LineTraceColor = FLinearColor(0.48f, 0.01f, 0.1f, 1.0f);
		}
		else
		{ UseImpactAsStart = false; 
		  break; }
	}

	float e = 0.03;
	if (UKismetMathLibrary::NotEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceStart, e) == false || UKismetMathLibrary::NotEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceEnd, e) == false
		|| UKismetMathLibrary::NotEqual_VectorVector(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(LineHit.Normal)), FVector(0, 0, 1), 0.8) == false)
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	FHitResult DirectionLineHit;
	const bool DirectionReturnHit = UKismetSystemLibrary::LineTraceSingle(TargetChar, LineHit.ImpactPoint - FVector(0, 0, 2) + (NormalToVector(CapsuleHit.Normal) * -8), 
			   LineHit.ImpactPoint - FVector(0, 0, 2) + (NormalToVector(CapsuleHit.Normal) * 4), MyTraceQuery, false, ToIgnore, TraceType, 
			   DirectionLineHit, true, FLinearColor(1, 0.22f, 0.0f, 1.0f), FLinearColor(0.9f, 0.98f, 0.0f, 1.0f), 0.06f);

	if (DirectionReturnHit == false)
	{ ReturnValid = false; ReturnLocation = StartLocation; return; }

	FVector RV = DirectionLineHit.ImpactPoint;
	RV = FVector(RV.X, RV.Y, LineHit.ImpactPoint.Z);
	FVector RN = NormalToVector(DirectionLineHit.Normal);
	
	bool LastCondition = true;
	if (IsFirst == true)
	{ LastCondition = true; }
	else
	{
		LastCondition = preValid && UKismetMathLibrary::EqualEqual_VectorVector(preNormal, RN) && abs(preLocation.Z - RV.Z) < 20;
		if(preValid==false)
		{ LastCondition = true; }
	}

	if (UKismetMathLibrary::Dot_VectorVector(RN, ForwardDirection) > 0.45 && LastCondition==true)
	{
		ReturnValid = true;
		ReturnLocation = RV;
		ReturnNormal = RN;
		ReturnComponent = DirectionLineHit.GetComponent();
		ReturnActorT = DirectionLineHit.GetComponent()->GetComponentTransform();
		return;
	}

	ReturnValid = false;
	ReturnLocation = StartLocation;
	ReturnNormal = ForwardDirection;
	return;

}


// Convert Normal To F Vector - Only Yaw
FVector UHelpfulFunctionsBPLibrary::NormalToVector(FVector Normal)
{
	FRotator n = UKismetMathLibrary::MakeRotFromX(Normal);
	n = FRotator(0,n.Yaw-180,0);
	return UKismetMathLibrary::GetForwardVector(n);
}

bool UHelpfulFunctionsBPLibrary::ClassToIgnore(UClass* Target, TArray<UClass*> ToIgnore)
{
	int ind = ToIgnore.Find(Target);
	if (ind == -1)
	{
		return true;
	}
	return false;
}

// :::::::::::::::::::::::::::::::  L E D G E    P O I N T    F I N D I N G  ( S T R U C T U R E )  :::::::::::::::::::::::::::::::
FCMC_SingleClimbPointC UHelpfulFunctionsBPLibrary::FindLedgePointStructure(const UObject* WorldContextObject, const ACharacter* TargetChar, FVector StartLocation,
	FVector ForwardDirection, TArray<UClass*> NotThisClass, float ForwardScale, float UpOffset, bool preValid, FVector preLocation, FVector preNormal, 
	bool IsFirst, bool ForLeftSide, int DebugIndex, ECollisionChannel TraceChannel, float NormalFindOffset)
{

	FCMC_SingleClimbPointC RetStruct;

	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (DebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}

	if (!IsValid(TargetChar))
	{
		RetStruct.ValidPoint = false;
		RetStruct.Location = StartLocation;
		RetStruct.Normal = ForwardDirection;
		return RetStruct;
	}

	TArray<AActor*> ToIgnore;
	ToIgnore.Add(TargetChar->GetOwner());

	ETraceTypeQuery MyTraceQuery = UEngineTypes::ConvertToTraceType(TraceChannel);

	FVector FirstTraceOrigin = StartLocation + FVector(0, 0, UpOffset);
	FHitResult CapsuleHit;

	float CapTraceHeight = 30;
	if (preValid == true && !IsFirst == true)
	{
		CapTraceHeight = 20;
	}

	const bool CapsuleReturnHit = UKismetSystemLibrary::CapsuleTraceSingle(TargetChar, FirstTraceOrigin, FirstTraceOrigin + (ForwardDirection * ForwardScale), 10.0,
		CapTraceHeight, ETraceTypeQuery::TraceTypeQuery1, false, ToIgnore, TraceType, CapsuleHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.06f);

	if (CapsuleReturnHit == false)
	{
		RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
	}

	if (UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.Location, CapsuleHit.TraceStart, 0.05) == true &&
		UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.Location, CapsuleHit.TraceEnd, 0.05) == true &&
		UKismetMathLibrary::NotEqual_VectorVector(CapsuleHit.ImpactPoint, CapsuleHit.TraceEnd, 0.05) == true &&
		UKismetMathLibrary::Dot_VectorVector(ForwardDirection, CapsuleHit.ImpactNormal) < 0.5)
	{
		CapTraceHeight = CapTraceHeight + 0.5;
	}
	else
	{
		RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
	}

	FVector SphereTraceOrigin = CapsuleHit.ImpactPoint + (CapsuleHit.Normal * 3.0);
	SphereTraceOrigin = FVector(SphereTraceOrigin.X, SphereTraceOrigin.Y, CapsuleHit.Location.Z);

	FHitResult SphereHit;
	const bool SphereReturnHit = UKismetSystemLibrary::SphereTraceSingle(TargetChar, SphereTraceOrigin + FVector(0, 0, -15), SphereTraceOrigin + FVector(0, 0, 15), 10,
		MyTraceQuery, false, ToIgnore, TraceType, SphereHit, true, FLinearColor(0.35f, 0.0f, 0.48f, 1.0f), FLinearColor::Yellow, 0.1f);
	if (SphereReturnHit == false)
	{
		RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
	}

	FHitResult LineHit;
	bool VerticalHitReturn = false;
	bool UseImpactAsStart = false;
	bool ClassToIgnoreValid = false;
	FVector LineImpactPoint;
	FVector LineTraceOrigin;
	FLinearColor LineTraceColor = FLinearColor(0, 0.05f, 0.5f, 1.0f);

	for (int i = 0; i < 2; i++)
	{
		LineTraceOrigin = CapsuleHit.ImpactPoint;
		LineTraceOrigin = FVector(LineTraceOrigin.X, LineTraceOrigin.Y, SphereHit.ImpactPoint.Z + 3);
		LineTraceOrigin = LineTraceOrigin + (NormalToVector(CapsuleHit.Normal) * 3);
		// SelectVector(Wartosc jezeli Prawda, Wartosc jezeli Falsz, Warunek Bool)

		VerticalHitReturn = UKismetSystemLibrary::LineTraceSingle(TargetChar, UKismetMathLibrary::SelectVector(LineImpactPoint - FVector(0, 0, 7), LineTraceOrigin + FVector(0, 0, 16), UseImpactAsStart),
			LineTraceOrigin + FVector(0, 0, -15), MyTraceQuery, false, ToIgnore, TraceType, LineHit, true, LineTraceColor, FLinearColor(0, 0.5f, 1.0f, 1.0f), 0.1f);


		if (VerticalHitReturn == false)
		{
			RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
		}

		if (IsValid(LineHit.GetActor()) == true)
		{
			ClassToIgnoreValid = !ClassToIgnore(LineHit.GetActor()->GetClass(), NotThisClass);
		}
		else
		{
			ClassToIgnoreValid = !IsValid(LineHit.GetComponent());
		}

		if (ClassToIgnoreValid==true)
		{
			RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
		}

		if (UKismetMathLibrary::EqualEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceStart, 0.8) == true)
		{
			LineImpactPoint = LineHit.ImpactPoint;
			UseImpactAsStart = true;
			LineTraceColor = FLinearColor(0.48f, 0.01f, 0.1f, 1.0f);
		}
		else
		{
			UseImpactAsStart = false;
			break;
		}
	}

	float e = 0.03; //Zamienic false na true
	if (UKismetMathLibrary::NotEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceStart, e) == false || UKismetMathLibrary::NotEqual_VectorVector(LineHit.ImpactPoint, LineHit.TraceEnd, e) == false
		|| UKismetMathLibrary::NotEqual_VectorVector(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::MakeRotFromX(LineHit.Normal)), FVector(0, 0, 1), 0.8) == false)
	{
		RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
	}

	FHitResult DirectionLineHit;
	const bool DirectionReturnHit = UKismetSystemLibrary::LineTraceSingle(TargetChar, LineHit.ImpactPoint - FVector(0, 0, 2) + (NormalToVector(CapsuleHit.Normal) * -8),
		LineHit.ImpactPoint - FVector(0, 0, 2) + (NormalToVector(CapsuleHit.Normal) * 4), MyTraceQuery, false, ToIgnore, TraceType,
		DirectionLineHit, true, FLinearColor(1, 0.22f, 0.0f, 1.0f), FLinearColor(0.9f, 0.98f, 0.0f, 1.0f), 0.06f);

	if (DirectionReturnHit == false)
	{
		RetStruct.ValidPoint = false; RetStruct.Location = StartLocation; RetStruct.Normal = ForwardDirection; return RetStruct;
	}

	FVector RV = DirectionLineHit.ImpactPoint;
	RV = FVector(RV.X, RV.Y, LineHit.ImpactPoint.Z);
	FVector RN = NormalToVector(DirectionLineHit.Normal);

	bool LastCondition = true;
	if (IsFirst == true)
	{
		LastCondition = true;
	}
	else
	{
		LastCondition = preValid && UKismetMathLibrary::EqualEqual_VectorVector(preNormal, RN) && abs(preLocation.Z - RV.Z) < 20;
		if (preValid == false)
		{
			LastCondition = true;
		}
	}

	if (UKismetMathLibrary::Dot_VectorVector(RN, ForwardDirection) > 0.45 && LastCondition == true)
	{
		RetStruct.ValidPoint = true;
		RetStruct.Location = RV;
		RetStruct.Normal = RN;
		RetStruct.ActorTransform = DirectionLineHit.GetComponent()->GetComponentTransform();
		RetStruct.Component = DirectionLineHit.GetComponent();
		return RetStruct;
	}

	RetStruct.ValidPoint = false;
	RetStruct.Location = StartLocation;
	RetStruct.Normal = ForwardDirection;
	return RetStruct;
}

// [][][][][][][][][][][][][][][][]   T R Y   F I N D   L E D G E   L I N E  [][][][][][][][][][][][][][][][] 
void UHelpfulFunctionsBPLibrary::TryFindLedgeLine(const UObject* WorldContextObject, bool& Valid, FCMC_SingleClimbPointC& LedgeStruct, const ACharacter* TargetChar,
	FVector StartLocation, FVector StartDirection, FVector2D AxisNormal, float ZOffset, float ForwardTraceLength, float RightOffsetScale, bool LeftDirection, 
	bool InverseTracing, int TraceDebugIndex, TArray<UClass*> NotThisClass, ECollisionChannel TraceChannel, float NormalFindOffset)
{
	bool AnyPointValid = false;
	bool ContinueFinding = false;
	bool IsFirstIndex = false;
	bool CurrentPointValid = false;
	bool SelectNormal = false;
	float RightOffsetValue = 0;
	TArray<FCMC_SingleClimbPointC> PointsArrayStruct;
	FCMC_SingleClimbPointC BestPointStruct;
	FCMC_SingleClimbPointC PrePointData;
	FCMC_SingleClimbPointC LedgeFunctionReturn;
	FVector CalcLocation;
	FVector CalcNormal;
	PrePointData.ValidPoint = false;

	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (TraceDebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (TraceDebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}

	if (!IsValid(TargetChar))
	{
		Valid = false;
		LedgeStruct.Location = StartLocation;
		LedgeStruct.Normal = StartDirection;
		LedgeStruct.ValidPoint = false;
		return;
	}
	// Start LOOP
	for (int i = 0; i < 4; i++)
	{
		if (InverseTracing == true)
		{
			if (AnyPointValid) { ContinueFinding = false; }
			else { ContinueFinding = true; }
		}
		else { ContinueFinding = true; }

		if (ContinueFinding == true)
		{
			// Sequencer Then (0)
			if (i == 0) { IsFirstIndex = true; }
			else { IsFirstIndex = false; }

			if (PointsArrayStruct.Num() == 1)
			{ PrePointData = PointsArrayStruct[0]; }
			if (PointsArrayStruct.Num() > 1)
			{ PrePointData = PointsArrayStruct[UKismetMathLibrary::ClampInt64(i-1, 0, PointsArrayStruct.Num()-1)]; }

			// Sequencer Then (1)
			RightOffsetValue = RightOffsetValue + 4;
			if (PointsArrayStruct.Num() > 0) 
			{
				if (PointsArrayStruct.Last().ValidPoint == true) { SelectNormal = true; }
				else { SelectNormal = false; }
			}
			else
			{ SelectNormal = false; }

			CalcLocation = StartLocation + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(StartDirection)) * 
						   (RightOffsetScale * UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(RightOffsetValue,4.0,16.0,16.0,4.0),RightOffsetValue,InverseTracing) * 
						   UKismetMathLibrary::SelectFloat(-1,1,LeftDirection)));

			if(SelectNormal==true && PointsArrayStruct.Num() > 0)
			{ CalcNormal = UKismetMathLibrary::SelectVector(UKismetMathLibrary::VLerp(StartDirection, PointsArrayStruct.Last().Normal, 0.5), StartDirection, SelectNormal); }
			else { CalcNormal = StartDirection; }

			//Sequence Then (2)
			LedgeFunctionReturn = FindLedgePointStructure(TargetChar, TargetChar, CalcLocation, CalcNormal, NotThisClass, ForwardTraceLength, ZOffset, PrePointData.ValidPoint, PrePointData.Location, 
								  PrePointData.Normal, IsFirstIndex, LeftDirection, TraceDebugIndex, TraceChannel, NormalFindOffset);
			if (LedgeFunctionReturn.ValidPoint == true)
			{
				PointsArrayStruct.Add(LedgeFunctionReturn);
				if (PointsArrayStruct.Num() > 0)
				{ AnyPointValid = true; }
			}
		}
	}
	//Return Section
	if (AnyPointValid == true)
	{
		if (InverseTracing == false) //JA PIERDOLE, TYLE GODZIN SZUKANIA ZEBY FINALNIE ZAMIENIC TRUE NA FALSE!!!
		{
			BestPointStruct = PointsArrayStruct.Last();
		}
		else
		{
			BestPointStruct = PointsArrayStruct[0];
		}
		Valid = BestPointStruct.ValidPoint;
		LedgeStruct = BestPointStruct;
		return;
	}
	else
	{
		Valid = false;
		LedgeStruct.Location = StartLocation;
		LedgeStruct.Normal = StartDirection;
		LedgeStruct.ValidPoint = false;
		return;
	}
}

// Ledge Validation Part 1
bool UHelpfulFunctionsBPLibrary::ClimbingLedgeValidP1(const UObject* WorldContextObject,bool IsValid, FCMC_SingleClimbPointC LeftStruct, FCMC_SingleClimbPointC RightStruct, ECollisionChannel TraceChannel, float UpAxisTollerance)
{
	if(IsValid==false)
	{ return false; }

	if (LeftStruct.ValidPoint == true && RightStruct.ValidPoint == true)
	{
		if (abs(LeftStruct.Location.Z - RightStruct.Location.Z) > UpAxisTollerance)
		{ return false; }
		TArray<AActor*> ToIgnoreActors;
		ETraceTypeQuery MyTraceQuery = UEngineTypes::ConvertToTraceType(TraceChannel);
		FHitResult LineHitResult;
		const bool LineHitValid = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, LeftStruct.Location + FVector(0, 0, 3), RightStruct.Location + FVector(0, 0, 3), 
				   MyTraceQuery, false, ToIgnoreActors, EDrawDebugTrace::None, LineHitResult, true, FColor::Red, FColor::Orange, 0.2);
		if (LineHitValid == false)
		{ return true; }
		else
		{ return false; }
	}
	return false;
}
// Foots IK System
void UHelpfulFunctionsBPLibrary::CalcFootsOffset(const UObject* WorldContextObject, ACharacter* Character, float EnableFootCurveValue, FName IKFootName, FName RootBone, FVector CurrentLocation, 
	 FVector CurrentLocationOffset, FRotator CurrentRotationOffset, float DeltaSecond, float IKTraceDistanceAboveFoot, float TraceUpOffset, float IKTraceDistanceBelowFoot, float FootHeight, FVector& NewLocation, 
	 FVector& NewLocationOffset, FRotator& NewRotationOffset, int DebugIndex, bool UseByFloorFinding, bool& TracingType)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	if (!IsValid(Character))
	{
		NewLocation = FVector(0, 0, 0); NewLocationOffset = FVector(0, 0, 0); NewRotationOffset = FRotator(0, 0, 0); return;
	}

	TArray<AActor*> ToIgnore;
	ToIgnore.Add(Character->GetOwner());
	ECollisionChannel TraceVisibility = ECollisionChannel::ECC_Visibility;
	ETraceTypeQuery MyTraceQuery = UEngineTypes::ConvertToTraceType(TraceVisibility);

	if (EnableFootCurveValue < 0.02)
	{ NewLocation = CurrentLocation; NewLocationOffset = FVector(0, 0, 0); NewRotationOffset = FRotator(0, 0, 0); return; }

	bool WalkableFloorDetected = false;
	bool TargetTracingType = false;
	float InterpMultipler = 0;
	FRotator TargetRotOffset=FRotator(0,0,0);
	FVector TargetLocOffset=FVector(0,0,0);
	FVector ImpactPoint=FVector(0,0,0);
	FVector ImpactNormal=FVector(0,0,1);
	FVector IKFootFloorLocation = Character->GetMesh()->GetSocketLocation(IKFootName);
	IKFootFloorLocation = FVector(IKFootFloorLocation.X, IKFootFloorLocation.Y, Character->GetMesh()->GetSocketLocation(RootBone).Z);

	FFindFloorResult FootFloorHit;
	FHitResult FootTraceHitResult;
	const bool FootTraceValid = UKismetSystemLibrary::LineTraceSingle(Character, IKFootFloorLocation + FVector(0, 0, IKTraceDistanceAboveFoot + TraceUpOffset), 
		       IKFootFloorLocation - FVector(0, 0, IKTraceDistanceBelowFoot), MyTraceQuery, false, ToIgnore, TraceType, FootTraceHitResult, true);
	if (Character->GetCharacterMovement()->IsWalkable(FootTraceHitResult) == true)
	{
		ImpactPoint = FootTraceHitResult.ImpactPoint;
		ImpactNormal = FootTraceHitResult.ImpactNormal;
		TargetRotOffset = FRotator(UKismetMathLibrary::DegAtan2(ImpactNormal.X, ImpactNormal.Z) * -1, 0, UKismetMathLibrary::DegAtan2(ImpactNormal.Y, ImpactNormal.Z));
		TargetLocOffset = (ImpactPoint + (ImpactNormal * FootHeight)) - (IKFootFloorLocation + (FVector(0, 0, 1) * FootHeight));
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, TargetLocOffset.ToString());
	}
	else if (UseByFloorFinding == true)
	{
		Character->GetCharacterMovement()->FindFloor(IKFootFloorLocation + FVector(0, 0, (IKTraceDistanceAboveFoot + TraceUpOffset) * 0.8), FootFloorHit, true);
		WalkableFloorDetected = FootFloorHit.bWalkableFloor;
		ImpactPoint = FootFloorHit.HitResult.ImpactPoint;
		if (WalkableFloorDetected == true)
		{
			FTransform RelativeOffset;
			FRotator CharRot = Character->GetActorRotation();
			if (UKismetMathLibrary::Vector_Distance2D(ImpactPoint, IKFootFloorLocation + FVector(0, 0, (IKTraceDistanceAboveFoot + TraceUpOffset) * 0.8)) > 4)
			{
				RelativeOffset = UKismetMathLibrary::MakeRelativeTransform(FTransform(Character->GetActorRotation(), ImpactPoint, FVector(1, 1, 1)), 
																		   FTransform(Character->GetActorRotation(), IKFootFloorLocation, FVector(1, 1, 1)));

				RelativeOffset = FTransform(RelativeOffset.GetRotation(), FVector(UKismetMathLibrary::FClamp(RelativeOffset.GetLocation().X, -4, 4), 
								 UKismetMathLibrary::FClamp(RelativeOffset.GetLocation().Y, -4, 4), RelativeOffset.GetLocation().Z),RelativeOffset.GetScale3D());

				ImpactPoint = IKFootFloorLocation + (UKismetMathLibrary::GetForwardVector(CharRot) * RelativeOffset.GetLocation().X)+ (UKismetMathLibrary::GetRightVector(CharRot) * RelativeOffset.GetLocation().Y);
				ImpactPoint = FVector(ImpactPoint.X, ImpactPoint.Y, UKismetMathLibrary::FClamp(FootFloorHit.HitResult.ImpactPoint.Z,IKFootFloorLocation.Z-6, FootFloorHit.HitResult.ImpactPoint.Z));
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, RelativeOffset.ToString());
				ImpactNormal = FVector(0, 0, 1);
				InterpMultipler = 0.8;
				TargetRotOffset = FRotator(UKismetMathLibrary::DegAtan2(ImpactNormal.X, ImpactNormal.Z) * -1, 0, UKismetMathLibrary::DegAtan2(ImpactNormal.Y, ImpactNormal.Z));
				TargetLocOffset = (ImpactPoint + (ImpactNormal * FootHeight)) - (IKFootFloorLocation + (FVector(0, 0, 1) * FootHeight));
				TargetTracingType = true;
			}
		}
	}
	if (CurrentLocationOffset.Z > CurrentLocation.Z)
	{
		TracingType = TargetTracingType;
		NewLocation = TargetLocOffset;
		NewLocationOffset = UKismetMathLibrary::VInterpTo(CurrentLocationOffset, TargetLocOffset, DeltaSecond, InterpMultipler * 30);
		NewRotationOffset = UKismetMathLibrary::RInterpTo(CurrentRotationOffset, TargetRotOffset, DeltaSecond, 30);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, NewRotationOffset.ToString());
		return;
	}
	else
	{
		TracingType = TargetTracingType;
		NewLocation = TargetLocOffset;
		NewLocationOffset = UKismetMathLibrary::VInterpTo(CurrentLocationOffset, TargetLocOffset, DeltaSecond, InterpMultipler * 15);
		NewRotationOffset = UKismetMathLibrary::RInterpTo(CurrentRotationOffset, TargetRotOffset, DeltaSecond, 30);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, NewRotationOffset.ToString());
		return;
	}
}
// .:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:.
// .:.:.:.:.:.:.:.:.:.        C_H_A_R_A_C_T_E_R____L_O_G_I_C       .:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.
// .:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:..:.:.:.:.:.:.:.:.:.

// Character - Calculate Diagonal Scale Amount (Advanced Locomotion System)
float UHelpfulFunctionsBPLibrary::CalculateDiagonalScale(UCurveFloat* Curve, FVelocityBlendCpp VelocityBlend)
{
	return Curve->GetFloatValue(abs(VelocityBlend.F+VelocityBlend.B));
}
// Character - Calculate Relative Acceleration Amount (Advanced Locomotion System)
FVector UHelpfulFunctionsBPLibrary::CalculateRelativeAcceleration(const UObject* WorldContextObject, const UCharacterMovementComponent* CharMove, FRotator CharRot, FVector Acceleration, FVector Velocity)
{
	FVector ReturnVec = FVector(0,0,0);
	if (IsValid(CharMove) == false) return FVector(999, 0, 0);

	FQuat CharRotQuat = UKismetMathLibrary::Conv_RotatorToQuaternion(CharRot);
	if (UKismetMathLibrary::Dot_VectorVector(Acceleration, Velocity) > 0)
	{
		ReturnVec = UKismetMathLibrary::Vector_ClampSizeMax(Acceleration, CharMove->GetMaxAcceleration());
		ReturnVec = ReturnVec / CharMove->GetMaxAcceleration();
		ReturnVec = UKismetMathLibrary::Quat_UnrotateVector(CharRotQuat, ReturnVec);
	}
	else
	{
		ReturnVec = UKismetMathLibrary::Vector_ClampSizeMax(Acceleration, CharMove->GetMaxBrakingDeceleration());
		ReturnVec = ReturnVec / CharMove->GetMaxBrakingDeceleration();
		ReturnVec = UKismetMathLibrary::Quat_UnrotateVector(CharRotQuat, ReturnVec);
	}
	return ReturnVec;
}
// Calculate Velocity Blend (Advanced Locomotion System)
FVelocityBlendCpp UHelpfulFunctionsBPLibrary::CalcVelocityBlendFast(const UObject* WorldContextObject, FVector Velocity)
{
	const UAnimInstance* AnimBP = Cast<UAnimInstance>(WorldContextObject);
	if (AnimBP != nullptr)
	{
		FVelocityBlendCpp ReturnStruct;
		UKismetMathLibrary::Vector_Normalize(Velocity, 0.1);
		FVector LocRelativeVelocity = UKismetMathLibrary::Quat_UnrotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(AnimBP->GetOwningActor()->GetActorRotation()),Velocity);
		float sum = abs(LocRelativeVelocity.X) + abs(LocRelativeVelocity.Y) + abs(LocRelativeVelocity.Z);
		FVector RelativeDirection = LocRelativeVelocity / sum;
		ReturnStruct.F = UKismetMathLibrary::FClamp(RelativeDirection.X, 0, 1);
		ReturnStruct.B = abs(UKismetMathLibrary::FClamp(RelativeDirection.X, -1, 0));
		ReturnStruct.L = abs(UKismetMathLibrary::FClamp(RelativeDirection.Y, -1, 0));
		ReturnStruct.R = UKismetMathLibrary::FClamp(RelativeDirection.Y, 0, 1);
		return ReturnStruct;
	}
	return FVelocityBlendCpp();
}
// Update Movement Values (ALSv4)
void UHelpfulFunctionsBPLibrary::UpdateMovementValues(const UObject* WorldContextObject, const UCharacterMovementComponent* CharMove, FRotator ActorRot, FVelocityBlendCpp VelocityBlend, float DeltaX, float VelocityBlendInterpSpeed,
	float AnimatedWalkSpeed, float AnimatedRunSpeed, float AnimatedSprintSpeed, float AnimatedCrouchSpeed, float Speed, UCurveFloat* DiagonalScaleAmountCurve, UCurveFloat* StrideBlend_N_Walk, 
	UCurveFloat* StrideBlend_N_Run, UCurveFloat* StrideBlend_C_Walk, FVector Velocity, FVector Acceleration, FVector2D LeanAmount, float GroundedLeanInterpSpeed, FName WeightCurveName, FName BasePoseCurveName, FVelocityBlendCpp& ReturnVelocityBlend,
	float& ReturnDiagonalScale, FVector& ReturnRelativeAcc, FVector2D& ReturnLeanAmount, float& ReturnWalkRunBlend, float& ReturnStrideBlend, float& ReturnStandingPlayRate, float& ReturnCrouchPlayRate)
{
	if (!IsValid(CharMove))
	{ return; }

	const UAnimInstance* AnimBP = Cast<UAnimInstance>(WorldContextObject);
	if (!IsValid(AnimBP))
	{ return; }
	// Calculate Smoothed Velocity Blend
	FVelocityBlendCpp TargetVelocityBlend = CalcVelocityBlendFast(WorldContextObject, Velocity);
	ReturnVelocityBlend.F = UKismetMathLibrary::FInterpTo(VelocityBlend.F, TargetVelocityBlend.F, DeltaX, VelocityBlendInterpSpeed);
	ReturnVelocityBlend.B = UKismetMathLibrary::FInterpTo(VelocityBlend.B, TargetVelocityBlend.B, DeltaX, VelocityBlendInterpSpeed);
	ReturnVelocityBlend.L = UKismetMathLibrary::FInterpTo(VelocityBlend.L, TargetVelocityBlend.L, DeltaX, VelocityBlendInterpSpeed);
	ReturnVelocityBlend.R = UKismetMathLibrary::FInterpTo(VelocityBlend.R, TargetVelocityBlend.R, DeltaX, VelocityBlendInterpSpeed);
	// Calculate Diagonal Scale Amount
	ReturnDiagonalScale = CalculateDiagonalScale(DiagonalScaleAmountCurve, ReturnVelocityBlend);
	// Calculate Relative Acceleration Amount
	ReturnRelativeAcc = CalculateRelativeAcceleration(WorldContextObject, CharMove, ActorRot, Acceleration, Velocity);
	// Make Smoothed Lead Amount
	ReturnLeanAmount = UKismetMathLibrary::Vector2DInterpTo(LeanAmount, FVector2D(ReturnRelativeAcc.Y, ReturnRelativeAcc.X), DeltaX, GroundedLeanInterpSpeed);
	// Calculate Stride Blend
	float StrideCurveValue = UKismetMathLibrary::Lerp(StrideBlend_N_Walk->GetFloatValue(Speed), StrideBlend_N_Run->GetFloatValue(Speed), GetClampedCurveValue(AnimBP, WeightCurveName, -1, 0, 1));
	ReturnStrideBlend = UKismetMathLibrary::Lerp(StrideCurveValue, StrideBlend_N_Walk->GetFloatValue(Speed), AnimBP->GetCurveValue(BasePoseCurveName));
	// Calculate Standing Play Rate
	float SPL = UKismetMathLibrary::Lerp(UKismetMathLibrary::SafeDivide(Speed, AnimatedWalkSpeed), UKismetMathLibrary::SafeDivide(Speed, AnimatedRunSpeed), GetClampedCurveValue(AnimBP, WeightCurveName, -1, 0, 1));
	SPL = UKismetMathLibrary::Lerp(SPL, UKismetMathLibrary::SafeDivide(Speed, AnimatedSprintSpeed), GetClampedCurveValue(AnimBP, WeightCurveName, -2, 0, 1));
	SPL = SPL / ReturnStrideBlend;
	SPL = SPL / AnimBP->GetOwningComponent()->K2_GetComponentScale().Z;
	ReturnStandingPlayRate = UKismetMathLibrary::FClamp(SPL, 0, 3);
	// Calculate Crouching PlayRate
	ReturnCrouchPlayRate = UKismetMathLibrary::FClamp(UKismetMathLibrary::SafeDivide(UKismetMathLibrary::SafeDivide(Speed, AnimatedCrouchSpeed), ReturnStrideBlend) 
																									/ AnimBP->GetOwningComponent()->K2_GetComponentScale().Z, 0, 3);

}
// Update Aiming Rotation Values (ALSv4)
void UHelpfulFunctionsBPLibrary::UpdateAimingValues(const UObject* WorldContextObject, const ACharacter* TargetChar, FRotator AimingRotation, FRotator SmoothedAimRotation, float InputYawOffsetTime,
	int RotationMode, bool HasMovementInput, FVector MovementInput, float DeltaX, float SmoothedAimingRotSpeed, float InputYawOffsetSpeed, FRotator& ReturnSmoothedAimRotation, FVector2D& ReturnAimingAngle, 
	FVector2D& ReturnSmoothedAimingAngle, float& ReturnAimSweepTime, FRotator& ReturnSpineRotation, float& ReturnInputYawOffsetTime, float& ReturnLeftYawTime, float& ReturnRightYawTime, float& ReturnForwardYawTime)
{
	if (!IsValid(TargetChar))
	{ return; }
	ReturnSmoothedAimRotation = UKismetMathLibrary::RInterpTo(SmoothedAimRotation, AimingRotation, DeltaX, SmoothedAimingRotSpeed);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimingRotation, TargetChar->GetActorRotation());
	ReturnAimingAngle = FVector2D(DeltaRot.Yaw, DeltaRot.Pitch);
	DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(SmoothedAimRotation, TargetChar->GetActorRotation());
	ReturnSmoothedAimingAngle = FVector2D(DeltaRot.Yaw, DeltaRot.Pitch);
	if (RotationMode == 1 || RotationMode == 2)
	{ 
	ReturnAimSweepTime = UKismetMathLibrary::MapRangeClamped(ReturnAimingAngle.Y, -90, 90, 1, 0);
	ReturnSpineRotation = FRotator(0, ReturnAimingAngle.X / 4, 0);
	ReturnInputYawOffsetTime = InputYawOffsetTime;
	}
	else
	{
		if (HasMovementInput == true)
		{ DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(MovementInput), TargetChar->GetActorRotation());
		  ReturnInputYawOffsetTime = UKismetMathLibrary::FInterpTo(InputYawOffsetTime, UKismetMathLibrary::MapRangeClamped(DeltaRot.Yaw, -180, 180, 0, 1), DeltaX, InputYawOffsetSpeed); }
		else
		{
			ReturnInputYawOffsetTime = InputYawOffsetTime;
		}
	}
	ReturnLeftYawTime = UKismetMathLibrary::MapRangeClamped(abs(ReturnSmoothedAimingAngle.X), 0, 180, 0.5, 0);
	ReturnRightYawTime = UKismetMathLibrary::MapRangeClamped(abs(ReturnSmoothedAimingAngle.X), 0, 180, 0.5, 1);
	ReturnForwardYawTime = UKismetMathLibrary::MapRangeClamped(ReturnSmoothedAimingAngle.X, -180, 180, 0, 1);
	return;
}
// ALS Anim Graph Function - Update Movement Values (Low Cost Calculation) | FOR AI
void UHelpfulFunctionsBPLibrary::UpdateMovementValuesLod(const UObject* WorldContextObject, FVector MovementLowDetail, FVector& ReturnMovementLowDetail, float& ReturnStandingPlayRate, 
	float& ReturnCrouchingPlayRate, float Speed, float DeltaX, const float AnimatedWalkSpeed, const float AnimatedRunSpeed, const float AnimatedSprintSpeed, const float AnimatedCrouchSpeed, FName WeightCurveName, bool ReturnOnlyDirection)
{
	const UAnimInstance* AnimBP = Cast<UAnimInstance>(WorldContextObject);
	if (!IsValid(AnimBP))
	{ return; }
	if (IsValid(AnimBP->TryGetPawnOwner()) == false)
	{
		ReturnMovementLowDetail = FVector(0, 0, 0);
		ReturnStandingPlayRate, ReturnCrouchingPlayRate = 0.0;
		return;
	}
	if (ReturnOnlyDirection != true)
	{
	// Calculate Standing Play Rate
	float SPL = UKismetMathLibrary::Lerp(UKismetMathLibrary::SafeDivide(Speed, AnimatedWalkSpeed), UKismetMathLibrary::SafeDivide(Speed, AnimatedRunSpeed), GetClampedCurveValue(AnimBP, WeightCurveName, -1, 0, 1));
	SPL = UKismetMathLibrary::Lerp(SPL, UKismetMathLibrary::SafeDivide(Speed, AnimatedSprintSpeed), GetClampedCurveValue(AnimBP, WeightCurveName, -2, 0, 1));
	SPL = SPL / AnimBP->GetOwningComponent()->K2_GetComponentScale().Z;
	ReturnStandingPlayRate = UKismetMathLibrary::FClamp(SPL, 0, 3);
	// Calculate Crouching PlayRate
	ReturnCrouchingPlayRate = UKismetMathLibrary::FClamp(UKismetMathLibrary::SafeDivide(UKismetMathLibrary::SafeDivide(Speed, AnimatedCrouchSpeed), 1)
																					   / AnimBP->GetOwningComponent()->K2_GetComponentScale().Z, 0, 3);
	}
	FVector Velo = AnimBP->TryGetPawnOwner()->GetVelocity();
	const float DirectionForwardHelper = UKismetMathLibrary::Dot_VectorVector(AnimBP->TryGetPawnOwner()->GetActorForwardVector(), UKismetMathLibrary::ClampVectorSize(Velo, -1, 1));
	const float DirectionRightHelper = UKismetMathLibrary::Dot_VectorVector(AnimBP->TryGetPawnOwner()->GetActorRightVector(), UKismetMathLibrary::ClampVectorSize(Velo, -1, 1));
	ReturnMovementLowDetail = UKismetMathLibrary::VInterpTo(MovementLowDetail, FVector(DirectionRightHelper, DirectionForwardHelper, ReturnStandingPlayRate), DeltaX, 15.0);
	
}
// ALS Anim Graph Function - Update Rotation Values
void UHelpfulFunctionsBPLibrary::UpdateRotationValues(const UObject* WorldContextObject, int& ReturnDirection, float& FYaw, float& BYaw, float& LYaw, float& RYaw, 
	int Gait, int RotationMode, int MovementDirection, FRotator AimingRotation, FVector Velocity, UCurveVector* YawOffset_FB, UCurveVector* YawOffset_LR)
{
	const UAnimInstance* AnimBP = Cast<UAnimInstance>(WorldContextObject);
	if (IsValid(AnimBP->TryGetPawnOwner()) == false)
	{ ReturnDirection = 0; FYaw, BYaw, LYaw, RYaw = 0.0; return; }

	int MD = 0;
	if (Gait == 2)
	{ MD = 0; }
	else if (RotationMode == 0)
	{ MD = 0; }
	else
	{
		MD = CalculateQuadrant(MovementDirection, 70.0, -70.0, 110.0, -110.0, 5, UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(Velocity), AimingRotation).Yaw);
	}
	float DeltaControlRot = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(Velocity), AnimBP->TryGetPawnOwner()->GetControlRotation()).Yaw;
	ReturnDirection = MD;
	if (IsValid(YawOffset_FB) == false || IsValid(YawOffset_LR) == false) return;
	FYaw = YawOffset_FB->GetVectorValue(DeltaControlRot).X;
	BYaw = YawOffset_FB->GetVectorValue(DeltaControlRot).Y;
	LYaw = YawOffset_LR->GetVectorValue(DeltaControlRot).X;
	RYaw = YawOffset_LR->GetVectorValue(DeltaControlRot).Y;
	return;
}
// ALS Anim Graph Function - Angle In Range = Return Bool Value
bool UHelpfulFunctionsBPLibrary::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer == true)
	{
		const bool InRange = UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle - Buffer, MaxAngle + Buffer, true, true);
		return InRange;
	}
	const bool InRange = UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle = Buffer, MaxAngle - Buffer, true, true);
	return InRange;
}
// ALS Anim Graph Function - Calculate Quadrant = Return Movement Direction State
int UHelpfulFunctionsBPLibrary::CalculateQuadrant(int MovementDirection, float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer, float Angle)
{
	const bool IncreaseForward = MovementDirection != 0 || MovementDirection != 3;
	const bool IncreaseLeft = MovementDirection != 1 || MovementDirection != 2;
	const bool IncreaseRight = MovementDirection != 1 || MovementDirection != 2;;
	bool AngleRange;
	AngleRange = AngleInRange(Angle, FL_Threshold, FR_Threshold, Buffer, IncreaseForward);
	if (AngleRange == true) { return 0; }
	AngleRange = AngleInRange(Angle, FR_Threshold, BR_Threshold, Buffer, IncreaseLeft);
	if (AngleRange == true) { return 1; }
	AngleRange = AngleInRange(Angle, BL_Threshold, FL_Threshold, Buffer, IncreaseRight);
	if (AngleRange == true) { return 2; }
	return 3;
}
// ALS Blueprint Logic - Set  Character Rotation With Smooth Effect
void UHelpfulFunctionsBPLibrary::SmoothCharacterRotation(const UObject* WorldContextObject, const ACharacter* Character, FRotator& NewTargetRotation, FRotator Target, FRotator CurrentTargetRotation, 
	float DeltaSecond, float TargetInterpSpeedConst, float TargetInterpSpeedSmooth, bool UpdateControl, FRotator& NewControlRotation)
{
	if (!IsValid(Character))
	{ return; }
	AActor* Player = (AActor*)Character;
	NewTargetRotation = UKismetMathLibrary::RInterpTo_Constant(CurrentTargetRotation, Target, DeltaSecond, TargetInterpSpeedConst);
	FQuat NewActorRot = UKismetMathLibrary::Conv_RotatorToQuaternion(UKismetMathLibrary::RInterpTo(Character->GetActorRotation(), NewTargetRotation, DeltaSecond, TargetInterpSpeedSmooth));
	Player->SetActorRotation(NewActorRot, ETeleportType::None);
	if (UpdateControl == true && IsValid(Character->GetController()))
	{
		FRotator SocketRot = Character->GetMesh()->GetSocketRotation(TEXT("headSocket"));
		SocketRot = FRotator(0, SocketRot.Yaw, 0);
		NewControlRotation = UKismetMathLibrary::RInterpTo(Character->GetControlRotation(), SocketRot, DeltaSecond, 2.0);
	}
	return;
}


// Animation Logic - Get Curve Value (Target Is Anim Instance)
float UHelpfulFunctionsBPLibrary::GetClampedCurveValue(const UObject* WorldContextObject, FName Curve, float Bias, float ClampMin, float ClampMax)
{
	const UAnimInstance* AnimBP = Cast<UAnimInstance>(WorldContextObject);
	if (AnimBP != nullptr)
	{
		return UKismetMathLibrary::FClamp(AnimBP->GetCurveValue(Curve)+Bias,ClampMin,ClampMax);
	}
	return 0.0f;
}


// ------------ PODSTAWOWA DEFINICJA ROTACJI --------------------
//                 FRotator(Pitch,Yaw,Roll)
// --------------------------------------------------------------

// ###################################################################################################
// ########################## -------> Interpolation Functions <------- ##############################
// ###################################################################################################
// Ealistic Out Interpolation Function
FVector UHelpfulFunctionsBPLibrary::ElasticOutInterpFast(FVector A, FVector B, float Alpha, float PowerBase)
{
	float t;
	t = (sin(-13.0 * (Alpha + 1) * (PI / 2)) * pow(PowerBase, -10 * Alpha));
	t = t + 1;
	FVector V = FMath::Lerp(A,B,t);
	return V;
}
// Bounce Out Interpolation Function
FVector UHelpfulFunctionsBPLibrary::BouceOutInterpFast(FVector A, FVector B, float Alpha)
{
	return FMath::Lerp(A, B, BounceOutTimeFunction(Alpha));
}

// Get Angle Between Two Vectors (In Radian)
float UHelpfulFunctionsBPLibrary::GetAngleBetween(FVector A, FVector B)
{
	float GetDot = UKismetMathLibrary::Dot_VectorVector(A,B);
	float l = UKismetMathLibrary::VSize(A) * UKismetMathLibrary::VSize(B);
	return UKismetMathLibrary::Acos(GetDot/l);
}

// Spherical Linear Interpolation (Vector)
FVector UHelpfulFunctionsBPLibrary::SlerpFast(FVector A, FVector B, float Alpha)
{
	FVector ForwardLook = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::FindLookAtRotation(A, B));
	FVector RelativeOrigin = FMath::Lerp(A, B, 0.5f);
	RelativeOrigin = RelativeOrigin + (ForwardLook * (UKismetMathLibrary::Vector_Distance(A, B) / 2));

	float Angle = GetAngleBetween(A-RelativeOrigin, B-RelativeOrigin);
	float MA = sin((1 - Alpha) * Angle) / sin(Angle);
	float MB = sin(Alpha * Angle) / sin(Angle);

	FVector R = (((A - RelativeOrigin) * MA) + ((B - RelativeOrigin) * MB))+RelativeOrigin;
	return R;
}

// Spherical Linear Interpolation To (Vector)
FVector UHelpfulFunctionsBPLibrary::SlerpToFast(FVector Current, FVector Target, float DeltaTime, float InterpSpeed)
{
	return SlerpFast(Current,Target,DeltaTime*InterpSpeed);
}

// Bounce Out Curve
float UHelpfulFunctionsBPLibrary::BounceOutTimeFunction(float time)
{
	const float n1 = 7.5625;
	const float d1 = 2.75;
	float x = time;
	if (x < 1.0 / d1) {
		return n1 * x * x;
	}
	else if (x < 2.0 / d1) {
		x -= 1.5 / d1;
		return n1 * x * x + 0.75;
	}
	else if (x < 2.5 / d1) {
		x -= 2.25 / d1;
		return n1 * x * x + 0.9375;
	}
	else {
		x -= 2.625 / d1;
		return n1 * x * x + 0.984375;
	}

}
// "'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"
// "'"'"'"'"'"'"'""'"'"'"'"'"'"'" T R A C E "'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'"
// "'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"'"'""'"'"'"'"'"

/** Static utility function to make a capsule */
static FCollisionShape MakeCapsuleRotated(const FVector& Extent)
{
	FCollisionShape CapsuleShape;
	CapsuleShape.SetCapsule(FVector3f(Extent));
	return CapsuleShape;
}

bool UHelpfulFunctionsBPLibrary::RotatedCapsuleTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, ETraceTypeQuery TraceChannel, 
	bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (DebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

	static const FName CapsuleTraceSingleName(TEXT("CapsuleTraceSingle"));
	FCollisionQueryParams Params;
	Params.TraceTag = FName(CapsuleTraceSingleName);
	Params.bTraceComplex = bTraceComplex;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bReturnPhysicalMaterial = true;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	bool const bHit = World ? World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, CollisionChannel, MakeCapsuleRotated(FVector(Radius, Radius, HalfHeight)), Params) : false;

//#if ENABLE_DRAW_DEBUG
//	DrawDebugCapsuleTraceSingle(World, Start, End, Radius, HalfHeight, TraceType, bHit, OutHit, TraceColor, TraceHitColor, DrawTime);
//#endif

	return bHit;
}
// Find Gap By Line Trace
bool UHelpfulFunctionsBPLibrary::WalkableLineTrace(const UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, 
	const TArray<AActor*>& ActorsToIgnore, int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime, int MaxIteracion)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }

	FVector NextTraceStart = Start;
	FHitResult LineHitResult;
	bool LineHitValid;
	LineHitValid = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Start, End, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
	if (LineHitValid == false)
	{ OutHit = LineHitResult; return false; }

	if (UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.ImpactPoint, LineHitResult.TraceStart, 0.08) == true) 
	{
		NextTraceStart = UKismetMathLibrary::VLerp(Start, End, 0.5);
		LineHitValid = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, NextTraceStart, End, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
		if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.ImpactPoint, LineHitResult.TraceStart, 0.08) == false)
		{ OutHit = LineHitResult; return LineHitValid; }

		if (UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.ImpactPoint, LineHitResult.TraceStart, 0.08) == true)
		{
			for (int i = 0; i < MaxIteracion; i++)
			{
				NextTraceStart = UKismetMathLibrary::VLerp(UKismetMathLibrary::VLerp(Start, End, 0.5), End, UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, MaxIteracion * 1.0, 0.1, 0.9));
				LineHitValid = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, NextTraceStart, End, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
				if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.ImpactPoint, LineHitResult.TraceStart, 0.08) == false)
				{ OutHit = LineHitResult; return LineHitValid; }
			}
		}
		for (int i = 0; i < MaxIteracion; i++)
		{
			NextTraceStart = UKismetMathLibrary::VLerp(Start, UKismetMathLibrary::VLerp(Start, End, 0.5), UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, MaxIteracion * 1.0, 0.1, 0.9));
			LineHitValid = UKismetSystemLibrary::LineTraceSingle(WorldContextObject, NextTraceStart, End, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
			if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.ImpactPoint, LineHitResult.TraceStart, 0.08) == false)
			{ OutHit = LineHitResult; return LineHitValid; }
		}
	}
	OutHit = LineHitResult;
	return LineHitValid;
}

bool UHelpfulFunctionsBPLibrary::FindGapBySphereTrace(const UObject* WorldContextObject, const FVector Start, const FVector End, float TraceRadius, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime, int MaxIteracion)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (DebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}

	FVector NextTraceStart = Start;
	FHitResult LineHitResult;
	bool LineHitValid;
	LineHitValid = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, Start, End, TraceRadius, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, TraceHitColor, DrawTime);
	if (LineHitValid == false)
	{
		OutHit = LineHitResult; return false;
	}

	if (UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.Location, LineHitResult.TraceStart, 0.08) == true)
	{
		NextTraceStart = UKismetMathLibrary::VLerp(Start, End, 0.5);
		LineHitValid = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, NextTraceStart, End, TraceRadius, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
		if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.Location, LineHitResult.TraceStart, 0.08) == false)
		{
			OutHit = LineHitResult; return LineHitValid;
		}

		if (UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.Location, LineHitResult.TraceStart, 0.08) == true)
		{
			for (int i = 0; i < MaxIteracion; i++)
			{
				NextTraceStart = UKismetMathLibrary::VLerp(UKismetMathLibrary::VLerp(Start, End, 0.5), End, UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, MaxIteracion * 1.0, 0.1, 0.9));
				LineHitValid = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, NextTraceStart, End, TraceRadius, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
				if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.Location, LineHitResult.TraceStart, 0.08) == false)
				{
					OutHit = LineHitResult; return LineHitValid;
				}
			}
		}
		for (int i = 0; i < MaxIteracion; i++)
		{
			NextTraceStart = UKismetMathLibrary::VLerp(Start, UKismetMathLibrary::VLerp(Start, End, 0.5), UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, MaxIteracion * 1.0, 0.1, 0.9));
			LineHitValid = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, NextTraceStart, End, TraceRadius, TraceChannel, bTraceComplex, ActorsToIgnore, TraceType, LineHitResult, bIgnoreSelf, TraceColor, FLinearColor::Yellow, DrawTime);
			if (LineHitValid == true && UKismetMathLibrary::EqualEqual_VectorVector(LineHitResult.Location, LineHitResult.TraceStart, 0.08) == false)
			{
				OutHit = LineHitResult; return LineHitValid;
			}
		}
	}
	OutHit = LineHitResult;
	return LineHitValid;
}

void UHelpfulFunctionsBPLibrary::FindNearPointToWallSurface(const UObject* WorldContextObject, const ACharacter* Character, bool& ValidPoint, FHitResult& OutHitResult, FVector ForwardDirection, 
	int Accuracy, bool UseNormalFromFirstTrace, float ConstForwardOffset, FVector RightOffset, int DebugIndex)
{
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> HitArray;
	TArray<float> DistancesArray;
	float AverageDistances=0;
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	// Check the Character Is Valid
	if (!IsValid(Character))
	{ return;}
	Character->GetAllChildActors(ActorsToIgnore, true);
	float CapsuleHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float VelocityRange = UKismetMathLibrary::MapRangeClamped(Character->GetVelocity().Length(), 0, 500, 0, 50);
	FVector TStart = Character->GetActorLocation() + RightOffset;
	FVector TEnd = TStart + ForwardDirection * (VelocityRange + Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + 20);
	FHitResult SphereHitResult;
	const bool SphereHitValid = UKismetSystemLibrary::SphereTraceSingle(Character, TStart, TEnd, 12, Channel, false, ActorsToIgnore, TraceType, SphereHitResult, true, FLinearColor(0.16, 0, 0.01, 1), FLinearColor::Red, 0.2);
	if (SphereHitValid == false)
	{ ValidPoint = false; return; }

	float MapIndex;
	FVector NextTraceDirection = UKismetMathLibrary::SelectVector(NormalToVector(SphereHitResult.Normal), ForwardDirection, UseNormalFromFirstTrace);
	NextTraceDirection = NextTraceDirection * (Character->GetCapsuleComponent()->GetScaledCapsuleRadius() + ConstForwardOffset + UKismetMathLibrary::MapRangeClamped(Character->GetVelocity().Length(), 0, 500, 0, 30));
	for (int i = 0; i < Accuracy; i++)
	{
		MapIndex = UKismetMathLibrary::MapRangeClamped(i * 1.0, 0, Accuracy * 1.0, CapsuleHeight * 2 - 10, 10);
		TStart = Character->GetActorLocation() - FVector(0, 0, CapsuleHeight) + FVector(0, 0, MapIndex) + RightOffset;
		TEnd = TStart + NextTraceDirection * 1;
		TStart = TStart + NextTraceDirection * 0.1;
		FHitResult Trace2Hit;
		const bool Trace2Valid = UKismetSystemLibrary::SphereTraceSingle(Character, TStart, TEnd, 8, Channel, false, ActorsToIgnore, TraceType, Trace2Hit, true, FLinearColor::Blue, FLinearColor::Green, 0.1);
		if (!(!Trace2Valid == true && i == 0) == true)
		{
			if (Character->GetCharacterMovement()->IsWalkable(Trace2Hit) == true)
			{ ValidPoint = false; return; }

			HitArray.Add(Trace2Hit);
			DistancesArray.Add(UKismetMathLibrary::Vector_Distance2D(Trace2Hit.ImpactPoint, Character->GetActorLocation()));
			AverageDistances = AverageDistances + UKismetMathLibrary::Vector_Distance2D(Trace2Hit.ImpactPoint, Character->GetActorLocation());
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, Trace2Hit.ImpactPoint.ToString());
		}
		else
		{ ValidPoint = false; return; }
	}
	FHitResult MinHitPoint;
	float MinFromDistance=0.0;
	int MinFromDistanceIndex=0;
	bool ReturnCondition=false;

	if (sizeof(DistancesArray) == 0)
	{ ValidPoint = false; return; }
	UKismetMathLibrary::MinOfFloatArray(DistancesArray, MinFromDistanceIndex, MinFromDistance);
	MinHitPoint = HitArray[MinFromDistanceIndex];
	ReturnCondition = MinHitPoint.ImpactPoint.Z - (Character->GetActorLocation().Z - CapsuleHeight) < Character->GetCharacterMovement()->MaxStepHeight;
	ReturnCondition = ReturnCondition && !(UKismetMathLibrary::NearlyEqual_FloatFloat(MinFromDistance, AverageDistances / sizeof(DistancesArray)));
	if (ReturnCondition == false)
	{
		ValidPoint = true;
		OutHitResult = MinHitPoint;
		return;
	}
	ValidPoint = false;
	return;
}
// ARRAY
void UHelpfulFunctionsBPLibrary::GetNearFromActorsArray(const UObject* WorldContextObject, TArray<AActor*> ActorsArray, FVector Origin, bool& ReturnValid, AActor*& OutActor)
{
	TArray<float> DistancesArray;
	if (ActorsArray.Num() == 0)
	{
		ReturnValid = false;
		OutActor = nullptr;
		return;
	}
	else if (ActorsArray.Num() == 1)
	{
		OutActor = ActorsArray[0];
		ReturnValid = IsValid(OutActor);
		return;
	}
	else
	{
		int MinIndex = 0;
		float MinValue = 0;
		for (AActor* DetectedActor : ActorsArray)
		{
			DistancesArray.Add(UKismetMathLibrary::Vector_Distance(DetectedActor->GetActorLocation(), Origin));
		}
		UKismetMathLibrary::MinOfFloatArray(DistancesArray, MinIndex, MinValue);
		OutActor = ActorsArray[MinIndex];
		ReturnValid = IsValid(OutActor);
		return;
	}
}
//ARRAY
void UHelpfulFunctionsBPLibrary::ClassToActorsArray(const UObject* WorldContextObject, TArray<UClass*> ActorsClass, TArray<AActor*> SingleActorToIgnore, TArray<AActor*>& ReturnArray)
{
	ReturnArray.SetNum(0);
	TArray<AActor*> ActorsOfClass;
	TArray<AActor*> ActorsToIgnore;
	for (UClass* ArrElement : ActorsClass)
	{
		UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ArrElement, ActorsOfClass);
		ActorsToIgnore.Append(ActorsOfClass);
	}
	ActorsToIgnore.Append(SingleActorToIgnore);
	ReturnArray = ActorsToIgnore;
	return;
}
// NAVIGATION
FVector UHelpfulFunctionsBPLibrary::FixMovePointWhenIs(UObject* WorldContextObject, FVector TargetPoint, ACharacter* TargetCharacter, UClass* CharactersArray,
	float OffsetRadius, int MaxIteractions, float MinAccetableDistance)
{
	if (IsValid(TargetCharacter) == false)
	{ return TargetPoint; }
	FVector NormalizedPoint = TargetPoint;
	FNavLocation OutLocation; // Zmiana poniewaz jakis problem byl w wersji silnika 5.1
	FNavLocation RandomNavLocation;

	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(TargetCharacter->GetWorld()); //WAZNE!!!!!!!!!!!!!
	// Nie mozna uzyskac GeWorld() z pliku wtyczki. Za to mozna uzyc opcji GEditor->GetEditorWorldContext().World()

	TArray<AActor*> ActorsOfClass;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, CharactersArray, ActorsOfClass);

	//NormalizedPoint = UNavigationSystemV1::ProjectPointToNavigation(WorldContextObject, TargetPoint, (ANavigationData*)0, nullptr, FVector(0, 0, 0));
	bool ProjectToValid = NavSystem->ProjectPointToNavigation(TargetPoint, OutLocation, FVector(0, 0, 0)); //Project Point To navigation
	if (ProjectToValid == true)
	{ NormalizedPoint = OutLocation.Location; }
	
	//Basic Trace Config
	TArray<AActor*> ActorsToIgnore; //Trace Parameter
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility); //Trace Parameter
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None; //Trace Parameter
	ActorsToIgnore.Emplace(TargetCharacter);

	// Trace Config - Start And End Location 
	const FVector TStartEnd = OutLocation.Location + FVector(0, 0, TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.1);
	const float THeight = TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.9;

	FHitResult CapsuleHitResult; //Run Capsule Trace
	const bool HitValid = UKismetSystemLibrary::CapsuleTraceSingle(WorldContextObject, TStartEnd, TStartEnd, 20.0, THeight, Channel, false, ActorsToIgnore, TraceType, CapsuleHitResult, true);
	if (HitValid == true)
	{
		for (int i = 0; i < MaxIteractions; i++)
		{
			//NormalizedPoint = UNavigationSystemV1::GetRandomReachablePointInRadius(WorldContextObject, OutLocation.Location, OffsetRadius, (ANavigationData*)0, nullptr);
			NavSystem->GetRandomReachablePointInRadius(OutLocation.Location, OffsetRadius, RandomNavLocation);
			NormalizedPoint = RandomNavLocation.Location;
			if (UKismetMathLibrary::Vector_Distance2D(NormalizedPoint, TargetPoint) > MinAccetableDistance)
			{ return NormalizedPoint; }
		}
		return NormalizedPoint;
	}
	else
	{ return NormalizedPoint; }
}

float UHelpfulFunctionsBPLibrary::RayCastSphereRoomCheckFast(UObject* WorldContextObject, FTransform Origin, float InRadius)
{
	const int LastIndex = 4;
	FVector Direction = FVector(1, 0, 0);
	float InverseValue = 1;
	FVector HitLocation = FVector(0, 0, 0);
	TArray<float> DistancesArray;
	float DistancesSum = 0;
	for (int i = 1; i <= LastIndex; i++)
	{
		if (i > LastIndex / 2)
		{ Direction = UKismetMathLibrary::GetRightVector(Origin.Rotator());}
		else
		{ Direction = UKismetMathLibrary::GetForwardVector(Origin.Rotator());}
		Direction = Direction * InRadius;
		if (i % 2 == 0)
		{ InverseValue = -1; }
		else
		{ InverseValue = 1; }
		UNavigationSystemV1::NavigationRaycast(WorldContextObject, Origin.GetLocation(), Origin.GetLocation() + (Direction * InverseValue), HitLocation);
		DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance2D(HitLocation, Origin.GetLocation()));
		DistancesSum = DistancesSum + UKismetMathLibrary::FClamp(UKismetMathLibrary::Vector_Distance2D(HitLocation, Origin.GetLocation()), 1, InRadius);
	}
	return UKismetMathLibrary::SafeDivide(DistancesSum, DistancesArray.Num() * 1.0);
}

float UHelpfulFunctionsBPLibrary::RayCastSphereComplexRoomCheckFast(UObject* WorldContextObject, FTransform Origin, float InRadius)
{
	const int LastIndex = 17;
	FVector Direction = FVector(1, 0, 0);
	FVector HitLocation = FVector(0, 0, 0);
	//TSubclassOf<UNavigationQueryFilter> FilterClass = 0;
	TArray<float> DistancesArray;
	float DistancesSum = 0;
	for (int i = 1; i <= LastIndex; i++)
	{
		Direction = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(FRotator(0, i * 21.0, 0)), UKismetMathLibrary::GetForwardVector(Origin.Rotator()));

		UNavigationSystemV1::NavigationRaycast(WorldContextObject, Origin.GetLocation(), Origin.GetLocation() + (Direction * InRadius), HitLocation);
		DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance2D(HitLocation, Origin.GetLocation()));
		DistancesSum = DistancesSum + UKismetMathLibrary::FClamp(UKismetMathLibrary::Vector_Distance2D(HitLocation, Origin.GetLocation()), 1, InRadius);
	}
	return UKismetMathLibrary::SafeDivide(DistancesSum, DistancesArray.Num() * 1.0);
}

bool UHelpfulFunctionsBPLibrary::IsNotHidingInFoliage(UObject* WorldContextObject, ACharacter* OwnerCharacter, ACharacter* Character, ECollisionChannel TraceChannel, int DebugIndex)
{
	if (IsValid(Character) == false)
	{ return true; }
	TArray<AActor*> ActorsToIgnore;
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(TraceChannel);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	FHitResult CapsuleHitResult;
	const bool CapsuleHitValid = UKismetSystemLibrary::CapsuleTraceSingle(Character, Character->GetActorLocation(), Character->GetActorLocation(), Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.2,
		Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), Channel, false, ActorsToIgnore, TraceType, CapsuleHitResult, true, FLinearColor::Red, FLinearColor::Green, 0.1);
	if (CapsuleHitValid == false)
	{ return true; }
	FHitResult LineHitResult;
	ActorsToIgnore.Emplace(Character);
	ActorsToIgnore.Emplace(OwnerCharacter);
	const bool LineHitValid = UKismetSystemLibrary::LineTraceSingle(Character, Character->GetActorLocation() + FVector(0, 0, 60), Character->GetActorLocation() + FVector(0, 0, -90), 
							  Channel, false, ActorsToIgnore, TraceType, LineHitResult, true, FLinearColor::Red, FLinearColor::Green, 0.1);
	if (LineHitValid == false)
	{ return true; }
	if (LineHitResult.ImpactPoint.Z - (Character->GetMesh()->GetSocketLocation(FName(TEXT("head"))).Z-8) > 0.8)
	{ return false; }
	else
	{ return true; }
	return false;
}

void UHelpfulFunctionsBPLibrary::FindBaseJumpAwayPosition(UObject* WorldContextObject, bool& ReturnValid, FVector& ReturnLocation, ACharacter* Character, FVector GrenadePosition, float MaxRadius, FVector2D Arc)
{
	FVector Direction = UKismetMathLibrary::GetForwardVector(FRotator(0, UKismetMathLibrary::FindLookAtRotation(GrenadePosition, Character->GetActorLocation()).Yaw, 0));
	FVector TraceDirection = FVector(0, 0, 0);
	FVector TStart;
	FVector TEnd;
	FVector HitLocation;
	TArray<FVector> Points;
	TArray<float> Distances;
	TSubclassOf<UNavigationQueryFilter> FilterClass = 0;
	int MaxIndex = 0;
	float MaxValue = 0;
	for (int i = 0; i <= 6; i++)
	{
		TraceDirection = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(FRotator(0, UKismetMathLibrary::MapRangeClamped(i * 1.0, 0, 6, Arc.X, Arc.Y), 0)), Direction);
		TStart = Character->GetActorLocation() - FVector(0, 0, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) + FVector(0, 0, 15);
		TEnd = TStart + (TraceDirection * MaxRadius);
		UNavigationSystemV1::NavigationRaycast(WorldContextObject, TStart, TEnd, HitLocation, FilterClass, nullptr);
		Points.Emplace(HitLocation);
		Distances.Emplace(UKismetMathLibrary::Vector_Distance(GrenadePosition, HitLocation) + UKismetMathLibrary::Vector_Distance2D(Character->GetActorLocation(), HitLocation));
	}
	if (Points.Num() > 0)
	{
		UKismetMathLibrary::MaxOfFloatArray(Distances, MaxIndex, MaxValue);
		ReturnLocation = Points[MaxIndex];
		ReturnValid = true;
		return;
	}
	ReturnValid = false;
	ReturnLocation = GrenadePosition;
	return;
}

void UHelpfulFunctionsBPLibrary::FFT(TArray<float> Data, int32 NumSamples)
{
	const float PII = 3.141592653589;
	// Check input data
	check(Data.Num() > 0);
	check(NumSamples > 0);

	// Check if input data is already a power of 2
	int32 PowerOfTwo = FMath::RoundToInt(FMath::Log2(float(NumSamples)));
	if(1 << PowerOfTwo != NumSamples)
	{
		// Resize data to power of 2
		NumSamples = 1 << PowerOfTwo;
		Data.SetNum(NumSamples);
		for (int32 i = NumSamples / 2; i < NumSamples; ++i)
		{
			Data[i] = 0;
		}
	}

	// Perform FFT
	int32 NumBits = FMath::RoundToInt(FMath::Log2(float(NumSamples)));
	int32 HalfSize = NumSamples / 2;
	int32 Target = 0;
	for(int32 i = 0; i < NumSamples; ++i)
	{
		if (i < Target)
		{
			std::swap(Data[i], Data[Target]);
		}

		int32 Mask = HalfSize;
		while (Target & Mask)
		{
			Target &= ~Mask;
			Mask >>= 1;
		}

		Target |= Mask;
	}

	// Process FFT
	for(int32 Step = 1; Step < NumSamples; Step <<= 1)
	{
		float WnReal = FMath::Cos(PII / Step);
		float WnImag = FMath::Sin(PII / Step);
		for (int32 Start = 0; Start < NumSamples; Start += Step * 2)
		{
			float WReal = 1;
			float WImag = 0;
			for (int32 j = 0; j < Step; ++j)
			{
				if (Data.IsValidIndex(Start + j + Step) == true && Data.IsValidIndex(Start + j + Step + 1) == true)
				{
					float TReal = WReal * Data[Start + j + Step] - WImag * Data[Start + j + Step + 1];
					float TImag = WReal * Data[Start + j + Step + 1] + WImag * Data[Start + j + Step];
					Data[Start + j + Step] = Data[Start + j] - TReal;
					Data[Start + j + Step + 1] = Data[Start + j + 1] - TImag;
					Data[Start + j] += TReal;
					Data[Start + j + 1] += TImag;
					float TWReal = WReal;
					WReal = TWReal * WnReal - WImag * WnImag;
					WImag = TWReal * WnImag + WImag * WnReal;
				}

			}
		}
	}

	// Normalize FFT
	for (int32 i = 0; i < NumSamples; ++i)
	{
		Data[i] /= NumSamples;
	}
}
// Funckcja Autokorelacji - Teoria Sygnalow
TArray<float> UHelpfulFunctionsBPLibrary::Autocorrelation(const TArray<FVector2D>& data)
{
	int32 n = data.Num();
	TArray<float> acf;
	acf.Init(0.0f, n);
	if (n == 0)
	{ return acf; }

	float mean = 0;
	for (int32 i = 0; i < n; ++i) {
		mean += data[i].Y;
	}
	mean /= n;

	for (int32 i = 0; i < n; ++i) {
		float numerator = 0;
		float denominator1 = 0;
		float denominator2 = 0;

		for (int32 j = 0; j < n - i; ++j) {
			numerator += (data[j].Y - mean) * (data[j + i].Y - mean);
			denominator1 += FMath::Pow((data[j].Y - mean), 2);
			denominator2 += FMath::Pow((data[j + i].Y - mean), 2);
		}
		acf[i] = numerator / (denominator1 * denominator2);
	}
	return acf;
}

void UHelpfulFunctionsBPLibrary::ExecuteInterfaceInput(UObject* WorldContextObject, UActorComponent* OptionalComponent, bool CallToAll, bool Hold, CALS_IntputType InputIndex)
{
	ACharacter* OverlapedChar = Cast<ACharacter>(WorldContextObject);
	IALS_BaseComponentsInterfaceCpp* CompInterface = nullptr;
	if (IsValid(WorldContextObject) == true && IsValid(OverlapedChar) == true)
	{
		if (CallToAll == true)
		{
			TArray<UActorComponent*> CompWithInterface;
			TSubclassOf<UALS_BaseComponentsInterfaceCpp> ALS_BPInterface = UALS_BaseComponentsInterfaceCpp::StaticClass();
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FVector(0,10,10).ToString());
			CompWithInterface = OverlapedChar->GetComponentsByInterface(ALS_BPInterface);
			for (UActorComponent* ArrElement : CompWithInterface)
			{
				CompInterface = Cast<IALS_BaseComponentsInterfaceCpp>(ArrElement); //Cast To Interface
				switch (InputIndex) {
				case CALS_IntputType::JumpAction:
					CompInterface->Execute_BPICpp_InputJumpAction(ArrElement, Hold, false);
					break;
				case CALS_IntputType::WalkAction:
					CompInterface->Execute_BPICpp_InputWalkAction(ArrElement, Hold, false);
					break;
				case CALS_IntputType::SprintAction:
					CompInterface->Execute_BPICpp_InputSprintAction(ArrElement, Hold, false);
					break;
				case CALS_IntputType::AimAction:
					CompInterface->Execute_BPICpp_InputAimAction(ArrElement, Hold, false);
					break;
				case CALS_IntputType::ShotAction:
					CompInterface->Execute_BPICpp_InputShotAction(ArrElement, Hold, false);
					break;
				case CALS_IntputType::StanceAction:
					CompInterface->Execute_BPICpp_InputStanceAction(ArrElement, Hold, false);
					break;
				default:
					CompInterface->Execute_BPICpp_InputJumpAction(ArrElement, Hold, false);
					break;
				}
			}
			return;
		}
		else
		{
			if (IsValid(OptionalComponent) == true)
			{
				CompInterface = Cast<IALS_BaseComponentsInterfaceCpp>(OptionalComponent); //Cast To Interface
				if (OptionalComponent->GetClass()->ImplementsInterface(UALS_BaseComponentsInterfaceCpp::StaticClass()) == false)
				{
					return;
				}
				switch (InputIndex) {
				case CALS_IntputType::JumpAction:
					CompInterface->Execute_BPICpp_InputJumpAction(OptionalComponent, Hold, false); return;
					break;
				case CALS_IntputType::WalkAction:
					CompInterface->Execute_BPICpp_InputWalkAction(OptionalComponent, Hold, false); return;
					break;
				case CALS_IntputType::SprintAction:
					CompInterface->Execute_BPICpp_InputSprintAction(OptionalComponent, Hold, false); return;
					break;
				case CALS_IntputType::AimAction:
					CompInterface->Execute_BPICpp_InputAimAction(OptionalComponent, Hold, false); return;
					break;
				case CALS_IntputType::ShotAction:
					CompInterface->Execute_BPICpp_InputShotAction(OptionalComponent, Hold, false); return;
					break;
				case CALS_IntputType::StanceAction:
					CompInterface->Execute_BPICpp_InputStanceAction(OptionalComponent, Hold, false); return;
					break;
				default:
					CompInterface->Execute_BPICpp_InputJumpAction(OptionalComponent, Hold, false); return;
					break;
				}
			}
		}
	}
	else
	{ return; }
}

//CONVERT WORLD TRANSFORM TO LOCAL (MATRIX) [WITH VALIDATION CHECK]
FCALS_ComponentAndTransform UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(FCALS_ComponentAndTransform World)
{
	if (IsValid(World.Component) == false)
	{ return World; }
	FCALS_ComponentAndTransform Local = {};
	Local.Transform = UKismetMathLibrary::Conv_MatrixToTransform(UKismetMathLibrary::Conv_TransformToMatrix(World.Transform) * 
	UKismetMathLibrary::Matrix_GetInverse(UKismetMathLibrary::Matrix_GetMatrixWithoutScale(UKismetMathLibrary::Conv_TransformToMatrix(World.Component->GetComponentTransform()))));
	Local.Component = World.Component;
	return Local;
}

//CONVERT LOCAL TRANSFORM TO WORLD (MATRIX) [WITH VALIDATION CHECK]
FCALS_ComponentAndTransform UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(FCALS_ComponentAndTransform Local)
{
	if (IsValid(Local.Component) == false)
	{ return Local; }
	FCALS_ComponentAndTransform ReturnWorld = {};
	ReturnWorld.Transform = UKismetMathLibrary::Conv_MatrixToTransform(UKismetMathLibrary::Conv_TransformToMatrix(Local.Transform) *
	UKismetMathLibrary::Matrix_GetMatrixWithoutScale(UKismetMathLibrary::Conv_TransformToMatrix(Local.Component->GetComponentTransform())));
	ReturnWorld.Component = Local.Component;
	return ReturnWorld;
}

float UHelpfulFunctionsBPLibrary::GetSafeFloatValue(UCurveFloat* FloatCurve, float Time)
{
	if (FloatCurve)
	{
		return FloatCurve->FloatCurve.Eval(Time);
	}
	return 0.0f;
}

bool UHelpfulFunctionsBPLibrary::AnimSlotIsActivated(UObject* WorldContextObject, FName SlotName)
{
	UAnimInstance* AnimInst = Cast<UAnimInstance>(WorldContextObject);
	if (AnimInst)
	{
		float GlobalWeight = AnimInst->GetSlotMontageGlobalWeight(SlotName);
		if (GlobalWeight > 0)
		{
			return true;
		}
		return false;
	}
	return false;
}

bool UHelpfulFunctionsBPLibrary::AnimSlotIsActivatedByTaget(UObject* WorldContextObject, UAnimInstance* Target, FName SlotName)
{
	if (Target)
	{
		float GlobalWeight = Target->GetSlotMontageGlobalWeight(SlotName);
		if (GlobalWeight > 0)
		{
			return true;
		}
		return false;
	}
	return false;
}

// ***************************************************************** GET ENEMY STATE BY USING ACTOR TAGS *****************************************************************
bool UHelpfulFunctionsBPLibrary::GetIsEnemyState(UObject* WorldContextObject, AActor* Checker, AActor* ActorToCheck)
{
	AActor* TargetChar = ActorToCheck;
	if (IsValid(TargetChar) == false)
	{
		TargetChar = Cast<AActor>(WorldContextObject);
	}
	if (IsValid(TargetChar) == false || IsValid(Checker) == false) return false;
	
	FString ClassName = UKismetSystemLibrary::GetClassDisplayName(TargetChar->GetClass());
	ClassName = ClassName.Left(ClassName.Len() - 2);
	ClassName = "EN_" + ClassName;
	FName ClassNameFName(*ClassName);
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, ClassName);
	for (const FName Tag : Checker->Tags)
	{
		if (Tag == ClassNameFName)
		{
			return true;
		}
	}
	return false;
}

bool UHelpfulFunctionsBPLibrary::FindNearestCollisionByCylinder(FVector& OutCircleCenter, float& OutCircleRadius, FVector& OutNormal, UObject* WorldContextObject, FTransform OriginTransform, TArray<AActor*> ToIgnore,
	float MaxRadius, float Height, int Precision, ECollisionChannel TraceChannel, bool DrawDebug)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(TraceChannel);
	FVector TEnd = FVector(0, 0, 0);
	FRotator RotateDirection = FRotator(0, 0, 0);

	const FVector ForwardD = UKismetMathLibrary::GetForwardVector(OriginTransform.Rotator());

	FVector OriginOffsetSum = FVector(0, 0, 0);
	FVector NormalsSum = FVector(0, 0, 0);
	TArray<float> RadiusOffset = {};
	int HitCount = 0;
	bool CheckRadius = false;

	for (int i = 0; i < Precision; i++)
	{
		RotateDirection = FRotator(0,UKismetMathLibrary::MapRangeClamped(i * 1.0, 0.0, Precision * 1.0, 0.0, 360), 0);
		TEnd = OriginTransform.GetLocation() + (UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(RotateDirection), 
			UKismetMathLibrary::GetForwardVector(OriginTransform.Rotator())) * MaxRadius);

		FHitResult FirstHit = {};
		const bool HitValid = UKismetSystemLibrary::BoxTraceSingle(WorldContextObject, OriginTransform.GetLocation(), TEnd, FVector(1, MaxRadius / (Precision * 1.0) * PI, Height), 
			RotateDirection, Channel, false, ToIgnore, EDrawDebugTrace::None, FirstHit, true);
		if (HitValid == true)
		{
			OriginOffsetSum = OriginOffsetSum + FirstHit.Normal * UKismetMathLibrary::Vector_Distance2D(FirstHit.Location, FirstHit.TraceEnd);
			HitCount++;
			CheckRadius = true;
			NormalsSum = NormalsSum + FirstHit.Normal;
		}
		if (FirstHit.Normal == FVector(0, 0, 0)) NormalsSum = NormalsSum + UKismetMathLibrary::GetForwardVector(OriginTransform.Rotator()) * -1.0;
	}
	FVector NewOrigin = OriginTransform.GetLocation() + (OriginOffsetSum/ HitCount);
	HitCount = 0;

	if (CheckRadius == false)
	{
		OutCircleCenter = OriginTransform.GetLocation();
		OutCircleRadius = MaxRadius;
		OutNormal = UKismetMathLibrary::GetForwardVector(OriginTransform.Rotator()) * -1.0;
		return false;
	}

	for (int ii = 0; ii < Precision; ii++)
	{
		RotateDirection = FRotator(0, UKismetMathLibrary::MapRangeClamped(ii * 1.0, 0.0, Precision * 1.0, 0.0, 360), 0);
		TEnd = NewOrigin + (UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(RotateDirection),
			UKismetMathLibrary::GetForwardVector(OriginTransform.Rotator())) * MaxRadius);

		FHitResult SecondHit = {};
		const bool HitValid = UKismetSystemLibrary::BoxTraceSingle(WorldContextObject, NewOrigin, TEnd, FVector(1, MaxRadius / (Precision * 1.0) * PI, Height),
			RotateDirection, Channel, false, ToIgnore, EDrawDebugTrace::None, SecondHit, true);
		if (HitValid == true)
		{
			RadiusOffset.Add(UKismetMathLibrary::Vector_Distance2D(SecondHit.Location, SecondHit.TraceEnd));
			HitCount++;
		}
		
	}
	OutCircleCenter = NewOrigin;
	float MaxValue = 0.0;
	UKismetMathLibrary::MaxOfFloatArray(RadiusOffset, HitCount, MaxValue);
	OutCircleRadius = MaxRadius - MaxValue;
	OutNormal = NormalsSum / Precision;
	
	if(DrawDebug == true) DrawDebugCylinder(World, NewOrigin - FVector(0, 0, Height * 0.5), NewOrigin + FVector(0, 0, Height * 0.5), OutCircleRadius, Precision, FColor::Cyan, false, -1.0);

	return true;
}

float UHelpfulFunctionsBPLibrary::GetCurveValueAtTime(UObject* WorldContextObject, UAnimSequence* AnimSequence, FName CurveName, float Time)
{
	if (!AnimSequence)
	{
		return 0.0f;
	}
	return AnimSequence->EvaluateCurveData(CurveName, Time, false);
}

FVector UHelpfulFunctionsBPLibrary::GetRootMotionValueAtTime(UObject* WorldContextObject, UAnimSequence* AnimSequence, float Time)
{
	if (!AnimSequence) return FVector(0, 0, 0);
	
	float X, Y, Z = 0.0;
	X = GetCurveValueAtTime(WorldContextObject, AnimSequence, TEXT("Extract_Root-Loc_X"), Time);
	Y = GetCurveValueAtTime(WorldContextObject, AnimSequence, TEXT("Extract_Root-Loc_Y"), Time);
	Z = GetCurveValueAtTime(WorldContextObject, AnimSequence, TEXT("Extract_Root-Loc_Z"), Time);
	return FVector(X, Y, Z);
}

FVector UHelpfulFunctionsBPLibrary::SplineLerpOnPathPoints(const FVector& Start, const TArray<FVector>& PathPoints, float Alpha)
{
	if (PathPoints.Num() < 2)
	{
		return FVector::ZeroVector;
	}

	// Create a temporary spline component for interpolation.
	USplineComponent* Spline = NewObject<USplineComponent>();
	Spline->SetClosedLoop(false);
	Spline->ClearSplinePoints();

	// Add points to the spline.
	for (const FVector& Point : PathPoints)
	{
		Spline->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
	}

	// Update spline to ensure correct point placement.
	Spline->UpdateSpline();

	// Clamp alpha to be between 0 and 1.
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	// Get the length of the spline.
	float TotalLength = Spline->GetSplineLength();

	// Get the position at the specified alpha along the spline.
	FVector Result = Spline->GetLocationAtDistanceAlongSpline(Alpha * TotalLength, ESplineCoordinateSpace::World);

	// Clean up the temporary spline component.
	Spline->DestroyComponent();

	return Result;
}



FVector UHelpfulFunctionsBPLibrary::VInterpToWithDelay(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
	// Zabezpieczenie przed dzieleniem przez zero i nieprawid³owymi wartoœciami
	if (DeltaTime == 0.f || InterpSpeed <= 0.f)
	{
		return Current;
	}

	// Obliczanie si³y interpolacji dla ka¿dej sk³adowej niezale¿nie
	auto InterpSingle = [DeltaTime, InterpSpeed](float CurrentSingle, float TargetSingle) -> float {
		const float Distance = TargetSingle - CurrentSingle;

		// Sprawdzanie, czy jesteœmy wystarczaj¹co blisko celu
		if (FMath::IsNearlyZero(Distance))
		{
			return TargetSingle;
		}

		// 'Alpha' kontroluje kszta³t krzywej interpolacji
		const float Alpha = FMath::Clamp(DeltaTime * InterpSpeed, 0.f, 1.f);

		// Implementacja funkcji sigmoidalnej
		const float ScaledAlpha = 1.f / FMath::Clamp<float>((1.f + FMath::Exp(-Alpha * 12.f + 6.f)),0.01,100.f); // Zakres [-6, 6] dla funkcji sigmoidalnej

		return CurrentSingle + Distance * ScaledAlpha;
		};

	// Interpolacja dla ka¿dej osi niezale¿nie
	return FVector(
		InterpSingle(Current.X, Target.X),
		InterpSingle(Current.Y, Target.Y),
		InterpSingle(Current.Z, Target.Z)
	);
}


FTransform UHelpfulFunctionsBPLibrary::GetBonePositionAtTimeFromSeq(const UAnimSequenceBase* AnimationSequenceBase, double Time, FName BoneName, FAnimPoseEvaluationConfig EvaluationOptions)
{
	if (AnimationSequenceBase && AnimationSequenceBase->GetSkeleton())
	{
		FMemMark Mark(FMemStack::Get());

		// asset to use for retarget proportions (can be either USkeletalMesh or USkeleton)
		UObject* AssetToUse;
		int32 NumRequiredBones;
		if (EvaluationOptions.OptionalSkeletalMesh)
		{
			AssetToUse = CastChecked<UObject>(EvaluationOptions.OptionalSkeletalMesh);
			NumRequiredBones = EvaluationOptions.OptionalSkeletalMesh->GetRefSkeleton().GetNum();
		}
		else
		{
			AssetToUse = CastChecked<UObject>(AnimationSequenceBase->GetSkeleton());
			NumRequiredBones = AnimationSequenceBase->GetSkeleton()->GetReferenceSkeleton().GetNum();
		}

		const USkeleton* Skeleton = AnimationSequenceBase->GetSkeleton();
		const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

		// Find the index of the bone
		int32 BoneNameToIndex = RefSkeleton.FindBoneIndex(BoneName);
		if (BoneNameToIndex == INDEX_NONE)
		{
			return FTransform::Identity;
		}


		TArray<FBoneIndexType> RequiredBoneIndexArray;
		RequiredBoneIndexArray.AddUninitialized(NumRequiredBones);
		for (int32 BoneIndex = 0; BoneIndex < RequiredBoneIndexArray.Num(); ++BoneIndex)
		{
			RequiredBoneIndexArray[BoneIndex] = static_cast<FBoneIndexType>(BoneIndex);
		}

		FBoneContainer RequiredBones;
		RequiredBones.InitializeTo(RequiredBoneIndexArray, UE::Anim::FCurveFilterSettings(EvaluationOptions.bEvaluateCurves ? UE::Anim::ECurveFilterMode::None : UE::Anim::ECurveFilterMode::DisallowAll), *AssetToUse);

		RequiredBones.SetUseRAWData(EvaluationOptions.EvaluationType == EAnimBaseEvalType::Raw);
		RequiredBones.SetUseSourceData(EvaluationOptions.EvaluationType == EAnimBaseEvalType::Source);

		RequiredBones.SetDisableRetargeting(!EvaluationOptions.bShouldRetarget);

		FCompactPose CompactPose;
		FBlendedCurve Curve;
		UE::Anim::FStackAttributeContainer Attributes;

		FAnimationPoseData PoseData(CompactPose, Curve, Attributes);
		FAnimExtractContext Context(0.0, EvaluationOptions.bExtractRootMotion);

		FCompactPose BasePose;
		BasePose.SetBoneContainer(&RequiredBones);

		CompactPose.SetBoneContainer(&RequiredBones);
		Curve.InitFrom(RequiredBones);


		const double EvalInterval = Time;

		bool bValidTime = true;

		Context.CurrentTime = EvalInterval;

		Curve.InitFrom(RequiredBones);

		FTransform BoneTransform;

		if (bValidTime)
		{
			if (AnimationSequenceBase->IsValidAdditive())
			{
				CompactPose.ResetToAdditiveIdentity();
				AnimationSequenceBase->GetAnimationPose(PoseData, Context);
				BoneTransform = PoseData.GetPose()[FCompactPoseBoneIndex(BoneNameToIndex)];
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, BoneTransform.ToString());
			}
			else
			{
				CompactPose.ResetToRefPose();
				AnimationSequenceBase->GetAnimationPose(PoseData, Context);
				BoneTransform = PoseData.GetPose()[FCompactPoseBoneIndex(BoneNameToIndex)];
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, BoneTransform.ToString());
			}

			if (BoneNameToIndex != 0)
			{
				int32 ParentIndex = RefSkeleton.GetParentIndex(BoneNameToIndex);
				while (ParentIndex != INDEX_NONE)
				{
					FTransform ParentTransform = PoseData.GetPose()[FCompactPoseBoneIndex(ParentIndex)];;
					BoneTransform = BoneTransform * ParentTransform;
					ParentIndex = RefSkeleton.GetParentIndex(ParentIndex);
				}
			}
			return BoneTransform;
		}
		
	}
	return FTransform::Identity;
}

UAnimSequence* UHelpfulFunctionsBPLibrary::GetAnimSequenceFromMontage(const UAnimMontage* TargetAnimMontage, int SequenceIndex)
{
	UAnimSequence* AnimSequence = nullptr;

	const UAnimMontage* AnimMontage = TargetAnimMontage;
	if (TargetAnimMontage && AnimMontage->SlotAnimTracks.Num() > 0 && AnimMontage->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() > SequenceIndex)
	{
		AnimSequence = Cast<UAnimSequence>(AnimMontage->SlotAnimTracks[0].AnimTrack.AnimSegments[SequenceIndex].GetAnimReference());
	}
	return AnimSequence;

}

bool UHelpfulFunctionsBPLibrary::CheckFloorTypeC(UObject* WorldContextObject, FVector& GlobalOrigin, float& GlobalRadius, FVector& ForwardOrigin, float& ForwardRadius, FVector FloorLocation,
	FVector Direction, ETraceTypeQuery TraceChannel, int AccuracyFactor, float MaxRadius, TArray<AActor*> ActorsToIgnore, bool ReturnInLocalSpace, int DrawDebugIndex)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DrawDebugIndex == 1)
	{
		TraceType = EDrawDebugTrace::ForOneFrame;
	}
	else if (DrawDebugIndex == 2)
	{
		TraceType = EDrawDebugTrace::ForDuration;
	}

	GlobalOrigin = FloorLocation;
	GlobalRadius = 100.0;
	ForwardOrigin = FloorLocation;
	ForwardRadius = 100.0;

	FHitResult HitResult;
	bool SphereTraceHitValid;
	SphereTraceHitValid = UKismetSystemLibrary::SphereTraceSingle(WorldContextObject, FloorLocation + FVector(0,0,20), FloorLocation + FVector(0,0,-30), 20.0, TraceChannel, false, ActorsToIgnore, TraceType, HitResult, true, FColor::Black, FColor::Blue, 0.5);
	if (SphereTraceHitValid == false)
	{
		return false;
	}
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	FVector DetectionCenter = HitResult.ImpactPoint; DetectionCenter.Z = FloorLocation.Z;

	const int LastIndex = FMath::Clamp<int>(AccuracyFactor * 4, 0, 39);

	FVector CollisionCheckingPoint = FVector(0, 0, 0);
	FVector CollisionPoint;
	FVector Center;
	float Radius = 0.0;
	TArray<FVector> ForwardArray;

	int i = 0;

	for (i = 0; i <= LastIndex; i++)
	{
		FQuat Rot = UKismetMathLibrary::Conv_RotatorToQuaternion(FRotator(0, UKismetMathLibrary::MapRangeClamped(float(i), 0, float(LastIndex), 0, 360), 0));

		CollisionCheckingPoint = DetectionCenter + (UKismetMathLibrary::Quat_RotateVector(Rot, Direction) * (MaxRadius * 1.8));

		HitComponent->GetClosestPointOnCollision(CollisionCheckingPoint, CollisionPoint, HitResult.BoneName);

		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Blue, CollisionPoint.ToString(), true);

		if (DrawDebugIndex == 2) 
		{
			UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
			DrawDebugPoint(World, CollisionPoint, 18, FColor::Cyan, false, 0.5, 0);
		}

		Center = Center + CollisionPoint;
		Radius = Radius + UKismetMathLibrary::Vector_Distance2D(CollisionPoint, DetectionCenter);
		if (i == LastIndex / 2 || i == 0)
		{
			ForwardArray.Add(CollisionPoint);
		}
	}

	if (i == 0) { return false; }

	if (ReturnInLocalSpace)
	{
		FTransform TransformWS;
		TransformWS.SetLocation(Center / (LastIndex + 1));
		TransformWS.SetRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(UKismetMathLibrary::FindLookAtRotation(DetectionCenter, Center / (LastIndex + 1))));
		FCALS_ComponentAndTransform StructureWS;
		StructureWS.Transform = TransformWS;
		StructureWS.Component = HitComponent;
		FCALS_ComponentAndTransform StructureLS = ConvertWorldToLocalFastMatrix(StructureWS);
		GlobalOrigin = StructureLS.Transform.GetLocation();
		GlobalRadius = Radius / (LastIndex + 1);
		if (ForwardArray.Num() > 1)
		{
			ForwardOrigin = UKismetMathLibrary::VLerp(ForwardArray[0], ForwardArray[1], 0.5);
			ForwardRadius = UKismetMathLibrary::Vector_Distance(ForwardArray[0], ForwardArray[1]);
		}
		return true;
	}
	else
	{
		GlobalOrigin = Center / (LastIndex+1);
		GlobalRadius = Radius / (LastIndex+1);
		if (ForwardArray.Num() > 1)
		{
			ForwardOrigin = UKismetMathLibrary::VLerp(ForwardArray[0], ForwardArray[1], 0.5);
			ForwardRadius = UKismetMathLibrary::Vector_Distance(ForwardArray[0], ForwardArray[1]);
		}
		return true;
	}
	
}







