

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALS_HookActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UALS_HookActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELPFULFUNCTIONS_API IALS_HookActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Get")
	void HAFSI_Get_HookTargetMeshPosition(FVector& Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Get")
	void HAFSI_Get_IsHookActor(bool& IsActorForHook);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Get")
	void HAFSI_Get_ItsCurrentUsed(bool& ItsUsed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Set")
	void HAFSI_Set_IsUsed(bool& Updated, bool Using);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Execute")
	void HAFSI_Play_AnimIn();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Execute")
	void HAFSI_Play_AnimOut();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Get")
	void HAFSI_Get_HookTargetComp(UPrimitiveComponent*& ForwardComponent, UPrimitiveComponent*& BackwardComponent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Execute")
	void HAFSI_DetachRopeAndRollUp();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hook Actor Interface|Get")
	void HAFSI_Get_ParticleNormalValidation(float& Angle);

};
