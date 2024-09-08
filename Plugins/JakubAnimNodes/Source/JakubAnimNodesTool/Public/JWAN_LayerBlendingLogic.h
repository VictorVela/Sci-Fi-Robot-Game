

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimData/BoneMaskFilter.h"
#include "JWAN_LayerBlendingLogic.generated.h"

USTRUCT(BlueprintType)
struct FCurvesData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Pelvis = TEXT("Layering_Pelvis");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Spine = TEXT("Layering_Spine");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName SpineAdd = TEXT("Layering_Spine_Add");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Head = TEXT("Layering_Head");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName HeadAdd = TEXT("Layering_Head_Add");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Legs = TEXT("Layering_Legs");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmL = TEXT("Layering_Arm_L");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmL_LS = TEXT("Layering_Arm_L_LS");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmL_Add = TEXT("Layering_Arm_L_Add");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName HandL = TEXT("Layering_Hand_L");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmR = TEXT("Layering_Arm_R");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmR_LS = TEXT("Layering_Arm_R_LS");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmR_Add = TEXT("Layering_Arm_R_Add");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName HandR = TEXT("Layering_Hand_R");
};

USTRUCT(BlueprintType)
struct FSlotsName
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Legs = TEXT("Legs");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Pelvis = TEXT("Pelvis");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Spine = TEXT("Spine");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Head = TEXT("Head");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmL = TEXT("Arm L");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ArmR = TEXT("Arm R");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Curves = TEXT("Curves");
};

USTRUCT(BlueprintType)
struct JAKUBANIMNODESTOOL_API FJWAN_LayerBlendingLogic : public FAnimNode_Base
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
        FPoseLink BaseLayerInput;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
        FPoseLink OverlayLayerInput;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
        FPoseLink BasePosesInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FCurvesData CurvesName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FSlotsName AllSlotsName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = State)
        FName SlotName = TEXT("MovementActionGroup.BaseLayer");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, editfixedsize, Category = Config, meta = (PinShownByDefault))
		float ArmL_MS = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, editfixedsize, Category = Config, meta = (PinShownByDefault))
		float ArmR_MS = 1.0;

    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void GatherDebugData(FNodeDebugData& DebugData) override;
    // End of FAnimNode_Base interface


	//podstawowe mieszanie puz za pomoc¹ wartoœci alpha
    virtual void BlendTwoPosesContext(FPoseContext& PoseA, FPoseContext& PoseB, float Alpha, FPoseContext& OP);

	//Funkcja twrz¹ca animacjê additive z dwóch wejœæ - wynikiem jest poza additive
    virtual void MakeAdditiveContext(FPoseContext& Base, FPoseContext& Add, bool MeshSpace, FPoseContext& OP);

	//podstawowa funckja zawierajace ro¿ne sopsoby mieszania puz
    virtual void ApplyAdditiveWithSlotEvaluate(FPoseContext& OP, FPoseContext& LBase, FPoseContext& LOverlay, FPoseContext& LAdditive, float AddAlpha, float BlendAlpha, 
        FName AnimSlotName, FSlotNodeWeightInfo SlotWeight, bool UseMeshSpace);

	//funkcja optymalizuj¹ce - wybiera elementy które musz¹ byæ z aktualiowane, oraz takie, które mo¿na pomin¹æ
    virtual void ApplyAdditiveWithSlotUpdate(FPoseContext& LBase, FPoseContext& LOverlay, FPoseContext& LAdditive, float AddAlpha, float BlendAlpha, FName AnimSlotName);

	//Utworz definicje mieszania poprzez wagi przypisanych do kosci
	virtual void BlendPerBoneEvaluate(FPoseContext& OP, FPoseContext& PoseA, FPoseContext& PoseB, bool UseMeshSpaceRotation, bool UseMeshSpaceScaleBlend, 
		TArray<FPerBoneBlendWeight> CurrentBoneBlendW, bool UseBlendByWeight, float BlendAlpha, FBlendedCurve CurvesA, FBlendedCurve CurvesB);

	//Get Curve Value From Skeleton
	virtual float CurveValue(FName CurveName);

	//Update Slot - Bez tego nie bedzie mozliwosci wykonania animacji montage dla wybranego slotu
	virtual void SlotUpdate(const FAnimationUpdateContext& Context, FName SName, FPoseLink InputPose, FSlotNodeWeightInfo& Weight);

	// Rebuild cache per bone blend weights from the skeleton
	void RebuildPerBoneBlendWeights(const USkeleton* InSkeleton, TArray<FPerBoneBlendWeight>& PBBW, TArray<FInputBlendPose> InLayerSetup, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid);

	// Check whether per-bone blend weights are valid according to the skeleton (GUID check)
	bool ArePerBoneBlendWeightsValid(const USkeleton* InSkeleton, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid);

	// Update cached data if required
	void UpdateCachedBoneData(const FBoneContainer& RequiredBones, const USkeleton* Skeleton, TArray<FPerBoneBlendWeight>& CurrentBoneBlendOutput, FGuid& InSkelGuid, 
		FGuid& InVirtualBonesGuid, uint16& RBSN, TArray<uint8>& CPSI, TArray<FInputBlendPose> InLayerSetup, float BlendAlpha);

	void UpdateMaskWeightsValue(const FAnimationUpdateContext& C, TArray<FPerBoneBlendWeight>& DBBW, TArray<FPerBoneBlendWeight> CBBW, TArray<float> BlendWeightsPerPose, 
		bool BlendRootMotionBasedOnRoot, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid, uint16& RBSN, TArray<uint8>& CPSI, TArray<FInputBlendPose> InLayerSetup, float BlendAlpha);

	void UpdateBlendWeightStrenght(TArray<FPerBoneBlendWeight>& BonesWeight, float Alpha);

	bool RebuildAllBlendWeights(const USkeleton* InSkeleton);

	void UpdateLayerBlendCurvesSource(const FBoneContainer& RequiredBones, const USkeleton* Skeleton, TArray<FPerBoneBlendWeight> BonesWeight);

	virtual TArray<FInputBlendPose> CreateLayerBlendValue(TArray<FName> BonesName, TArray<int> BlendDepth);

	FGuid SkeletonGuid;
	FGuid VirtualBoneGuid;

protected:
	//Initialize AnimSlots
    FSlotNodeWeightInfo WeightData0;
	FSlotNodeWeightInfo WeightData1;
	FSlotNodeWeightInfo WeightData2;
	FSlotNodeWeightInfo WeightData3;
	FSlotNodeWeightInfo WeightData4;
	FSlotNodeWeightInfo WeightData5;
	FGraphTraversalCounter SlotNodeInitializationCounter0;
	FGraphTraversalCounter SlotNodeInitializationCounter1;
	FGraphTraversalCounter SlotNodeInitializationCounter2;
	FGraphTraversalCounter SlotNodeInitializationCounter3;
	FGraphTraversalCounter SlotNodeInitializationCounter4;
	FGraphTraversalCounter SlotNodeInitializationCounter5;

	UAnimInstance* AnimInst;
	USkeleton* Skel;


	TArray<FPerBoneBlendWeight>	PerBoneBlendWeights;
	TArray<FPerBoneBlendWeight> DesiredBoneBlendWeights;
	TArray<FPerBoneBlendWeight> CurrentBoneBlendWeights;

	//varialbes for configuring LayerBlendPerBone
	TBaseBlendedCurve<FDefaultAllocator, UE::Anim::FCurveElementIndexed> CurvePoseSourceIndices;
	TArray<FPerBoneBlendWeight> BoneBlendWeightPelvis;
	TArray<FPerBoneBlendWeight> BoneBlendWeightSpine;
	TArray<FPerBoneBlendWeight> BoneBlendWeightHead;
	TArray<FPerBoneBlendWeight> BoneBlendWeightArmR;
	TArray<FPerBoneBlendWeight> BoneBlendWeightArmL;
	TArray<FPerBoneBlendWeight> BoneBlendWeightHandL;
	TArray<FPerBoneBlendWeight> BoneBlendWeightHandR;
	TArray<FPerBoneBlendWeight> BoneBlendWeightCurves;
	TArray<FPerBoneBlendWeight> BoneBlendWeightOutput;
	TArray<FPerBoneBlendWeight> BoneBlendWeightArmRls;
	TArray<FPerBoneBlendWeight> BoneBlendWeightArmLls;

public:
	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterPelvis;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterSpine;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterHead;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterArmL;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterArmR;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterHandL;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterHandR;

	UPROPERTY(EditAnywhere, editfixedsize, Category = BlendingLayers)
		TArray<FInputBlendPose> BlendFilterCurves;

	TArray<FInputBlendPose> BlendFilterEmpty;
};
