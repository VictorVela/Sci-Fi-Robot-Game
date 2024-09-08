

// Copyright Epic Games, Inc. All Rights Reserved.
#include "MyAnimGraphNode.h"
#include "ToolMenus.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphCommands.h"
#include "ScopedTransaction.h"

#include "DetailLayoutBuilder.h"
#include "Kismet2/CompilerResultsLog.h"
/////////////////////////////////////////////////////
// UAnimGraphNode_LayeredBoneBlend

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_LayeredBone::UAnimGraphNode_LayeredBone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Node.AddPose();
}

FLinearColor UAnimGraphNode_LayeredBone::GetNodeTitleColor() const
{
	return FLinearColor(0.0, 0.1, 0.3, 1.0);
}

FText UAnimGraphNode_LayeredBone::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_LayeredBoneBlend_Tooltip", "Layered Blend Per Bone Debug");
}

FText UAnimGraphNode_LayeredBone::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_LayeredBoneBlend_Title", "Layered Blend Per Bone Debug");
}

void UAnimGraphNode_LayeredBone::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);

	// Reconstruct node to show updates to PinFriendlyNames.
	if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_LayeredBone, BlendMode))
	{
		// If we  change blend modes, we need to resize our containers
		//FScopedTransaction Transaction(LOCTEXT("ChangeBlendMode", "Change Blend Mode"));
		Modify();

		const int32 NumPoses = Node.BlendPoses.Num();
		if (Node.BlendMode == ELayeredBlendMode::BlendMask)
		{
			Node.LayerSetup.Reset();
			Node.BlendMasks.SetNum(NumPoses);
		}
		else
		{
			Node.BlendMasks.Reset();
			Node.LayerSetup.SetNum(NumPoses);
		}

		//FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FString UAnimGraphNode_LayeredBone::GetNodeCategory() const
{
	return TEXT("JakubW|Blends");
}

void UAnimGraphNode_LayeredBone::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> NodeHandle = DetailBuilder.GetProperty(FName(TEXT("Node")), GetClass());

	if (Node.BlendMode != ELayeredBlendMode::BranchFilter)
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_LayeredBone, LayerSetup)));
	}

	if (Node.BlendMode != ELayeredBlendMode::BlendMask)
	{
		DetailBuilder.HideProperty(NodeHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAnimNode_LayeredBone, BlendMasks)));
	}

	Super::CustomizeDetails(DetailBuilder);
}

void UAnimGraphNode_LayeredBone::PreloadRequiredAssets()
{
	// Preload our blend profiles in case they haven't been loaded by the skeleton yet.
	if (Node.BlendMode == ELayeredBlendMode::BlendMask)
	{
		int32 NumBlendMasks = Node.BlendMasks.Num();
		for (int32 MaskIndex = 0; MaskIndex < NumBlendMasks; ++MaskIndex)
		{
			UBlendProfile* BlendMask = Node.BlendMasks[MaskIndex];
			PreloadObject(BlendMask);
		}
	}

	Super::PreloadRequiredAssets();
}


void UAnimGraphNode_LayeredBone::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (!Context->bIsDebugging)
	{
		{
			FToolMenuSection& Section = Menu->AddSection("AnimGraphNodeLayeredBoneblend", LOCTEXT("LayeredBoneBlend", "Layered Bone Blend"));
			if (Context->Pin != NULL)
			{
				// we only do this for normal BlendList/BlendList by enum, BlendList by Bool doesn't support add/remove pins
				if (Context->Pin->Direction == EGPD_Input)
				{
					//@TODO: Only offer this option on arrayed pins
					Section.AddMenuEntry(FAnimGraphCommands::Get().RemoveBlendListPin);
				}
			}
			else
			{
				Section.AddMenuEntry(FAnimGraphCommands::Get().AddBlendListPin);
			}
		}
	}
}

void UAnimGraphNode_LayeredBone::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	UAnimGraphNode_Base::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	bool bCompilationError = false;
	// Validate blend masks
	if (Node.BlendMode == ELayeredBlendMode::BlendMask)
	{
		int32 NumBlendMasks = Node.BlendMasks.Num();
		for (int32 MaskIndex = 0; MaskIndex < NumBlendMasks; ++MaskIndex)
		{
			const UBlendProfile* BlendMask = Node.BlendMasks[MaskIndex];
			if (BlendMask == nullptr)
			{
				//MessageLog.Error(*FText::Format(LOCTEXT("LayeredBlendNullMask", "@@ has null BlendMask for Blend Pose {0}. "), FText::AsNumber(MaskIndex)).ToString(), this, BlendMask);
				bCompilationError = true;
				continue;
			}
			else if (BlendMask->Mode != EBlendProfileMode::BlendMask)
			{
				//MessageLog.Error(*FText::Format(LOCTEXT("LayeredBlendProfileModeError", "@@ is using a BlendProfile(@@) without a BlendMask mode for Blend Pose {0}. "), FText::AsNumber(MaskIndex)).ToString(), this, BlendMask);
				bCompilationError = true;
			}
		}
	}

	// Don't rebuild the node's data if compilation failed. We may be attempting to do so with invalid data.
	if (bCompilationError)
	{
		return;
	}

	// ensure to cache the per-bone blend weights
	//if (!Node.ArePerBoneBlendWeightsValid(ForSkeleton))
	//{
	//	Node.RebuildPerBoneBlendWeights(ForSkeleton);
	//}
}
#undef LOCTEXT_NAMESPACE
