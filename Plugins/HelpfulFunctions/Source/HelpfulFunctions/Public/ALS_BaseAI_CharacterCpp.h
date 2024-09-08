

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ALS_DamageConfigData.h"
#include "ALS_HumanAI_InterfaceCpp.h"
#include "ALS_BaseAI_CharacterCpp.generated.h"

UENUM(BlueprintType)
enum class CALS_Gait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
};

UENUM(BlueprintType)
enum class CALS_MovementState : uint8
{
	None UMETA(DisplayName = "None"),
	Grounded UMETA(DisplayName = "Grounded"),
	InAir UMETA(DisplayName = "In Air"),
	Mantling UMETA(DisplayName = "Mantling"),
	Ragdoll UMETA(DisplayName = "Ragdoll"),
	Crawl UMETA(DisplayName = "Crawl"),
	Prone UMETA(DisplayName = "Prone")
};

UENUM(BlueprintType)
enum class CALS_OverlayState : uint8
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	Pistol1H,
	Pistol2H,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel,
	Rope,
	Axe,
	Knife
};

UENUM(BlueprintType)
enum class CALS_RotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

UENUM(BlueprintType)
enum class CALS_Stance : uint8
{
	Standing,
	Crouching
};

UENUM(BlueprintType)
enum class CALS_MovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
};

USTRUCT(BlueprintType)
struct FCALSMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;
};


UCLASS()
class HELPFULFUNCTIONS_API AALS_BaseAI_CharacterCpp : public ACharacter, public IALS_HumanAI_InterfaceCpp
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AALS_BaseAI_CharacterCpp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	ACharacter* Self;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FVector PrevVelocityC = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	float PrevAimYawC = 0.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config", meta = (AllowedClasses = "ALS_DamageConfigData"))
	UClass* DamageDataClass;

	// Basic Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FVector AccelerationC = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	float SpeedC = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	bool IsMovingC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FRotator LastVelocityRotationC = FRotator(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FRotator LastMovementInputRotationC = FRotator(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	float AimYawRateC = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	float MovementInputAmountC = 0.0f;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	bool HasMovementInputC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FRotator SmoothTargetAimingC = FRotator(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FRotator TargetRotationC = FRotator(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FRotator InAirRotationC = FRotator(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	ACharacter* TargetEnemyActorC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	float DetectedEnemyTimeC = 0.0f;
	// States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True")) //Zmieniono np. TEnumAsByte<CALS_Gait> na CALS_Gait !!!
		CALS_Gait GaitC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_Gait DesiredGaitC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_OverlayState OverlayStateC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_Stance StanceC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_RotationMode RotationModeC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_MovementAction MovementActionC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	CALS_MovementState MovementStateC;
	//Structures
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	FCALSMovementSettings CurrentMovementSettingsC;

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Is Character", Keywords = "ALS Character"))
	bool IsCharacter();

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Calc Aiming Rotation Fast", Keywords = "ALS Character"))
	static FRotator CalculateAimingRotatation();

	UFUNCTION(BlueprintCallable, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Grounded Rotation Fast", Keywords = "ALS Character"))
	virtual void CalculateGroundedRotation();

	UFUNCTION(BlueprintCallable, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Smoothed Character Rotation Fast", Keywords = "ALS Character"))
	virtual void SmoothedCharRotation(FRotator Target, float TargetInterpSpeedConst, float ActorInterpSpeedSmooth, bool UpdateControl);

	UFUNCTION(BlueprintCallable, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Limit Rotation Fast", Keywords = "ALS Character"))
	virtual void LimitRotationFast(float AimYawMin, float AimYawMax, float InterpSpeed);

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Can Update Moving Rotation Fast", Keywords = "ALS Character"))
	virtual bool CanUpdateRotation();

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Calculate Grounded Rotation Rate Fast", Keywords = "ALS Character"))
	virtual float CalcGroundedRotationRate();

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Mapped Speed Fast", Keywords = "ALS Character"))
	virtual float GetMappedSpeedFast();

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Can Sprint Fast", Keywords = "ALS Character"))
	virtual bool CalcCanSprint();

	UFUNCTION(BlueprintCallable, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Dynamic Movement Settings Fast", Keywords = "ALS Character"))
	virtual void UpdateMovementSettings(CALS_Gait AllowedGait, FCALSMovementSettings CurrentMovement);

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Allowed Gait Fast", Keywords = "ALS Character"))
	virtual CALS_Gait GetAllowedGaitFast();

	UFUNCTION(BlueprintPure, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Actual Gait Fast", Keywords = "ALS Character"))
	virtual CALS_Gait GetActualGaitFast(CALS_Gait AllowedGait);

	UFUNCTION(BlueprintCallable, Category = "ALS Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Cover Fast", Keywords = "ALS Character"))
	virtual void FindCoverFast(FVector StartLocation, FVector StartDirection, int DebugIndex, bool& CanCover, float& WallHeigh, FVector& Impact, FVector& Normal);
};
