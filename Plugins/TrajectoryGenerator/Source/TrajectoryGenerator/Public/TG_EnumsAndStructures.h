

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "TG_EnumsAndStructures.generated.h"

USTRUCT(BlueprintType)
struct FTG_TimeAndVectorValue : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Value = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Frame = 0;
};

USTRUCT(BlueprintType)
struct FTG_TimeAndRotatorValue : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Value = FRotator(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Frame = 0;
};


class TRAJECTORYGENERATOR_API TG_EnumsAndStructures
{
public:
	TG_EnumsAndStructures();
	~TG_EnumsAndStructures();
};
