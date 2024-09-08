// Copyright Jakub W, All Rights Reserved. 

#include "Cpp_MatchedMontageComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "HelpfulFunctionsBPLibrary.h"

// Sets default values for this component's properties
UCpp_MatchedMontageComponent::UCpp_MatchedMontageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCpp_MatchedMontageComponent::BeginPlay()
{
	Super::BeginPlay();
	CharC = Cast<ACharacter>(this->GetOwner());
	// ...
	if (MotionCurvesNameC.Num() == 0) //Default Motion Curves Name
	{
		MotionCurvesNameC.Emplace(FName("Extract_Root-Loc_Y"));
		MotionCurvesNameC.Emplace(FName("Extract_Root-Loc_X"));
		MotionCurvesNameC.Emplace(FName("Extract_Root-Loc_Z"));
		MotionCurvesNameC.Emplace(FName("Extract_Root-Rot_Yaw"));
	}
	SetComponentTickEnabled(false); //On Begin Play Disable Tick Event
}


void UCpp_MatchedMontageComponent::MatchingMultiplePointsC()
{
	//STEP 0)
	int TimeIndex = 0;
	float ScalarInterpAlpha = 0.0;
	if (IsValid(AnimInst) == false || IsValid(CharC) == false)
	{ return; }
	FName ManyPointCurveName = FName("MM_PointWeigth");
	if (UKismetMathLibrary::Round(AnimInst->GetCurveValue(ManyPointCurveName)) > TPI)
	{ TimeIndex = UKismetMathLibrary::Round(AnimInst->GetCurveValue(ManyPointCurveName)); }
	else
	{ TimeIndex = TPI; }
	float TimeNormalized = UKismetMathLibrary::MapRangeClamped(GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent2), 0.0, TimelineRange, 1, 0);
	// STEP 1) Get Interpolation Curves
	FVector CurvesValue1 = FVector(0, 0, 0);
	float VX, VY, VZ = 0.0;
	bool VXV, VYV, VZV = false;
	if (UseMotionCurvesFromAnimation == true)
	{
		VXV = GetAnimCurveC(VX, MotionCurvesNameC[0], true, LockDecreasingXYZ.X);
		VYV = GetAnimCurveC(VY, MotionCurvesNameC[1], true, LockDecreasingXYZ.Y);
		VZV = GetAnimCurveC(VZ, MotionCurvesNameC[2], true, LockDecreasingXYZ.Z);
		CurvesValue1 = FVector(VX, VY, VZ);
	}
	else
	{
		CurvesValue1 = GetCustomCurveValueC(TimeNormalized);
	}
	//STEP 2)
	FVector CurvesValue2 = FVector(0, 0, 0);
	VX = UKismetMathLibrary::MapRangeClamped(CurvesValue1.X, (TimeIndex * 1.0) / 10.0, (TimeIndex + 1.0) / 10.0, 0.0, 1.0);
	VY = UKismetMathLibrary::MapRangeClamped(CurvesValue1.Y, (TimeIndex * 1.0) / 10.0, (TimeIndex + 1.0) / 10.0, 0.0, 1.0);
	VZ = UKismetMathLibrary::MapRangeClamped(CurvesValue1.Z, (TimeIndex * 1.0) / 10.0, (TimeIndex + 1.0) / 10.0, 0.0, 1.0);
	CurvesValue2 = FVector(VX, VY, VZ);
	//STEP 3)
	if (ApplyTimelineAlphaAtEnd == true)
	{
		ScalarInterpAlpha = UKismetMathLibrary::Ease(0.0, 1.0, UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(TimeNormalized, OutBlendDuration.X, 
		OutBlendDuration.Y, 0.0, 1.0), 0.0, TimeNormalized >= OutBlendDuration.X), EEasingFunc::EaseInOut, 2.0, 2);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FVector(ScalarInterpAlpha,0,0).ToString());
	}
	UpdateLockingDecreasingCurvesC(CurvesValue1.X, CurvesValue1.Y, CurvesValue1.Z);
	//STEP 4)
	FTransform TStart, TMiddle, TEnd = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
	if (TargetPoints.IsValidIndex(TimeIndex) == true && TargetPoints.Num() > 0)
	{ 
		TStart = ConvertToWorldCondition(TargetPoints[TimeIndex]).Transform;
		TMiddle = ConvertToWorldCondition(TargetPoints[UKismetMathLibrary::Clamp(TimeIndex+1, 0, TargetPoints.Num() - 1)]).Transform;
		TEnd = ConvertToWorldCondition(TargetPoints[UKismetMathLibrary::Clamp(TargetPoints.Num() - 1,0,20)]).Transform;
	}
	FTransform NewCharTransform = ExtractedInterpolationC(TStart, UKismetMathLibrary::TLerp(TMiddle, TEnd, ScalarInterpAlpha), CurvesValue2.X, CurvesValue2.Y, 
	CurvesValue2.Z, CurvesValue2.Z, ScalarInterpAlpha, Rotation180Direction, UseInterFor180Rot);
	CharC->SetActorLocationAndRotation(NewCharTransform.GetLocation(), NewCharTransform.GetRotation(), false);
	//STEP 5)
	if (SetZeroToVelocity == true)
	{ CharC->GetCharacterMovement()->Velocity = FVector(0, 0, 0); }
	if (CurvesValue2.X >= 0.995 && CurvesValue2.Y >= 0.995 && CurvesValue2.Z >= 0.995 && FinishWhenAlphaAtEnd == true && TimeIndex >= UKismetMathLibrary::Clamp(TargetPoints.Num() - 2, 0, 20))
	{
		FinishMatchedMontageC(); //Finish Montage
	}
	MatchingMotionNormalized(TimeNormalized);
	TPI = TimeIndex;
}

void UCpp_MatchedMontageComponent::MatchingSinglePointsC()
{
	FVector MotionInterpValueNotInvert, MotionInterpValue = FVector(0, 0, 0);
	//STEP 1)
	float VX, VY, VZ = 0.0;
	float AlphaValue = 1.0;
	float ScalarInterpAlpha = 0.0;
	bool VXV, VYV, VZV = false;
	if (UseMotionCurvesFromAnimation == true)
	{
		VXV = GetAnimCurveC(VX, MotionCurvesNameC[0], true, LockDecreasingXYZ.X);
		VYV = GetAnimCurveC(VY, MotionCurvesNameC[1], true, LockDecreasingXYZ.Y);
		VZV = GetAnimCurveC(VZ, MotionCurvesNameC[2], true, LockDecreasingXYZ.Z);
		MotionInterpValue = FVector(VX, VY, VZ);
		if (AnimAlphaCurveName != FName("None") && AnimAlphaCurveName != FName(""))
		{
			GetAnimCurveC(AlphaValue, AnimAlphaCurveName, false, 0.0);
			AlphaValue = UKismetMathLibrary::FClamp(AlphaValue, 0.05, 1.0);
		}
		MotionInterpValue = MotionInterpValue / AlphaValue;
	}
	else
	{
		MotionInterpValue = GetCustomCurveValueC(GetTimelinePlayBackNormalized());
	}
	MotionInterpValueNotInvert = MotionInterpValue;
	//STEP 2)
	if (ApplyTimelineAlphaAtEnd == true)
	{
		ScalarInterpAlpha = UKismetMathLibrary::Ease(0.0, 1.0, UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(GetTimelinePlayBackNormalized(), OutBlendDuration.X,
		OutBlendDuration.Y, 0.0, 1.0), 0.0, GetTimelinePlayBackNormalized() >= OutBlendDuration.X), EEasingFunc::EaseInOut, 2.0, 2);
	}
	UpdateLockingDecreasingCurvesC(MotionInterpValue.X, MotionInterpValue.Y, MotionInterpValue.Z);
	//STEP 3)
	if (TargetPoints.Num() < 2)
	{ return; }
	FTransform TargetCharTransform = ExtractedInterpolationC(ConvertToWorldCondition(TargetPoints[0]).Transform, ConvertToWorldCondition(TargetPoints[1]).Transform, 
	MotionInterpValue.X, MotionInterpValue.Y, MotionInterpValue.Z, GetRotationCurve(RotInterpType), ScalarInterpAlpha, Rotation180Direction, UseInterFor180Rot);
	CharC->SetActorLocationAndRotation(TargetCharTransform.GetLocation(), TargetCharTransform.GetRotation(), false);
	//STEP 4)
	if (SetZeroToVelocity == true)
	{ CharC->GetCharacterMovement()->Velocity = FVector(0, 0, 0); }
	if (MotionInterpValue.X >= 0.998 && MotionInterpValue.Y >= 0.998 && MotionInterpValue.Z >= 0.998 && FinishWhenAlphaAtEnd == true)
	{
		FinishMatchedMontageC(); //Finish Montage Early
	}
	MatchingMotionNormalized(GetTimelinePlayBackNormalized());
	return;
}

//Initialize Matched Montage
void UCpp_MatchedMontageComponent::PlayMatchedMontageManyPointsC(UAnimMontage* MontageAsset, FMatchedMontageManyPoints ConfigStruct)
{
	LockDecreasingXYZ = FVector(0, 0, 0);
	TPI = 0;
	CharC = ConfigStruct.Character;
	AnimInst = ConfigStruct.AnimInstance;
	UseMotionCurvesFromAnimation = ConfigStruct.UseMotionCurvesFromAnimation;
	RemapCurves = ConfigStruct.RemapCurves;
	LocationCurve = ConfigStruct.LocationCurve;
	RotationCurve = ConfigStruct.RotationCurve;
	TargetPoints = ConfigStruct.TargetPoints;
	ConvertToWorld = ConfigStruct.ConvertTransformsToWorld;
	FinishWhenAlphaAtEnd = ConfigStruct.FinishWhenAlphaAtEnd;
	UseInterFor180Rot = ConfigStruct.UseInterFor180Rot;
	OutBlendDuration = ConfigStruct.OutBlendingDuration;
	Rotation180Direction = ConfigStruct.RotationDirection180;
	ApplyTimelineAlphaAtEnd = ConfigStruct.ApplyTimelineAlphaAtEnd;
	float AnimDuration = 0.0;
	if (IsValid(AnimInst) == true)
	{
		AnimDuration = AnimInst->Montage_Play(MontageAsset, ConfigStruct.PlayRate, EMontagePlayReturnType::Duration, ConfigStruct.StartMontageAt, true);
		SetComponentTickEnabled(true);
		TimelineRange = AnimDuration;
		GetWorld()->GetTimerManager().SetTimer(TimelineEvent2, this, &UCpp_MatchedMontageComponent::TimerFunction2, AnimDuration, false);
	}
	return;
}

void UCpp_MatchedMontageComponent::PlayMatchedMontageTwoPointsC(UAnimMontage* MontageAsset, FMatchedMontageTwoPoints ConfigStruct)
{
	LockDecreasingXYZ = FVector(0, 0, 0);
	TargetPoints = {};
	TPI = 0;
	CharC = ConfigStruct.Character;
	AnimInst = ConfigStruct.AnimInstance;
	UseMotionCurvesFromAnimation = ConfigStruct.UseMotionCurvesFromAnimation;
	RemapCurves = ConfigStruct.RemapCurves;
	LocationCurve = ConfigStruct.LocationCurve;
	RotationCurve = ConfigStruct.RotationCurve;
	ConvertToWorld = ConfigStruct.ConvertTransformsToWorld;
	FinishWhenAlphaAtEnd = ConfigStruct.FinishWhenAlphaAtEnd;
	UseInterFor180Rot = ConfigStruct.UseInterFor180Rot;
	OutBlendDuration = ConfigStruct.TimelineAlphaEndConfig;
	Rotation180Direction = ConfigStruct.RotationDirection180;
	RotInterpType = ConfigStruct.CustomRotationInterpType;
	ApplyTimelineAlphaAtEnd = ConfigStruct.ApplyTimelineAlphaAtEnd;
	TargetPoints.Emplace(ConfigStruct.StartTransform);
	TargetPoints.Emplace(ConfigStruct.EndTransform);
	float AnimDuration = 0.0;
	if (IsValid(AnimInst) == true)
	{
		AnimDuration = AnimInst->Montage_Play(MontageAsset, ConfigStruct.PlayRate, EMontagePlayReturnType::Duration, ConfigStruct.StartMontageAt, ConfigStruct.StopAllMontages);
		if (AnimDuration == 0.0)
		{ return; }
		TimelineRange = AnimDuration;
		if (ConfigStruct.NormalizeTimeToAnimLength == false)
		{
			TimelineRange = UKismetMathLibrary::FClamp(UKismetMathLibrary::SafeDivide(ConfigStruct.TimelineLength, ConfigStruct.PlayRate) - ConfigStruct.StartMontageAt, 0.1, 100);
		}
		SetComponentTickEnabled(true);

		GetWorld()->GetTimerManager().SetTimer(TimelineEvent1, this, &UCpp_MatchedMontageComponent::TimerFunction1, TimelineRange, false);
	}
	return;
}

//GET ANIM INST CURVE VALUE
bool UCpp_MatchedMontageComponent::GetAnimCurveC(float& Value, FName CurveName, bool WithLock, float LockVariable)
{
	if (IsValid(AnimInst) == false)
	{ Value = 0.0; return false; }
	TArray<FName> ActiveCurves = {};
	AnimInst->GetActiveCurveNames(EAnimCurveType::AttributeCurve, ActiveCurves);
	if (ActiveCurves.Find(CurveName) == false)
	{
		Value = UKismetMathLibrary::MapRangeClamped(GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent2), 0.0, TimelineRange, 1, 0);
		return false;
	}
	float CurveValue = AnimInst->GetCurveValue(CurveName);
	if (WithLock == true) 
	{
		Value = UKismetMathLibrary::FClamp(CurveValue, UKismetMathLibrary::FClamp(LockVariable - 0.01, 0.0, 10.0), 10.0);
		return true; 
	}
	Value = CurveValue; return true;
}

//GET CUSTOM CURVE VALUE
FVector UCpp_MatchedMontageComponent::GetCustomCurveValueC(float InTime)
{
	if (IsValid(LocationCurve) == false)
	{ return FVector(0, 0, 0); }

	float MinTime, MaxTime = 0.0;
	LocationCurve->GetTimeRange(MinTime, MaxTime);
	return LocationCurve->GetVectorValue(UKismetMathLibrary::MapRangeClamped(InTime, 0, 1, 0, MaxTime));
}

//EXTRACTED INTERPOLATION FUNCTION - CURVED INTERPOLATION
FTransform UCpp_MatchedMontageComponent::ExtractedInterpolationC(FTransform A, FTransform B, float VX, float VY, float VZ, float ROT, float Alpha, float Direction180, bool UseInterpFor180Rot)
{
	FQuat AQuat = A.GetRotation();
	FVector UA = UKismetMathLibrary::Quat_UnrotateVector(AQuat, A.GetLocation());
	FVector UB = UKismetMathLibrary::Quat_UnrotateVector(AQuat, B.GetLocation());
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, UA.ToString());
	float LX, LY, LZ = 0.0;
	LX = UKismetMathLibrary::Lerp(UA.X, UB.X, VX);
	LY = UKismetMathLibrary::Lerp(UA.Y, UB.Y, VY);
	LZ = UKismetMathLibrary::Lerp(UA.Z, UB.Z, VZ);
	FVector TargetLoc = FVector(LX, LY, LZ);
	TargetLoc = UKismetMathLibrary::Quat_RotateVector(AQuat, TargetLoc);
	TargetLoc = UKismetMathLibrary::VLerp(TargetLoc, B.GetLocation(), Alpha);
	// ------------ PODSTAWOWA DEFINICJA ROTACJI --------------------
	//                 FRotator(Pitch,Yaw,Roll)
	// --------------------------------------------------------------
	FRotator TargetRot, RR = FRotator(0, 0, 0);
	if (UseInterFor180Rot == true)
	{
		RR = UKismetMathLibrary::RLerp(A.Rotator(), FRotator(0, B.Rotator().Yaw + Direction180, 0), UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(ROT, 0, 0.5, 0, 1), 1.0, ROT < 0.5), true);
		TargetRot = UKismetMathLibrary::RLerp(RR, B.Rotator(), UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(ROT, 0.5, 1, 0, 1), 0.0, ROT >= 0.5), true);
	}
	else
	{
		TargetRot = UKismetMathLibrary::RLerp(A.Rotator(), B.Rotator(), ROT,true);
	}
	return FTransform(TargetRot, TargetLoc, FVector(1, 1, 1));
}

//UPDATE LOCKING DECREASING CURVES
void UCpp_MatchedMontageComponent::UpdateLockingDecreasingCurvesC(float Y, float X, float Z)
{
	if (X > LockDecreasingXYZ.X)
	{ LockDecreasingXYZ = FVector(X, LockDecreasingXYZ.Y, LockDecreasingXYZ.Z); }
	if (Y > LockDecreasingXYZ.Y)
	{ LockDecreasingXYZ = FVector(LockDecreasingXYZ.X, Y, LockDecreasingXYZ.Z); }
	if (Z > LockDecreasingXYZ.Z)
	{ LockDecreasingXYZ = FVector(LockDecreasingXYZ.X, LockDecreasingXYZ.Y, Z); }
}

//GET TIMELINE PLAYBACK (NORMALIZED OR NOT)
float UCpp_MatchedMontageComponent::GetTimelinePlayBackNormalized()
{
	if (RemapCurves == true)
	{
		return UKismetMathLibrary::MapRangeClamped(GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent1), 0.0, TimelineRange, 1.0, 0);
	}
	return UKismetMathLibrary::MapRangeClamped(GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent1), 0.0, TimelineRange, TimelineRange, 0);
}

//GET INTERPOLATION VALUE FOR ROTATION - YAW
float UCpp_MatchedMontageComponent::GetRotationCurve(int CurveIndex)
{
	float T = GetTimelinePlayBackNormalized();
	if (IsValid(RotationCurve) == true)
	{ return RotationCurve->GetFloatValue(T); }
	else
	{
		if (CurveIndex == 0)
		{ return UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(T, 0.0, 0.3, 0, 1), 1.0, T < 0.3); }
		else if(CurveIndex == 1)
		{ return UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(T, 0.0, 0.5, 0, 1), 1.0, T < 0.5); }
		else
		{ return UKismetMathLibrary::SelectFloat(UKismetMathLibrary::MapRangeClamped(T, 0.0, 0.8, 0, 1), 1.0, T < 0.8); }
	}
}

// CONVERT WORLD TRANSFORM TO LOCAL - BUT ONLY WHEN CONDITION IS TRUE
FCALS_ComponentAndTransform UCpp_MatchedMontageComponent::ConvertToWorldCondition(FCALS_ComponentAndTransform InStructure)
{
	if (ConvertToWorld == true)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, InStructure.Transform.ToString());
		return UHelpfulFunctionsBPLibrary::ConvertLocalToWorldFastMatrix(InStructure);
	}
	return InStructure;
}

// Called every frame
void UCpp_MatchedMontageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (IsValid(AnimInst) == true)
	{
		if (GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent2) != -1)
		{
			MatchingMultiplePointsC(); //Run Matched Montage Update
		}
		else if (GetWorld()->GetTimerManager().GetTimerRemaining(TimelineEvent1) != -1)
		{
			MatchingSinglePointsC(); //Run Matched Montage Update
		}
	}
}


// ------------------ Zadeklaruj funkcje które bêd¹ potrzebne do uruchomienia Timera -----------
//TIMER FUNCTION
void UCpp_MatchedMontageComponent::TimerFunction1()
{
	FinishMatchedMontageC();
}
//TIMER FUNCTION
void UCpp_MatchedMontageComponent::TimerFunction2()
{
	FinishMatchedMontageC();
}
//-----------------------------------------------------------------------------------------------

//Funcje, które mog¹ byæ nadpisanie poprzez blueprint

void UCpp_MatchedMontageComponent::FinishMatchedMontageC_Implementation()
{
	LockDecreasingXYZ = FVector(0, 0, 0);
	SetComponentTickEnabled(false);
	TargetPoints = {};
}

void UCpp_MatchedMontageComponent::MatchingMotionNormalized_Implementation(float CurrentTime)
{
	return;
}

