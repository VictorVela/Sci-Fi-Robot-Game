
#pragma once

#include "JakubAnimNodesTool/Public/JWAN_LayerBlendingLogic.h"
#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "JWAN_LayerBlendingGraph.generated.h"
/**
 *
 */
UCLASS()
class JAKUBANIMNODES_API UJWAN_LayerBlendingGraph : public UAnimGraphNode_Base
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "Settings")
        FJWAN_LayerBlendingLogic Node;

    //~ Begin UEdGraphNode Interface.
    virtual FLinearColor GetNodeTitleColor() const override;
    virtual FText GetTooltipText() const override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    //~ End UEdGraphNode Interface.

    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog) override;

    //~ Begin UAnimGraphNode_Base Interface
    virtual FString GetNodeCategory() const override;
    //~ End UAnimGraphNode_Base Interface

};


//class JAKUBANIMNODES_API JWAN_LayerBlendingGraph



