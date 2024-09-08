
#include "JWAN_LayerBlendingLogic.h"
#include "AnimationRuntime.h"
#include "Animation/AnimTrace.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNode_Inertialization.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/Skeleton.h"

#define DEFAULT_SOURCEINDEX 0xFF
#define MIN_DESIRED_BONE_WEIGHT 0.005

void FJWAN_LayerBlendingLogic::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    //Funkcja inicjuj¹ca noda
    AnimInst = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject());
    BaseLayerInput.Initialize(Context);
    OverlayLayerInput.Initialize(Context);
    BasePosesInput.Initialize(Context);

    WeightData0.Reset(); WeightData1.Reset(); WeightData2.Reset(); WeightData3.Reset(); WeightData4.Reset(); WeightData5.Reset();

}

void FJWAN_LayerBlendingLogic::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
    //Funkcja inicjuj¹ca noda
    //DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
    BaseLayerInput.CacheBones(Context);
    OverlayLayerInput.CacheBones(Context);
    BasePosesInput.CacheBones(Context);

    Skel = Context.AnimInstanceProxy->GetSkeleton();
}

void FJWAN_LayerBlendingLogic::Update_AnyThread(const FAnimationUpdateContext& Context)
{
    GetEvaluateGraphExposedInputs().Execute(Context);
    BaseLayerInput.Update(Context);
    OverlayLayerInput.Update(Context);
    BasePosesInput.Update(Context);

    SlotUpdate(Context, AllSlotsName.Legs, BaseLayerInput, WeightData0);
    SlotUpdate(Context, AllSlotsName.Pelvis, BaseLayerInput, WeightData1);
    SlotUpdate(Context, AllSlotsName.Spine, BaseLayerInput, WeightData2);
    SlotUpdate(Context, AllSlotsName.Head, BaseLayerInput, WeightData3);
    SlotUpdate(Context, AllSlotsName.ArmL, BaseLayerInput, WeightData4);
    SlotUpdate(Context, AllSlotsName.ArmR, BaseLayerInput, WeightData5);

    //UpdateBlendWeightStrenght(BoneBlendWeightPelvis, 1.0);
    //UpdateBlendWeightStrenght(BoneBlendWeightSpine, 1.0);
    //UpdateBlendWeightStrenght(BoneBlendWeightHead, 1.0);
    UpdateBlendWeightStrenght(BoneBlendWeightArmR, ArmR_MS);
    UpdateBlendWeightStrenght(BoneBlendWeightArmRls, CurveValue(CurvesName.ArmR_LS));
    UpdateBlendWeightStrenght(BoneBlendWeightArmL, ArmL_MS);
    UpdateBlendWeightStrenght(BoneBlendWeightArmLls, CurveValue(CurvesName.ArmL_LS));
    UpdateBlendWeightStrenght(BoneBlendWeightHandL, CurveValue(CurvesName.HandL));
    UpdateBlendWeightStrenght(BoneBlendWeightHandR, CurveValue(CurvesName.HandR));
}

void FJWAN_LayerBlendingLogic::Evaluate_AnyThread(FPoseContext& Output)
{
    FPoseContext PoseBase(Output);
    FPoseContext PoseOverlay(Output);
    FPoseContext PoseDef(Output);
    BaseLayerInput.Evaluate(PoseBase);
    OverlayLayerInput.Evaluate(PoseOverlay);
    BasePosesInput.Evaluate(PoseDef);

    // Make Dynamic Additives Pose
    FPoseContext BaseAdditiveMS(Output);
    FPoseContext BaseAdditiveLSpace(Output);
    //Utworzenie animacji Additive z wejœæ noda
    MakeAdditiveContext(PoseDef, PoseBase, true, BaseAdditiveMS);
    MakeAdditiveContext(PoseDef, PoseBase, false, BaseAdditiveLSpace);

    //Zapis wyników do zmiennych lokalnych dla poszczególnych partii szkieletu
    FPoseContext PoseLegs(Output);
    FPoseContext PosePelvis(Output);
    FPoseContext PoseSpine(Output);
    FPoseContext PoseHead(Output);
    FPoseContext PoseArmL(Output);
    FPoseContext PoseArmR(Output);

    FPoseContext LayerBlendOut1(Output);
    FPoseContext LayerBlendOut2(Output);
    FPoseContext LayerBlendCurvesOut(Output);

    //Zaaplikowanie animacji Additive dla poszczególnych czêsci szkieletu - przestrzeñ szkieletu
    ApplyAdditiveWithSlotEvaluate(PoseLegs, PoseBase, PoseOverlay, BaseAdditiveMS, 1.0, CurveValue(CurvesName.Legs), AllSlotsName.Legs, WeightData0, true);
    ApplyAdditiveWithSlotEvaluate(PosePelvis, PoseBase, PoseOverlay, BaseAdditiveMS, 1.0, CurveValue(CurvesName.Pelvis), AllSlotsName.Pelvis, WeightData1, true);
    ApplyAdditiveWithSlotEvaluate(PoseSpine, PoseBase, PoseOverlay, BaseAdditiveMS, CurveValue(CurvesName.SpineAdd), CurveValue(CurvesName.Spine), AllSlotsName.Spine, WeightData2, true);
    ApplyAdditiveWithSlotEvaluate(PoseHead, PoseBase, PoseOverlay, BaseAdditiveMS, CurveValue(CurvesName.HeadAdd), CurveValue(CurvesName.Head), AllSlotsName.Head, WeightData3, true);
    //Zaaplikowanie animacji Additive dla poszczególnych czêsci szkieletu - przestrzeñ lokalna
    ApplyAdditiveWithSlotEvaluate(PoseArmL, PoseBase, PoseOverlay, BaseAdditiveLSpace, CurveValue(CurvesName.ArmL_Add), CurveValue(CurvesName.ArmL), AllSlotsName.ArmL, WeightData4, false);
    ApplyAdditiveWithSlotEvaluate(PoseArmR, PoseBase, PoseOverlay, BaseAdditiveLSpace, CurveValue(CurvesName.ArmR_Add), CurveValue(CurvesName.ArmR), AllSlotsName.ArmR, WeightData5, false);

    BlendPerBoneEvaluate(LayerBlendOut1, PoseLegs, PosePelvis, true, false, BoneBlendWeightPelvis, false, 1.0, PoseLegs.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(LayerBlendOut2, LayerBlendOut1, PoseSpine, true, false, BoneBlendWeightSpine, false, CurveValue(CurvesName.Spine), LayerBlendOut1.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, LayerBlendOut2, PoseHead, true, false, BoneBlendWeightHead, false, CurveValue(CurvesName.Head), LayerBlendOut2.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseArmL, true, false, BoneBlendWeightArmL, false, ArmL_MS, PoseBase.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseArmL, true, false, BoneBlendWeightArmLls, false, CurveValue(CurvesName.ArmL_LS), PoseBase.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseArmR, true, false, BoneBlendWeightArmR, false, ArmR_MS, PoseBase.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseArmR, true, false, BoneBlendWeightArmRls, false, CurveValue(CurvesName.ArmR_LS), PoseBase.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseOverlay, false, false, BoneBlendWeightHandL, false, CurveValue(CurvesName.HandL), PoseBase.Curve, PoseOverlay.Curve);
    BlendPerBoneEvaluate(Output, Output, PoseOverlay, false, false, BoneBlendWeightHandR, false, CurveValue(CurvesName.HandR), PoseBase.Curve, PoseOverlay.Curve);
    //Filtering Anim Curves
    BlendPerBoneEvaluate(LayerBlendCurvesOut, PoseBase, PoseOverlay, false, false, BoneBlendWeightCurves, true, 1.0, PoseBase.Curve, PoseOverlay.Curve);
    //Final OUTPUT
    BlendPerBoneEvaluate(Output, Output, LayerBlendCurvesOut, false, false, BoneBlendWeightOutput, false, 1.0, Output.Curve, LayerBlendCurvesOut.Curve);

}


void FJWAN_LayerBlendingLogic::GatherDebugData(FNodeDebugData& DebugData)
{
    FString DebugLine = DebugData.GetNodeName(this);


    DebugData.AddDebugItem(DebugLine);

    BaseLayerInput.GatherDebugData(DebugData);
}

void FJWAN_LayerBlendingLogic::BlendTwoPosesContext(FPoseContext& PoseA, FPoseContext& PoseB, float Alpha, FPoseContext& OP)
{
    if (Alpha == 0.0)
    {
        OP = PoseA;
        return;
    }
    else if (Alpha == 1.0)
    {
        OP = PoseB;
        return;
    }
    FAnimationPoseData BlendedAnimationPoseData(OP);
    const FAnimationPoseData AnimationPoseOneData(PoseA);
    const FAnimationPoseData AnimationPoseTwoData(PoseB);
    FAnimationRuntime::BlendTwoPosesTogether(AnimationPoseOneData, AnimationPoseTwoData, Alpha, BlendedAnimationPoseData);
    //OP.Curve.Lerp(PoseA.Curve, PoseB.Curve, Alpha);
    return;
}

//MAKE ADDITIVE POSE FROM TWO ANIMATIONS
void FJWAN_LayerBlendingLogic::MakeAdditiveContext(FPoseContext& Base, FPoseContext& Add, bool MeshSpace, FPoseContext& OP)
{
    FPoseContext BaseEvalContext(OP);
    BaseEvalContext = Base;
    OP = Add;
    if (MeshSpace)
    {
        FAnimationRuntime::ConvertPoseToMeshRotation(OP.Pose);
        FAnimationRuntime::ConvertPoseToMeshRotation(BaseEvalContext.Pose);
    }
    FAnimationRuntime::ConvertPoseToAdditive(OP.Pose, BaseEvalContext.Pose);
    OP.Curve.ConvertToAdditive(Base.Curve);
    UE::Anim::Attributes::ConvertToAdditive(OP.CustomAttributes, Base.CustomAttributes);
}

//LAYER BLENDING SINGLE - EVALUATE
void FJWAN_LayerBlendingLogic::ApplyAdditiveWithSlotEvaluate(FPoseContext& OP, FPoseContext& LBase, FPoseContext& LOverlay, FPoseContext& LAdditive,
    float AddAlpha, float BlendAlpha, FName AnimSlotName, FSlotNodeWeightInfo SlotWeight, bool UseMeshSpace)
{
    FPoseContext OverlayWithSlot(OP);
    OverlayWithSlot = LOverlay;

    if (BlendAlpha == 0.0)
    {
        OP = LBase;
        return;
    }
    float a = 0.0;
    //Próba zaaplikowania slotu
    if (SlotWeight.SlotNodeWeight <= ZERO_ANIMWEIGHT_THRESH)
    {
        OverlayWithSlot = LOverlay;
    }
    else
    {
        //GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, AnimSlotName.ToString());
        const FAnimationPoseData SourcePoseData(LOverlay);
        FAnimationPoseData OutputPoseData(OverlayWithSlot);
        OverlayWithSlot.AnimInstanceProxy->SlotEvaluatePose(AnimSlotName, SourcePoseData, SlotWeight.SourceWeight, OutputPoseData, SlotWeight.SlotNodeWeight, SlotWeight.TotalNodeWeight);
        checkSlow(!OverlayWithSlot.ContainsNaN());
        checkSlow(OverlayWithSlot.IsNormalized());
    }
    //Overlay Slot Powinien teraz zawierac zaaplikowany slot
    //Dodanie Animacji Additive
    if (AddAlpha > 0.0)
    {
        FAnimationPoseData AddBase(OverlayWithSlot);
        FAnimationPoseData AddAdditive(LAdditive);
        a = FMath::Clamp<float>(AddAlpha, 0.f, 1.f);
        if (UseMeshSpace == true)
        {
            FAnimationRuntime::AccumulateMeshSpaceRotationAdditiveToLocalPose(AddBase, LAdditive, a);
        }
        else
        {
            FAnimationRuntime::AccumulateAdditivePose(AddBase, LAdditive, a, AAT_LocalSpaceBase);
        }
        OverlayWithSlot.Pose.NormalizeRotations();
        //OverlayWithSlot.Curve.Accumulate(AddBase.GetCurve(), 1-a);

    }
    if (BlendAlpha >= 1.0)
    {
        OP = OverlayWithSlot;
        return;
    }
    a = FMath::Clamp<float>(BlendAlpha, 0.f, 1.f);
    FAnimationPoseData BlendedAnimationPoseData(OP);
    const FAnimationPoseData AnimationPoseOneData(LBase);
    const FAnimationPoseData AnimationPoseTwoData(OverlayWithSlot);
    FAnimationRuntime::BlendTwoPosesTogether(AnimationPoseTwoData, AnimationPoseOneData, a, BlendedAnimationPoseData);
    //OP.Curve.Lerp(LBase.Curve, OverlayWithSlot.Curve, a);
    return;
}

//LAYER BLENDING SINGLE - UPDATE
void FJWAN_LayerBlendingLogic::ApplyAdditiveWithSlotUpdate(FPoseContext& LBase, FPoseContext& LOverlay, FPoseContext& LAdditive,
    float AddAlpha, float BlendAlpha, FName AnimSlotName)
{
    return;
}



void FJWAN_LayerBlendingLogic::BlendPerBoneEvaluate(FPoseContext& OP, FPoseContext& PoseA, FPoseContext& PoseB, bool UseMeshSpaceRotation, bool UseMeshSpaceScaleBlend, 
    TArray<FPerBoneBlendWeight> CurrentBoneBlendW, bool UseBlendByWeight, float BlendAlpha, FBlendedCurve CurvesA, FBlendedCurve CurvesB)
{
    const int NumPoses = 1;
    FPoseContext BasePoseContext(OP);
    BasePoseContext = PoseA;

    TArray<float> BlendWeights = { 1 };

    TArray<FCompactPose> TargetBlendPoses;
    TargetBlendPoses.SetNum(NumPoses);

    TArray<FBlendedCurve> TargetBlendCurves;
    TargetBlendCurves.SetNum(NumPoses);

    TArray<UE::Anim::FStackAttributeContainer> TargetBlendAttributes;
    TargetBlendAttributes.SetNum(NumPoses);

    if (BlendAlpha <= 0.0)
    {
        OP = PoseA;
        return;
    }

    if (FAnimWeight::IsRelevant(BlendAlpha))
    {
        FPoseContext CurrentPoseContext(PoseB);
        CurrentPoseContext = PoseB;
        //GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, FVector(TargetBlendCurves[0].CurveWeights.Num(), 0, 0).ToString());
        TargetBlendPoses[0].MoveBonesFrom(CurrentPoseContext.Pose);
        TargetBlendCurves[0].MoveFrom(CurvesB);
        TargetBlendAttributes[0].MoveFrom(CurrentPoseContext.CustomAttributes);
    }
    else
    {
        TargetBlendPoses[0].ResetToRefPose(PoseA.Pose.GetBoneContainer());
        TargetBlendCurves[0].InitFrom(CurvesA);
    }

    // filter to make sure it only includes curves that are linked to the correct bone filter
    UE::Anim::FNamedValueArrayUtils::RemoveByPredicate(BasePoseContext.Curve, CurvePoseSourceIndices,
        [](const UE::Anim::FCurveElement& InOutBasePoseElement, const UE::Anim::FCurveElementIndexed& InSourceIndexElement)
        {
            // if source index is set, remove base pose curve value
            return (InSourceIndexElement.Index != INDEX_NONE);
        });

    // Filter child pose curves
    for (int32 ChildIndex = 0; ChildIndex < NumPoses; ++ChildIndex)
    {
        UE::Anim::FNamedValueArrayUtils::RemoveByPredicate(TargetBlendCurves[ChildIndex], CurvePoseSourceIndices,
            [ChildIndex](const UE::Anim::FCurveElement& InOutBasePoseElement, const UE::Anim::FCurveElementIndexed& InSourceIndexElement)
            {
                // if not source, remove it
                return (InSourceIndexElement.Index != INDEX_NONE) && (InSourceIndexElement.Index != ChildIndex);
            });
    }

    FAnimationRuntime::EBlendPosesPerBoneFilterFlags BlendFlags = FAnimationRuntime::EBlendPosesPerBoneFilterFlags::None;
    if (UseMeshSpaceRotation)
    { BlendFlags |= FAnimationRuntime::EBlendPosesPerBoneFilterFlags::MeshSpaceRotation; }
    if (UseMeshSpaceScaleBlend)
    { BlendFlags |= FAnimationRuntime::EBlendPosesPerBoneFilterFlags::MeshSpaceScale; }

    //Set Curve BlendOption
    TEnumAsByte<enum ECurveBlendOption::Type> CurveBlendOption;
    if (UseBlendByWeight == true)
    { CurveBlendOption = ECurveBlendOption::BlendByWeight; }
    else
    { CurveBlendOption = ECurveBlendOption::Override; }

    FAnimationPoseData AnimationPoseData(OP);
    FAnimationRuntime::BlendPosesPerBoneFilter(BasePoseContext.Pose, TargetBlendPoses, BasePoseContext.Curve, TargetBlendCurves, BasePoseContext.CustomAttributes, TargetBlendAttributes, AnimationPoseData, CurrentBoneBlendW, BlendFlags, CurveBlendOption);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Simple Curve Value From Anim Inst
float FJWAN_LayerBlendingLogic::CurveValue(FName CurveName)
{
    //return 1.0;
    if(IsValid(AnimInst)==true)
    {
        //FString cur = CurveName.ToString() + " " + FVector(AnimInst->GetCurveValue(CurveName), 0, 0).ToString();
        //GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, cur);
        return FMath::Clamp<float>(AnimInst->GetCurveValue(CurveName), 0.0, 1.0);
    }
    return 0.0f;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ||| L A Y E R   B L E N D   P E R   B O N E |||
void FJWAN_LayerBlendingLogic::UpdateMaskWeightsValue(const FAnimationUpdateContext& C, TArray<FPerBoneBlendWeight>& DBBW, TArray<FPerBoneBlendWeight> CBBW, 
    TArray<float> BlendWeightsPerPose, bool BlendRootMotionBasedOnRoot, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid, uint16& RBSN, TArray<uint8>& CPSI, 
    TArray<FInputBlendPose> InLayerSetup, float BlendAlpha)
{
    return;
}

void FJWAN_LayerBlendingLogic::UpdateBlendWeightStrenght(TArray<FPerBoneBlendWeight>& BonesWeight, float Alpha)
{
    if (BonesWeight.Num() > 2)
    {
        for (int i = 0; i < BonesWeight.Num(); i++)
        {
            if (BonesWeight[i].BlendWeight >= MIN_DESIRED_BONE_WEIGHT)
            {
                if (abs(BonesWeight[i].BlendWeight - Alpha) < 0.01) { return; }
                BonesWeight[i].BlendWeight = FMath::Clamp<float>(Alpha, MIN_DESIRED_BONE_WEIGHT + (0.5 * MIN_DESIRED_BONE_WEIGHT), 1.0);
            }
        }
        return;
    }
    return;
}

bool FJWAN_LayerBlendingLogic::RebuildAllBlendWeights(const USkeleton* InSkeleton)
{
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightPelvis, BlendFilterPelvis, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightSpine, BlendFilterSpine, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightHead, BlendFilterHead, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightArmL, BlendFilterArmL, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightArmLls, BlendFilterArmL, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightArmR, BlendFilterArmR, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightArmRls , BlendFilterArmR, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightHandL, BlendFilterHandL, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightHandR, BlendFilterHandR, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightCurves, BlendFilterCurves, SkeletonGuid, VirtualBoneGuid);
    RebuildPerBoneBlendWeights(InSkeleton, BoneBlendWeightOutput, BlendFilterEmpty, SkeletonGuid, VirtualBoneGuid);
    return true;
}

void FJWAN_LayerBlendingLogic::UpdateLayerBlendCurvesSource(const FBoneContainer& RequiredBones, const USkeleton* Skeleton, TArray<FPerBoneBlendWeight> BonesWeight)
{
    // Build curve source indices
    {
        DesiredBoneBlendWeights = BonesWeight;

        CurvePoseSourceIndices.Empty();
        CurvePoseSourceIndices.Reserve(Skeleton->GetNumCurveMetaData());

        Skeleton->ForEachCurveMetaData([this, &RequiredBones](const FName& InCurveName, const FCurveMetaData& InMetaData)
            {
                for (const FBoneReference& LinkedBone : InMetaData.LinkedBones)
                {
                    FCompactPoseBoneIndex CompactPoseIndex = LinkedBone.GetCompactPoseIndex(RequiredBones);
                    if (CompactPoseIndex != INDEX_NONE)
                    {
                        if (DesiredBoneBlendWeights[CompactPoseIndex.GetInt()].BlendWeight > 0.f)
                        {
                            CurvePoseSourceIndices.Add(InCurveName, DesiredBoneBlendWeights[CompactPoseIndex.GetInt()].SourceIndex);
                            break;
                        }
                    }
                }
            });
    }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ||| L A Y E R   B L E N D   P E R   B O N E |||  Rebuild cache per bone blend weights from the skeleton
void FJWAN_LayerBlendingLogic::RebuildPerBoneBlendWeights(const USkeleton* InSkeleton, TArray<FPerBoneBlendWeight>& PBBW, TArray<FInputBlendPose> InLayerSetup, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid)
{
    if (InSkeleton)
    {
        FAnimationRuntime::CreateMaskWeights(PBBW, InLayerSetup, InSkeleton);
    }
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ||| L A Y E R   B L E N D   P E R   B O N E |||  Check whether per-bone blend weights are valid according to the skeleton (GUID check)
bool FJWAN_LayerBlendingLogic::ArePerBoneBlendWeightsValid(const USkeleton* InSkeleton, FGuid& InSkelGuid, FGuid& InVirtualBonesGuid)
{
    return (InSkeleton != nullptr && InSkeleton->GetGuid() == InSkelGuid && InSkeleton->GetVirtualBoneGuid() == InVirtualBonesGuid);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ||| L A Y E R   B L E N D   P E R   B O N E |||  Update Cached Bone Data
void FJWAN_LayerBlendingLogic::UpdateCachedBoneData(const FBoneContainer& RequiredBones, const USkeleton* Skeleton, TArray<FPerBoneBlendWeight>& CurrentBoneBlendOutput, FGuid& InSkelGuid,
    FGuid& InVirtualBonesGuid, uint16& RBSN, TArray<uint8>& CPSI, TArray<FInputBlendPose> InLayerSetup, float BlendAlpha)
{

    //For LayerBlendPerBone
    if (!ArePerBoneBlendWeightsValid(Skeleton, SkeletonGuid, VirtualBoneGuid))
    {
        SkeletonGuid = Skeleton->GetGuid();
        VirtualBoneGuid = Skeleton->GetVirtualBoneGuid();
    }


}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ||| L A Y E R   B L E N D   P E R   B O N E ||| MACRO - CREATE LayerBlend Mask
TArray<FInputBlendPose> FJWAN_LayerBlendingLogic::CreateLayerBlendValue(TArray<FName> BonesName, TArray<int> BlendDepth)
{
    TArray<FInputBlendPose> LBP = {};
    FInputBlendPose AddBlendPose;
    FBranchFilter AddFilter;
    int ii = -1;
    if (BonesName.Num() > 0 && BlendDepth.Num() > 0)
    {
        for (FName& CurrentName : BonesName)
        {
            ii = ii + 1;
            AddFilter.BoneName = CurrentName;
            if (BlendDepth.IsValidIndex(ii) == true)
            { AddFilter.BlendDepth = BlendDepth[ii]; }
            else
            { AddFilter.BlendDepth = 0; }
            AddBlendPose.BranchFilters.Add(AddFilter);
        }
        LBP.Add(AddBlendPose);
        return LBP;
    }
    return LBP;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SLOT UPDATE
void FJWAN_LayerBlendingLogic::SlotUpdate(const FAnimationUpdateContext& Context, FName SName, FPoseLink InputPose, FSlotNodeWeightInfo& Weight)
{
    // Update weights.
    Context.AnimInstanceProxy->GetSlotWeight(SName, Weight.SlotNodeWeight, Weight.SourceWeight, Weight.TotalNodeWeight);
    // Update cache in AnimInstance.
    Context.AnimInstanceProxy->UpdateSlotNodeWeight(SName, Weight.SlotNodeWeight, Context.GetFinalBlendWeight());

    //GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FVector(Weight.SlotNodeWeight, Weight.SourceWeight, Weight.TotalNodeWeight).ToString());

    UE::Anim::FSlotInertializationRequest InertializationRequest;
    if (Context.AnimInstanceProxy->GetSlotInertializationRequest(SName, InertializationRequest))
    {
        UE::Anim::IInertializationRequester* InertializationRequester = Context.GetMessage<UE::Anim::IInertializationRequester>();
        if (InertializationRequester)
        {
            InertializationRequester->RequestInertialization(InertializationRequest.Get<0>(), InertializationRequest.Get<1>());
        }
        else
        {
            FAnimNode_Inertialization::LogRequestError(Context, InputPose);
        }
    }
}



