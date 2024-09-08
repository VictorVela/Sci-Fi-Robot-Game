
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimExecutionContext.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimNodeReference.h"
#include "PoseSnapshotLibrary.generated.h"


struct FJWAN_PoseSnapShotLogic;

USTRUCT(BlueprintType)
struct FPoseSnapshotReference : public FAnimNodeReference
{
	GENERATED_BODY()

	typedef FJWAN_PoseSnapShotLogic FInternalNodeType;
};


UCLASS()
class JAKUBANIMNODESTOOL_API UPoseSnapshotLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Get a Pose Snapshot context from an anim node context. */
	UFUNCTION(BlueprintCallable, Category = "Pose Snapshot", meta = (BlueprintThreadSafe, ExpandEnumAsExecs = "Result"))
	static FPoseSnapshotReference ConvertToPoseSnapshot(const FAnimNodeReference& Node, EAnimNodeReferenceConversionResult& Result);

	/** Get the PoseSnapshot structure from PoseSnapshot saver. */
	UFUNCTION(BlueprintPure, Category = "Pose Snapshot", meta = (BlueprintThreadSafe))
	static FPoseSnapshot GetSavedPoseSnapshot(const FPoseSnapshotReference& PoseSnapshot);

	/* Return bone transform in component space*/
	UFUNCTION(BlueprintPure, Category = "Pose Snapshot", meta = (BlueprintThreadSafe))
	static FTransform GetBoneTransformFromPoseSnapshot(const FPoseSnapshot& PoseSnapshot, FName BoneName, const UAnimInstance* AnimInstance);

	/* Draw pose from PoseSnapshot by using points and lines*/
	UFUNCTION(BlueprintCallable, Category = "Pose Snapshot", meta = (WorldContext = "WorldContextObject"))
	static bool DrawSkeletonFromPoseSnapshot(const UObject* WorldContextObject, const UAnimInstance* AnimInstance, const FPoseSnapshot& PoseSnapshot, 
		TArray<FName> LimbsName, FName RootBoneName = TEXT("Root"));

	/* Return parents bones name from skeleton hierarchy*/
	UFUNCTION(BlueprintCallable, Category = "Animation", meta = (BlueprintThreadSafe))
	static TArray<FName> GetBoneParents(USkeletalMeshComponent* SkeletalMeshComponent, FName BoneName);

private:
	static void DrawSingleLimbFromPoseSnapshot(const UWorld* World, const FPoseSnapshot& PoseSnapshot, FName BoneName, FName RootBoneName, const UAnimInstance* AnimInstance);

};