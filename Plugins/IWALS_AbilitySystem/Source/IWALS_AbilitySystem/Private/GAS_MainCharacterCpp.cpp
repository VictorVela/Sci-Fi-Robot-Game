


#include "GAS_MainCharacterCpp.h"
#include "GameplayTagsManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
//include Kismet
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"
//include math
#include "Math/Vector.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"


// Sets default values
AGAS_MainCharacterCpp::AGAS_MainCharacterCpp()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComp");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	Attributes = CreateDefaultSubobject<UIWALS_BaseAttributeSet>("Attributes");

}

float AGAS_MainCharacterCpp::GetTargetSpeedWithStrafeC(FVector SpeedVector)
{

	if (StrafeSpeedMapCurveC)
	{
		float StrafeSpeedMap = StrafeSpeedMapCurveC->GetFloatValue(abs(UKismetAnimationLibrary::CalculateDirection(this->GetCharacterMovement()->Velocity,GetActorRotation())));
		if (StrafeSpeedMap < 1.0)
		{
			return UKismetMathLibrary::MapRangeClamped(StrafeSpeedMap, 0.0, 1.0, SpeedVector.X, SpeedVector.Y);
		}
		else
		{
			return UKismetMathLibrary::MapRangeClamped(StrafeSpeedMap, 1.0, 2.0, SpeedVector.Y, SpeedVector.Z);
		}
	}
	return 300.0;
}

float AGAS_MainCharacterCpp::GetMappedSpeedC(float SpeedScale)
{
	const float LocWalkSpeed = GetTargetSpeedWithStrafeC(CurrentMovementSettingsC.WalkSpeed);
	const float LocRunSpeed = GetTargetSpeedWithStrafeC(CurrentMovementSettingsC.RunSpeed);
	const float LocSprintSpeed = GetTargetSpeedWithStrafeC(CurrentMovementSettingsC.SprintSpeed);

	if (SpeedC > LocRunSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocRunSpeed, LocSprintSpeed, 2.0, 3.0);
	}
	else if (SpeedC > LocWalkSpeed)
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, LocWalkSpeed, LocRunSpeed, 1.0, 2.0);
	}
	else
	{
		return UKismetMathLibrary::MapRangeClamped(SpeedC, 0, LocWalkSpeed, 0.0, 1.0);
	}
}

void AGAS_MainCharacterCpp::SmoothCharacterRotationC(FRotator TargetRotation, float ActorInterpSpeed)
{
	
	FRotator NewRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->DeltaTimeSeconds, ActorInterpSpeed);
	const FQuat TargetQuat = UKismetMathLibrary::Conv_RotatorToQuaternion(NewRotation);
	SetActorRotation(TargetQuat, ETeleportType::None);
}

float AGAS_MainCharacterCpp::CalculateGroundedRotationSpeedC(float Scale, FVector2D YawScaleRange)
{
	if (IsValid(CurrentMovementSettingsC.RotationRateCurve) == false) { return 15.0; }

	const float InterpRate = CurrentMovementSettingsC.RotationRateCurve->GetFloatValue(GetMappedSpeedC());
	const float YawScale = UKismetMathLibrary::MapRangeClamped(AimYawRateC, 0.0, 300, YawScaleRange.X, YawScaleRange.Y);
	return InterpRate * YawScale * UKismetMathLibrary::SelectFloat(1.2, 1.0, IsStartedMovementOnTargetC);
}

void AGAS_MainCharacterCpp::GetControlVectorsC(FVector& ForwardVector,FVector& RightVector)
{
	ForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0.0, GetControlRotation().Yaw, 0.0));
	RightVector = UKismetMathLibrary::GetRightVector(FRotator(0.0, GetControlRotation().Yaw, 0.0));
}

FVector AGAS_MainCharacterCpp::GetCapsuleBaseLocationC(float ZOffset)
{
	if (GetCapsuleComponent())
	{
		const UCapsuleComponent* CC = GetCapsuleComponent();
		return CC->GetComponentLocation() - (CC->GetUpVector() * (CC->GetScaledCapsuleHalfHeight() + ZOffset));
	}
	return GetActorLocation();
}

FVector AGAS_MainCharacterCpp::FloorToCapsuleLocationC(FVector BaseLocation, float ZOffset, bool ByDefSize)
{
	return BaseLocation + FVector(0, 0, (UKismetMathLibrary::SelectFloat(DefCapsuleSizeC.Y, GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), ByDefSize) + ZOffset));
}

// Called when the game starts or when spawned
void AGAS_MainCharacterCpp::BeginPlay()
{
	Super::BeginPlay();

	DefCapsuleSizeC.X = GetCapsuleComponent()->GetScaledCapsuleRadius();
	DefCapsuleSizeC.Y = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}


// Called every frame
void AGAS_MainCharacterCpp::Tick(float DeltaTime)
{

	// Update Essential Values
	float SafeDelta = GetWorld()->DeltaTimeSeconds;
	if (SafeDelta == 0.0) { SafeDelta = 0.01; }

	AccelerationC = (GetVelocity() - PreviousVelocityC) / SafeDelta;
	MovementSpeedDifferenceC = FVector(GetVelocity().X, GetVelocity().Y, 0.0).Length() - FVector(PreviousVelocityC.X, PreviousVelocityC.Y, 0.0).Length();

	SpeedC = FVector(GetVelocity().X, GetVelocity().Y, 0.0).Length();
	if (SpeedC > 1.0) { IsMovingC = true; }
	else { IsMovingC = false; }

	FVector AccelerationXY = this->GetCharacterMovement()->GetCurrentAcceleration();
	AccelerationXY.Z = 0.0;
	MovementInputAmountC = AccelerationXY.Length() / this->GetCharacterMovement()->GetMaxAcceleration();
	if (MovementInputAmountC > 0.0) { HasMovementInputC = true; }
	else { HasMovementInputC = false; }

	AimYawRateC = abs((GetControlRotation().Yaw - PreviousAimYawC) / SafeDelta);

	IsSwimmingC = this->GetCharacterMovement()->IsSwimming();

	//Update Base Rotation Values
	if (HasMovementInputC == true)
	{
		LastMovementInputRotationC = UKismetMathLibrary::MakeRotFromX(this->GetCharacterMovement()->GetCurrentAcceleration());
	}
	if (IsMovingC == true)
	{
		LastVelocityRotationC = UKismetMathLibrary::MakeRotFromX(GetVelocity());
	}

	//Update Cached Variables
	PreviousVelocityC = GetVelocity();
	PreviousAimYawC = GetControlRotation().Yaw;

	/* Calculate Floor Velocity */
	if (this->GetCharacterMovement()->CurrentFloor.bBlockingHit == true && this->GetCharacterMovement()->CurrentFloor.HitResult.Location.IsNearlyZero() == false)
	{
		FloorVelocityC = (this->GetCharacterMovement()->CurrentFloor.HitResult.Location - PrevFloorLocation) / DeltaTime;
		FloorVelocityC = FloorVelocityC - FVector(this->GetCharacterMovement()->Velocity.X, this->GetCharacterMovement()->Velocity.Y, 0.0);
		PrevFloorLocation = this->GetCharacterMovement()->CurrentFloor.HitResult.Location;
	}
	else
	{
		FloorVelocityC = FVector(0, 0, 0);
		PrevFloorLocation = GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}

	/* Experimental function. Improves the behavior of the capsule in a non-inertial reference frame (the floor moves relative to the world space) */
	if (CorrectNonInertialFloor)
	{
		if (this->GetCharacterMovement()->IsFalling() == true)
		{
			if (AddedFloorForce == false)
			{
				LaunchCharacter(FVector(PrevFloorVelocityC.X, PrevFloorVelocityC.Y, 0.0), false, false);
				AddedFloorForce = true;
			}
		}
		else
		{
			PrevFloorVelocityC = FloorVelocityC;
			AddedFloorForce = false;
		}
	}

	// Overlay Unequip Events
	if (OverlayStateLeavingStarted == true)
	{
		DoWhenOverlayLeaving(DeltaTime);
		CanUpdateFromDesiredOverlay = true;
	}
	if (OverlayStateLeavingStarted == false && CanUpdateFromDesiredOverlay == true)
	{
		CanUpdateFromDesiredOverlay = false;
		OverlayLeavingFinshed();
	}

	Super::Tick(DeltaTime);

}

FGameplayTag AGAS_MainCharacterCpp::ConvertLiteralNameToTag(FName TagName)
{
	return FGameplayTag::RequestGameplayTag(TagName);
}

FString AGAS_MainCharacterCpp::GetSubTag(const FGameplayTag& Tag, int32 DesiredDepth)
{
	FString FullTagName = Tag.ToString();
	TArray<FString> SplitTags;
	FullTagName.ParseIntoArray(SplitTags, TEXT("."));

	// Jeœli DesiredDepth przekracza iloœæ segmentów lub jest ujemny, zwróæ pusty string.
	if (DesiredDepth < 0 || DesiredDepth >= SplitTags.Num())
	{
		return FString();
	}

	// ZnajdŸ odpowiedni¹ g³êbokoœæ od koñca.
	int32 IndexFromEnd = SplitTags.Num() - DesiredDepth - 1;
	if (IndexFromEnd >= 0 && IndexFromEnd < SplitTags.Num())
	{
		return SplitTags[IndexFromEnd];
	}

	return FString();
}

bool AGAS_MainCharacterCpp::IsTagLeaf(const FGameplayTag& Tag)
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	TSharedPtr<FGameplayTagNode> NodePtr = TagsManager.FindTagNode(Tag);
	const FGameplayTagNode* Node = NodePtr.Get();

	if (Node)
	{
		return Node->GetChildTagNodes().Num() == 0;
	}

	return false;
}

bool AGAS_MainCharacterCpp::SwitchOnOwnedTags(const FGameplayTag& NewState)
{
	if (IsValid(AbilitySystemComponent) == false) return false;

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	const bool LastInGroup = IsTagLeaf(NewState);

	TArray<FGameplayTag> TagArray;
	OwnedTags.GetGameplayTagArray(TagArray);

	TArray<FGameplayTag> TagsToRemove = {};

	for (FGameplayTag& Tag : TagArray)
	{
		int SelectDepth = 0;
		if (LastInGroup == true) SelectDepth = 1;
		const FString a = GetSubTag(Tag, 1);
		const FString b = GetSubTag(NewState, SelectDepth);
		if (a == b)
		{
			TagsToRemove.Add(Tag);
		}
	}
	//Convert To TagContainer
	FGameplayTagContainer TagsContainerToRemove;
	for (const FGameplayTag& Tag : TagsToRemove)
	{
		TagsContainerToRemove.AddTag(Tag);
	}
	//Remove Tags
	AbilitySystemComponent->RemoveLooseGameplayTags(TagsContainerToRemove);
	if (LastInGroup == true)
	{
		AbilitySystemComponent->AddLooseGameplayTag(NewState);
	}
	return true;
}

bool AGAS_MainCharacterCpp::SwitchOnOwnedTagsWithIgnore(const FGameplayTag& NewState, const FGameplayTagContainer& DoNotEdit)
{
	if (IsValid(AbilitySystemComponent) == false) return false;

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	const bool LastInGroup = IsTagLeaf(NewState);

	TArray<FGameplayTag> TagArray;
	OwnedTags.GetGameplayTagArray(TagArray);

	TArray<FGameplayTag> TagsToRemove = {};

	for (FGameplayTag& Tag : TagArray)
	{
		int SelectDepth = 0;
		if (LastInGroup == true) SelectDepth = 1;
		const FString a = GetSubTag(Tag, 1);
		const FString b = GetSubTag(NewState, SelectDepth);
		//Find Match
		FGameplayTagContainer TagAsContainer;
		TagAsContainer.AddTag(Tag);

		if (a == b && TagAsContainer.HasAllExact(DoNotEdit) == false)
		{
			TagsToRemove.Add(Tag);
		}
	}
	//Convert To TagContainer
	FGameplayTagContainer TagsContainerToRemove;
	for (const FGameplayTag& Tag : TagsToRemove)
	{
		TagsContainerToRemove.AddTag(Tag);
	}
	//Remove Tags
	AbilitySystemComponent->RemoveLooseGameplayTags(TagsContainerToRemove);
	if (LastInGroup == true)
	{
		AbilitySystemComponent->AddLooseGameplayTag(NewState);
	}
	return true;
}

// Called to bind functionality to input
void AGAS_MainCharacterCpp::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* AGAS_MainCharacterCpp::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGAS_MainCharacterCpp::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AGAS_MainCharacterCpp::GiveAbilities()
{
	//GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, "Pysk", true);
	if (AbilitiesData && AbilitySystemComponent)
	{
		AbilitiesData->GiveAbilities(AbilitySystemComponent, this);
	}
}

void AGAS_MainCharacterCpp::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent->AbilityActorInfo.IsValid() == false)
	{
		GEngine->AddOnScreenDebugMessage(0, 3, FColor::Red, "GAS ERROR - AbilityActorInfo is NOT valid ", true);
		return;
	}
	AbilitySystemComponent->AbilityActorInfo->InitFromActor(this, this, AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	InitializeAttributes();
	GiveAbilities();

}

void AGAS_MainCharacterCpp::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAttributes();
}

void AGAS_MainCharacterCpp::TryCreateInputsGAS()
{
	if (AbilitySystemComponent && InputComponent)
	{
		FTopLevelAssetPath PathToEnum = FTopLevelAssetPath(GetPathNameSafe(UClass::TryFindTypeSlow<UEnum>("EIWALS_AbilityInputBinds")));
		const FGameplayAbilityInputBinds Binds("JumpAction", "AimActionType_2", PathToEnum, static_cast<int32>(EIWALS_AbilityInputBinds::JumpAction), static_cast<int32>(EIWALS_AbilityInputBinds::AimActionType_2));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

