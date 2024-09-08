#include "JWAN_PoseSnapShotGraph.h"
#include "Kismet2/BlueprintEditorUtils.h"

FText UJWAN_PoseSnapShotGraph::GetTooltipText() const
{
    return FText::FromString("Saves the current pose to a snapshot.");
}

FText UJWAN_PoseSnapShotGraph::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString("Save Current Pose To Snapshot");
}
