// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALS_BaseComponentsInterfaceCpp.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UALS_BaseComponentsInterfaceCpp : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELPFULFUNCTIONS_API IALS_BaseComponentsInterfaceCpp
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputJumpAction(bool Hold, bool Start);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputWalkAction(bool Hold, bool Start);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputSprintAction(bool Hold, bool Start);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputAimAction(bool Hold, bool Start);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputShotAction(bool Hold, bool Start);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inputs")
		void BPICpp_InputStanceAction(bool Hold, bool Start);
};
