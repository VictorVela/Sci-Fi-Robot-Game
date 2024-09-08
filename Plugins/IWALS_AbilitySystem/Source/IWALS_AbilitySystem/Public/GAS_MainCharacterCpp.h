

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "IWALS_BaseAttributeSet.h"
#include "IWALS_GameplayAbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include <GameplayEffectTypes.h>
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "GAS_MainCharacterCpp.generated.h"

USTRUCT(BlueprintType)
struct FMovementSettingsStrafe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector WalkSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector RunSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	FVector SprintSpeed = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	UCurveVector* MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	UCurveFloat* RotationRateCurve = nullptr;
};


UCLASS()
class IWALS_ABILITYSYSTEM_API AGAS_MainCharacterCpp : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGAS_MainCharacterCpp();

	//Define Base Variables For ALS Character
	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		FVector2D DefCapsuleSizeC = FVector2D(30,90);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool IsMovingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool HasMovementInputC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool IsStartedMovementOnTargetC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True", DisplayName = "Start Interaction With Dynamic Prop C"))
		bool InteractionWithPropC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool IsLayBackC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool IsSwimmingC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
		bool RagdollOnGroundC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
		bool RagdollFaceUpC = false;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		FVector AccelerationC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		FVector RelativeAcceleractionC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		FRotator LastVelocityRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		FRotator LastMovementInputRotationC = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
		FVector PreviousVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Ragdoll System", meta = (AllowPrivateAccess = "True"))
		FVector LastRagdollVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		float SpeedC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		float MovementInputAmountC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		float MovementSpeedDifferenceC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		float AimYawRateC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
		float PreviousAimYawC = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Cached Variables", meta = (AllowPrivateAccess = "True"))
		FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		UCurveFloat* StrafeSpeedMapCurveC = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
		FMovementSettingsStrafe CurrentMovementSettingsC;

	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
		FVector FloorVelocityC = FVector(0,0,0);

	UPROPERTY(BlueprintReadWrite, Category = "Movement System", meta = (AllowPrivateAccess = "True"))
		FVector PrevFloorVelocityC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, Category = "Essential Information", meta = (AllowPrivateAccess = "True"))
		bool OverlayStateLeavingStarted = false;

		bool CanUpdateFromDesiredOverlay = false;

	/* Experimental function. Improves the behavior of the capsule in a non-inertial reference frame (the floor moves relative to the world space) */
	UPROPERTY(BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "True"))
		bool CorrectNonInertialFloor = true;

		FVector PrevFloorLocation = FVector(0, 0, 0);
		bool AddedFloorForce = false;

	UFUNCTION(BlueprintPure, Category = "Movement System", meta = (DisplayName = "Get Target Speed With Strafe", Keywords = "Movement"))
		virtual float GetTargetSpeedWithStrafeC(FVector SpeedVector);

	UFUNCTION(BlueprintPure, Category = "Movement System", meta = (DisplayName = "Get Mapped Speed", Keywords = "Movement"))
		virtual float GetMappedSpeedC(float SpeedScale = 1.0);

	UFUNCTION(BlueprintCallable, Category = "Rotation System", meta = (DisplayName = "Smooth Character Rotation", Keywords = "Rotation"))
		virtual void SmoothCharacterRotationC(FRotator TargetRotation = FRotator(0,0,0), float ActorInterpSpeed = 10.0);

	UFUNCTION(BlueprintPure, Category = "Rotation System", meta = (DisplayName = "Calculate Grounded Rotation Speed", Keywords = "Rotation"))
		virtual float CalculateGroundedRotationSpeedC(float Scale = 1.0, FVector2D YawScaleRange = FVector2D(1.0,3.0));

	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Get Control Vectors", Keywords = "Others"))
		virtual void GetControlVectorsC(FVector& ForwardVector, FVector& RightVector);

	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Get Capsule Base Location", Keywords = "Others"))
		virtual FVector GetCapsuleBaseLocationC(float ZOffset);

	UFUNCTION(BlueprintPure, Category = "Utility", meta = (DisplayName = "Floor To Capsule Location", Keywords = "Others"))
		virtual FVector FloorToCapsuleLocationC(FVector BaseLocation, float ZOffset, bool ByDefSize);




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
		UIWALS_GameplayAbilitySet* AbilitiesData;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
		TSubclassOf<class UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY()
		class UIWALS_BaseAttributeSet* Attributes;

	// Gameplay Tag Functions
	// -----------------------------------------------
	UFUNCTION(BlueprintPure, Category = "Abilities", meta = (DisplayName = "Convert Literal Name To Tag", Keywords = "Gameplay Tag"))
		virtual FGameplayTag ConvertLiteralNameToTag(FName TagName);

	UFUNCTION(BlueprintPure, Category = "Abilities", meta = (DisplayName = "Get Sub Tag", Keywords = "Gameplay Tag"))
		virtual FString GetSubTag(const FGameplayTag& Tag, int32 DesiredDepth);

	UFUNCTION(BlueprintPure, Category = "Abilities", meta = (DisplayName = "Is Tag Leaf", Keywords = "Gameplay Tag"))
		virtual bool IsTagLeaf(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "Abilities", meta = (DisplayName = "Switch On Owned Tags", Keywords = "Gameplay Tag"))
		virtual bool SwitchOnOwnedTags(const FGameplayTag& NewState);

	UFUNCTION(BlueprintCallable, Category = "Abilities", meta = (DisplayName = "Switch On Owned Tags With Ignore", Keywords = "Gameplay Tag"))
		virtual bool SwitchOnOwnedTagsWithIgnore(const FGameplayTag& NewState, const FGameplayTagContainer& DoNotEdit);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void InitializeAttributes();
	virtual void GiveAbilities();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Input", meta = (DisplayName = "Try Create Inputs Binds For GAS", Keywords = "Inputs Player"))
		virtual void TryCreateInputsGAS();


	// For Overlay System
	UFUNCTION(BlueprintImplementableEvent)
		void DoWhenOverlayLeaving(float DeltaSecond);

	UFUNCTION(BlueprintImplementableEvent)
		void OverlayLeavingFinshed();
};
