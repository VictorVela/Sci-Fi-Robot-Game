

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cpp_FallDamageAndSliding.generated.h"

USTRUCT(BlueprintType)
struct FCharMovementValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		float  WalkableFloorAngle = 45.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		float FrictionFactor = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		float BrakingFriction = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		float AirControl = 0.0;
};

USTRUCT(BlueprintType)
struct FSlidingConditionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		int MovementState = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		int MovementAction = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		int LocomotionMode = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharMovement")
		FVector Velocity = FVector(0,0,0);
};



UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_FallDamageAndSliding : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_FallDamageAndSliding();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//References Variable
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "CharacterC"))
		ACharacter* CharC = nullptr;

	//Sliding Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sliding", DisplayName = "StartSlidingC"))
		bool StartSlidingC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sliding", DisplayName = "AngleConditionC"))
		float AngleConditionC = 0.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sliding", DisplayName = "SmoothAccelerationC"))
		FVector SmoothAccelerationC = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sliding", DisplayName = "DebugTraceIndexC"))
		int DebugTraceIndexC = 0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sliding", DisplayName = "SavedCharMoveValuesC"))
		FCharMovementValues SavedCharMoveValuesC = {};

	
	//Sliding Protected Functions
	UFUNCTION(BlueprintCallable, Category = "Sliding", meta = (WorldContext = "WorldContextObject", DisplayName = "Simple Trajectory", Keywords = "Trajectory"))
		virtual void SimpleTrajectoryC(bool& NotNull, TArray<FHitResult>& ReturnAllHits, TArray<float>& ReturnDistances, TArray<FHitResult>& ReturnWalkableHits, 
		ACharacter* Char, FVector StartLocation, FVector StartDirection, int Iterations = 6, float OffsetScale = 3.0, FVector Velocity = FVector(0, 0, 0), 
		FVector Acceleration = FVector(0, 0, 0), bool ReturnOnlyValid = false);
	//Sliding System - Check Can Start Sliding
	UFUNCTION(BlueprintCallable, Category = "Sliding", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Can Start Sliding", Keywords = "Sliding"))
		virtual bool CheckCanStartSlidingC(FVector Acc = FVector(0,0,0), float SlopeDiffTollerance = 2.0);
	//Sliding System - Update Character Movement Braking Friction Factor Durning Sliding
	UFUNCTION(BlueprintCallable, Category = "Sliding", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Char Friction Durning Sliding", Keywords = "Sliding"))
		virtual void UpdateCharFrictionDurningSliding(float& ReturnTheta, UCharacterMovementComponent* CMove, float InterpSpeed=10.0, float FrictionScale=1.0);
	//Sliding System - Update Character Movement Walkable Z Angle Durning Sliding
	UFUNCTION(BlueprintCallable, Category = "Sliding", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Char Walk Angle Durning Sliding", Keywords = "Sliding"))
		virtual void UpdateCharWalkAngleDurningSliding(UCharacterMovementComponent* CMove, TArray<FHitResult> HitData, float ClampMin = 20.0, float ClampMax = 55.0, float InterpSpeed = 6.0);

	UFUNCTION(BlueprintCallable, Category = "Sliding", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Angle Correct Durning Sliding", Keywords = "Sliding"))
		virtual void UpdateAngleCorrectDurningSliding(float& ReturnDot, TArray<FHitResult> HitData, float Theta = 45.0, float SlidingMinAngle = 20.0, float InterpSpeed = 4.0, bool CheckHitValid = false);

	//Fall Damage Protected Function
	UFUNCTION(BlueprintCallable, Category = "Fall Damage", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Can Mantle On Edge", Keywords = "Mantle Damage"))
		virtual void CheckCanMantleOnEdge(bool& CanStart, FVector& TargetCapLocation,FVector& SecondPoint,FVector& HitNormal1, FVector& HitNormal2, UPrimitiveComponent*& HitComponent, 
		ECollisionChannel TChannel = ECollisionChannel::ECC_Visibility, int DrawDebugIndex = 0, float RoomCheckCapScale = 0.9);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Sliding", meta = (DisplayName = "Sliding Condition", Keywords = "Sliding"))
		bool SlidingConditionC();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sliding", meta = (DisplayName = "Prepare To Sliding", Keywords = "Sliding"))
		void PrepareToSlidingC(bool CanStart = true);

};

