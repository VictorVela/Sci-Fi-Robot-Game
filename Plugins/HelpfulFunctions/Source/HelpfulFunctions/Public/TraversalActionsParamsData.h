

#pragma once

#include "CoreMinimal.h"
#include "ALS_StructuresAndEnumsCpp.h"
#include "Engine/DataAsset.h"
#include "TraversalActionsParamsData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class HELPFULFUNCTIONS_API UTraversalActionsParamsData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Actions")
	TMap<int, FTraversalStateEvaluation> ActionStates;

};
