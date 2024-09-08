// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "JakubCablePhysic.h"
#include "DrawDebugHelpers.h"
#include "ALS_HookActorInterface.h"
#include "ModifyClimbingParamsVolume.h"
#include "Cpp_DynamicClimbingComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_DynamicClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_DynamicClimbingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Create Only Cpp Variables
	UCpp_DynamicClimbingComponent* SelfComp;

	// Create Essential Variables

	//Base Bools Values
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Base", DisplayName = "IsClimbingC"))
		bool IsClimbingC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Base", DisplayName = "FreeHangC"))
		bool FreeHangC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Base", DisplayName = "StartNarrowFloorMovementC"))
		bool StartNarrowFloorMovementC = false;
	//Main Action Enum
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Base", DisplayName = "ActionC"))
		CMC_ActionTypeC ActionC = CMC_ActionTypeC::None;
	//Transformations
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Transforms", DisplayName = "LedgePointsLS C"))
		FCMC_LedgeC LedgePointsLS_C;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Transforms", DisplayName = "CachedLedgePointsLS C"))
		FCMC_LedgeC CachedLedgePointsLS_C;
	//Init Values
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "DefCapsuleSizeC"))
		FVector2D DefCapsuleSizeC = FVector2D(0.0,0.0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "NarrowFloorCapRadiusC"))
		float NarrowFloorCapRadiusC = 15.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "CapsuleUpOffsetC"))
		float CapsuleUpOffsetC = 65.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Pickaxe Climbing|Config", DisplayName = "ConstCapsuleOffsetBetWallC"))
		float ConstCapsuleOffsetBetWallC = 5.0; //Picaxe Climbing
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Pickaxe Climbing|Config", DisplayName = "PickaxeClimbChannelC"))
		TEnumAsByte<ECollisionChannel> PickaxeClimbChannelC = ECollisionChannel::ECC_Visibility;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "CharacterC"))
		ACharacter* CharacterC = nullptr;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "ForClimbingChannelC"))
		TEnumAsByte<ECollisionChannel> ForClimbingChannelC = ECollisionChannel::ECC_Visibility;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "ClassToIgnoreByLedge"))
		TArray<UClass*> ClassToIgnoreByLedgeC = TArray<UClass*>();
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "DebugTraceIndex"))
		int DebugTraceIndexC = 0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Config", DisplayName = "FootsDefOffsetC"))
		FTwoVectors FootsDefOffsetsC = {};
	//Other
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Inputs", DisplayName = "AxisValuesInterpC"))
		FVector2D AxisValuesInterpC = FVector2D(0.0, 0.0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Inputs", DisplayName = "AxisValuesInterpSlowC"))
		FVector2D AxisValuesInterpSlowC = FVector2D(0.0, 0.0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Inputs", DisplayName = "AxisValuesC"))
		FVector2D AxisValuesC = FVector2D(0.0, 0.0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Time", DisplayName = "FootsRelativeVelocityC"))
		FVector FootsRelativeVelocityC = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Animations", DisplayName = "JumpBackPoseAlphaC"))
		FVector2D JumpBackPoseAlphaC = FVector2D(0.0, 0.0);
	//Inputs Bools
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Inputs", DisplayName = "ShiftPressedC"))
		bool ShiftPressedC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "New Climbing System|Inputs", DisplayName = "SpaceBarImpulseC"))
		bool SpaceBarImpulseC = false;

	// [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-] - [-]
	// ROPE SWING SYSTEM VARIABLES:

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System|Config", DisplayName = "ConstSwingLenghtOffsetC"))
		float ConstSwingLenghtOffsetC = 100.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System|Config", DisplayName = "SwingDebugIndexC"))
		int SwingDebugIndexC = 0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "CableSimComponentC"))
		UJakubCablePhysic* CableSimC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "HookActorC"))
		AActor* HookActorC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "IsSwingingC"))
		bool bIsSwingingC = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "bIsFallingStartedC"))
		bool bIsFallingStartedC = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "AnchorPointInterpC"))
		FVector AnchorPointInterpC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "SwingRadiusSmoothC"))
		float SwingRadiusSmoothC = 500.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "TargetForceC"))
		FVector TargetForceC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "GravityStabilityForceC"))
		FVector GravityStabilityForceC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "CollisionIndexC"))
		int CollisionIndexC = -1;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "SwingAnimPropertyC"))
		FVector2D SwingAnimPropertyC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Rope Swing System", DisplayName = "TargetRopeLenghtC"))
		float TargetRopeLenghtC = 500.0;

	// ______________________________________________________________________________________________________________________________________________________________________________________________
	// ______________________________________________________________________________________________________________________________________________________________________________________________

	// FUNCTIONS (NOT OVERRIDE)
	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Create Axis Values With Interp", Keywords = "Axis"))
		virtual void CreateAxisValuesWithInterpFast(float InterpSpeed=5.0);

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge To Cap Position", Keywords = "Transformation CPP"))
		virtual FCALS_ComponentAndTransform ConvertLedgeToCapPositionC(FCALS_ComponentAndTransform Center);

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Floor To Cap Position", Keywords = "Transformation CPP"))
		virtual FCALS_ComponentAndTransform ConvertFloorToCapPositionC(FCALS_ComponentAndTransform Center);

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Choose Ledge Finding Transform", Keywords = "Transformation CPP"))
		virtual void ChooseLedgeFindingTransformC(bool GetByLedge, FVector& ReturnLocation, FVector& ReturnDirection);

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Direction By Axis Input", Keywords = "Vector"))
		virtual FVector GetDirectionByInputC(float LerpWithForward=0.2);

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge Vector To Transform WS", Keywords = "Transformation CPP"))
		virtual FCALS_ComponentAndTransform ConvertLedgeStructToWS(FCMC_SingleClimbPointC SingleClimbPointWS = FCMC_SingleClimbPointC());

	UFUNCTION(BlueprintPure, Category = "New Climbing System|Transformation", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Ledge Vector To Transform LS", Keywords = "Transformation CPP"))
		virtual FCALS_ComponentAndTransform ConvertLedgeStructToLS(FCMC_SingleClimbPointC SingleClimbPointWS = FCMC_SingleClimbPointC());

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Ledge Finding", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Ledge Is Valid Part 2", Keywords = "Transformation CPP"))
		virtual bool LedgeValidationPart2C(bool Valid = false, FCMC_SingleClimbPointC LeftStruct = FCMC_SingleClimbPointC(), FCMC_SingleClimbPointC RightStruct = FCMC_SingleClimbPointC(), 
		float MinDistanceBetweenPoints = 14.0, float RotationTollerance = 0.4, float CapsuleUpOffset = -50.0, FVector2D CapsuleChecking = FVector2D(90.0, 30.0));

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Start Or End", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Can Drop To Ledge", Keywords = "Corner CPP"))
		virtual bool CheckCanDropToLedgeC(FCMC_LedgeC& LedgeStructWS);

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Resize Radius To Default", Keywords = "CPP"))
		virtual void ResizeCapsuleToDefaultC(float InterpSpeed=100);

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Do While Climbing", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Ledge Per Frame", Keywords = "CPP"))
		virtual void UpdateLedgePerFrameC(FCMC_LedgeC& OutLedge, FVector& OutOrigin, FVector2D SlopeScale = FVector2D(1,0.4),float ConstMovementOffset = 2, bool InputLock = false);

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|IK", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Foot IK Valid", Keywords = "CPP"))
		virtual bool CheckFootIkValidC(FTransform Transform, bool ForRightFoot, float TraceUpOffset);

	UFUNCTION(BlueprintCallable, Category = "New Climbing System|Other", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Foots Relative Velocity", Keywords = "CPP"))
		virtual FVector GetFootsRelativeVelocityC();

	UFUNCTION(BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Axis To Name", Keywords = "CPP"))
		virtual FName ConvertAxisToNameC();

	UFUNCTION(BlueprintCallable, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Player Can Move To Wall", Keywords = "CPP"))
		virtual bool CheckPlayerCanMoveToWallC(bool Check, FCALS_ComponentAndTransform TransformWS, FCALS_ComponentAndTransform& ReturnTransformWS);

	UFUNCTION(BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Convert Wall To Cap Position", Keywords = "CPP", CompactNodeTitle = "WallToCap"))
		virtual FCALS_ComponentAndTransform ConvertWallToCapPositionC(FCALS_ComponentAndTransform TransformWS);

	UFUNCTION(BlueprintCallable, Category = "Pickaxe Climbing|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Try Find Tangent For Wall", Keywords = "CPP"))
		virtual void TryFindTangentForWallC(bool& ReturnValid, FCALS_ComponentAndTransform& TransformWS, FVector FindingLocation, FVector FindingDirection, float FindingLength = 80, 
		float FirstRadius = 20, float DistanceOffsetScale = -0.5, int VerticalAccuracy = 2, FVector2D CapsuleSize = FVector2D(30, 90));

	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Rope Swing Update Physic ", Keywords = "CPP", AdvancedDisplay = 5))
	virtual void RopeSwingUpdatePhysicC
	(
		UCurveVector* RegulationCurve = nullptr,
		float SwingMinRange = 100.0,
		float SwingMaxRange = 800.0,
		int HandAttachIndex = 5,
		bool AddtiveCondition = true,
		float HysteresisMin = 100, 
		float GravityStabilityFactor = 0.1, 
		float ForceScaleFactor = 1.0, 
		float InterpSpeedIn = 18.0, 
		float SphereOriginInterpSpeed = 20.0,
		float RadiusInterpSpeed = 10.0,
		FVector2D SwingLenghtFactor = FVector2D(1.4, 1.3)
	);

	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Try Find Hook Point", Keywords = "CPP", AdvancedDisplay = 5))
	virtual bool TryFindHookPointC
	(
		AActor*& HookActor,
		TEnumAsByte<EObjectTypeQuery> TraceObject,
		float FindingRadius = 200.0,
		float CapsuleHeightScale = 5.0,
		FVector Direction = FVector(1, 0, 0),
		float DistancePioryty = 0.5,
		int DrawDebug = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Rope Lenght Update", Keywords = "CPP", AdvancedDisplay = 4))
	virtual void RopeLenghtUpdateC
	(
		UCurveVector* ForceCurve = nullptr,
		float RopeMinLeght = 50.0,
		float RopeMaxLenght = 800.0,
		int HandAttachIndex = 5,
		FName TimerName = TEXT("RopeSwingTimer2"),
		float TimerMaxTime = 4.0,
		float UpForceStrenght = -800.0,
		int ExpandDivite = 4,
		FVector2D InterpSpeedRange = FVector2D(6, 12)
	);

	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Reducing Velocity When Swing", Keywords = "CPP"))
	virtual void ReducingVelocityWhenSwingC
	(
		float ReductionDampingFactor = 0.25,
		float VelocityTrigger = 500.0,
		float SwingingDampingFactor = 0.6,
		int ActionIndex = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Rope Swing System|Base", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Air Control", Keywords = "CPP", AdvancedDisplay = 4))
	virtual void UpdateAirControlC
	(
		int ActionIndex = 0,
		float RopeMinLenght = 50.0,
		float SwingMinRange = 100.0,
		float SwingMaxRange = 800.0,
		FVector2D AirControlRange = FVector2D(0.07, 0.13),
		float SwingMinBias = 100.0,
		float InterpToSpeed = 8.0,
		float ReduceToZeroSpeed = 10.0
	);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Functions Library (Can Be Override!!!)
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "New Climbing System|Other", meta = (DisplayName = "Get Character Axis", Keywords = "Axis Character"))
		void GetCharacterAxisC(float& Forward, float& Right);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Ledge Finding", meta = (DisplayName = "Try Create Ledge Structure", Keywords = "CPP Ledge"))
		void TryCreateLedgeStructureC(bool& Valid, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, bool& FirstNotValid,
			FVector TraceOrigin = FVector(0.0, 0.0, 0.0), FVector TraceDirection = FVector(0.0, 0.0, 0.0), FVector2D AxisNormals = FVector2D(0.0, 0.0), float Z_Offset = 0.0,
			float ForwardTraceLength = 45.0, float RightOffsetScale = 1.0, bool UseWallCondition = true, bool InverseTracing = true);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Do While Climbing|Corner", meta = (DisplayName = "Check Can Start Corner", Keywords = "CPP Corner"))
		void CheckCanStartCornerC(bool& DetectedCorner, bool& OuterType, FCMC_LedgeC& TargetLedgeStruct, bool Valid=true, bool InputLock=false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "New Climbing System|Do While Climbing|Jumps", meta = (DisplayName = "Check Can Jump Back", Keywords = "CPP"))
		void CheckCanJumpBackC(bool& ReturnValue, FCMC_SingleClimbPointC& LeftPoint, FCMC_SingleClimbPointC& RightPoint, FCMC_SingleClimbPointC& OriginPoint, 
		bool UseCameraCondition = true, float JumpMaxDistance = 220);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Rope Swing System|Base", meta = (DisplayName = "Rope Hooked Condition", Keywords = "Rope Swing System"))
		bool RopeHookedConditionC();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Rope Swing System|Base", meta = (DisplayName = "Check Normal For Point", Keywords = "Rope Swing System"))
		bool CheckNormalForPointC(FExposedCableParticle& InParticle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Detach Rope Or End Swing", Keywords = "Rope Swing System"))
		bool DetachRopeOrEndSwingC();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rope Swing System|Base", meta = (DisplayName = "Finish Rope Swing", Keywords = "Rope Swing System"))
		bool FinishRopeSwingC();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Zipline System", meta = (DisplayName = "Started Zipline", Keywords = "Zipline"))
		bool StartedZiplineC();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Pickaxe Climbing|Base", meta = (DisplayName = "Started Pickaxe Climb ", Keywords = "Zipline"))
		bool StartedPickaxeClimbC();
	
	//Modify VOLUME ACTOR
	AModifyClimbingParamsVolume* CurrentModifyVolume = nullptr;

};
