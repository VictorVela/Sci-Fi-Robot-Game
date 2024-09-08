

#include "Cpp_CoverSystemV2.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "HelpfulFunctionsBPLibrary.h"

// Sets default values for this component's properties
UCpp_CoverSystemV2::UCpp_CoverSystemV2()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCpp_CoverSystemV2::BeginPlay()
{
	Super::BeginPlay();
	CharC = Cast<ACharacter>(this->GetOwner()); //Get Character Owner
	if (IsValid(CharC) == true)
	{
		DefCapSizeC = FVector2D(CharC->GetCapsuleComponent()->GetScaledCapsuleRadius(), CharC->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}
	
}


bool UCpp_CoverSystemV2::TryStartCoverSystemC(FHitResult& ReturnWall, int DebugIndex)
{
	float Dire = 0.0;
	FHitResult LineResult = {};
	bool LineValid = false;
	FVector TStart, TEnd = FVector(0, 0, 0);
	//Configure Base Trace Settings
	TArray<AActor*> ToIgnore; ToIgnore.Add(CharC);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (DebugIndex == 1) TraceType = EDrawDebugTrace::ForOneFrame;
	else if (DebugIndex == 2) TraceType = EDrawDebugTrace::ForDuration;
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);

	for (int i = 0; i <= 1; i++)
	{
		if (i == 0)
		{ Dire = TConfigS.LeftOffset; }
		else
		{ Dire = TConfigS.RightOffset; }

		TStart = CharC->GetActorLocation() + (CharC->GetActorRightVector() * Dire);
		TEnd = TStart + (CharC->GetActorForwardVector() * (CharC->GetCapsuleComponent()->GetScaledCapsuleRadius() + TConfigS.OffsetToRadius));
		TStart = TStart + (CharC->GetActorForwardVector() * 20.0);

		LineValid = UKismetSystemLibrary::LineTraceSingle(CharC, TStart, TEnd, Channel, false, ToIgnore, TraceType, LineResult, true, FLinearColor::Yellow, FLinearColor::Green, 0.1);
		if (LineValid == false)
		{	ReturnWall = LineResult; return false; }
		ACharacter* AsCharacter = Cast<ACharacter>(LineResult.GetActor());
		if (IsValid(LineResult.GetActor()) == false)
		{ ReturnWall = LineResult; return false; }
		if (LineResult.GetActor()->ActorHasTag(TConfigS.TagNameToIgnore) == true || IsValid(AsCharacter) == true)
		{ ReturnWall = LineResult; return false; }
	}
	bool ValidPoints = false;
	UHelpfulFunctionsBPLibrary::FindNearPointToWallSurface(CharC, CharC, ValidPoints, LineResult, CharC->GetActorForwardVector(), 7, true, 15.0, FVector(0, 0, 0), DebugIndex);
	if (ValidPoints == false)
	{ return false; }
	ACharacter* AsChar = Cast<ACharacter>(LineResult.GetActor());
	ReturnWall = LineResult;
	if (IsValid(AsChar) == true)
	{ return false; }
	TStart = LineResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(LineResult.Normal) * DefCapSizeC.X * -1.06);
	TStart = FVector(TStart.X, TStart.Y, CharC->GetActorLocation().Z);
	if (UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnore(CharC, CharC, TStart, { CharC }, 0.98, 0.9, false) == true)
	{ 
		return true; 
	}
	return false;
}

void UCpp_CoverSystemV2::MoveCharToWallC(bool& WallValid, FHitResult& ReturnWall,float MoveToWallSpeed, int DebugIndex)
{
	FHitResult WallResult = {};
	bool ValidPoints = false;
	UHelpfulFunctionsBPLibrary::FindNearPointToWallSurface(CharC, CharC, ValidPoints, WallResult, UHelpfulFunctionsBPLibrary::NormalToVector(CachedWallNormalC), 7, 
		true, 15.0, FVector(0, 0, 0), DebugIndex);
	if (ValidPoints == false)
	{
		CachedWallNormalC = UKismetMathLibrary::VInterpTo_Constant(CachedWallNormalC, CharC->GetActorForwardVector() * -1, UGameplayStatics::GetWorldDeltaSeconds(this), 15.0);
		ReturnWall = WallResult;
		WallValid = false;
		return;
	}
	CachedWallNormalC = UKismetMathLibrary::VLerp(CachedWallNormalC, WallResult.Normal, 0.5);
	// Step 1) Calculate Delta multiply
	float DeltaMultiply = WallPullingStrengthC * UKismetMathLibrary::SelectFloat(0.25, 1.0, Axis2d.X < 0) * UGameplayStatics::GetWorldDeltaSeconds(this);
	// Step 2) Set Target Capsule Location And Rotation
	FVector TargetCapLoc = FVector(0, 0, 0);
	FRotator TargetCapRot = FRotator(0, 0, 0);
	TargetCapLoc = WallResult.ImpactPoint + (UHelpfulFunctionsBPLibrary::NormalToVector(WallResult.Normal) * CharC->GetCapsuleComponent()->GetScaledCapsuleRadius() * -1.0);
	TargetCapRot = UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(SmoothWallNormalC(WallResult.ImpactPoint, WallResult.Normal, MoveDirectionAxisC)));
	// Step 3) Calculate Delta Offset
	FVector DeltaOffset = UKismetMathLibrary::Quat_UnrotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(TargetCapRot), TargetCapLoc - CharC->GetActorLocation());
	DeltaOffset = FVector(DeltaOffset.X, DeltaOffset.Y * MoveToWallSpeed, 0.0) * DeltaMultiply;
	DeltaOffset = UKismetMathLibrary::Quat_RotateVector(UKismetMathLibrary::Conv_RotatorToQuaternion(TargetCapRot), DeltaOffset);
	// Step 4) Smoothing Delta Offset
	SmoothOffsetToWallC = UKismetMathLibrary::TInterpTo(SmoothOffsetToWallC, FTransform(TargetCapRot, DeltaOffset, FVector(1, 1, 1)), UGameplayStatics::GetWorldDeltaSeconds(this), 10.0);
	// Step 5) Calculate Delta Rotation From Wall Rotation And Current Actor Rotation
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TargetCapRot, CharC->GetActorRotation()) * UKismetMathLibrary::FClamp(DeltaMultiply * 0.5, 0.0, 1.0);
	// Step 6) Apply Location And Rotation Offset To Character
	CharC->AddActorWorldOffset(SmoothOffsetToWallC.GetLocation(), true);
	CharC->AddActorWorldRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(DeltaRotation), false);
	ReturnWall = WallResult;
	WallValid = ValidPoints;
	return;
}

FVector UCpp_CoverSystemV2::SmoothWallNormalC(FVector TraceLocation, FVector Normal, float DirectionAxis)
{
	if (UseSmoothingNormalsC == false || DirectionAxis==0.0) //Check the Smoothig Function Is Activated In Config
	{ return Normal; } //If Not return Normal From Input

	int HitNumber = 0;
	FVector TStart, TEnd, NormalsAvg = FVector(0, 0, 0);
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (TraceDebugIndexC == 1) TraceType = EDrawDebugTrace::ForOneFrame;
	else if (TraceDebugIndexC == 2) TraceType = EDrawDebugTrace::ForDuration;
	for (int i = 0; i <= 2; i++)
	{
		float a, b = 0.0;
		a = (i * 1.0 * 18.0) + 18.0;
		b = -35.0;
		TStart = TraceLocation + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(Normal))) * DirectionAxis * a);
		TEnd = TStart + (UHelpfulFunctionsBPLibrary::NormalToVector(Normal) * 12.0);
		TStart = TStart + (UHelpfulFunctionsBPLibrary::NormalToVector(Normal) * b);
		FHitResult LineResult = {};
		bool LineValid = UKismetSystemLibrary::LineTraceSingle(CharC, TStart, TEnd, Channel, false, { CharC }, TraceType, LineResult, true, FLinearColor::Gray, FLinearColor::Blue, 0.15);
		if (LineValid == true && LineResult.bStartPenetrating == false && UKismetMathLibrary::Dot_VectorVector(Normal, LineResult.Normal) > 0.65)
		{
			NormalsAvg = NormalsAvg + LineResult.Normal;
			HitNumber = HitNumber + 1;
		}

	}
	if (HitNumber > 0)
	{
		return UKismetMathLibrary::VLerp(NormalsAvg / HitNumber, Normal, 0.25);
	}
	return Normal;
}

bool UCpp_CoverSystemV2::CheckCanStartMoveC(float& DesiredWalkAxis, bool WallValid, FHitResult WallHit, FCoverSystemMoveConfig Config)
{
	float MoveDesiredDirection = 0.0;

	if (WallValid == false || (Axis2d.Y == 0 && Axis2d.X == 0))
	{
		DesiredWalkAxis = 0.0;
		return false;
	}

	if (CharC->GetVelocity().Length() > 0.5)
	{
		FVector NormalizedVelo = CharC->GetVelocity();
		NormalizedVelo.Normalize(0.001);
		MoveDesiredDirection = UKismetMathLibrary::Dot_VectorVector(NormalizedVelo, 
		UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(UHelpfulFunctionsBPLibrary::NormalToVector(WallHit.Normal))));
		MoveDesiredDirection = UKismetMathLibrary::SelectFloat(-1.0, 1.0, MoveDesiredDirection < 0.0) * abs(Axis2d.Y);
	}
	else
	{
		MoveDesiredDirection = Axis2d.Y;
	}

	FTwoVectors HitData = {};
	HitData.v1 = WallHit.ImpactPoint; HitData.v2 = WallHit.Normal; //Save Input Wall Hit ImpactPoint And Normal To Variable

	float RightOffset = 0.0;
	float DotAvg = 0.0;
	bool NextWallHitValid = false;
	//Trace Config Parameters
	FVector TStart, TEnd = FVector(0, 0, 0);
	ETraceTypeQuery Channel = UEngineTypes::ConvertToTraceType(Config.TraceChannel);
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	if (TraceDebugIndexC == 1) TraceType = EDrawDebugTrace::ForOneFrame;
	else if (TraceDebugIndexC == 2) TraceType = EDrawDebugTrace::ForDuration;

	float  indexf = 0.0;
	for (int i = 0; i <= Config.LoopCount; i++)
	{
		if (NextWallHitValid == true)
		{ RightOffset = Config.FirstRightOffset; }
		else
		{ RightOffset = (i * 1.0 * Config.MultiplyOffsetRight) + Config.FirstRightOffset; }
		TStart = HitData.v1 - FVector(0, 0, 2.5) + (UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(HitData.v2)) * MoveDesiredDirection * -1.0 * RightOffset);
		TEnd = TStart + (UHelpfulFunctionsBPLibrary::NormalToVector(UKismetMathLibrary::VLerp(HitData.v2, WallHit.Normal, Config.LerpWithInputWall)) * Config.TOffsetBackward);
		TStart = TStart + (UHelpfulFunctionsBPLibrary::NormalToVector(UKismetMathLibrary::VLerp(HitData.v2, WallHit.Normal, Config.LerpWithInputWall)) * Config.TOffsetForward);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, (UHelpfulFunctionsBPLibrary::NormalToVector(UKismetMathLibrary::VLerp(HitData.v2, WallHit.Normal, Config.LerpWithInputWall) * 15.0)).ToString());
		FHitResult SphereResult = {};
		NextWallHitValid = UKismetSystemLibrary::SphereTraceSingle(CharC, TStart, TEnd, Config.TraceRadius, Channel, false, { CharC }, TraceType, SphereResult, true, 
		FLinearColor(0.35, 0.0, 0.0, 1.0), FLinearColor(1.0, 0.4, 0.0, 1.0), 0.05);
		if (NextWallHitValid == true)
		{
			HitData.v1 = SphereResult.ImpactPoint + FVector(0,0,2.5); 
			HitData.v2 = SphereResult.Normal;
			DotAvg = DotAvg + UKismetMathLibrary::Dot_VectorVector(SphereResult.Normal, WallHit.Normal);
		}
		else
		{ DesiredWalkAxis = MoveDesiredDirection; return false; } //if hit is not valid the character cant move in this direction
		indexf = i * 1.f;
	}

	if (UHelpfulFunctionsBPLibrary::CapsuleHaveRoomWithIgnore(CharC, CharC, ConvertWallToCapPositionC(HitData.v1, HitData.v2), { CharC }, 0.6, 0.9, TraceDebugIndexC>0) == true)
	{
		if ((UKismetMathLibrary::SafeDivide(DotAvg, indexf + 1.0) > Config.AngleTollerance) == true)
		{
			DesiredWalkAxis = MoveDesiredDirection;
			return true;
		}
	}
	DesiredWalkAxis = MoveDesiredDirection;
	return false;
}

FVector UCpp_CoverSystemV2::ConvertWallToCapPositionC(FVector WallLocation, FVector WallNormal)
{
	FVector f = FVector(WallLocation.X, WallLocation.Y, CharC->GetActorLocation().Z);
	f = f + UHelpfulFunctionsBPLibrary::NormalToVector(WallNormal) * -1.0 * CharC->GetCapsuleComponent()->GetScaledCapsuleRadius();
	return f;
}

void UCpp_CoverSystemV2::UpdateDirectionValuesC(bool IsCrouch, int DirectionState, float PerDirectionInterpSpeed, float MovementInterpSpeed, float SmoothAxisInterpSpeed)
{
	float IsCrouchFloat = 0.0;
	if (IsCrouch == true) IsCrouchFloat = 1.0;

	AlphaPerDirectionC = UKismetMathLibrary::Vector2DInterpTo(AlphaPerDirectionC, FVector2D(abs(FMath::Clamp<float>(MoveDirectionAxisC, -1.0, 0)) * IsCrouchFloat,
	abs(FMath::Clamp<float>(MoveDirectionAxisC, 0, 1.0)) * IsCrouchFloat), UGameplayStatics::GetWorldDeltaSeconds(this), PerDirectionInterpSpeed);
	
	MoveDirectionSmoothC = UKismetMathLibrary::Vector2DInterpTo(MoveDirectionSmoothC, FVector2D(MoveDirectionAxisC, 0.0), UGameplayStatics::GetWorldDeltaSeconds(this), MovementInterpSpeed);

	float StateAsFloat = 0.0;
	if (DirectionState == 1) StateAsFloat = 1.0;
	if (DirectionState == 2) StateAsFloat = -1.0;

	AxisSmoothC = UKismetMathLibrary::FInterpTo(AxisSmoothC, StateAsFloat, UGameplayStatics::GetWorldDeltaSeconds(this), SmoothAxisInterpSpeed);
}

// Called every frame
void UCpp_CoverSystemV2::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Axis2d = SetAxisValuesForCpp();
	// ...
}

FVector2D UCpp_CoverSystemV2::SetAxisValuesForCpp_Implementation()
{
	return FVector2D(1.0, 0.0);
}

void UCpp_CoverSystemV2::StartCoverModeC_Implementation(bool& StartedCover)
{
	return;
}

void UCpp_CoverSystemV2::FinishCoverModeC_Implementation()
{
	return;
}

