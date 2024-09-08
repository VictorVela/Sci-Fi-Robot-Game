

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cpp_CoverSystemV2.generated.h"

USTRUCT(BlueprintType)
struct FCoverTraceStartConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RightOffset = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LeftOffset = -30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float OffsetToRadius = 20.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ForwardOffsetScale = 20.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName TagNameToIgnore = TEXT("IgnoreForCover");
};

USTRUCT(BlueprintType)
struct FCoverSystemMoveConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FirstRightOffset = 11.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MultiplyOffsetRight = 11.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LerpWithInputWall = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float TOffsetForward = 15.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float TOffsetBackward = -15.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float TraceRadius = 4.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AngleTollerance = 0.56;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int LoopCount = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;
};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_CoverSystemV2 : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_CoverSystemV2();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//References Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "CharC"))
		ACharacter* CharC = nullptr;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "DefCapSize"))
		FVector2D DefCapSizeC = FVector2D(30.0, 90.0);
	//System Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "StartCover"))
		bool StartCoverC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "CanMoveLeftRight"))
		bool CanMoveLeftRightC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "AlphaPerDirection"))
		FVector2D AlphaPerDirectionC = FVector2D(0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "MoveDirectionSmooth"))
		FVector2D MoveDirectionSmoothC = FVector2D(0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "MoveDirectionAxis"))
		float MoveDirectionAxisC = 0.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "DirectionAxisSmooth"))
		float AxisSmoothC = 0.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "CachedWallNormal"))
		FVector CachedWallNormalC = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "CoverSystem", DisplayName = "SmoothOffsetToWall"))
		FTransform SmoothOffsetToWallC;
	//Config Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "TraceConfigWhenNotCover"))
		FCoverTraceStartConfig TConfigS = {};
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "WallPullingStrength"))
		float WallPullingStrengthC = 10.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "CapRadiusDurningCover"))
		float CapRadiusDurningCoverC = 22.0;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "UseSmoothingNormals"))
		bool UseSmoothingNormalsC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "TraceDebugIndex"))
		int TraceDebugIndexC = 0;
	//Only Cpp Variables
	FVector2D Axis2d = FVector2D(0, 0);

	UFUNCTION(BlueprintCallable, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Try Start Cover System", Keywords = "Cover System"))
		virtual bool TryStartCoverSystemC(FHitResult& ReturnWall, int DebugIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Move Char To Wall", Keywords = "Cover System"))
		virtual void MoveCharToWallC(bool& WallValid, FHitResult& ReturnWall, float MoveToWallSpeed = 1.0, int DebugIndex = 0);

	UFUNCTION(BlueprintPure, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Smoothing Wall Normal", Keywords = "Cover System"))
		virtual FVector SmoothWallNormalC(FVector TraceLocation, FVector Normal, float DirectionAxis);

	UFUNCTION(BlueprintCallable, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Can Start Move", Keywords = "Cover System"))
		virtual bool CheckCanStartMoveC(float& DesiredWalkAxis, bool WallValid, FHitResult WallHit, FCoverSystemMoveConfig Config);

	UFUNCTION(BlueprintPure, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Wall To Cap", Keywords = "Cover System"))
		virtual FVector ConvertWallToCapPositionC(FVector WallLocation, FVector WallNormal);

	UFUNCTION(BlueprintCallable, Category = "CoverSystem", meta = (WorldContext = "WorldContextObject", DisplayName = "Update Direction Values", Keywords = "Cover System", AdvancedDisplay = "PerDirectionInterpSpeed, MovementInterpSpeed, SmoothAxisInterpSpeed"))
		virtual void UpdateDirectionValuesC(bool IsCrouch = false, int DirectionState = 0, float PerDirectionInterpSpeed = 10, float MovementInterpSpeed = 10, float SmoothAxisInterpSpeed = 8);


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CoverSystem", meta = (DisplayName = "Start Cover Mode Call", Keywords = "Cover System"))
		void StartCoverModeC(bool& StartedCover);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CoverSystem", meta = (DisplayName = "Finish Cover Mode Call", Keywords = "Cover System"))
		void FinishCoverModeC();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "CoverSystem", meta = (DisplayName = "Set Axis Values For Cpp", Keywords = "Cover System"))
		FVector2D SetAxisValuesForCpp();

		
};
