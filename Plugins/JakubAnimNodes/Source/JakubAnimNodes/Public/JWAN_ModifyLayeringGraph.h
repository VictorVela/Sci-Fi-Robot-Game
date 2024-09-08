
//class JAKUBANIMNODES_API JWAN_ModifyLayeringGraph


#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "AnimGraphNode_Base.h"
#include "JakubAnimNodesTool/Public/JWAN_ModifyLayeringLogic.h"
#include "JWAN_ModifyLayeringGraph.generated.h"

class FMenuBuilder;

/** Easy way to modify curve values on a pose */
UCLASS(MinimalAPI)
class UJWAN_ModifyLayeringGraph : public UAnimGraphNode_Base
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_ModifyLayering Node;

public:

	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	// End of UAnimGraphNode_Base interface

	// UK2Node interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UK2Node interface

private:

};