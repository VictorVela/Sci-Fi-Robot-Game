
#include "JWAN_LayerBlendingGraph.h"

#define LOCTEXT_NAMESPACE "A3Nodes"


FLinearColor UJWAN_LayerBlendingGraph::GetNodeTitleColor() const
{
    return FLinearColor(0.0, 0.1, 0.3, 1.0);
}

FText UJWAN_LayerBlendingGraph::GetTooltipText() const
{
    return LOCTEXT("LayerBlending For ALS", "LayerBlending For ALS");
}

FText UJWAN_LayerBlendingGraph::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("Layer Blending", "Layer Blending");
}

void UJWAN_LayerBlendingGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Modify();

    Node.BlendFilterPelvis.SetNum(1);
    Node.BlendFilterSpine.SetNum(1);
    Node.BlendFilterHead.SetNum(1);
    Node.BlendFilterArmL.SetNum(1);
    Node.BlendFilterArmR.SetNum(1);
    Node.BlendFilterHandL.SetNum(1);
    Node.BlendFilterHandR.SetNum(1);
    Node.BlendFilterCurves.SetNum(1);
    Node.BlendFilterEmpty.SetNum(1);

    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UJWAN_LayerBlendingGraph::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
    if (!Node.ArePerBoneBlendWeightsValid(ForSkeleton, Node.SkeletonGuid, Node.VirtualBoneGuid))
    {
        Node.RebuildAllBlendWeights(ForSkeleton);
    }
}

FString UJWAN_LayerBlendingGraph::GetNodeCategory() const
{
    return TEXT("JakubW|Blends");
}

#undef LOCTEXT_NAMESPACE