// Copyright Jakub W, All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "JakubCableComponentBPLibrary.generated.h"

UCLASS()
class UJakubCableComponentBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Collision Trace Flag", Keywords = "For More Information About Static Mesh"), Category = "Collision")
	static ECollisionTraceFlag GetCollisionTraceFlag(UStaticMeshComponent* Object);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Perform PID Control", Keywords = "Interpolation"), Category = "Math|interpolation")
	static float PerformPIDControl(float CurrentValue, float TargetValue, float IntegralSum, float PreviousError, float Kp, float Ki, float Kd, float dt);
};
