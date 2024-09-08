

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "IWALS_AnimLayersClassData.generated.h"


class UAnimInstance;

UCLASS(BlueprintType)
class IWALS_ABILITYSYSTEM_API UIWALS_AnimLayersClassData : public UDataAsset
{
	GENERATED_UCLASS_BODY()

	UIWALS_AnimLayersClassData();

	UPROPERTY(EditAnywhere, Category = OverlaySet)
		TArray<TSubclassOf<UAnimInstance>>	OverlayStatesFirstLayer;

	UPROPERTY(EditAnywhere, Category = OverlaySet)
		TArray<TSubclassOf<UAnimInstance>>	OverlayStatesSecondLayer;

	UPROPERTY(EditAnywhere, Category = OverlaySet)
		TArray<TSubclassOf<UAnimInstance>>	OverlayStatesThirdLayer;

	UFUNCTION(BlueprintCallable, Category = "Animation|Linked ANim Graphs", meta = (WorldContext = "WorldContextObject", DisplayName = "Apply Anims Layer For Anim Inst"))
		void ApplyAnimsLayerForAnimInst(ACharacter* TargetChar = nullptr, FName OverlayTag = TEXT("OverlayLayer")) const;
	
};
