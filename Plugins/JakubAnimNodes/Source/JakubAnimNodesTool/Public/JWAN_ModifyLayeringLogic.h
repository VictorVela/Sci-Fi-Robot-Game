

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/InputScaleBias.h"
#include "JWAN_ModifyLayeringLogic.generated.h"

UENUM()
enum class EModifyLayeringApplyMode : uint8
{
	Blend,
	Add,
	Scale,
	RemapCurve,
};

UENUM()
enum class ELayeringCurvesName : uint8
{
	Layering_Legs,
	Layering_Pelvis,
	Layering_Spine,
	Layering_Spine_Add,
	Layering_Arm_L,
	Layering_Arm_L_Add,
	Layering_Arm_L_LS,
	Layering_Hand_L,
	Layering_Arm_R,
	Layering_Arm_R_Add,
	Layering_Arm_R_LS,
	Layering_Hand_R,
	Layering_Head,
	Layering_Head_Add,
	Enable_HandIK_L,
	Enable_HandIK_R,
	Enable_SpineRotation
};

USTRUCT(BlueprintType)
struct FLayeringCurvesData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ELayeringCurvesName CurveName = ELayeringCurvesName::Layering_Legs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Value = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EModifyLayeringApplyMode BlendMode = EModifyLayeringApplyMode::Blend;
};

/** Easy way to modify curve values on a pose */
USTRUCT(BlueprintInternalUseOnly)
struct JAKUBANIMNODESTOOL_API FAnimNode_ModifyLayering : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
		FPoseLink SourcePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ModifyLayering, meta = (PinHiddenByDefault))
		TArray<FLayeringCurvesData> CurvesStructure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ModifyLayering)
		EModifyLayeringApplyMode ApplyMode = EModifyLayeringApplyMode::Blend;

	//From TwoWayBlend
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
	void ProcessCurveOperation(const EModifyLayeringApplyMode& InApplyMode, FPoseContext& Output, const FName& CurveName, float CurrentValue, float NewValue);
	float InternalBlendAlpha;
	UAnimInstance* AnimInst;

public:
	FAnimNode_ModifyLayering()
		: AlphaInputType(EAnimAlphaInputType::Float)
		, Alpha(0.0f)
		, AlphaCurveName(NAME_None)
		, InternalBlendAlpha(0.0f)
	{
	}

};