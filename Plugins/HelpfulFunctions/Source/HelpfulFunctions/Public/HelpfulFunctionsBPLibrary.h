// Copyright Jakub W, All Rights Reserved. 
#pragma once

#include "ALS_StructuresAndEnumsCpp.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Curves/CurveFloat.h"
#include "HelpfulFunctionsBPLibrary.generated.h"



USTRUCT(BlueprintType)
struct FSingleClimbPoint
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		bool ValidPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FVector Location = FVector(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FVector Normal = FVector(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		FTransform ActorTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClimbingPoint")
		UPrimitiveComponent* Component = nullptr;

};

USTRUCT(BlueprintType)
struct FVelocityBlendCpp
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
		float F = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
		float B = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
		float L = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
		float R = 0.0;
};

UENUM(BlueprintType)
enum class EAnimBaseEvalType : uint8
{
	// Evaluates the original Animation Source data 
	Source,
	// Evaluates the original Animation Source data with additive animation layers
	Raw,
	// Evaluates the compressed Animation data - matching runtime (cooked)
	Compressed
};

USTRUCT(BlueprintType)
struct FAnimPoseEvaluationConfig
{
	GENERATED_BODY()

	// Type of evaluation which should be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Pose")
	EAnimBaseEvalType EvaluationType = EAnimBaseEvalType::Raw;

	// Whether or not to retarget animation during evaluation
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	bool bShouldRetarget = true;

	// Whether or not to extract root motion values
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	bool bExtractRootMotion = false;

	// Whether or not to force root motion being incorporated into retrieved pose
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	bool bIncorporateRootMotionIntoPose = true;

	// Optional skeletal mesh with proportions to use when evaluating a pose
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	TObjectPtr<USkeletalMesh> OptionalSkeletalMesh = nullptr;

	// Whether or additive animations should be applied to their base-pose 
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	bool bRetrieveAdditiveAsFullPose = true;

	// Whether or not to evaluate Animation Curves
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Animation|Pose")
	bool bEvaluateCurves = true;
};


UENUM(BlueprintType)
enum class CALS_IntputType : uint8
{
	JumpAction,
	WalkAction,
	SprintAction,
	AimAction,
	ShotAction,
	StanceAction
};

UCLASS()
class UHelpfulFunctionsBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "JakubW"), Category = "JakubW_FunctionsLibrary")
	static float HelpfulFunctionsSampleFunction(float Param);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Character Capsule Start Location", Keywords = "Character Capsule Component"))
		static FVector GetPlayerCapsuleStartLocation(const UObject* WorldContextObject, const ACharacter* TargetChar);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Component", meta = (DisplayName = "Force Destroy Component", Keywords = "Component"))
		static void ForceDestroyComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (DisplayName = "Elastic Out Interp (Vector)", Keywords = "Interpolation Vector"))
		static FVector ElasticOutInterpFast(FVector A, FVector B, float Alpha, float PowerBase=2);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (DisplayName = "Bounce Out Interp (Vector)", Keywords = "Interpolation Vector"))
		static FVector BouceOutInterpFast(FVector A, FVector B, float Alpha);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (DisplayName = "Get Angle Between Vectors", Keywords = "Vector Math Angle"))
		static float GetAngleBetween(FVector A, FVector B);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (DisplayName = "Slerp (Vector)", Keywords = "Vector Math Interpolation"))
		static FVector SlerpFast(FVector A, FVector B, float Alpha);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (DisplayName = "SlerpTo (Vector)", Keywords = "Vector Math Interpolation"))
		static FVector SlerpToFast(FVector Current, FVector Target, float DeltaTime, float InterpSpeed);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Capsule Have Room (With Ignore)", Keywords = "Character Trace"))
		static bool CapsuleHaveRoomWithIgnore(const UObject* WorldContextObject, const ACharacter* TargetChar, FVector Location, TArray<AActor*> ToIgnore, 
											  float ScaleRadius=1, float ScaleHeight=1, const bool DrawTrace=false);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Capsule Have Room (With Ignore) Transform", Keywords = "Character Trace"))
		static bool CapsuleHaveRoomWithIgnoreTransform(const UObject* WorldContextObject, const ACharacter* TargetChar, FTransform TargetTransform, TArray<AActor*> ToIgnore,
			float ScaleRadius = 1, float ScaleHeight = 1, const bool DrawTrace = false);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| ClimbingSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Climbing Foot IK Trace V3", Keywords = "Character Trace IK Climbing"))
		static void ClimbingFootIK(const UObject* WorldContextObject, bool& ReturnHit, FVector& ReturnLocation, FVector& ReturnNormal, 
									const ACharacter* TargetChar, FVector FootLocation, FVector Direction, float UpOffset, float TraceRadius=18, FVector2D TraceLength=FVector2D(12.0,18.0), 
									bool UseTwoBoneIK=false, FName RootBoneName="Thigh_L", FName JoinBoneName="calf_L", FName EndBoneName="Foot_L", float FootUpOffsetGround=14, int DebugIndex=0);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (DisplayName = "Distance Point To Line", Keywords = "Vector Math Distance"))
		static float DistancePointToLine(FVector LineV1, FVector LineV2, FVector PointVector);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| ClimbingSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Single Ledge Point", Keywords = "Character Trace Climbing"))
		static void FindLedgePoint(const UObject* WorldContextObject, bool& ReturnValid, FVector& ReturnLocation, FVector& ReturnNormal, FTransform& ReturnActorT, UPrimitiveComponent*& ReturnComponent,
			const ACharacter* TargetChar, FVector StartLocation, FVector ForwardDirection, TArray<UClass*> NotThisClass, float ForwardScale = 40, float UpOffset = 20, bool preValid = false, 
			FVector preLocation = FVector(0, 0, 0), FVector preNormal = FVector(0, 0, 0), bool IsFirst = false, bool ForLeftSide = false, int DebugIndex = 0, ECollisionChannel TraceChannel= ECollisionChannel::ECC_Visibility);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (DisplayName = "N To F Vector (Yaw)", Keywords = "Vector Math Rotation"))
		static FVector NormalToVector(FVector Normal);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Array", meta = (DisplayName = "Check Is Not This Class", Keywords = "Actors Class Array"))
		static bool ClassToIgnore(UClass* Target,TArray<UClass*> ToIgnore);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| ClimbingSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Single Ledge Point (Structure)", Keywords = "Character Trace Climbing"))
		static FCMC_SingleClimbPointC FindLedgePointStructure(const UObject* WorldContextObject, const ACharacter* TargetChar, FVector StartLocation, FVector ForwardDirection, TArray<UClass*> NotThisClass,
			float ForwardScale = 40, float UpOffset = 20, bool preValid = false, FVector preLocation = FVector(0, 0, 0), FVector preNormal = FVector(0, 0, 0), bool IsFirst = false, 
			bool ForLeftSide = false, int DebugIndex = 0, ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility, float NormalFindOffset=2.0);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| ClimbingSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Try Find Ledge Line", Keywords = "Climbing Trace Character"))
		static void TryFindLedgeLine(const UObject* WorldContextObject, bool& Valid, FCMC_SingleClimbPointC& LedgeStruct, const ACharacter* TargetChar, FVector StartLocation, FVector StartDirection,
			FVector2D AxisNormal, float ZOffset, float ForwardTraceLength, float RightOffsetScale, bool LeftDirection, bool InverseTracing, int TraceDebugIndex, TArray<UClass*> NotThisClass,
			ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility, float NormalFindOffset=0.2);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| ClimbingSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Climbing - Ledge Valid P1", Keywords = "Climbing Validation Character"))
		static bool ClimbingLedgeValidP1(const UObject* WorldContextObject, bool IsValid, FCMC_SingleClimbPointC LeftStruct, FCMC_SingleClimbPointC RightStruct, ECollisionChannel TraceChannel, float UpAxisTollerance = 25);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Foot IK Offset", Keywords = "Foots IK Character", AutoCreateRefTerm="CurrentLocation"))
		static void CalcFootsOffset(const UObject* WorldContextObject, ACharacter* Character, float EnableFootCurveValue, FName IKFootName, FName RootBone, FVector CurrentLocation, FVector CurrentLocationOffset, 
			FRotator CurrentRotationOffset, float DeltaSecond, float IKTraceDistanceAboveFoot, float TraceUpOffset, float IKTraceDistanceBelowFoot, float FootHeight, FVector& NewLocation, FVector& NewLocationOffset, 
			FRotator& NewRotationOffset, int DebugIndex, bool UseByFloorFinding, bool& TracingType);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (DisplayName = "Calculate Diagonal Scale Amount (Fast)", Keywords = "Movement Animations Character", BlueprintThreadSafe))
		static float CalculateDiagonalScale(UCurveFloat* Curve, FVelocityBlendCpp VelocityBlend);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Calculate Relative Acceleration Amount (Fast)", Keywords = "Movement Animations Character", BlueprintThreadSafe))
		static FVector CalculateRelativeAcceleration(const UObject* WorldContextObject, const UCharacterMovementComponent* CharMove, FRotator CharRot = FRotator(0,0,0), FVector Acceleration = FVector(0,0,0), FVector Velocity = FVector(0,0,0));

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Calculate Velocity Blend Fast", Keywords = "Character AnimCurve", BlueprintThreadSafe))
		static FVelocityBlendCpp CalcVelocityBlendFast(const UObject* WorldContextObject, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Update Movement Values (AnimBP)", Keywords = "Character Anim Graph"))
		static void UpdateMovementValues(const UObject* WorldContextObject, const UCharacterMovementComponent* CharMove, FRotator ActorRot, FVelocityBlendCpp VelocityBlend, float DeltaX, float VelocityBlendInterpSpeed, float AnimatedWalkSpeed,
			   float AnimatedRunSpeed, float AnimatedSprintSpeed, float AnimatedCrouchSpeed, float Speed, UCurveFloat* DiagonalScaleAmountCurve, UCurveFloat* StrideBlend_N_Walk, UCurveFloat* StrideBlend_N_Run, 
			   UCurveFloat* StrideBlend_C_Walk, FVector Velocity, FVector Acceleration, FVector2D LeanAmount, float GroundedLeanInterpSpeed, FName WeightCurveName, FName BasePoseCurveName, FVelocityBlendCpp& ReturnVelocityBlend, 
			   float& ReturnDiagonalScale, FVector& ReturnRelativeAcc, FVector2D& ReturnLeanAmount, float& ReturnWalkRunBlend, float& ReturnStrideBlend, float& ReturnStandingPlayRate, float& ReturnCrouchPlayRate);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Aiming Values (AnimBP)", Keywords = "Character Anim Graph"))
		static void UpdateAimingValues(const UObject* WorldContextObject, const ACharacter* TargetChar, FRotator AimingRotation, FRotator SmoothedAimRotation, float InputYawOffsetTime, int RotationMode, 
				bool HasMovementInput, FVector MovementInput, float DeltaX, float SmoothedAimingRotSpeed, float InputYawOffsetSpeed, FRotator& ReturnSmoothedAimRotation, FVector2D& ReturnAimingAngle, 
			    FVector2D& ReturnSmoothedAimingAngle, float& ReturnAimSweepTime, FRotator& ReturnSpineRotation, float& ReturnInputYawOffsetTime, float& ReturnLeftYawTime, float& ReturnRightYawTime, float& ReturnForwardYawTime);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Update Movement Values LOD (AnimBP)", Keywords = "Character Anim Graph",
		AdvancedDisplay = "AnimatedWalkSpeed,AnimatedRunSpeed,AnimatedSprintSpeed,AnimatedCrouchSpeed,WeightCurveName"))
		static void UpdateMovementValuesLod(const UObject* WorldContextObject, FVector MovementLowDetail, FVector& ReturnMovementLowDetail, float& ReturnStandingPlayRate, float& ReturnCrouchingPlayRate, 
			float Speed, float DeltaX, const float AnimatedWalkSpeed=150.0, const float AnimatedRunSpeed=350.0, const float AnimatedSprintSpeed=600.0, const float AnimatedCrouchSpeed=150.0, FName WeightCurveName = FName(TEXT("Weight_Gait")), bool ReturnOnlyDirection=false);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Update Rotation Values (AnimBP)", Keywords = "Character Anim Graph",
		AdvancedDisplay = "YawOffset_LR,YawOffset_FB"))
		static void UpdateRotationValues(const UObject* WorldContextObject, int& ReturnDirection, float& FYaw, float& BYaw, float& LYaw, float& RYaw, int Gait, int RotationMode, 
			int MovementDirection, FRotator AimingRotation, FVector Velocity, UCurveVector* YawOffset_FB, UCurveVector* YawOffset_LR);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (WorldContext = "WorldContextObject", DisplayName = "Angle In Range", Keywords = "Float State Direction"))
		static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Calculate Quadrant", Keywords = "Float State Direction", AdvancedDisplay = "FL_Threshold,BR_Threshold,FR_Threshold,BL_Threshold"))
		static int CalculateQuadrant(int MovementDirection, float FR_Threshold, float FL_Threshold, float BR_Threshold, float BL_Threshold, float Buffer, float Angle);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Smoothed Character Rotation", Keywords = "Character Rotation", AdvancedDisplay = "TargetInterpSpeedConst,TargetInterpSpeedSmooth,UpdateControl"))
		static void SmoothCharacterRotation(const UObject* WorldContextObject, const ACharacter* Character, FRotator& NewTargetRotation, FRotator Target, FRotator CurrentTargetRotation, float DeltaSecond, float TargetInterpSpeedConst,
			float TargetInterpSpeedSmooth, bool UpdateControl, FRotator& NewControlRotation);

	// NOT WORKING!!!
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Trace", meta = (WorldContext = "WorldContextObject", DisplayName = "Rotated Capsule Trace By Channel", Keywords = "Trace Collision"))
		static bool RotatedCapsuleTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, ETraceTypeQuery TraceChannel, 
			bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime); 
	// Simple Line Trace Function With Loop
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Trace", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Gap By Line Trace", Keywords = "Trace Collision", AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "TraceColor,TraceHitColor,DrawTime,MaxIteracion"))
		static bool WalkableLineTrace(const UObject* WorldContextObject, const FVector Start, const FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, 
			int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime=0.5f, int MaxIteracion=6);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Trace", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Gap By Sphere Trace", Keywords = "Trace Collision", AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "bIgnoreSelf,TraceColor,TraceHitColor,DrawTime,MaxIteracion"))
		static bool FindGapBySphereTrace(const UObject* WorldContextObject, const FVector Start, const FVector End, float TraceRadius, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore,
			int DebugIndex, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 0.5f, int MaxIteracion = 6);
	
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Trace", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Near Point To Wall Surface (Fast)", Keywords = "Trace CoverSystem Collision"))
		static void FindNearPointToWallSurface(const UObject* WorldContextObject, const ACharacter* Character,bool& ValidPoint, FHitResult& OutHitResult, FVector ForwardDirection, int Accuracy=7,bool UseNormalFromFirstTrace=false, 
			float ConstForwardOffset=15.0, FVector RightOffset=FVector(0,0,0), int DebugIndex=0);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Array", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Near From Actors Array ", Keywords = "Array Actor"))
		static void GetNearFromActorsArray(const UObject* WorldContextObject, TArray<AActor*> ActorsArray, FVector Origin, bool& ReturnValid, AActor*& OutActor);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Array", meta = (WorldContext = "WorldContextObject", DisplayName = "Class To Actors Array ", Keywords = "Array Actor"))
		static void ClassToActorsArray(const UObject* WorldContextObject, TArray<UClass*> ActorsClass, TArray<AActor*> SingleActorToIgnore, TArray<AActor*>& ReturnArray);

	//ENG: The function's task is to add an offset to the target point so as to eliminate the collision with other AI controllers. It may happen that several controllers will reach to one point. This function tries to eliminate this situation.
	//PL : Zadaniem funckji jest dodanie przesuniêcia wzglêdem punktu celu, tak aby wyeliminowaæ kolizjê wzglêdemi innych kontrolerów AI.Mo¿e siê zda¿yæ taka sytuacja ¿e kilka kotrolerów bêdzie do¿yæ do jednego punktu.Ta funkcja stara siê wyeliminowaæ tak¹ sytuacjê.
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Navigation", meta = (WorldContext = "WorldContextObject", DisplayName = "Fix Move Point When Is Busy ", Keywords = "AI Navigation"))
		static FVector FixMovePointWhenIs(UObject* WorldContextObject, FVector TargetPoint, ACharacter* TargetCharacter, UClass* CharactersArray, float OffsetRadius=60, int MaxIteractions=6, float MinAccetableDistance=25);

	// Working Only When Navigation System Is Valid
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Collision", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Room By Sphere Ray Cast ", Keywords = "Collision Nav"))
		static float RayCastSphereRoomCheckFast(UObject* WorldContextObject, FTransform Origin, float InRadius=80);
	// Working Only When Navigation System Is Valid
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Collision", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Room By Sphere Complex Ray Cast ", Keywords = "Collision Nav"))
		static float RayCastSphereComplexRoomCheckFast(UObject* WorldContextObject, FTransform Origin, float InRadius = 80);
	// Function For AI Controllers
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Is Not Hiding In Foliage ", Keywords = "Character Trace"))
		static bool IsNotHidingInFoliage(UObject* WorldContextObject, ACharacter* OwnerCharacter, ACharacter* Character, ECollisionChannel TraceChannel, int DebugIndex);
	
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Navigation", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Base Jump Away Position ", Keywords = "Navigation RayCast"))
		static void FindBaseJumpAwayPosition(UObject* WorldContextObject, bool& ReturnValid, FVector& ReturnLocation, ACharacter* Character, FVector GrenadePosition, float MaxRadius=120, FVector2D Arc=FVector2D(-50,50));

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Math", meta = (WorldContext = "WorldContextObject", DisplayName = "FFT", Keywords = "Fast Fourier Transform"))
		static void FFT(TArray<float> Data, int32 NumSamples);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Math", meta = (WorldContext = "WorldContextObject", DisplayName = "Autocorrelation", Keywords = "Autocorrelation Function Array"))
		static TArray<float> Autocorrelation(const TArray<FVector2D>& data);
	// >>>>>>>>>>>>>>>>>>>>> ZMIANY WPROWADZONE PO WYDANIU AKTUALIZACJI 2.7 <<<<<<<<<<<<<<<<<<<<<<<<
	//My Custom Input Conection With All Components With Specified Interface
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Interface", meta = (WorldContext = "WorldContextObject", DisplayName = "Execute Interface Input", CompactNodeTitle = "Inputs Interface", Keywords = "Inputs Interface"))
		static void ExecuteInterfaceInput(UObject* WorldContextObject, UActorComponent* OptionalComponent, bool CallToAll, bool Hold, CALS_IntputType InputIndex);

	//Convert a transformation related to the global coordinate system with a transformation relative to a specific component. Calculations are made on matrices.
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (WorldContext = "WorldContextObject", DisplayName = "Cpp World To Local (Matrix)", Keywords = "Transform Convert"))
		static FCALS_ComponentAndTransform ConvertWorldToLocalFastMatrix(FCALS_ComponentAndTransform World);

	//Convert transform in local space to global space. Calculations are made on matrices.
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (WorldContext = "WorldContextObject", DisplayName = "Cpp Local To World (Matrix)", Keywords = "Transform Convert"))
		static FCALS_ComponentAndTransform ConvertLocalToWorldFastMatrix(FCALS_ComponentAndTransform Local);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Math", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Get Safe Float Value", Keywords = "Curves"))
		static float GetSafeFloatValue(UCurveFloat* FloatCurve, float Time);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Anim Slot Is Activated ", Keywords = "Animation Instance"))
		static bool AnimSlotIsActivated(UObject* WorldContextObject, FName SlotName);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Anim Slot Is Activated By Target ", Keywords = "Animation Instance"))
		static bool AnimSlotIsActivatedByTaget(UObject* WorldContextObject, UAnimInstance* Target, FName SlotName);

	//Get Us Enemy By Using Actor Tags. An actor checking whether "ActorToCheck" is to be treated as an enemy. When ActorToCheck have Tag with EN_[ClassName] then return = true
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Is Enemy Value", Keywords = "Character AI"))
		static bool GetIsEnemyState(UObject* WorldContextObject, AActor* Checker = nullptr, AActor* ActorToCheck = nullptr);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Collision", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Nearest Collision By Cylinder", Keywords = "Collision"))
		static bool FindNearestCollisionByCylinder(FVector& OutCircleCenter, float& OutCircleRadius, FVector& OutNormal, UObject* WorldContextObject, FTransform OriginTransform, TArray<AActor*> ToIgnore,
			float MaxRadius = 100.0, float Height = 5.0, int Precision = 16, ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility, bool DrawDebug = false);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Anim Curve Value At Time", Keywords = "Animations"))
		static float GetCurveValueAtTime(UObject* WorldContextObject, UAnimSequence* AnimSequence, FName CurveName, float Time);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Root Motion Curve Value At Time", Keywords = "Animations"))
		static FVector GetRootMotionValueAtTime(UObject* WorldContextObject, UAnimSequence* AnimSequence, float Time);

	// Lerping between points by creating temporary Spline component.
	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Math", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject"))
		static FVector SplineLerpOnPathPoints(const FVector& Start, const TArray<FVector>& PathPoints, float Alpha);

	// Interpolation value by using Sigmoid curve
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject"))
		static FVector VInterpToWithDelay(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Bone Transform At Time", Keywords = "Animations"))
	static FTransform GetBonePositionAtTimeFromSeq(const UAnimSequenceBase* AnimationSequenceBase, double Time, FName BoneName, FAnimPoseEvaluationConfig EvaluationOptions);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Animation", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Anim Sequence From Montage", Keywords = "Animations"))
	static UAnimSequence* GetAnimSequenceFromMontage(const UAnimMontage* TargetAnimMontage, int SequenceIndex);

	UFUNCTION(BlueprintCallable, Category = "JakubW_FunctionsLibrary| Collision", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Floor Type By Trace", Keywords = "Collision", 
		AutoCreateRefTerm = "ActorsToIgnore", AdvancedDisplay = "TraceChannel,AccuracyFactor,MaxRadius,ActorsToIgnore,ReturnInLocalSpace,DrawDebugIndex"))
	static bool CheckFloorTypeC(UObject* WorldContextObject, FVector& GlobalOrigin, float& GlobalRadius, FVector& ForwardOrigin, float& ForwardRadius, FVector FloorLocation, FVector Direction, ETraceTypeQuery TraceChannel, int AccuracyFactor,
		float MaxRadius, TArray<AActor*> ActorsToIgnore, bool ReturnInLocalSpace, int DrawDebugIndex);



private:
	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Interpolation", meta = (DisplayName = "Bounce Out Time Function", Keywords = "Interpolation Vector"))
		static float BounceOutTimeFunction(float time);

	UFUNCTION(BlueprintPure, Category = "JakubW_FunctionsLibrary| Character", meta = (BlueprintThreadSafe, WorldContext = "WorldContextObject", DisplayName = "Get Anim Curve Value Clamped", Keywords = "Character AnimCurve"))
		static float GetClampedCurveValue(const UObject* WorldContextObject, FName Curve, float Bias=0, float ClampMin=0, float ClampMax=1);



};

