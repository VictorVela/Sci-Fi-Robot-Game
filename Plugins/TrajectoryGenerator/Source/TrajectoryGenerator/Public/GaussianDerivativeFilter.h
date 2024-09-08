

#pragma once

#include "CoreMinimal.h"
#include "Filters/CurveEditorFilterBase.h"
#include "GaussianDerivativeFilter.generated.h"

/**
 * 
 */
UCLASS()
class TRAJECTORYGENERATOR_API UGaussianDerivativeFilter : public UCurveEditorFilterBase
{
	GENERATED_BODY()
	virtual void ApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor, const TMap<FCurveModelID, FKeyHandleSet>& InKeysToOperateOn, TMap<FCurveModelID, FKeyHandleSet>& OutKeysToSelect) override;

	virtual bool CanApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BlurSigma = 1.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BlurRadius = 1.0;

protected:
	TArray<FKeyPosition> ApplyGaussianBlur(TArray<FKeyPosition> KeyPositions, float Sigma, float Radius);
	TArray<FKeyPosition> ComputeDerivative(const TArray<FKeyPosition>& KeyPositions);

};
