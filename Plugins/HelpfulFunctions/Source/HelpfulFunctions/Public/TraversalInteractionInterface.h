

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Cpp_TraversalActionComponent.h"
#include "TraversalInteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTraversalInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELPFULFUNCTIONS_API ITraversalInteractionInterface
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Set_InteractionTransform(FTransform NewTransform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_InteractionTransform(FTransform& ReturnTransform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Set_TraversalAction(bool StartedAction, CALS_TraversalAction StateAction, bool& Updated);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_StartedTraversalAction(bool& ReturnStarted, CALS_TraversalAction& ReturnActionState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Set_PredictableJump(bool StartedJump, bool& Updated);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_StartedPredictableJump(bool& StartedJump);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_RootTransform(FTransform& ReturnTransform);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Traversal Interface", meta = (BlueprintThreadSafe))
	void BPI_Get_ControlTransform(FTransform& ReturnTransform);

};
  