

#pragma once

#include "CoreMinimal.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Engine/DataAsset.h"
#include "MantleAssetData.generated.h"


UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UMantleAssetData : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	UAnimMontage* Default = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	UAnimMontage* LeftHand = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	UAnimMontage* RightHand = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	UAnimMontage* TwoHands = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	FVector StartingOffset = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float LowHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float HighHeight = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float MinAnimStartAt = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float MaxAnimStartAt = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float MinPlayRate = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Properties")
	float MaxPlayRate = 1.1;

};
