

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Cpp_LadderClimbingComponent.generated.h"


USTRUCT(BlueprintType)
struct FCALS_LadderAnimData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
		UAnimSequence* Default = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
		UAnimSequence* Climb = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
		UAnimSequence* FreeHang = nullptr;
};


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HELPFULFUNCTIONS_API UCpp_LadderClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCpp_LadderClimbingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Ladder System", DisplayName = "StartLadderClimbC"))
		bool StartLadderClimbC = false;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "CharacterC"))
		ACharacter* CharacterC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "DefCapsuleSizeC"))
		FVector2D DefCapsuleSizeC = FVector2D(0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "References", DisplayName = "RungComponentC"))
		UPrimitiveComponent* RungComponentC = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Inverse Kinematic", DisplayName = "FootOffset_L"))
		FVector FootOffsetLC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Inverse Kinematic", DisplayName = "FootOffset_R"))
		FVector FootOffsetRC = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Base", DisplayName = "RungLenghtC"))
		float RungLenghtC = 0.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Sequencer", DisplayName = "SequenceLenghtC"))
		float SequenceLenghtC = 0.0;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "Base", DisplayName = "PosesConfigValuesC"))
		FLinearColor PosesConfigValuesC = FLinearColor(0,0,0,0);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Check Can Start Climb Ladder", Keywords = "Ladder System Cpp"))
		bool CheckCanStartClimbLadder(bool CheckMainCondition, bool Param2, FVector InLocation, FRotator InRotation, 
			FCALS_ComponentAndTransform& TargetCapPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Check Can Start Climb Ladder With Loop", Keywords = "Ladder System Cpp"))
		bool CheckCanStartClimbLadderIteration(bool CheckMainCondition, bool FromClimb, FVector InStartLocation, FVector InEndLocation, FRotator InRotation, int Iterations,
			FCALS_ComponentAndTransform& TargetCapPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Try Finish Ladder Climb", Keywords = "Ladder System Cpp"))
		bool TryFinishLadderClimb(bool IgnoreCondition = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Try Start Ladder Climb", Keywords = "Ladder System Cpp"))
		bool TryStartLadderClimb(FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Try Start Ladder Climb When Hold Ledge", Keywords = "Ladder System Cpp"))
		bool TryStartLadderClimbWhenHoldLedge(FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Initialize Component", Keywords = "Ladder System Cpp"))
		bool InitComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Get Main Values", Keywords = "Ladder System Cpp"))
		bool GetMainValues(bool& StartedLadderClimb, FCALS_ComponentAndTransform& TargetPositionWS, AActor*& LadderActor, UPrimitiveComponent*& RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Set Main Values", Keywords = "Ladder System Cpp"))
		bool SetMainValues(bool StartedLadderClimb, FCALS_ComponentAndTransform TargetPositionWS, AActor* LadderActor, UPrimitiveComponent* RungComp);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ladder System", meta = (DisplayName = "Choose Main Poses Index", Keywords = "Ladder System Cpp"))
		int ChooseMainPosesIndex(FVector InitLocation, FRotator InitRotation);

		
};
