

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IWALS_OverlayLayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIWALS_OverlayLayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IWALS_ABILITYSYSTEM_API IIWALS_OverlayLayerInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Overlay Layer Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_OverlayHaveBlendOutState(bool& HaveState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Overlay Layer Interface", meta = (BlueprintThreadSafe))
	void BPI_Set_OverlayHaveBlendOutState(bool UseBlendState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Overlay Layer Interface", meta = (BlueprintThreadSafe))
	void BPI_Set_RequiredOverlayStateIndex(int NewStateIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Overlay Layer Interface")
	void BPI_Set_OnOverlayStateChangeInit(int ActionIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Overlay Layer Interface")
	void BPI_Get_OnUnequipFinished(bool& Finished);

};
