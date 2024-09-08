#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimInstanceProxy.h"
#include "JWAN_PoseSnapShotLogic.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct JAKUBANIMNODESTOOL_API FJWAN_PoseSnapShotLogic : public FAnimNode_Base
{
    GENERATED_USTRUCT_BODY()

public:
    // Input pose link
    UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
        FPoseLink SourcePose;

    /** Snapshot to use. This should be populated at first by calling SnapshotPose */
        FPoseSnapshot Snapshot;

    /* This information is currently not using */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot", meta = (PinShownByDefault))
        FName SnapshotName;

    /* Chech this if you want to node save in PoseSnapshot bones name. Without this Eveluate saving only bone transforms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snapshot")
        bool SaveBonesNameToPose = false;


    bool bInitializeLastValuesMap;
    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
    // End of FAnimNode_Base interface

    // Initialize the snapshot pointer
    void InitializeSnapshot(FPoseSnapshot* InSnapshot);

    virtual FPoseSnapshot GetSavedPoseStructure();

private:
    UAnimInstance* AnimInst;

};