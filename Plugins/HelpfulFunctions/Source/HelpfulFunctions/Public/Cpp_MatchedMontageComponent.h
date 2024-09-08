// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "UObject/Object.h"
#include "Cpp_MatchedMontageComponent.generated.h"

USTRUCT(BlueprintType)
struct FMatchedMontageTwoPoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		ACharacter* Character = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UAnimInstance* AnimInstance = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UAnimMontage* MontageAsset = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool NormalizeTimeToAnimLength = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float TimelineLength = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float PlayRate = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float StartMontageAt = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool StopAllMontages = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool ConvertTransformsToWorld = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		FCALS_ComponentAndTransform StartTransform = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		FCALS_ComponentAndTransform EndTransform = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool UseMotionCurvesFromAnimation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UCurveVector* LocationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UCurveFloat* RotationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool RemapCurves = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool ApplyTimelineAlphaAtEnd = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		FVector2D TimelineAlphaEndConfig = FVector2D(0.8, 1.0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool FinishWhenAlphaAtEnd = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool UseInterFor180Rot = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float RotationDirection180 = -90.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		int CustomRotationInterpType = 0;
};

USTRUCT(BlueprintType)
struct FMatchedMontageManyPoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		ACharacter* Character = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UAnimInstance* AnimInstance = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float TimelineLength = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float PlayRate = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float StartMontageAt = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool StopAllMontages = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool ConvertTransformsToWorld = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		TArray<FCALS_ComponentAndTransform> TargetPoints = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool UseMotionCurvesFromAnimation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UCurveVector* LocationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		UCurveFloat* RotationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool RemapCurves = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool ApplyTimelineAlphaAtEnd = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		FVector2D OutBlendingDuration = FVector2D(0.8, 1.0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool FinishWhenAlphaAtEnd = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		bool UseInterFor180Rot = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		float RotationDirection180 = -90.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
		int CustomRotationInterpType = 0;
};

USTRUCT(BlueprintType)
struct FMMEndingEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
	UObject* TargetObject = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchedMontage")
	FName EventName = FName("MontageEnd");
};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_MatchedMontageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_MatchedMontageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Create Variables (Only C++)
	ACharacter* CharC = nullptr;
	UAnimInstance* AnimInst = nullptr;
	bool ConvertToWorld = true;
	bool UseMotionCurvesFromAnimation = false;
	bool RemapCurves = true;
	bool ApplyTimelineAlphaAtEnd = false;
	bool FinishWhenAlphaAtEnd = false;
	bool UseInterFor180Rot = false;
	float TimelineRange = 1.0;
	float Rotation180Direction = -90.0;
	int RotInterpType = 0;
	int TPI = 0;
	UCurveVector* LocationCurve = nullptr;
	UCurveFloat* RotationCurve = nullptr;
	FVector2D OutBlendDuration = FVector2D(0.8, 1.0);
	FVector LockDecreasingXYZ = FVector(0, 0, 0);
	FTimerHandle TimelineEvent2; //Timer Variable
	FTimerHandle TimelineEvent1; //Timer Variable
	TArray<FCALS_ComponentAndTransform> TargetPoints = {};
	FMMEndingEvent MontageEndEvent = {};
	//Exposed Variables (Visible In Blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "MatchedMontage", DisplayName = "MotionCurvesName"))
		TArray<FName> MotionCurvesNameC = {};
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "MatchedMontage", DisplayName = "SetZeroToVelocity"))
		bool SetZeroToVelocity = true;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "MatchedMontage", DisplayName = "AnimAlphaCurveName"))
		FName AnimAlphaCurveName = FName("None");

	UFUNCTION(BlueprintCallable, Category = "Matched|Update", meta = (WorldContext = "WorldContextObject", DisplayName = "Matching Multiple Points", Keywords = "MatchedMontage"))
		virtual void MatchingMultiplePointsC();

	UFUNCTION(BlueprintCallable, Category = "Matched|Update", meta = (WorldContext = "WorldContextObject", DisplayName = "Matching Single Points", Keywords = "MatchedMontage"))
		virtual void MatchingSinglePointsC();

	UFUNCTION(BlueprintCallable, Category = "Matched|Prepare", meta = (WorldContext = "WorldContextObject", DisplayName = "Play Matched Montage (ManyPoints)", Keywords = "MatchedMontage"))
		virtual void PlayMatchedMontageManyPointsC(UAnimMontage* MontageAsset, FMatchedMontageManyPoints ConfigStruct);

	UFUNCTION(BlueprintCallable, Category = "Matched|Prepare", meta = (WorldContext = "WorldContextObject", DisplayName = "Play Matched Montage (TwoPoints)", Keywords = "MatchedMontage"))
		virtual void PlayMatchedMontageTwoPointsC(UAnimMontage* MontageAsset, FMatchedMontageTwoPoints ConfigStruct);

	UFUNCTION(BlueprintPure, Category = "Matched|Construction", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Anim Curve", Keywords = "MatchedMontage"))
		virtual bool GetAnimCurveC(float& Value, FName CurveName, bool WithLock, float LockVariable);

	UFUNCTION(BlueprintPure, Category = "Matched|Construction", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Custom Curve Value", Keywords = "MatchedMontage"))
		virtual FVector GetCustomCurveValueC(float InTime = 0.0);

	UFUNCTION(BlueprintPure, Category = "Matched|Construction", meta = (WorldContext = "WorldContextObject", DisplayName = "Extracted Transfororm Interpolation", Keywords = "MatchedMontage"))
		virtual FTransform ExtractedInterpolationC(FTransform A, FTransform B, float VX = 0.0, float VY = 0.0, float VZ = 0.0, float ROT = 0.0, float Alpha = 0.0, 
		float Direction180 = -90.0, bool UseInterpFor180Rot = false);

		void UpdateLockingDecreasingCurvesC(float Y = 0, float X = 0, float Z = 0);

		float GetTimelinePlayBackNormalized();

		float GetRotationCurve(int CurveIndex = 0);

		FCALS_ComponentAndTransform ConvertToWorldCondition(FCALS_ComponentAndTransform InStructure);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Can Override In Blueprint
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Matched|End", meta = (DisplayName = "Finish Timeline And Seq", Keywords = "Matched Finish"))
		void FinishMatchedMontageC();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Matched|Update", meta = (DisplayName = "Matching Motion Normalized", Keywords = "Matched Update"))
		void MatchingMotionNormalized(float CurrentTime);

	//Utworz funkcje czasu - s³u¿¹ do zdefiniowania timera
	void TimerFunction1();
	void TimerFunction2();


};
