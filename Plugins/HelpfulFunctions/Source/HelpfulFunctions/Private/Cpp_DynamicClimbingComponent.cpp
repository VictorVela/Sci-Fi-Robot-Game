// Copyright Jakub W, All Rights Reserved. 

#include "Cpp_DynamicClimbingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Curves/CurveVector.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "Animation/AnimInstance.h"
#include "HelpfulFunctionsBPLibrary.h"
#include <tuple>


// Sets default values for this component's properties
UCpp_DynamicClimbingComponent::UCpp_DynamicClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCpp_DynamicClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	CharacterC = Cast<ACharacter>(this->GetOwner());
	if (IsValid(CharacterC) == true)
	{
		DefCapsuleSizeC = FVector2D(CharacterC->GetCapsuleComponent()->GetScaledCapsuleRadius(), CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}

}



// Called every frame
void UCpp_DynamicClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UCpp_DynamicClimbingComponent::CheckNormalForPointC_Implementation(FExposedCableParticle& InParticle)
{
	if (HookActorC && HookActorC->Implements<UALS_HookActorInterface>())
	{
		IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(HookActorC); //Get Interface

		float AngleValue = 0.0;
		HookInterface->Execute_HAFSI_Get_ParticleNormalValidation(HookActorC, AngleValue);

		if (AngleValue <= 0.0) { return true; }

		EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
		if (SwingDebugIndexC == 1) TraceType = EDrawDebugTrace::ForOneFrame;
		if (SwingDebugIndexC == 2) TraceType = EDrawDebugTrace::ForDuration;
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		TEnumAsByte<ETraceTypeQuery> Channel = ETraceTypeQuery::TraceTypeQuery1;

		FHitResult R1, R2;
		const bool HitValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, InParticle.Position, InParticle.OldPosition + FVector(0, 0, 0.2), CableSimC->CableWidth * 1.1, 
			Channel, false, ToIgnore, TraceType, R1, true, FLinearColor::Black, FLinearColor::White, 0.2);
		if (HitValid == false) { return true; }

		const bool HitValid2 = UKismetSystemLibrary::SphereTraceSingle(CharacterC, R1.ImpactPoint + (R1.Normal * -6.0) + FVector(0, 0, 12), R1.ImpactPoint + (R1.Normal * -6.0) + FVector(0, 0, -12), 
			CableSimC->CableWidth * 1.1, Channel, false, ToIgnore, TraceType, R2, true, FLinearColor::Gray, FLinearColor::Blue, 0.3);
		if (HitValid2 == false) { return true; }

		const float Dot = UKismetMathLibrary::Dot_VectorVector(R2.Normal, FVector(0, 0, 1));

		return Dot > AngleValue;
	}
	return true;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::RopeHookedConditionC_Implementation()
{
	return true;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::DetachRopeOrEndSwingC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::StartedZiplineC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::StartedPickaxeClimbC_Implementation()
{
	return false;
}

// (IMPLEMENTATION)
bool UCpp_DynamicClimbingComponent::FinishRopeSwingC_Implementation()
{
	return false;
}

//MAIN LEDGE CREATION FUNCTION (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::TryCreateLedgeStructureC_Implementation(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, 
	FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid, FVector TraceOrigin, FVector TraceDirection, FVector2D AxisNormals, float Z_Offset, float ForwardTraceLength, 
	float RightOffsetScale, bool UseWallCondition, bool InverseTracing)
{
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	bool Continue = false;
	FCMC_SingleClimbPointC RightLedgeStruct = {};
	FCMC_SingleClimbPointC LeftLedgeStruct = {};
	FCMC_SingleClimbPointC CenterLedgeStruct = {};
	bool RightLedgeValid = false;
	bool LeftLedgeValid = false;
	int TraceDebugIndex = DebugTraceIndexC;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }

	float TraceForward = ForwardTraceLength;
	if (CurrentModifyVolume)
	{
		if (CurrentModifyVolume->LedgeSearchForwardRange >= 0.0 && CharacterC->GetVelocity().Z > CurrentModifyVolume->VerticalVelocityTollerance)
		{ TraceForward = CurrentModifyVolume->LedgeSearchForwardRange; }
	}

	if (UseWallCondition == true)
	{
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		FHitResult WalleHitResult;
		const bool WallHitValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TraceOrigin, TraceOrigin + (TraceDirection * TraceForward),
								  18.0, Channel, false, ToIgnore, TraceType, WalleHitResult, true);
		Continue = WallHitValid;
	}
	else
	{ Continue = true; }
	if (Continue == false)
	{ Valid = false; FirstNotValid = true; return; }

	if (CurrentModifyVolume)
	{
		int LedgeIterationsCount = CurrentModifyVolume->LedgeUpOffsetIterations - 1;
		if (CharacterC->GetVelocity().Z <= CurrentModifyVolume->VerticalVelocityTollerance)
		{ LedgeIterationsCount = 0; }

		for (int i = 0; i <= LedgeIterationsCount; i++)
		{
			float NewOffset = Z_Offset + ((float)i * CurrentModifyVolume->UpOffsetDeltaValue);

			// RIGHT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
			UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, RightLedgeValid, RightLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, NewOffset,
				TraceForward, RightOffsetScale, false, InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2);
			if (RightLedgeValid == false)
			{
				Valid = false; FirstNotValid = false; 
			}

			// LEFT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
			UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, LeftLedgeValid, LeftLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, NewOffset,
				TraceForward, RightOffsetScale, true, InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2);
			if (LeftLedgeValid == false)
			{
				Valid = false; FirstNotValid = false; 
			}

			if (RightLedgeValid && LeftLedgeValid)
			{
				break;
			}

		}
	}
	else
	{
		// RIGHT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
		UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, RightLedgeValid, RightLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, Z_Offset,
			ForwardTraceLength, RightOffsetScale, false, InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2);
		if (RightLedgeValid == false)
		{
			Valid = false; FirstNotValid = false; return;
		}

		// LEFT FINDING LEDGE POINT ---> IF NOT VALID FINISH FUNCTION
		UHelpfulFunctionsBPLibrary::TryFindLedgeLine(CharacterC, LeftLedgeValid, LeftLedgeStruct, CharacterC, TraceOrigin, TraceDirection, AxisNormals, Z_Offset,
			ForwardTraceLength, RightOffsetScale, true, InverseTracing, TraceDebugIndex, ClassToIgnoreByLedgeC, ForClimbingChannelC, 0.2);
		if (LeftLedgeValid == false)
		{
			Valid = false; FirstNotValid = false; return;
		}
	}

	if (UHelpfulFunctionsBPLibrary::ClimbingLedgeValidP1(CharacterC, true, LeftLedgeStruct, RightLedgeStruct, ForClimbingChannelC, 25.0) == false 
		|| LedgeValidationPart2C(true, LeftLedgeStruct, RightLedgeStruct, 14.0, 0.4, -50.0, DefCapsuleSizeC)==false)
	{ 
		Valid = false; FirstNotValid = false; return; 
	}

	RightLedgeStruct.Component = LeftLedgeStruct.Component;
	CenterLedgeStruct = LeftLedgeStruct;
	CenterLedgeStruct.Location = UKismetMathLibrary::VLerp(LeftLedgeStruct.Location, RightLedgeStruct.Location, 0.5);
	CenterLedgeStruct.Normal = UKismetMathLibrary::VLerp(LeftLedgeStruct.Normal, RightLedgeStruct.Normal, 0.5);
	//SET RETURN VALUES:
	Valid = LeftLedgeValid;
	LeftPoint = LeftLedgeStruct;
	RightPoint = RightLedgeStruct;
	FirstNotValid = false;
	OriginPoint = CenterLedgeStruct;
	return;
}

//GET CHARACTER AXIS (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::GetCharacterAxisC_Implementation(float& Forward, float& Right)
{
	Forward = AxisValuesC.X;
	Right = AxisValuesC.Y;
}

// CHECK CAN START CORNER (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::CheckCanStartCornerC_Implementation(bool& DetectedCorner, bool& OuterType, FCMC_LedgeC& TargetLedgeStruct, bool Valid, bool InputLock)
{
	const bool MainCondition = AxisValuesC.X == 0.0 && AxisValuesC.Y != 0.0 && StartNarrowFloorMovementC == false && ActionC == CMC_ActionTypeC::None;
	if (MainCondition == false && InputLock == false)
	{ DetectedCorner = false; OuterType = true; return; }

	//Main Local Variables
	FCMC_LedgeC ReturnStruct = {};
	FCMC_LedgeC LWS = {};
	FCALS_ComponentAndTransform T = {};
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	float OffsetF = 5.0;
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	//Step 1) Convert Local Ledge Structure To Global Space
	T.Component = LedgePointsLS_C.Component;
	LWS.Component = LedgePointsLS_C.Component;
	T.Transform = LedgePointsLS_C.LeftPoint;
	LWS.LeftPoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;
	T.Transform = LedgePointsLS_C.RightPoint;
	LWS.RightPoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;
	T.Transform = LedgePointsLS_C.Origin;
	LWS.Origin = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(T).Transform;

	//Step 2) Run First Trace
	FVector TOrigin = CharacterC->GetActorLocation() + FVector(0, 0, 16) + (CharacterC->GetActorForwardVector() * 0.5);
	FHitResult TResult;
	const bool TValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (CharacterC->GetActorRightVector() * AxisValuesC.Y * 1), 
	TOrigin + (CharacterC->GetActorRightVector() * AxisValuesC.Y * 50.0), 18, 38, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), 
	false, ToIgnore, TraceType, TResult, true, FLinearColor::Blue, FLinearColor::White, 0.1);

	//Step 3) Select Finding Type - Outer/Inner
	FHitResult SecondResult;
	bool SecondValid = false;
	bool LedgeValid = false;
	if (!TValid == true)
	{
		for (int i = 0; i <= 2; i++)
		{
			TOrigin = UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).GetLocation() + 
			(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * OffsetF);
			TOrigin = TOrigin + FVector(0, 0, -6);
			OffsetF = OffsetF + 10.0;
			SecondValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint,
			LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * 14.0 * AxisValuesC.Y), TOrigin + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::SelectTransform(LWS.RightPoint,
			LWS.LeftPoint, AxisValuesC.Y > 0).Rotator()) * -2.0 * AxisValuesC.Y), 12, 25, Channel, false, ToIgnore, TraceType, SecondResult, true);
			if (SecondValid == true)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, TOrigin.ToString());
				if (abs(UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()))) < 0.45 
				&& UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()),CharacterC->GetActorForwardVector()))<20.0)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TOrigin.ToString());
					TOrigin = FVector(SecondResult.ImpactPoint.X, SecondResult.ImpactPoint.Y, UKismetMathLibrary::Lerp(SecondResult.ImpactPoint.Z, TOrigin.Z + 6.0, 0.5));
					TOrigin = TOrigin + (UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal) * -20.0) + FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(6.0, 12.0));
					//Create Result Structures
					FCMC_SingleClimbPointC LP, RP, OP;
					//Execute Ledge Finding Function
					TryCreateLedgeStructureC_Implementation(LedgeValid, LP, RP, OP, SecondValid, TOrigin, UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), 
					FVector2D(0, 0), 0, 45, 1, false, true);
					if (LedgeValid == true)
					{
						ReturnStruct.LeftPoint = ConvertLedgeStructToWS(LP).Transform;
						ReturnStruct.RightPoint = ConvertLedgeStructToWS(RP).Transform;
						ReturnStruct.Origin = ConvertLedgeStructToWS(OP).Transform;
						ReturnStruct.Component = OP.Component;
						DetectedCorner = IsValid(ReturnStruct.Component);
						OuterType = true;
						TargetLedgeStruct = ReturnStruct;
						return;
					}
				}
			}
		}
		DetectedCorner = false; OuterType = true; return;
	}
	else
	{
		OffsetF = -30;
		for (int i = 0; i <= 2; i++)
		{
			TOrigin = UKismetMathLibrary::SelectTransform(LWS.RightPoint, LWS.LeftPoint, AxisValuesC.Y > 0).GetLocation() +
				(UKismetMathLibrary::GetForwardVector(CharacterC->GetActorRotation()) * OffsetF);
			TOrigin = TOrigin - FVector(0, 0, 6);
			OffsetF = OffsetF - 10.0;
			SecondValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TOrigin + (UKismetMathLibrary::GetRightVector(CharacterC->GetActorRotation()) * 1 * AxisValuesC.Y), 
			TOrigin + (UKismetMathLibrary::GetRightVector(CharacterC->GetActorRotation()) * 40 * AxisValuesC.Y), 12, 25, Channel, false, ToIgnore, TraceType, SecondResult, true);
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, TOrigin.ToString());
			if (SecondValid == true)
			{
				if (abs(UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal), UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()))) <= 1
					&& UKismetMathLibrary::RadiansToDegrees(UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetForwardVector(LWS.Origin.Rotator()), CharacterC->GetActorForwardVector())) < 20.0)
				{
					TOrigin = FVector(SecondResult.ImpactPoint.X, SecondResult.ImpactPoint.Y, UKismetMathLibrary::Lerp(SecondResult.ImpactPoint.Z, TOrigin.Z + 6.0, 0.5));
					TOrigin = TOrigin + (UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal) * -20.0) + FVector(0, 0, UKismetMathLibrary::RandomFloatInRange(6.0, 12.0));
					//Create Result Structures
					FCMC_SingleClimbPointC LP, RP, OP;
					//Execute Ledge Finding Function
					TryCreateLedgeStructureC_Implementation(LedgeValid, LP, RP, OP, SecondValid, TOrigin, UHelpfulFunctionsBPLibrary::NormalToVector(SecondResult.Normal),
						FVector2D(0, 0), 0, 45, 1, false, true);
					if (LedgeValid == true)
					{
						ReturnStruct.LeftPoint = ConvertLedgeStructToWS(LP).Transform;
						ReturnStruct.RightPoint = ConvertLedgeStructToWS(RP).Transform;
						ReturnStruct.Origin = ConvertLedgeStructToWS(OP).Transform;
						ReturnStruct.Component = OP.Component;
						DetectedCorner = IsValid(ReturnStruct.Component);
						OuterType = false;
						TargetLedgeStruct = ReturnStruct;
						return;
					}
				}
			}
		}
		DetectedCorner = false; OuterType = false; return;
	}
	DetectedCorner = false; OuterType = false; return;
	return;
}

// CHECK CAN JUMP BACK (IMPLEMENTATION)
void UCpp_DynamicClimbingComponent::CheckCanJumpBackC_Implementation(bool& ReturnValue, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint,
	FCMC_SingleClimbPointC& OriginPoint, bool UseCameraCondition, float JumpMaxDistance)
{
	bool CamCondition = false;
	if (UseCameraCondition == true){
		APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		CamCondition = UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::GetForwardVector(FRotator(PCM->GetCameraRotation().Yaw, 0, 0)), CharacterC->GetActorForwardVector()) < 0.22;
		if (CamCondition == false)
		{
			JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), UGameplayStatics::GetWorldDeltaSeconds(this), 8);
			ReturnValue = false; return; 
		}}

	if (ActionC == CMC_ActionTypeC::None && StartNarrowFloorMovementC == false && AxisValuesC.X != 0 && AxisValuesC.Y == 0)
	{
		bool LedgeOutputValid = false;
		float DownScale = UKismetMathLibrary::SelectFloat(1.2, 1, AxisValuesC.X < 0);
		float DistanceToWall = 0;
		float UpOffsetValue = 60;
		float ForwardOffsetValue = 0.8;
		float JumpMaxDist = JumpMaxDistance; //Input Parameter
		FVector LedgeLoc, LedgeDir = FVector(0, 0, 0);
		FHitResult FTHR = {};
		ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
		EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
		if (DebugTraceIndexC > 0)
		{TraceType = EDrawDebugTrace::ForOneFrame;}
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);

		ChooseLedgeFindingTransformC(false, LedgeLoc, LedgeDir);
		const bool FirstTraceValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, LedgeLoc + (LedgeDir * -1 * 3), LedgeLoc + (LedgeDir * -1 * JumpMaxDist) + 
		FVector(0, 0, AxisValuesC.X * 50), 25, 60, Channel, false, ToIgnore, TraceType, FTHR, true, FLinearColor::Blue, FLinearColor::Green, 0.1);
		if (FirstTraceValid == false)
		{   JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), UGameplayStatics::GetWorldDeltaSeconds(this), 8);
			ReturnValue = false; return; }
		DistanceToWall = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(FTHR.Location, FTHR.TraceStart), 0, UKismetMathLibrary::Vector_Distance(FTHR.TraceEnd, FTHR.TraceStart), 0, 1);
		DistanceToWall = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::FClamp(DistanceToWall, 0, 0.9), 0.3, 0.9, 1.4, 0.7);
		//Update Animation Alpha
		JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(1, UKismetMathLibrary::MapRangeClamped(AxisValuesInterpC.X,-1,1,0,1)), 
		UGameplayStatics::GetWorldDeltaSeconds(this), 8);
		if (SpaceBarImpulseC == true) // Continue Function Only When Player Pressed Space Bar
		{
			FHitResult STHR = {};
			ChooseLedgeFindingTransformC(false, LedgeLoc, LedgeDir);
			for (int i = 0; i <= 3; i++)
			{
				const bool SphereTraceValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, LedgeLoc + (LedgeDir * -3), LedgeLoc + (LedgeDir * -1 * JumpMaxDist * ForwardOffsetValue) +
				FVector(0, 0, 1.25 * DownScale * DistanceToWall * AxisValuesC.X * UpOffsetValue), 25, Channel, false, ToIgnore, TraceType, STHR, true, FLinearColor(0.35f, 0.0f, 0.1f, 1.0f), 
				FLinearColor(1.0f, 0.1f, 0.2f, 1.0f), 0.05);
				if (SphereTraceValid == true)
				{
					//{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}
					for (int j = 0; j <= 1; j++)
					{
						bool LineTraceValid = UKismetSystemLibrary::LineTraceSingle(CharacterC, STHR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * 3) +
						FVector(0, 0, (j + 1) * 24.0), STHR.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * 3) + FVector(0, 0, -12), 
						Channel, false, ToIgnore, TraceType, FTHR, true);
						if (LineTraceValid == true)
						{
							TryCreateLedgeStructureC_Implementation(LedgeOutputValid, LeftPoint, RightPoint, OriginPoint, LineTraceValid, UKismetMathLibrary::SelectVector((FTHR.TraceStart + FTHR.TraceEnd) / 2,
							FTHR.ImpactPoint, FTHR.bStartPenetrating) + (UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal) * -20), UHelpfulFunctionsBPLibrary::NormalToVector(STHR.Normal), 
							FVector2D(0, 0), 0, 45, 1, false, true);
							if (LedgeOutputValid == true && UKismetMathLibrary::Dot_VectorVector(OriginPoint.Normal,CharacterC->GetActorForwardVector())<-0.55)
							{
								ReturnValue = LedgeOutputValid;
								return; //finish function with true value
							}
						}
					}
				}
				UpOffsetValue = UKismetMathLibrary::FClamp(UpOffsetValue - 20, -40, 100);
				ForwardOffsetValue = UKismetMathLibrary::FClamp(ForwardOffsetValue + 0.1, 0, 1.0);
			}
			ReturnValue = false; return;
		}
		ReturnValue = false; return;
	}
	else
	{
		JumpBackPoseAlphaC = UKismetMathLibrary::Vector2DInterpTo(JumpBackPoseAlphaC, FVector2D(0, 0), UGameplayStatics::GetWorldDeltaSeconds(this), 8);
		ReturnValue = false; return;
	}
}

//CREATE AXIS VALUES WITH INTERPOLATION
void UCpp_DynamicClimbingComponent::CreateAxisValuesWithInterpFast(float InterpSpeed)
{
	float AxisForward = 0.0;
	float AxisRight = 0.0;
	float Delta = UGameplayStatics::GetWorldDeltaSeconds(this);
	if (IsValid(CharacterC) == false)
	{ return; }
	float CurveValue = CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(FName("DLCv2_Movement_Speed"));
	GetCharacterAxisC(AxisForward, AxisRight);
	AxisValuesInterpC = FVector2D(UKismetMathLibrary::FInterpTo(AxisValuesInterpC.X, AxisForward, Delta, InterpSpeed), 
								  UKismetMathLibrary::FInterpTo(AxisValuesInterpC.Y, AxisRight, Delta, InterpSpeed));

	AxisValuesInterpSlowC = FVector2D(UKismetMathLibrary::FInterpTo(AxisValuesInterpSlowC.X, AxisForward, Delta, InterpSpeed * 0.4), 
	UKismetMathLibrary::FClamp(UKismetMathLibrary::FInterpTo(AxisValuesInterpSlowC.Y, AxisRight, Delta, InterpSpeed * 0.4) + CurveValue,-1,1));

}

//CONVERT LEDGE TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeToCapPositionC(FCALS_ComponentAndTransform Center)
{
	FTransform ReturnTransform = FTransform(Center.Transform.Rotator(), Center.Transform.GetLocation() + 
	(UKismetMathLibrary::GetForwardVector(Center.Transform.Rotator()) * (DefCapsuleSizeC.X * -1.0)) + FVector(0,0,CapsuleUpOffsetC * -1.0), FVector(1, 1, 1));
	FCALS_ComponentAndTransform ReturnStructure = {};
	ReturnStructure.Transform = ReturnTransform;
	ReturnStructure.Component = Center.Component;
	return ReturnStructure;
}

//CONVERT FLOOR TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertFloorToCapPositionC(FCALS_ComponentAndTransform Center)
{
	FCALS_ComponentAndTransform ReturnStructure = {};
	ReturnStructure.Transform = FTransform(Center.Transform.Rotator(), Center.Transform.GetLocation() +
		FVector(0, 0, CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FVector(1, 1, 1));
	ReturnStructure.Component = Center.Component;
	return ReturnStructure;
}

//CHOOSE LEDGE FINDING TRANSFORM
void UCpp_DynamicClimbingComponent::ChooseLedgeFindingTransformC(bool GetByLedge, FVector& ReturnLocation, FVector& ReturnDirection)
{
	if (GetByLedge == true)
	{
		FCALS_ComponentAndTransform LedgeToSinglePoint = {};
		LedgeToSinglePoint.Transform = LedgePointsLS_C.Origin;
		LedgeToSinglePoint.Component = LedgePointsLS_C.Component;
		LedgeToSinglePoint = UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(LedgeToSinglePoint);
		ReturnLocation = LedgeToSinglePoint.Transform.GetLocation() + (UKismetMathLibrary::GetForwardVector(LedgeToSinglePoint.Transform.Rotator()) * DefCapsuleSizeC.X * -0.9);
		ReturnDirection = UKismetMathLibrary::GetForwardVector(LedgeToSinglePoint.Transform.Rotator());
	}
	else
	{
		ReturnLocation = CharacterC->GetActorLocation() + FVector(0, 0, CapsuleUpOffsetC);
		ReturnDirection = CharacterC->GetActorForwardVector();
	}
}

//GET DIRECTION BY PLAYER AXIS INPUT
FVector UCpp_DynamicClimbingComponent::GetDirectionByInputC(float LerpWithForward)
{
	float AxisForward = 0.0;
	float AxisRight = 0.0;
	GetCharacterAxisC(AxisForward, AxisRight);
	if (AxisForward == 0.0 && AxisRight == 0.0)
	{ AxisForward = 1.0; AxisRight = 0.0; }
	return UKismetMathLibrary::VLerp(UKismetMathLibrary::ClampVectorSize((CharacterC->GetActorForwardVector() * abs(AxisForward)) + 
		   (CharacterC->GetActorRightVector() * AxisRight), -1.0, 1.0), CharacterC->GetActorForwardVector(), LerpWithForward);
}

//CONVERT SINGLE LEDGE STRUCTURE (AS TWO VECTORS - LOCATION & ROTATION) TO TRANSFORM & COMPONENT STRUCTURE
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeStructToWS(FCMC_SingleClimbPointC SingleClimbPointWS)
{
	FCALS_ComponentAndTransform ReturnStruct = {};
	ReturnStruct.Transform = FTransform(FRotator(UKismetMathLibrary::MakeRotFromX(SingleClimbPointWS.Normal)), FVector(SingleClimbPointWS.Location), FVector(1, 1, 1));
	ReturnStruct.Component = SingleClimbPointWS.Component;
	return ReturnStruct;
}

//CONVERT SINGLE LEDGE STRUCTURE (AS TWO VECTORS - LOCATION & ROTATION) TO TRANSFORM & COMPONENT STRUCTURE [BUT WITH LOCAL SPACE CONVERT]
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertLedgeStructToLS(FCMC_SingleClimbPointC SingleClimbPointWS)
{
	FCALS_ComponentAndTransform ReturnStruct = {};
	ReturnStruct.Transform = FTransform(FRotator(UKismetMathLibrary::MakeRotFromX(SingleClimbPointWS.Normal)), FVector(SingleClimbPointWS.Location), FVector(1, 1, 1));
	ReturnStruct.Component = SingleClimbPointWS.Component;
	return UHelpfulFunctionsBPLibrary::ConvertWorldToLocalFastMatrix(ReturnStruct);
}

//MAKE SURE THE DETECTED LEDGE IS VALID
bool UCpp_DynamicClimbingComponent::LedgeValidationPart2C(bool Valid, FCMC_SingleClimbPointC LeftStruct, FCMC_SingleClimbPointC RightStruct, 
	float MinDistanceBetweenPoints, float RotationTollerance, float CapsuleUpOffset, FVector2D CapsuleChecking)
{
	if (Valid == false)
	{ return false; }

	FVector TraceStart = FVector(0, 0, 0);
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;

	if (LeftStruct.Location != RightStruct.Location && LeftStruct.Location != FVector(0, 0, 0) 
		&& RightStruct.Location != FVector(0, 0, 0) 
		&& UKismetMathLibrary::Vector_Distance(LeftStruct.Location, RightStruct.Location) > MinDistanceBetweenPoints 
		&& UKismetMathLibrary::Dot_VectorVector(UHelpfulFunctionsBPLibrary::NormalToVector(LeftStruct.Normal), 
		   UHelpfulFunctionsBPLibrary::NormalToVector(RightStruct.Normal))>RotationTollerance)
	{
		// SelectVector(Wartosc jezeli Prawda, Wartosc jezeli Falsz, Warunek Bool)
		TArray<AActor*> ToIgnore;
		ToIgnore.Add(CharacterC);
		FHitResult CapsuleHitResult;

		TraceStart = UKismetMathLibrary::VLerp(LeftStruct.Location, RightStruct.Location, 0.5) + 
		(UHelpfulFunctionsBPLibrary::NormalToVector(UKismetMathLibrary::VLerp(LeftStruct.Normal, RightStruct.Normal, 0.5)) * CapsuleChecking.X);
		TraceStart = TraceStart + FVector(0, 0, CapsuleUpOffset);

		const bool HitValid = UKismetSystemLibrary::CapsuleTraceSingle(CharacterC, TraceStart, TraceStart, CapsuleChecking.X * 0.6, 
		UKismetMathLibrary::SelectFloat(NarrowFloorCapRadiusC * 0.8, CapsuleChecking.Y * 0.7, StartNarrowFloorMovementC), Channel, false, ToIgnore, TraceType, CapsuleHitResult, true);
		if (HitValid == false)
		{
			return true; 
		}
		else
		{
			return false; 
		}
	}
	else
	{ return false; }
}

//CHECK CAN DROP TO LEDGE
bool UCpp_DynamicClimbingComponent::CheckCanDropToLedgeC(FCMC_LedgeC& LedgeStructWS)
{
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ForClimbingChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	TArray<AActor*> ToIgnore;
	FHitResult LineHitResult;
	FVector LineTraceOrigin = CharacterC->GetActorLocation() - FVector(0, 0, CharacterC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight())-FVector(0,0,10);
	FVector Offset = FVector(0, 0, 10);

	const bool LineHitValid = UKismetSystemLibrary::LineTraceSingle(CharacterC, LineTraceOrigin + (CharacterC->GetActorForwardVector() * 45.0), 
	LineTraceOrigin + (CharacterC->GetActorForwardVector() * -5.0), Channel, false, ToIgnore, TraceType, LineHitResult, true, FLinearColor::Black, FLinearColor::Green, 0.6);

	if (LineHitValid == true && LineHitResult.bStartPenetrating == false)
	{
		//Check Capsule Have Room - If Not finish funtion
		LineTraceOrigin = LineHitResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal) * DefCapsuleSizeC.X * -1.0);
		if (UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnoreTransform(CharacterC, CharacterC, 
		FTransform(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal)), LineTraceOrigin, FVector(1, 1, 1)), ToIgnore, 0.8, 0.9, false) == false)
		{ return false; }
		for (int i = 0; i <= 1; i++)
		{
			LineTraceOrigin = LineHitResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal) * DefCapsuleSizeC.X * -0.6);
			if (i > 0)
			{ Offset = FVector(0, 0, 25); }
			bool LedgeValid = false;
			bool WallValid = true;
			FCMC_SingleClimbPointC Right;
			FCMC_SingleClimbPointC Left;
			FCMC_SingleClimbPointC Origin;
			FCMC_LedgeC ReturnStruct;

			TryCreateLedgeStructureC_Implementation(LedgeValid, Left, Right, Origin, WallValid, LineTraceOrigin + Offset, 
			UHelpfulFunctionsBPLibrary::NormalToVector(LineHitResult.Normal), FVector2D(0, 0), 0, 45, 1, true, true);
			if (LedgeValid == true)
			{
				ReturnStruct.LeftPoint = ConvertLedgeStructToWS(Left).Transform;
				ReturnStruct.RightPoint = ConvertLedgeStructToWS(Right).Transform;
				ReturnStruct.Origin = ConvertLedgeStructToWS(Origin).Transform;
				ReturnStruct.Component = Origin.Component;
				LedgeStructWS = ReturnStruct;
				return true;
			}
		}
	}
	return false;
}

//RESIZE CAPSULE RADIUS TO DEFAULT
void UCpp_DynamicClimbingComponent::ResizeCapsuleToDefaultC(float InterpSpeed)
{
	if (StartNarrowFloorMovementC == true)
	{ return; }
	if (DefCapsuleSizeC.X != CharacterC->GetCapsuleComponent()->GetUnscaledCapsuleRadius())
	{
		CharacterC->GetCapsuleComponent()->SetCapsuleRadius(UKismetMathLibrary::FInterpTo_Constant(CharacterC->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 
		DefCapsuleSizeC.X, UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed), true);
	}
	return;
}

//DO WHEN IS CLIMBING - CHECK LEDGE PER FRAME
void UCpp_DynamicClimbingComponent::UpdateLedgePerFrameC(FCMC_LedgeC& OutLedge, FVector& OutOrigin, FVector2D SlopeScale, float ConstMovementOffset, bool InputLock)
{
	//Update Cached Ledge
	if (AxisValuesC.X == 0 && AxisValuesC.Y == 0)
	{ CachedLedgePointsLS_C = LedgePointsLS_C; }

	//Calculate Slope Offset
	float SlopeOffset = 0;
	FCALS_ComponentAndTransform LP, RP;
	LP.Transform = LedgePointsLS_C.LeftPoint;
	LP.Component = LedgePointsLS_C.Component;
	RP.Transform = LedgePointsLS_C.RightPoint;
	RP.Component = LedgePointsLS_C.Component;
	SlopeOffset = (UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(LP).Transform.GetLocation().Z - 
	UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(RP).Transform.GetLocation().Z) * AxisValuesC.Y * -1;
	if (SlopeOffset < 0)
	{ SlopeOffset = SlopeOffset * SlopeScale.Y; }
	else
	{ SlopeOffset = SlopeOffset * SlopeScale.X; }

	//Set Detection Origin
	FVector DetectionNormal = FVector(0, 0, 0);
	FVector DetectionOrigin = FVector(0, 0, 0);
	ChooseLedgeFindingTransformC(false, DetectionOrigin, DetectionNormal);
	DetectionOrigin = DetectionOrigin + FVector(0, 0, SlopeOffset) + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(DetectionNormal)) 
	* AxisValuesC.Y * ConstMovementOffset * UKismetMathLibrary::SelectFloat(0, 1, InputLock));
	OutOrigin = DetectionOrigin;
	//Find Ledge
	if (StartNarrowFloorMovementC == false)
	{
		bool LedgeValid, WallHitValid = false;
		FCMC_SingleClimbPointC LPR, RPR, OPR;
		FCMC_LedgeC OutStruct;
		TryCreateLedgeStructureC_Implementation(LedgeValid, LPR, RPR, OPR, WallHitValid, DetectionOrigin, DetectionNormal, FVector2D(0, 0), 0, 45, 1, true, true);
		OutStruct.LeftPoint = ConvertLedgeStructToLS(LPR).Transform;
		OutStruct.RightPoint = ConvertLedgeStructToLS(RPR).Transform;
		OutStruct.Origin = ConvertLedgeStructToLS(OPR).Transform;
		OutStruct.Component = OPR.Component;
		OutLedge = OutStruct;
		return;
	}
	else
	{ OutLedge = LedgePointsLS_C; return; }
}

//CHECK FOOTS INVERSE KINEMATIC 
bool UCpp_DynamicClimbingComponent::CheckFootIkValidC(FTransform Transform, bool ForRightFoot, float TraceUpOffset)
{
	FVector FootOffset = FootsDefOffsetsC.v1;
	FVector V = FVector(0, 0, 0);
	if (ForRightFoot == true)
	{ FootOffset = FootsDefOffsetsC.v2; }
	
	FootOffset = UKismetMathLibrary::Quat_RotateVector(CharacterC->GetMesh()->GetComponentTransform().GetRotation(), FootOffset);
	FootOffset = FootOffset + CharacterC->GetMesh()->GetComponentTransform().GetLocation();
	V = UKismetMathLibrary::Quat_RotateVector(Transform.GetRotation(), UKismetMathLibrary::MakeRelativeTransform(Transform, CharacterC->GetActorTransform()).GetLocation());
	V = V + FootOffset;
	bool HitValid = false;
	FTwoVectors HitTransform;
	UHelpfulFunctionsBPLibrary::ClimbingFootIK(CharacterC, HitValid, HitTransform.v1, HitTransform.v2, CharacterC, V, UKismetMathLibrary::GetForwardVector(Transform.Rotator()), 
	0, 18, FVector2D(8, 45), false, "Thigh_L", "calf_L", "Foot_L", 18, DebugTraceIndexC);
	return HitValid;
}

//GET FOOTS RELATIVE VELOCITY
FVector UCpp_DynamicClimbingComponent::GetFootsRelativeVelocityC()
{
	FVector LocalVelocity = FVector(0, 0, 0);
	FTransform RelFootsT = UKismetMathLibrary::MakeRelativeTransform(UKismetMathLibrary::TLerp(CharacterC->GetMesh()->GetSocketTransform("Foot_L", ERelativeTransformSpace::RTS_World),
	CharacterC->GetMesh()->GetSocketTransform("Foot_R", ERelativeTransformSpace::RTS_World), 0.5), FTransform(UKismetMathLibrary::Conv_RotatorToQuaternion(CharacterC->GetActorRotation()), 
	CharacterC->GetActorLocation(), FVector(1, 1, 1)));
	LocalVelocity = (RelFootsT.GetLocation() - FootsRelativeVelocityC)/ UGameplayStatics::GetWorldDeltaSeconds(this);
	FootsRelativeVelocityC = RelFootsT.GetLocation();
	return LocalVelocity;
}

// [][][][][][][][]  PICKAXE CLIMBING FUNCTIONS  [][][][][][][][]

//CONVERT AXIS TO NAME
FName UCpp_DynamicClimbingComponent::ConvertAxisToNameC()
{
	if (AxisValuesC.X == 1 && AxisValuesC.Y == 0)
	{ return "U"; }
	else if(abs(AxisValuesC.X) == 1 && abs(AxisValuesC.Y) == 1)
	{ return "UL"; }
	else if (abs(AxisValuesC.X) == 0 && abs(AxisValuesC.Y) == 1)
	{ return "L"; }
	else if (AxisValuesC.X < 0 && abs(AxisValuesC.Y) == 0)
	{ return "D"; }
	return "U";
}

//CHECK PLAYER CAN MOVE TO WALL
bool UCpp_DynamicClimbingComponent::CheckPlayerCanMoveToWallC(bool Check, FCALS_ComponentAndTransform TransformWS, FCALS_ComponentAndTransform& ReturnTransformWS)
{
	if (Check == false)
	{ ReturnTransformWS = TransformWS; return false;}
	FHitResult BoxResult = {};
	const ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DebugTraceIndexC > 1)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	FTransform WallT = ConvertWallToCapPositionC(TransformWS).Transform;
	const bool BoxValid = UKismetSystemLibrary::BoxTraceSingle(CharacterC, WallT.GetLocation() + (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4) +
		(UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4 * -0.08), WallT.GetLocation() + (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4)
		+ (UKismetMathLibrary::GetUpVector(WallT.Rotator()) * DefCapsuleSizeC.Y * 0.4 * 0.08), FVector(10, DefCapsuleSizeC.X * 0.84, 25), WallT.Rotator(), Channel, false, ToIgnore, TraceType,
		BoxResult, true, FLinearColor::Black, FLinearColor::Red, 0.15);
	if (BoxValid == true)
	{ ReturnTransformWS = TransformWS; return false; }
	FHitResult SphereResult = {};
	const bool SphereValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, CharacterC->GetActorLocation(), WallT.GetLocation(), 8, Channel, false, ToIgnore, TraceType, SphereResult,
		false, FLinearColor::Gray, FLinearColor::Red, 0.18);
	if (SphereValid == true)
	{ ReturnTransformWS = TransformWS; return false; }
	ReturnTransformWS = TransformWS;
	return true;
}

//CONVERT WALL TO CAPSULE POSITION
FCALS_ComponentAndTransform UCpp_DynamicClimbingComponent::ConvertWallToCapPositionC(FCALS_ComponentAndTransform TransformWS)
{
	FCALS_ComponentAndTransform Cap;
	Cap.Transform = FTransform(TransformWS.Transform.Rotator(), TransformWS.Transform.GetLocation() + UKismetMathLibrary::GetForwardVector(TransformWS.Transform.Rotator())
	* ((DefCapsuleSizeC.X + ConstCapsuleOffsetBetWallC) * -1), FVector(1, 1, 1));
	Cap.Component = TransformWS.Component;
	return Cap;
}

// TRY FIND WALL TANGENT FOR CHARACTER
void UCpp_DynamicClimbingComponent::TryFindTangentForWallC(bool& ReturnValid, FCALS_ComponentAndTransform& TransformWS, FVector FindingLocation, 
	FVector FindingDirection, float FindingLength, float FirstRadius, float DistanceOffsetScale, int VerticalAccuracy, FVector2D CapsuleSize)
{
	//Default Trace Parameters
	const ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(PickaxeClimbChannelC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugTraceIndexC > 0)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DebugTraceIndexC > 1)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	//Run First Trace
	FHitResult FTHR = {};
	const bool FirstValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, FindingLocation, FindingLocation + (FindingDirection * FindingLength), FirstRadius, Channel, false, ToIgnore,
	TraceType, FTHR, true, FLinearColor(0.2f, 0.0f, 0.5f, 1.0f), FLinearColor(0.0f, 0.4f, 1.0f, 1.0f), 0.05);
	if (FirstValid == false)
	{ ReturnValid = false; return; }

	FVector WallImpactWithOffset = FVector(0, 0, 0);
	FVector WallForwardVector = FVector(0, 0, 0);

	WallImpactWithOffset = UKismetMathLibrary::FindClosestPointOnLine(FTHR.ImpactPoint, UKismetMathLibrary::VLerp(FTHR.TraceStart, FTHR.TraceEnd, 0.5), FindingDirection);
	WallImpactWithOffset = FTHR.ImpactPoint + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(FTHR.ImpactPoint, WallImpactWithOffset)) * 
	(UKismetMathLibrary::Vector_Distance(FTHR.ImpactPoint, WallImpactWithOffset) * UKismetMathLibrary::SelectFloat(DistanceOffsetScale, 0, UKismetMathLibrary::Vector_Distance(FTHR.ImpactPoint, WallImpactWithOffset) > 8));
	WallForwardVector = UHelpfulFunctionsBPLibrary::NormalToVector(FTHR.Normal);

	//Init Local Variables
	FVector TStart, TEnd, AvgNormalL, AvgNormalR = FVector(0, 0, 0);
	FTwoVectors AvgHits, CurrentHit;
	TArray<FTwoVectors> HitArray; //Array Variable
	FHitResult SR = {};
	int HitCounter = 0;

	for (int i = 0; i <= 1; i++)
	{
		for (int j = 0; j <= VerticalAccuracy; j++)
		{
			TStart = WallImpactWithOffset + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallForwardVector)) * CapsuleSize.X * 0.6 * UKismetMathLibrary::SelectFloat(-1, 1, i > 0));
			TStart = TStart + FVector(0, 0, UKismetMathLibrary::MapRangeClamped(j * 1.0, 0, VerticalAccuracy * 1.0, -40, 40));

			const bool SphereValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TStart + (WallForwardVector * -25.0), TStart + (WallForwardVector * 22), 4, Channel, false, 
			ToIgnore, TraceType, SR, true, FLinearColor::Red, FLinearColor::Yellow, 0.07);
			if (SphereValid == true)
			{
				HitCounter = HitCounter + 1;
				CurrentHit = FTwoVectors(SR.ImpactPoint, UKismetMathLibrary::SelectVector(UKismetMathLibrary::VLerp(SR.Normal, FTHR.Normal, abs(UKismetMathLibrary::Dot_VectorVector(SR.Normal, 
				FVector(0, 0, 1)))), SR.Normal, abs(UKismetMathLibrary::Dot_VectorVector(SR.Normal, FVector(0, 0, 1))) > 0.55));
				HitArray.Emplace(CurrentHit);
				AvgHits = AvgHits + CurrentHit;
				if (i == 0)
				{ AvgNormalR = AvgNormalR + CurrentHit.v2; }
				else
				{ AvgNormalL = AvgNormalL + CurrentHit.v2; }
			}
		}
	}
	if (HitArray.Num() > 3)
	{
		FCALS_ComponentAndTransform OutputTransform;
		FHitResult LastResult;

		if (UKismetMathLibrary::Dot_VectorVector(AvgNormalL / 3, AvgNormalR / 3) < 0.5)
		{ ReturnValid = false; return; }

		OutputTransform.Transform = FTransform(UKismetMathLibrary::MakeRotFromX((AvgHits.v2 / HitCounter) * -1.0), AvgHits.v1 / HitCounter, FVector(1, 1, 1));
		OutputTransform.Component = FTHR.GetComponent();

		TStart = OutputTransform.Transform.GetLocation() + (UKismetMathLibrary::GetForwardVector(OutputTransform.Transform.Rotator()) * -1 * (CapsuleSize.X + ConstCapsuleOffsetBetWallC));
		TStart = TStart + FVector(0, 0, -15.0);
		TEnd = TStart + (UKismetMathLibrary::GetUpVector(OutputTransform.Transform.Rotator()) * CapsuleSize.Y * 0.55*-1.0);
		TStart = TStart + (UKismetMathLibrary::GetUpVector(OutputTransform.Transform.Rotator()) * CapsuleSize.Y * 0.55 * 1.0);

		const bool LastTraceValid = UKismetSystemLibrary::SphereTraceSingle(CharacterC, TStart, TEnd, CapsuleSize.X * UKismetMathLibrary::SelectFloat(0.5, 0.85, IsClimbingC), 
		UEngineTypes::ConvertToTraceType(ForClimbingChannelC), false, ToIgnore, TraceType, LastResult, true, FLinearColor::Red, FLinearColor::Gray, 0.06);
		//Final Condition
		if (LastTraceValid == false)
		{
			ReturnValid = IsValid(OutputTransform.Component); //Try Return TRUE
			TransformWS = OutputTransform;
			return;
		}
		else
		{ ReturnValid = false; return; }
	}
	else
	{ ReturnValid = false; return; }
}

// ROPE SWING FUNCTION - FORCE CALCULATION
void UCpp_DynamicClimbingComponent::RopeSwingUpdatePhysicC(UCurveVector* RegulationCurve, float SwingMinRange, float SwingMaxRange, int HandAttachIndex, bool AddtiveCondition, float HysteresisMin,
	float GravityStabilityFactor, float ForceScaleFactor, float InterpSpeedIn, float SphereOriginInterpSpeed, float RadiusInterpSpeed, FVector2D SwingLenghtFactor)
{
	if (RopeHookedConditionC() == false || IsValid(CableSimC) == false) { return; }
	if(AddtiveCondition == false) { return; }

	const float dt = GetWorld()->GetDeltaSeconds();

	//Part 1: Find Collision Index
	TArray<FExposedCableParticle> Particles = CableSimC->GetCableParticlesStructure();
	FVector AnchorPoint = AnchorPointInterpC;
	int CollisionIndex = -1;

	for (int i = HandAttachIndex + 1; i < Particles.Num(); i++)
	{
		if (Particles[i].bIsColliding == true || Particles[i].bIsFree == false)
		{
			if (CheckNormalForPointC(Particles[i]) == true)
			{
				AnchorPoint = Particles[i].Position;
				CollisionIndex = i;
				break;
			}
		}
	}
	if (CollisionIndex == -1) { return; }

	CollisionIndexC = CollisionIndex;

	//Part 2:
	AnchorPointInterpC = UKismetMathLibrary::VInterpTo(AnchorPointInterpC, AnchorPoint, dt, SphereOriginInterpSpeed);

	const TArray<float> StretchArray = CableSimC->GetStretchTolleranceValuePerSegment();
	float StretchValue = StretchArray.Last();
	StretchValue = UKismetMathLibrary::MapRangeClamped(StretchValue, 0.9, 1.1, SwingLenghtFactor.X, SwingLenghtFactor.Y);

	//Part 3:
	float MaxLenght = 0.0;
	const TArray<int> AttachPoints = CableSimC->GetIndicesOfAttachedPoints();
	if (AttachPoints.Num() < 2) { return; }

	const int ParticlesDelta = AttachPoints[2] - AttachPoints[1];
	const float LastSegmet = CableSimC->GetCableInitSegmentsLength()[FMath::Clamp<int>(2, 0, CableSimC->GetCableInitSegmentsLength().Num() - 1)];

	MaxLenght = (LastSegmet / (float)ParticlesDelta) * (CollisionIndex - AttachPoints[1]) * StretchValue;
	MaxLenght = MaxLenght + ConstSwingLenghtOffsetC;
	MaxLenght = FMath::Clamp<float>(MaxLenght, SwingMinRange, SwingMaxRange);

	SwingRadiusSmoothC = UKismetMathLibrary::FInterpTo(SwingRadiusSmoothC, MaxLenght, dt, RadiusInterpSpeed);

	//Part 4: Calculate Regulation Factor From Curve Data. This Values control Force Strenght
	FVector RegulationConstrolFactors = RegulationCurve->GetVectorValue(MaxLenght - (CharacterC->GetActorLocation()-AnchorPointInterpC).Length());
	RegulationConstrolFactors.Y = RegulationConstrolFactors.Y * FMath::Lerp<float>(CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue("CMW_MotionStrenght"), 1, 1.25);

	//Part 5: Calculate Target Final Force For Player
	FVector ExtoritionForce = FVector::Zero();

	if (UKismetMathLibrary::Vector_Distance(CharacterC->GetActorLocation(), AnchorPointInterpC) > (MaxLenght - HysteresisMin))
	{
		FVector Direction = CharacterC->GetActorLocation() - AnchorPointInterpC;
		Direction.Normalize();

		const float Scale = UKismetMathLibrary::Dot_VectorVector(CharacterC->GetVelocity(), CharacterC->GetActorLocation() - AnchorPointInterpC);
		float ForceStrenght = Scale / (MaxLenght * RegulationConstrolFactors.X); // By using this Player can increase distance between anchor point
		ForceStrenght = ForceStrenght + (UKismetMathLibrary::Vector_Distance(AnchorPointInterpC, CharacterC->GetActorLocation()) * RegulationConstrolFactors.Y); // By using this Player can reduce distance between anchor point

		ExtoritionForce = Direction * ForceStrenght * (1 / dt) * CharacterC->GetCharacterMovement()->Mass * ForceScaleFactor * -1.0;
	}

	//Part 6: Smoothing Force
	if ((CharacterC->GetActorLocation().Z - AnchorPoint.Z) < 0.0)
	{
		TargetForceC = UKismetMathLibrary::VInterpTo(TargetForceC, ExtoritionForce, dt, InterpSpeedIn);
	}
	else
	{
		TargetForceC = UKismetMathLibrary::VInterpTo(TargetForceC, FVector(0,0,0), dt, InterpSpeedIn);
	}
	
	//Part 7: Calculate Gravity Stabilization Target Force (Optional)
	FVector TargetGravityForce = FVector::Zero();

	if (GravityStabilityFactor > 0)
	{
		float GravityReduceStrenght = CharacterC->GetCharacterMovement()->Mass * CharacterC->GetCharacterMovement()->GetGravityZ();
		GravityReduceStrenght = GravityReduceStrenght * (UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetUpVector(UKismetMathLibrary::FindLookAtRotation(CharacterC->GetActorLocation(), 
			AnchorPointInterpC)), FVector(0, 0, 1)) / 1.5707);

		GravityReduceStrenght = GravityReduceStrenght * -1.0 * GravityStabilityFactor;

		if (CharacterC->GetActorLocation().Z - AnchorPointInterpC.Z >= 0.0) { GravityReduceStrenght = 0.0; }

		TargetGravityForce = FVector(0, 0, GravityReduceStrenght);
	}
	GravityStabilityForceC = UKismetMathLibrary::VInterpTo(GravityStabilityForceC, TargetGravityForce, dt, InterpSpeedIn);

	//Part 8: Apply Final Forces To Character
	CharacterC->GetCharacterMovement()->AddForce(UKismetMathLibrary::ClampVectorSize(TargetForceC + GravityStabilityForceC, -1000000, 1000000));
	return;
}

// ROPE SWING FUNCTION - TRY FIND HOOK ACTOR
bool UCpp_DynamicClimbingComponent::TryFindHookPointC(AActor*& HookActor, TEnumAsByte<EObjectTypeQuery> TraceObject, float FindingRadius, float CapsuleHeightScale, 
	FVector Direction, float DistancePioryty, int DrawDebug)
{
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DrawDebug > 0) { TraceType = EDrawDebugTrace::ForOneFrame; }
	if (DrawDebug > 1) { TraceType = EDrawDebugTrace::ForDuration; }
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(CharacterC);

	TArray< TEnumAsByte<EObjectTypeQuery>> Objects;
	Objects.Add(TraceObject);

	const FVector TStart = CharacterC->GetActorLocation() + FVector(0,0,40) + (Direction * FindingRadius * 0.5);

	TArray<FHitResult> HitsResult;
	const bool HitValid = UKismetSystemLibrary::SphereTraceMultiForObjects(CharacterC, TStart, TStart + (Direction * FindingRadius * 0.5 * CapsuleHeightScale), FindingRadius, 
		Objects, false, ToIgnore, TraceType, HitsResult, true, FLinearColor::Red, FLinearColor::Yellow, 0.2);

	AActor* HitActor = nullptr;
	//Define Pioriting Arrays
	TArray<AActor*> HitActorsArray; 
	TArray<float> WeightArray;

	for (FHitResult SingleHit : HitsResult)
	{
		HitActor = SingleHit.GetActor();

		if (HitActor && HitActor->Implements<UALS_HookActorInterface>())
		{
			IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(HitActor); //Get Interface
			
			bool ValidActor = false;
			HookInterface->Execute_HAFSI_Get_IsHookActor(HitActor, ValidActor);
			if (ValidActor == true)
			{
				HookInterface->Execute_HAFSI_Get_ItsCurrentUsed(HitActor, ValidActor);
				if (ValidActor == false)
				{
					const float DistanceMap = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(HitActor->GetActorLocation(), CharacterC->GetActorLocation()), 100.0, 
						UKismetMathLibrary::Vector_Distance(SingleHit.TraceStart, SingleHit.TraceEnd) + FindingRadius + 50.0, 0.0, 1.0);

					const float RotationMap = 1.0 - (UKismetMathLibrary::Dot_VectorVector(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(HitActor->GetActorLocation(), 
						CharacterC->GetActorLocation())), UKismetMathLibrary::GetForwardVector(CharacterC->GetControlRotation())) * -1.0);

					ToIgnore.Add(HitActor);
					FHitResult SecondResult;
					TEnumAsByte<ETraceTypeQuery> Channel = ETraceTypeQuery::TraceTypeQuery1;

					const bool SecondHit = UKismetSystemLibrary::SphereTraceSingle(CharacterC, CharacterC->GetActorLocation(), HitActor->GetActorLocation(), 25, Channel, false, ToIgnore, 
						TraceType, SecondResult, true, FLinearColor::Black, FLinearColor::Red, 0.25);

					if (SecondHit == false)
					{
						HitActorsArray.Add(HitActor);
						WeightArray.Add(FMath::Lerp<float>(RotationMap, DistanceMap, DistancePioryty));
					}
				}
			}
		}
	}
	if (HitActorsArray.Num() == 0) { HookActor = nullptr;  return false; }

	float MinInArray = 0.0; int MinIndex = 0;
	UKismetMathLibrary::MinOfFloatArray(WeightArray, MinIndex, MinInArray);

	HookActor = HitActorsArray[MinIndex];

	if (IsValid(HookActor) == false) { return false; }

	for (AActor* A : HitActorsArray)
	{
		if (HookActor != A)
		{
			//Clear Widget For Other Actors
			IALS_HookActorInterface* HookInterface = Cast<IALS_HookActorInterface>(A); //Get Interface
			HookInterface->Execute_HAFSI_Play_AnimOut(A);
		}
	}

	return IsValid(HookActor);
}

void UCpp_DynamicClimbingComponent::RopeLenghtUpdateC(UCurveVector* ForceCurve, float RopeMinLeght, float RopeMaxLenght, int HandAttachIndex, FName TimerName, float TimerMaxTime,
	float UpForceStrenght, int ExpandDivite, FVector2D InterpSpeedRange)
{
	if (IsValid(ForceCurve) == false || IsValid(CableSimC) == false) { return; }

	const float dt = GetWorld()->GetDeltaSeconds();

	TArray<FVector> PLocations;
	CableSimC->GetCableParticleLocations(PLocations);
	const FVector P1 = PLocations[HandAttachIndex];
	// Section 1:
	if (UKismetMathLibrary::Vector_Distance(AnchorPointInterpC, P1) < RopeMinLeght && CableSimC->CableLength < RopeMinLeght)
	{ TargetRopeLenghtC = FMath::Clamp<float>(TargetRopeLenghtC + 10, RopeMinLeght, RopeMaxLenght); }

	//Section 2:
	const float NewTargetValue = FMath::Clamp<float>(TargetRopeLenghtC + 10, RopeMinLeght, RopeMaxLenght);
	float InterpSpeedValue = FMath::Lerp<float>(InterpSpeedRange.X, InterpSpeedRange.Y, CharacterC->GetMesh()->GetAnimInstance()->GetCurveValue(TEXT("CMW_MotionStrenght")));

	//Section 3:
	if (TargetRopeLenghtC > RopeMinLeght)
	{
		const float NewLeght = UKismetMathLibrary::FInterpTo(CableSimC->CableLength, NewTargetValue, dt, InterpSpeedValue);
		CableSimC->UpdateCableLength(NewLeght, true);
	}
	//Section 4: stretching the rope
	float TimeElapsed = UKismetSystemLibrary::K2_GetTimerElapsedTime(this, TimerName.ToString());
	if (TimeElapsed != -1 && CollisionIndexC > 0 && CollisionIndexC != CableSimC->NumSegments)
	{
		const float TimeMapped = ForceCurve->GetVectorValue(UKismetMathLibrary::MapRangeClamped(TimeElapsed, 0.0, TimerMaxTime, 0.0, 1.0)).Y;
		FVector Force = FVector(CharacterC->GetVelocity().X, CharacterC->GetVelocity().Y, 0.0) * TimeMapped;
		Force = Force + FVector(0, 0, TimeMapped * UpForceStrenght);

		CableSimC->ApplyConstForceToParticle(Force, HandAttachIndex + 1, true, CableSimC->NumSegments / 4, 0.1);
	}
	else
	{
		CableSimC->ReduceForceForParticles(0, HandAttachIndex + 1 + (CableSimC->NumSegments / 4) + 2, 6, dt);
	}
	return;
}


void UCpp_DynamicClimbingComponent::ReducingVelocityWhenSwingC(float ReductionDampingFactor, float VelocityTrigger, float SwingingDampingFactor, int ActionIndex)
{
	if (IsValid(CharacterC) == false) { return; }

	if (bIsFallingStartedC == false && CharacterC->GetVelocity().Length() > VelocityTrigger)
	{
		if (CableSimC->GetAnyPointIsColliding(6, CableSimC->NumSegments - 1) == true)
		{
			const FVector Impulse = CharacterC->GetVelocity() * -1.0 * ReductionDampingFactor;
			CharacterC->GetCharacterMovement()->AddImpulse(Impulse, true);
		}
	}
	else
	{
		if (ActionIndex < 8) {
			const FVector Impulse = CharacterC->GetVelocity() * FMath::Clamp<float>(1.0 - SwingingDampingFactor, 0.0, 1.0);
			CharacterC->GetCharacterMovement()->AddImpulse(Impulse, false);
		}
	}
	return;
}


void UCpp_DynamicClimbingComponent::UpdateAirControlC(int ActionIndex, float RopeMinLenght, float SwingMinRange, float SwingMaxRange, FVector2D AirControlRange, 
	float SwingMinBias, float InterpToSpeed, float ReduceToZeroSpeed)
{
	const float dt = GetWorld()->GetDeltaSeconds();

	if (ActionIndex >= 8)
	{
		CharacterC->GetCharacterMovement()->AirControl = UKismetMathLibrary::FInterpTo(CharacterC->GetCharacterMovement()->AirControl, 0.0, dt, ReduceToZeroSpeed);
	}
	else
	{
		float TargetValue = 0.0;
		const int FreeParticles = FMath::Clamp<int>(CableSimC->NumSegments - CollisionIndexC, 1, 100);
		if (SwingRadiusSmoothC < (SwingMinRange + SwingMinBias) / (float)FreeParticles)
		{
			TargetValue = 0.0;
		}
		else
		{
			const float MappedRange = UKismetMathLibrary::MapRangeClamped(TargetRopeLenghtC, RopeMinLenght, FMath::Clamp<float>(RopeMinLenght + 250.0, 0.0, SwingMaxRange), 0.0, 1.0);
			TargetValue = FMath::Lerp<float>(AirControlRange.X, AirControlRange.Y, MappedRange);
			CharacterC->GetCharacterMovement()->AirControl = UKismetMathLibrary::FInterpTo(CharacterC->GetCharacterMovement()->AirControl, TargetValue, dt, InterpToSpeed);
		}

	}
	return;
}

