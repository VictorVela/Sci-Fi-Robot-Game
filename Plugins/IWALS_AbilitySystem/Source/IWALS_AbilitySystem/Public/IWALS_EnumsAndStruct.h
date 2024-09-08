// JakubW

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "IWALS_EnumsAndStruct.generated.h"

UENUM(BlueprintType)
enum class HipsDirectionC : uint8
{
	F,
	B,
	RF,
	RB,
	LF,
	LB
};

USTRUCT(BlueprintType)
struct FLeanAmoutC : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean")
		float LR = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lean")
		float FB = 0.0;
};

USTRUCT(BlueprintType)
struct FVelocityBlendC : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward")
		float F = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backward")
		float B = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Left")
		float L = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Right")
		float R = 0.0;
};

USTRUCT(BlueprintType)
struct FTurnInPlaceAssetC : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward")
		UAnimSequence* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backward")
		float AnimatedAngle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Left")
		FName SlotName = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Right")
		float PlayRate = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Right")
		bool ScaleTurnAngle = true;
};


class IWALS_ABILITYSYSTEM_API IWALS_EnumsAndStruct : public UObject
{
public:
	IWALS_EnumsAndStruct();
	~IWALS_EnumsAndStruct();
};
