

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/InputScaleBias.h"
#include "JWAN_CurveSmootherLogic.generated.h"

USTRUCT(BlueprintType)
struct FCurveSmootherSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetCurveName = FName("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseConstantInterp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ReducePeakChange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakSpeedThreshold = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakInterpSpeed = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpingTime = 0.25;
};



USTRUCT(BlueprintInternalUseOnly)
struct JAKUBANIMNODESTOOL_API FAnimNode_CurveSmoother : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
		FPoseLink SourcePose;

	/* Configure Smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothingSetting)
		TArray<FCurveSmootherSetting> SmoothingData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlphaSettings)
		EAnimAlphaInputType AlphaInputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlphaSettings)
		FInputScaleBias AlphaScaleBias;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlphaSettings, meta = (PinShownByDefault))
		float Alpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlphaSettings, meta = (PinShownByDefault))
		FName AlphaCurveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AlphaSettings)
		FInputScaleBiasClamp AlphaScaleBiasClamp;

	bool bInitializeLastValuesMap;
	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	// End of FAnimNode_Base interface

private:
	
	float ProcessCurveOperation(FPoseContext& Output, const FName& CurveName, float CurrentValue, float NewValue, FCurveSmootherSetting Settings, float dt);
	float InternalBlendAlpha;
	TArray<float> InterpValues;
	TArray<float> InterpValuesLayer2;
	TArray<float> SpeedValues;
	TArray<float> PrevCurvesValue;
	TArray<float> ElapsedTimes;
	UAnimInstance* AnimInst;

public:
	FAnimNode_CurveSmoother()
		: AlphaInputType(EAnimAlphaInputType::Float)
		, Alpha(0.0f)
		, AlphaCurveName(NAME_None)
		, InternalBlendAlpha(0.0f)
	{
	}

};