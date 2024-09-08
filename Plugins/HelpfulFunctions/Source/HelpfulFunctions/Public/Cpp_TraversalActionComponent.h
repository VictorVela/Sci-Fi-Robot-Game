

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HelpfulFunctionsBPLibrary.h"
#include "Cpp_LadderClimbingComponent.h"
#include "Cpp_DynamicClimbingComponent.h"
#include "TraversalActionsParamsData.h"
#include "MantleAssetData.h"
//include engine
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
//Enums Collision
#include "Engine/EngineTypes.h"
//Main
#include "Cpp_TraversalActionComponent.generated.h"

UENUM(BlueprintType)
enum class CALS_TraversalAction : uint8
{
	None,
	Hurdle,
	Vault,
	Mantle,
	HighVault,
	ToLedgeHold,
};

UENUM(BlueprintType)
enum class CALS_MantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch,
	None,
};



USTRUCT(BlueprintType)
struct FTraversalCollisionCheck
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ActionIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform FrontLedgeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform BackLedgeTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasBackFloor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform BackFloorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BackLedgeHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ChoosedMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1.0;
};


USTRUCT(BlueprintType)
struct FTraversalChooserValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CALS_TraversalAction ActionType = CALS_TraversalAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GaitIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth = 0.0;
};

USTRUCT(BlueprintType)
struct FTraversalTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLedgeHeight = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLedgeHeight = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReachDistance = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardTraceRadius = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownwardTraceRadius = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalizationLenght = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxForwarIterations = 2;
};

USTRUCT(BlueprintType)
struct FTraversalGroundInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GlobalOrigin = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalRadius = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ForwardOffsetOrigin = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardOffsetRadius = 0.0;
};

USTRUCT(BlueprintType)
struct FTraversalJumpTrajectory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform StartPositionLS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform LandPositionLS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPrimitiveComponent* Component = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform StartPositionWS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCALS_ComponentAndTransform LandPositionWS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTraversalGroundInfo StartGroundType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTraversalGroundInfo LandGroundType;
};

USTRUCT(BlueprintType)
struct FMantleChooserValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CALS_MantleType MantleType = CALS_MantleType::FallingCatch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GaitIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VelocityZ = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MantleHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0;
};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_TraversalActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_TraversalActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "Character"))
	ACharacter* RefChar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References"))
	float DefJumpMaxZ = 400;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References"))
	float DefAirControlBoostMultipler = 1.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References"))
	float DefAirControlBoostVelocity = 1.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Debug Settings"))
	int DebugTraceIndex = 0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Debug Settings"))
	float DrawDebugTime = 0.5;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Debug Settings"))
	bool DrawDebugShapes = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Debug Settings"))
	float ShapesDebugTime = 1;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))
	FTraversalTraceSettings GroudedTraceSettings;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))
	FTraversalTraceSettings InAirTraceSettings;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))
	FTraversalTraceSettings NoneTraceSettings;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))
	TArray<AActor*> ActorsToIgnoreList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))  // <- New States Chooser Method. Then we need to set this in variable. Now states chooser parameters is store in data asset
	UTraversalActionsParamsData* TraversalActionsDataGrounded;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings")) // <- New States Chooser Method. Then we need to set this in variable. Now states chooser parameters is store in data asset
	UTraversalActionsParamsData* TraversalActionsDataInAir;

	//UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))  <- Do not more show this variable in blueprint
	TMap<int, FTraversalStateEvaluation> TraversalActionStatesGrounded;

	//UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))  <- Do not more show this variable in blueprint
	TMap<int, FTraversalStateEvaluation> TraversalActionStatesInAir;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Settings"))
	float BackLedgeSearchLenght = 140.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Predictable Jump Settings"))
	FVector2D UpOffsetRangePerPoint = FVector2D(-120, 90);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Predictable Jump Core"))
	bool UsePredictableJumps = true;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Predictable Jump Core"))
	bool StartedPredictableJump = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Predictable Jump Core"))
	FTraversalJumpTrajectory JumpTrajectory;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Action Core"))
	bool StartedTraversalAction = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Action Core"))
	bool StartedTraversalClimb = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Action Core"))
	FTraversalCollisionCheck TraversalData;

	// previously Traversal Transforms Data - Used to calculate offset for Character when is climb on movable object
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Traversal Action Core"))
	FTraversalCollisionCheck PrevTraversalData;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	bool StartedMantleAction = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	FCALS_ComponentAndTransform MantleLedge;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	FCALS_ComponentAndTransform VaultLedge;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	FCALS_ComponentAndTransform MantleCapsulePos;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	CALS_MantleType MantleActionType = CALS_MantleType::None;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Mantle Action"))
	UMantleAssetData* SelectedMantleAsset;

public:	

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float GetShapesDrawTime();

	bool MakeSureConditionIsRequired(int ConditionIndex, TArray<bool> ConditionsValidList);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Check Traversal Action", Keywords = "Traversal Parkour system", AdvancedDisplay = "TraceChannel"))
	bool CheckTraversalActionFast(FTraversalCollisionCheck& CheckingResult, FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel);

	virtual bool CheckTraversalActionFast_Implementation(FTraversalCollisionCheck& CheckingResult, FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Calculate Land Position For Jump", Keywords = "Traversal Parkour system", AdvancedDisplay = "TraceChannel"))
	bool CalculateLandPositionForJump(FCALS_ComponentAndTransform& LandPosition, FCALS_ComponentAndTransform& BeginPosition, FVector& ReturnVelocity, FVector StartPositionOffset, float ProjectileRadius, 
		bool DrawDebug, float GroundDetectionRadius, float MaxVelocityZ, float AirControlBoostMultipler, float AirControlBoostVelocity, float VelocityBias, ETraceTypeQuery TraceChannel);

	virtual bool CalculateLandPositionForJump_Implementation(FCALS_ComponentAndTransform& LandPosition, FCALS_ComponentAndTransform& BeginPosition, FVector& ReturnVelocity, FVector StartPositionOffset, float ProjectileRadius,
		bool DrawDebug, float GroundDetectionRadius, float MaxVelocityZ, float AirControlBoostMultipler, float AirControlBoostVelocity, float VelocityBias, ETraceTypeQuery TraceChannel);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Get Reach Offset", Keywords = "Traversal Parkour system"))
	float GetReachOffset();

	virtual float GetReachOffset_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Get Player Movement Input", Keywords = "Traversal Parkour system"))
	FVector GetPlayerMovementInput();

	virtual FVector GetPlayerMovementInput_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Try Activate Traversal Action", Keywords = "Traversal Parkour system"))
	bool TryActivateTraversalAction(float& Duration, UAnimMontage*& SelectedMontage, FTraversalCollisionCheck InCheckingResult);

	bool TryActivateTraversalAction_Implementation(float& Duration, UAnimMontage*& SelectedMontage, FTraversalCollisionCheck InCheckingResult);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Try Activate And Start Traversal For Climb", Keywords = "Traversal Parkour system"))
	bool TryActivateTraversalForClimb(float& Duration, int& ActionIndex, FTraversalCollisionCheck InCheckingResult, FCALS_ComponentAndTransform LedgeL, FCALS_ComponentAndTransform LedgeR, FCALS_ComponentAndTransform LedgeOrigin);

	bool TryActivateTraversalForClimb_Implementation(float& Duration, int& ActionIndex, FTraversalCollisionCheck InCheckingResult, FCALS_ComponentAndTransform LedgeL, FCALS_ComponentAndTransform LedgeR, FCALS_ComponentAndTransform LedgeOrigin);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Play Traversal Anim Action", Keywords = "Traversal Parkour system"))
	void PlayTraversalAnimAction(FTraversalCollisionCheck OurParams);

	virtual void PlayTraversalAnimAction_Implementation(FTraversalCollisionCheck OurParams);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Try Activate Predictable Jump", Keywords = "Traversal Parkour system", AdvancedDisplay = "TraceChannel"))
	bool TryActivatePredictableJump(ETraceTypeQuery TraceChannel, bool& StartNormalJump);

	virtual bool TryActivatePredictableJump_Implementation(ETraceTypeQuery TraceChannel, bool& StartNormalJump);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Predictable Jump Stop", Keywords = "Traversal Parkour system"))
	bool PredictableJumpStop(int& LandAnimIndex);

	virtual bool PredictableJumpStop_Implementation(int& LandAnimIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Update Traveral States Data", Keywords = "Traversal Parkour system"))
	bool UpdateTraveralStatesData();

	virtual bool UpdateTraveralStatesData_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core| Mantle", meta = (DisplayName = "Check Mantle", Keywords = "Traversal Parkour system", AdvancedDisplay = "TraceChannel,CapHeightScale,CapRadiusScale,CapSizeByDef,VelocityOffsetScale,LedgeFindingRadius,DebugTime"))
	bool CheckMantle(float& MantleHeight, FCALS_ComponentAndTransform& MantleLedgeWS, FCALS_ComponentAndTransform& VaultLedgeWS, FCALS_ComponentAndTransform& MantleCapsuleWS, CALS_MantleType& MantleType,
		FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel, float CapHeightScale = 0.85, float CapRadiusScale = 0.9, bool CapSizeByDef = false, float VelocityOffsetScale = 1.0, float LedgeFindingRadius = 2.0, float DebugTime = 0.5);

	virtual bool CheckMantle_Implementation(float& MantleHeight, FCALS_ComponentAndTransform& MantleLedgeWS, FCALS_ComponentAndTransform& MantleCapsuleWS, FCALS_ComponentAndTransform& VaultLedgeWS, CALS_MantleType& MantleType,
		FTraversalTraceSettings TraceSettings, ETraceTypeQuery TraceChannel, float CapHeightScale = 0.85, float CapRadiusScale = 0.9, bool CapSizeByDef = false, float VelocityOffsetScale = 1.0, float LedgeFindingRadius = 2.0, float DebugTime = 0.5);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core| Mantle", meta = (DisplayName = "Check And Start Mantle", Keywords = "Traversal Parkour system"))
	bool CheckAndStartMantle(FTraversalTraceSettings InTraceSettings);

	virtual bool CheckAndStartMantle_Implementation(FTraversalTraceSettings InTraceSettings);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core| Mantle", meta = (DisplayName = "Mantle Stop", Keywords = "Traversal Parkour system"))
	bool MantleStop();

	virtual bool MantleStop_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Traversal Core| Mantle", meta = (DisplayName = "Mantle Start", Keywords = "Traversal Parkour system"))
	bool MantleStart(float MantleHeight, FCALS_ComponentAndTransform MantleLedgeWS, FCALS_ComponentAndTransform VaultLedgeWS, FCALS_ComponentAndTransform MantleCapsuleWS, CALS_MantleType MantleType);

	virtual bool MantleStart_Implementation(float MantleHeight, FCALS_ComponentAndTransform MantleLedgeWS, FCALS_ComponentAndTransform VaultLedgeWS, FCALS_ComponentAndTransform MantleCapsuleWS, CALS_MantleType MantleType);



	UFUNCTION(BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Convert States Data Asset To Map", Keywords = "Traversal Parkour system"))
	void ConvertStatesDataAssetToMap(const UTraversalActionsParamsData* DataGrounded, const UTraversalActionsParamsData* DataInAir);

	UFUNCTION(BlueprintPure, Category = "Traversal Core", meta = (DisplayName = "Choose Traversal Action State", Keywords = "Traversal Parkour system"))
	int ChooseTraversalActionState(bool& StateValid, const TMap<int, FTraversalStateEvaluation> StatesData, FTraversalCollisionCheck CheckingResult);

	UFUNCTION(BlueprintPure, Category = "Traversal Core", meta = (DisplayName = "Normalize Ledge To Object Center", Keywords = "Traversal Parkour system"))
	bool NormalizeLedgeToObjectCenter(FVector& NewPosition, UPrimitiveComponent* InComponent, FVector DownLocation, FVector InNormal, FTraversalTraceSettings InTraceSettings);
	
	UFUNCTION(BlueprintPure, Category = "Traversal Core", meta = (DisplayName = "Get Location From Base", Keywords = "Traversal Parkour system"))
	FVector GetLocationFromBase(float ZOffset = 0.0);

	UFUNCTION(BlueprintPure, Category = "Traversal Core", meta = (DisplayName = "Floor To Capsule Location", Keywords = "Traversal Parkour system"))
	FVector ToCapsuleLocation(FVector FloorLocation, float ZOffset = 2.0, bool ByDef = false);

	UFUNCTION(BlueprintCallable, Category = "Traversal Core", meta = (DisplayName = "Update Offsets For Character", Keywords = "Traversal Parkour system"))
	bool UpdateOffsetsForCharacter(FTransform& FrontLedgeOffset, FTransform& BackLedgeOffset, FTransform& BackFloorOffset, FTraversalCollisionCheck CurrentData, float MaxVectorSize = 1000);


	UFUNCTION(BlueprintPure, Category = "Traversal Core", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Try Get Traversal Component", Keywords = "Traversal Component"))
	static UCpp_TraversalActionComponent* TryGetTraversalComponent(UObject* WorldContextObject, ACharacter* Target);

private:

	UCpp_LadderClimbingComponent* TryGetLadderComponent();
	UCpp_DynamicClimbingComponent* TryGetClimbingComponent();
	void DrawSimpleDebugCircle(FVector Location, FRotator Rotation, float Radius, FColor Color, float DrawTime, float Thickness);

};
