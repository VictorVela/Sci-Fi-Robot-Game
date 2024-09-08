

#include "PoseSnapshotLibrary.h"
#include "Animation/AnimNode_Inertialization.h"
#include "JakubAnimNodesTool/Public/JWAN_PoseSnapShotLogic.h"
#include "Engine/World.h"
#include "AnimationRuntime.h"

FPoseSnapshotReference UPoseSnapshotLibrary::ConvertToPoseSnapshot(const FAnimNodeReference& Node, EAnimNodeReferenceConversionResult& Result)
{
	return FAnimNodeReference::ConvertToType<FPoseSnapshotReference>(Node, Result);
}

FPoseSnapshot UPoseSnapshotLibrary::GetSavedPoseSnapshot(const FPoseSnapshotReference& PoseSnapshot)
{
	FPoseSnapshot PSS;
	PoseSnapshot.CallAnimNodeFunction<FJWAN_PoseSnapShotLogic>( TEXT("GetSavedPoseStructure"), [&PSS](FJWAN_PoseSnapShotLogic& InPoseSnap)
		{
			PSS = InPoseSnap.GetSavedPoseStructure();
		});
	return PSS;
}

FTransform UPoseSnapshotLibrary::GetBoneTransformFromPoseSnapshot(const FPoseSnapshot& PoseSnapshot, FName BoneName, const UAnimInstance* AnimInstance)
{
    FTransform BoneTransform = FTransform::Identity;

    if (!AnimInstance || !PoseSnapshot.bIsValid)
    {
        return BoneTransform;
    }

    FTransform CalculatedTransform = FTransform::Identity;

    USkeletalMeshComponent* Mesh = AnimInstance->GetSkelMeshComponent();
    TArray<FName> ParentBones = GetBoneParents(Mesh, BoneName);

    ParentBones.Add(BoneName);

    for (FName CurrentName : ParentBones)
    {
        int BoneIndex = Mesh->GetBoneIndex(CurrentName);

        if (PoseSnapshot.LocalTransforms.IsValidIndex(BoneIndex) == true)
        {
            CalculatedTransform = PoseSnapshot.LocalTransforms[BoneIndex] * CalculatedTransform;
            //GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, CurrentName.ToString(), false);
        }
    }

    return CalculatedTransform;
}


TArray<FName> UPoseSnapshotLibrary::GetBoneParents(USkeletalMeshComponent* SkeletalMeshComponent, FName BoneName)
{
    TArray<FName> Parents;

    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetSkeletalMeshAsset())
    {
        return Parents;
    }

    const FReferenceSkeleton& RefSkeleton = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetRefSkeleton();
    int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);

    while (BoneIndex != INDEX_NONE)
    {
        // Get the parent bone index
        int32 ParentBoneIndex = RefSkeleton.GetParentIndex(BoneIndex);
        if (ParentBoneIndex == INDEX_NONE)
        {
            break; // We reached the root bone
        }

        // Add the parent bone's name to the array
        FName ParentBoneName = RefSkeleton.GetBoneName(ParentBoneIndex);
        Parents.Add(ParentBoneName);

        // Move up the hierarchy
        BoneIndex = ParentBoneIndex;
    }

    // Reverse the array so it goes from the immediate parent up to the root
    Algo::Reverse(Parents);

    return Parents;
}

/* Draw pose from PoseSnapshot by using points and lines*/
bool UPoseSnapshotLibrary::DrawSkeletonFromPoseSnapshot(const UObject* WorldContextObject, const UAnimInstance* AnimInstance, 
    const FPoseSnapshot& PoseSnapshot, TArray<FName> LimbsName, FName RootBoneName)
{
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World && AnimInstance && PoseSnapshot.bIsValid == true)
    {
        for (FName CurrentName : LimbsName)
        {
            DrawSingleLimbFromPoseSnapshot(World, PoseSnapshot, CurrentName, RootBoneName, AnimInstance);
        }
        return true;
    }
    return false;
}

/* Draw Single Limb */
void UPoseSnapshotLibrary::DrawSingleLimbFromPoseSnapshot(const UWorld* World, const FPoseSnapshot& PoseSnapshot, FName BoneName, 
    FName RootBoneName, const UAnimInstance* AnimInstance)
{

    if (!AnimInstance || !PoseSnapshot.bIsValid)
    { return; }

    FTransform PrevTransform = FTransform::Identity;
    FTransform CalculatedTransform = FTransform::Identity;

    USkeletalMeshComponent* Mesh = AnimInstance->GetSkelMeshComponent();
    TArray<FName> ParentBones = GetBoneParents(Mesh, BoneName);

    ParentBones.Add(BoneName);

    for (FName CurrentName : ParentBones)
    {
        int BoneIndex = Mesh->GetBoneIndex(CurrentName);

        if (PoseSnapshot.LocalTransforms.IsValidIndex(BoneIndex) == true)
        {

            CalculatedTransform = PoseSnapshot.LocalTransforms[BoneIndex] * CalculatedTransform;
            FTransform ConvertedToWorld = CalculatedTransform * Mesh->GetSocketTransform(RootBoneName, ERelativeTransformSpace::RTS_World);
            DrawDebugPoint(World, ConvertedToWorld.GetLocation(), 16.0, FColor::Cyan, false, 0.0);
            if (PrevTransform.GetLocation() != FVector(0,0,0))
            {
                FTransform PrevToWorld = PrevTransform * Mesh->GetSocketTransform(RootBoneName, ERelativeTransformSpace::RTS_World);
                DrawDebugLine(World, ConvertedToWorld.GetLocation(), PrevToWorld.GetLocation(), FColor::Green, false, 0.0, 0, 2.0);
            }
            PrevTransform = CalculatedTransform;
        }
    }
    return;
}
