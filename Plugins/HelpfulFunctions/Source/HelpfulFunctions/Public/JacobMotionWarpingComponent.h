// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Curves/CurveVector.h"
#include "JacobMotionWarpingComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UJacobMotionWarpingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UJacobMotionWarpingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Create Base Variables
	//References (Public)
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "Character"))
	ACharacter* CharacterC = nullptr;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "WarpingTarget"))
	AActor* WarpingTarget = nullptr;
	//Config Variables (Public)
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "DevMode"))
	bool DevMode = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "LockWapringActor"))
	bool LockWarpingActor = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "ExcludeRootMotion"))
	bool ExcludeRootMotionV = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "DeltaOffset"))
	FVector DeltaOffset = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Config", DisplayName = "TargetActorLocation"))
	FVector TargetActorLocation = FVector(0, 0, 0);
	//Variables (Not Visible Via Blueprint)
	UPROPERTY(Transient)
	float BlendingDuration = 0;
	UPROPERTY(Transient)
	float AlphaByTimeline = 0;
	UPROPERTY(Transient) //VECTOR
		FVector ConstOffset = FVector(0, 0, 0);
	UPROPERTY(Transient) //VECTOR
		FVector ExcludedRootValue = FVector(0, 0, 0);
	UPROPERTY(Transient)
	FRotator SavedWarpActorRot = FRotator(0, 0, 0);
	UPROPERTY(Transient)
	FRotator RotationOffset = FRotator(0, 0, 0);
	UPROPERTY(Transient)
	TArray<FName> CurvesName = {};
	UPROPERTY(Transient)
	TArray<FName> WarpKeys = {};
	UPROPERTY(Transient)
	FCALS_ComponentAndTransform TargetActorLocationLS = {};
	UPROPERTY(Transient)
	UPrimitiveComponent* FloorComponent = nullptr;

	//Function Library - PUBLIC (CAN BE EXECUTED BY BLUEPRINT)
	//Reset MotionWarping Variables
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Reset Values", Keywords = "MotionWarping"))
	virtual void ResetValues();

	// PL: 
	// Ustaw punkt relacji pomiedzy MotionWarping a TargetActor. Za ka¿dym razem kiedy bêdzie u¿ywany MotionWarping, zostanie pobrana aktualna 
	// pozycja TargetActor, aby skorygowaæ ruch. Dodanie 'Const Offset' spowoduje przesuniecie pozycji Akora o ten wektor. Opcja 'Exclude 
	// Root Motion' daje rezultat tylko wtedy gdy nasz Aktor odgrywa Animacjê z wyodrêbnionym RootMotion, który zosta³ zapisany jako Anim Curve. 
	// W³¹czenie tego spowodujê usuniêcie przesuniecia RootMotion z aktualniej pozycji Aktora.
	//
	// ENG:
	// Set a relationship point between MotionWarping and TargetActor. Each time MotionWarping is used, the current TargetActor position will 
	// be retrieved to correct for motion. Adding 'Const Offset' will move the Actor's position by this vector. The 'Exclude Root Motion' option 
	// only works when our Actor plays an Animation with the extracted RootMotion, which has been saved as Anim Curve. Enabling this will remove 
	// the RootMotion shift from the current position of the Actor.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Set Motion Warping Target", Keywords = "MotionWarping"))
	virtual void SetMotionWarpingTarget(FName WarpTargetName, AActor* TargetActor, FVector ConstantOffset, FRotator ConstRotOffset, bool ExcludeRootMotion);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Stop Updating Warp Point", Keywords = "MotionWarping"))
	virtual void StopUpdatingWarpPoint(bool StopUpdating = true);

	//Main Function - Executed By AnimNotify PerFrame
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Warping Update", Keywords = "MotionWarping"))
	virtual void WarpingUpdate(FName X = FName(TEXT("Root")), FName Y = FName(TEXT("Root")), FName Z = FName(TEXT("Root")), FName R = FName(TEXT("Root")), bool UseUpAxis = false,
		bool WithSweep = true, bool WarpRotation = true, float StrengthAlpha = 1.0, FName ConstAlphaCurve = FName(TEXT("ss")));

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Predicted Single Motion Position", Keywords = "MotionWarping", NoExport))
	void PredictedSingleMotionPosition(bool& Valid, bool& HaveRoom, FVector& ReturnLocation, ACharacter* WarpTargetActor, FVector ConstantOffset, float NormalizedTime,
		UCurveVector* RootMotionChar, UCurveVector* RootMotionWarpRot, bool ExcludeWarpActorRoot, bool UseUpAxis, FRotator ApplyCustomRotator);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Predicted Single Motion Position Auto", Keywords = "MotionWarping", NoExport))
	void PredictedSingleMotionPositionAuto(bool& Valid, bool& HaveRoom, FVector& ReturnLocation, ACharacter* WarpTargetActor, UAnimSequence* AnimSeqAtt, UAnimSequence* AnimSeqVic,
		FVector WarpingOrigin, FVector ConstantOffset, float NormalizedTime, bool ExcludeWarpActorRoot, bool UseUpAxis, FRotator ApplyCustomRotator, bool InvertAxis);


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TimerFunction1();
	FTimerHandle TimerHandle1;

protected:
	//FUNCTION LIBRARY

	float GetCurveValueByIndex(int ArrIndex = 0);
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", DisplayName = "Get Origin Position", Keywords = "MotionWarping", NoExport))
	FVector GetOriginPosition();
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", DisplayName = "Get Target Axis", Keywords = "MotionWarping", NoExport))
	void GetTargetAxis(FVector& Forward, FVector& Right, FVector& Up, float ScaleXY = 1);

	FVector GetWarpActorInfo(AActor* InActor, FVector Offset, FRotator Rotation);

	void GetWarpTagetCurveValue(float& X, float& Y, float& Z);

	FVector TryFixZPosition(bool Use = true, FVector CharLocation = FVector(0, 0, 0));

	FCALS_ComponentAndTransform TryConvertToRelative(FVector VectorWS);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Timeline Blend In", Keywords = "MotionWarping", NoExport))
	void TimelineBlendIn(float Duration = 0.2);

};
