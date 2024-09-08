// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Perception/AIPerceptionTypes.h"

#include "ALS_HumanAI_ControllerCpp.generated.h"



/**
 * 
 */


UCLASS()
class HELPFULFUNCTIONS_API AALS_HumanAI_ControllerCpp : public AAIController
{
	GENERATED_BODY()

public:
	AALS_HumanAI_ControllerCpp();
	

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	// Create Only Cpp Variables
	ACharacter* Own;
	AALS_HumanAI_ControllerCpp* Self;

	// Create Essential Variables
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		bool TracePerceptionEnemyValidC = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		bool DetectedEnemyC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		ACharacter* EnemyFromTraceC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		ACharacter* EnemyFromPerceptionC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		ACharacter* TargetEnemyActorC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		UStaticMeshComponent* VisionStaticMeshC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		UAIPerceptionComponent* SightComponentC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		int LODsStateIndexC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		int AttackStateIndexC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		float CombatModeTimeC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		float InteligenceC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		float DetectedEnemyTimeC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		int RotationModeIndexC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		int LocomotionModeIndexC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		int FoliageChannelIndexC;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
		FRotator TargetControlRotationC;

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Interface Test", Keywords = "AI"))
		virtual bool InterfaceTest(ACharacter* Char);

	UFUNCTION(BlueprintCallable, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Sight Perception By Static Mesh", Keywords = "AI"))
		virtual void SightPerceptionByMeshFast(FName HeadSocketName, bool& AndDetected, TArray<AActor*>& CharactersList, int DebugIndex, float MaxDistance=5000);

	UFUNCTION(BlueprintCallable, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Sight Perception By Trace Fast", Keywords = "AI"))
		virtual void SightPerceptionByTraceFast(FName HeadSocketName, bool& AndDetected, TArray<AActor*>& CharactersList, int DebugIndex, float MaxDistance = 1800);

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Best Enemy From Sight", Keywords = "AI"))
		virtual void GetBestEnemyFromSight(float PlayerDetectionPriority, bool& ReturnValid, ACharacter*& ReturnCharacter);

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Check Enemy Is In Foliage Fast", Keywords = "AI"))
		virtual bool CheckIsInFoliage(ACharacter* EnemyCharacter);

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Use Trace Sight Perception Fast", Keywords = "AI"))
		virtual bool UseTraceSightPercept();

	UFUNCTION(BlueprintCallable, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Find Other Comrate In Radius Fast", Keywords = "AI"))
		virtual void FindOtherComrate(bool& ReturnValid, TArray<ACharacter*>& ReturnCharacters, float Radius=400);

	UFUNCTION(BlueprintCallable, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Choose Enemy Actor Fast", Keywords = "AI"))
		virtual void ChooseEnemyActor(float InterpSpeedMultiplyDown=1.0, float InterpSpeedMultiplyUp=1.0, float DetectedStateTolerance=0.9);

	UFUNCTION(BlueprintCallable, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Set Focus Actor Transform Fast", Keywords = "AI"))
		virtual void SetFocusActorTransformFast(AActor* FocusActor, FTransform NewTransform, float InterpSpeedA = 1.0, float InterpSpeedB = 3.0, bool PrintInfo = false);

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Solider Enemy From Other AI Fast", Keywords = "AI"))
		virtual void GetEnemyFromOtherAIs(bool CheckDistanceToPlayer, bool& ReturnValid, ACharacter*& ReturnCharacter);

	UFUNCTION(BlueprintPure, Category = "ALS Human AI", meta = (WorldContext = "WorldContextObject", DisplayName = "Get The Average Value Of Spotted Enemies Fast", Keywords = "AI"))
		virtual float GetTheAverageValueOfSpottedEnemiesFast(float InRadius=1000);

private:

};
