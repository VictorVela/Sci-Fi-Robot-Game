// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimSequence.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "TrajectoryGeneratorBPLibrary.generated.h"


UENUM(BlueprintType)
enum class ECurveVectorAxis : uint8
{
	X,
	Y,
	Z
};



UCLASS()
class UTrajectoryGeneratorBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "TrajectoryGenerator sample test testing"), Category = "TrajectoryGeneratorTesting")
	static float TrajectoryGeneratorSampleFunction(float Param);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static bool AddKeyToCurveVector(UCurveVector* CurveVector, float Time, FVector Value, TEnumAsByte<ERichCurveInterpMode> InterpMode = RCIM_Linear);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static bool AddKeyToCurveFloat(UCurveFloat* CurveFloat, float Time, float Value, TEnumAsByte<ERichCurveInterpMode> InterpMode = RCIM_Linear);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static void ClearAllKeysFromCurveFloat(UCurveFloat* CurveFloat);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static void ClearAllKeysFromCurveVector(UCurveVector* CurveVector);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static UCurveVector* CreateCurveVectorAsset(const FString& AssetName, const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static UCurveFloat* CreateCurveFloatAsset(const FString& AssetName, const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves", meta = (DeterminesOutputType = "AssetClass"))
	static TArray<UObject*> LoadAssetsFromPath(TSubclassOf<UObject> AssetClass, const FString& AssetPath);

	// Return all keyframes in CurveFloat
	UFUNCTION(BlueprintPure, Category = "Math|Curves")
	static int32 GetNumKeyFramesCurveFloat(UCurveFloat* Curve);

	// Return all keyframes from CurveVector by selected axis
	UFUNCTION(BlueprintPure, Category = "Math|Curves")
	static int32 GetNumKeyFramesCurveVector(UCurveVector* Curve, ECurveVectorAxis Axis);

	// Return float value from CurveFloat by keyframe index
	UFUNCTION(BlueprintPure, Category = "Math|Curves")
	static float GetFloatValueAtKeyFrame(UCurveFloat* Curve, int32 KeyFrameIndex);

	// Return float value from CurveVector by keyframe index
	UFUNCTION(BlueprintPure, Category = "Math|Curves")
	static float GetVectorValueAtKeyFrame(UCurveVector* Curve, int32 KeyFrameIndex, ECurveVectorAxis Axis);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static bool RemoveKeyFrameFromCurveFloat(UCurveFloat* Curve, int32 KeyFrameIndex);

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	static bool RemoveKeyFrameFromCurveVector(UCurveVector* Curve, int32 KeyFrameIndex, ECurveVectorAxis Axis);


};
