#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_Base.h"
#include "JakubAnimNodesTool/Public/JWAN_PoseSnapShotLogic.h"
#include "JWAN_PoseSnapShotGraph.generated.h"

// This will be displayed in the AnimGraph
UCLASS()
class JAKUBANIMNODES_API UJWAN_PoseSnapShotGraph : public UAnimGraphNode_Base
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Settings")
    FJWAN_PoseSnapShotLogic Node;

    // Get the tooltip for this node
    virtual FText GetTooltipText() const override;

    // Get the node title for this node
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

    // Get the menu category for this node
    //virtual FString GetMenuCategory() const override;
};