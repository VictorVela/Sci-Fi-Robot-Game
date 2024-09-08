

#include "MyAnimNode.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Components/SkinnedMeshComponent.h"
#include "Animation/AnimTrace.h"

#define DEFAULT_SOURCEINDEX 0xFF
/////////////////////////////////////////////////////
// FAnimNode_LayeredBoneBlend

void FAnimNode_LayeredBone::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	FAnimNode_Base::Initialize_AnyThread(Context);

	const int NumPoses = BlendPoses.Num();
	checkSlow(BlendWeights.Num() == NumPoses);

	// initialize children
	BasePose.Initialize(Context);

	if (NumPoses > 0)
	{
		for (int32 ChildIndex = 0; ChildIndex < NumPoses; ++ChildIndex)
		{
			BlendPoses[ChildIndex].Initialize(Context);
		}
	}
}

void FAnimNode_LayeredBone::RebuildPerBoneBlendWeights(const USkeleton* InSkeleton)
{
	//GEngine->AddOnScreenDebugMessage(-1, 7, FColor::Yellow, FVector(LayerSetup[0].BranchFilters.Num(), 1000, 0).ToString()); //Wypisz ile jest branch filtrów

	if (InSkeleton)
	{
		if (BlendMode == ELayeredBlendMode::BranchFilter)
		{
			FAnimationRuntime::CreateMaskWeights(PerBoneBlendWeights, LayerSetup, InSkeleton);
		}
		else
		{
			FAnimationRuntime::CreateMaskWeights(PerBoneBlendWeights, BlendMasks, InSkeleton);
		}

		SkeletonGuid = InSkeleton->GetGuid();
		VirtualBoneGuid = InSkeleton->GetVirtualBoneGuid();
	}
}

bool FAnimNode_LayeredBone::ArePerBoneBlendWeightsValid(const USkeleton* InSkeleton) const
{
	return (InSkeleton != nullptr && InSkeleton->GetGuid() == SkeletonGuid && InSkeleton->GetVirtualBoneGuid() == VirtualBoneGuid);
}

void FAnimNode_LayeredBone::UpdateCachedBoneData(const FBoneContainer& RequiredBones, const USkeleton* Skeleton)
{
	if (RequiredBones.GetSerialNumber() == RequiredBonesSerialNumber)
	{
		return;
	}

	if (!ArePerBoneBlendWeightsValid(Skeleton))
	{
		RebuildPerBoneBlendWeights(Skeleton);
	}

	// build desired bone weights
	const TArray<FBoneIndexType>& RequiredBoneIndices = RequiredBones.GetBoneIndicesArray();
	const int32 NumRequiredBones = RequiredBoneIndices.Num();
	DesiredBoneBlendWeights.SetNumZeroed(NumRequiredBones);
	for (int32 RequiredBoneIndex = 0; RequiredBoneIndex < NumRequiredBones; RequiredBoneIndex++)
	{
		const int32 SkeletonBoneIndex = RequiredBones.GetSkeletonIndex(FCompactPoseBoneIndex(RequiredBoneIndex));
		if (ensure(SkeletonBoneIndex != INDEX_NONE))
		{
			DesiredBoneBlendWeights[RequiredBoneIndex] = PerBoneBlendWeights[SkeletonBoneIndex];
		}
	}

	CurrentBoneBlendWeights.Reset(DesiredBoneBlendWeights.Num());
	CurrentBoneBlendWeights.AddZeroed(DesiredBoneBlendWeights.Num());

	//Reinitialize bone blend weights now that we have cleared them
	FAnimationRuntime::UpdateDesiredBoneWeight(DesiredBoneBlendWeights, CurrentBoneBlendWeights, BlendWeights);

	// Build curve source indices
	{
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
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	CurvePoseSourceIndices.Empty();

	RequiredBonesSerialNumber = RequiredBones.GetSerialNumber();
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Red, FVector(CurrentBoneBlendWeights.Num(), DesiredBoneBlendWeights.Num(), 0).ToString()); //Print Lenght
}

void FAnimNode_LayeredBone::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	BasePose.CacheBones(Context);
	int32 NumPoses = BlendPoses.Num();
	for (int32 ChildIndex = 0; ChildIndex < NumPoses; ChildIndex++)
	{
		BlendPoses[ChildIndex].CacheBones(Context);
	}

	UpdateCachedBoneData(Context.AnimInstanceProxy->GetRequiredBones(), Context.AnimInstanceProxy->GetSkeleton());
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Red, FVector(LayerSetup.Num(), LayerSetup[0].BranchFilters[0].BlendDepth, 10000).ToString() + (LayerSetup[0].BranchFilters[0].BoneName).ToString()); //Print Lenght
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Red, FVector(CurrentBoneBlendWeights.Num(), DesiredBoneBlendWeights.Num(), RequiredBonesSerialNumber).ToString()); //Print Lenght
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Orange, FVector(CurvePoseSourceIndices.Num(), PerBoneBlendWeights.Num(), RequiredBonesSerialNumber).ToString()); //Print Lenght

	AnimInst = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject());
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Green, FVector(CurrentBoneBlendWeights.Num(), DesiredBoneBlendWeights.Num(), RequiredBonesSerialNumber).ToString()); //Print Lenght
	//GEngine->AddOnScreenDebugMessage(-1, 6, FColor::Yellow, FVector(CurvePoseSourceIndices.Num(), PerBoneBlendWeights.Num(), RequiredBonesSerialNumber).ToString()); //Print Lenght

	Skel = Context.AnimInstanceProxy->GetSkeleton();
}

void FAnimNode_LayeredBone::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)
	bHasRelevantPoses = false;
	int32 RootMotionBlendPose = -1;
	float RootMotionWeight = 0.f;
	const float RootMotionClearWeight = bBlendRootMotionBasedOnRootBone ? 0.f : 1.f;

	if (IsLODEnabled(Context.AnimInstanceProxy))
	{
		GetEvaluateGraphExposedInputs().Execute(Context);

		for (int32 ChildIndex = 0; ChildIndex < BlendPoses.Num(); ++ChildIndex)
		{
			const float ChildWeight = BlendWeights[ChildIndex];
			if (FAnimWeight::IsRelevant(ChildWeight))
			{
				if (bHasRelevantPoses == false)
				{
					// Update cached data now we know we might be valid
					UpdateCachedBoneData(Context.AnimInstanceProxy->GetRequiredBones(), Context.AnimInstanceProxy->GetSkeleton());

					// Update weights
					FAnimationRuntime::UpdateDesiredBoneWeight(DesiredBoneBlendWeights, CurrentBoneBlendWeights, BlendWeights);
					bHasRelevantPoses = true;

					if (bBlendRootMotionBasedOnRootBone && !CurrentBoneBlendWeights.IsEmpty())
					{
						const float NewRootMotionWeight = CurrentBoneBlendWeights[0].BlendWeight;
						if (NewRootMotionWeight > ZERO_ANIMWEIGHT_THRESH)
						{
							RootMotionWeight = NewRootMotionWeight;
							RootMotionBlendPose = CurrentBoneBlendWeights[0].SourceIndex;
						}
					}
				}

				const float ThisPoseRootMotionWeight = (ChildIndex == RootMotionBlendPose) ? RootMotionWeight : RootMotionClearWeight;
				BlendPoses[ChildIndex].Update(Context.FractionalWeightAndRootMotion(ChildWeight, ThisPoseRootMotionWeight));
			}
		}
	}

	// initialize children
	const float BaseRootMotionWeight = 1.f - RootMotionWeight;

	if (BaseRootMotionWeight < ZERO_ANIMWEIGHT_THRESH)
	{
		BasePose.Update(Context.FractionalWeightAndRootMotion(1.f, BaseRootMotionWeight));
	}
	else
	{
		BasePose.Update(Context);
	}

	TRACE_ANIM_NODE_VALUE(Context, TEXT("Num Poses"), BlendPoses.Num());
}

void FAnimNode_LayeredBone::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	ANIM_MT_SCOPE_CYCLE_COUNTER(BlendPosesInGraph, !IsInGameThread());

	FPoseContext CurrentPoseContext(Output);

	const int NumPoses = BlendPoses.Num();
	if ((NumPoses == 0) || !bHasRelevantPoses)
	{
		BasePose.Evaluate(Output);
	}
	else
	{
		FPoseContext BasePoseContext(Output);

		// evaluate children
		BasePose.Evaluate(BasePoseContext);

		TArray<FCompactPose> TargetBlendPoses;
		TargetBlendPoses.SetNum(NumPoses);

		TArray<FBlendedCurve> TargetBlendCurves;
		TargetBlendCurves.SetNum(NumPoses);

		TArray<UE::Anim::FStackAttributeContainer> TargetBlendAttributes;
		TargetBlendAttributes.SetNum(NumPoses);

		for (int32 ChildIndex = 0; ChildIndex < NumPoses; ++ChildIndex)
		{
			if (FAnimWeight::IsRelevant(BlendWeights[ChildIndex]))
			{
				
				BlendPoses[ChildIndex].Evaluate(CurrentPoseContext);

				TargetBlendPoses[ChildIndex].MoveBonesFrom(CurrentPoseContext.Pose);
				TargetBlendCurves[ChildIndex].MoveFrom(CurrentPoseContext.Curve);
				TargetBlendAttributes[ChildIndex].MoveFrom(CurrentPoseContext.CustomAttributes);
			}
			else
			{
				TargetBlendPoses[ChildIndex].ResetToRefPose(BasePoseContext.Pose.GetBoneContainer());
				TargetBlendCurves[ChildIndex].InitFrom(Output.Curve);
			}
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
		if (bMeshSpaceRotationBlend)
		{
			BlendFlags |= FAnimationRuntime::EBlendPosesPerBoneFilterFlags::MeshSpaceRotation;
		}
		if (bMeshSpaceScaleBlend)
		{
			BlendFlags |= FAnimationRuntime::EBlendPosesPerBoneFilterFlags::MeshSpaceScale;
		}

		for (int ii = 0; ii < CurrentBoneBlendWeights.Num(); ii++)
		{
			int ttt = CurrentBoneBlendWeights[ii].SourceIndex;
			FString TargetString = "NONE";
			if (Skel)
			{
				TargetString = FVector(ttt, CurrentBoneBlendWeights[ii].BlendWeight, 0).ToString() + " = " + Skel->GetReferenceSkeleton().GetBoneName(ii).ToString();
			}
			//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, TargetString); //Print Lenght
		}

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FVector(CurrentBoneBlendWeights.Num(), CurrentPoseContext.Curve.Num(), BasePoseContext.Curve.Num()).ToString());
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Orange, FVector(TargetBlendCurves[0].Num(), TargetBlendAttributes.Num(), CurrentBoneBlendWeights.Num()).ToString());

		FAnimationPoseData AnimationPoseData(Output);
		FAnimationRuntime::BlendPosesPerBoneFilter(BasePoseContext.Pose, TargetBlendPoses, BasePoseContext.Curve, TargetBlendCurves, BasePoseContext.CustomAttributes, TargetBlendAttributes, AnimationPoseData, CurrentBoneBlendWeights, BlendFlags, CurveBlendOption);
	}
}


void FAnimNode_LayeredBone::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(GatherDebugData)
	const int NumPoses = BlendPoses.Num();

	FString DebugLine = DebugData.GetNodeName(this);
	DebugLine += FString::Printf(TEXT("(Num Poses: %i)"), NumPoses);
	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData.BranchFlow(1.f));

	for (int32 ChildIndex = 0; ChildIndex < NumPoses; ++ChildIndex)
	{
		BlendPoses[ChildIndex].GatherDebugData(DebugData.BranchFlow(BlendWeights[ChildIndex]));
	}
}

void FAnimNode_LayeredBone::SetBlendMask(int32 InPoseIndex, UBlendProfile* InBlendMask)
{
	check(BlendMode == ELayeredBlendMode::BlendMask);
	check(BlendPoses.IsValidIndex(InPoseIndex));
	check(BlendMasks.IsValidIndex(InPoseIndex));

	BlendMasks[InPoseIndex] = InBlendMask;

	InvalidatePerBoneBlendWeights();
}