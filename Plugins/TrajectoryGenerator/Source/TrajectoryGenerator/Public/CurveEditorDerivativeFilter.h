

#pragma once

#include "CoreMinimal.h"
#include "Filters/CurveEditorFilterBase.h"
#include "CurveEditorDerivativeFilter.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Simple Approximate Derivative")
class TRAJECTORYGENERATOR_API UCurveEditorDerivativeFilter : public UCurveEditorFilterBase
{
    GENERATED_BODY()

protected:
    virtual void ApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor, const TMap<FCurveModelID, FKeyHandleSet>& InKeysToOperateOn, TMap<FCurveModelID, FKeyHandleSet>& OutKeysToSelect) override;
	
    virtual bool CanApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor) override;
};
