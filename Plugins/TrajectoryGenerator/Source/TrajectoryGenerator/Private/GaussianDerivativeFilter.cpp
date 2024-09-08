
#include "GaussianDerivativeFilter.h"

#include "CurveEditor.h"
#include "Curves/RealCurve.h"
#include "Math/UnrealMathUtility.h"
#include "Curves/CurveFloat.h"

void UGaussianDerivativeFilter::ApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor, const TMap<FCurveModelID, FKeyHandleSet>& InKeysToOperateOn, TMap<FCurveModelID, FKeyHandleSet>& OutKeysToSelect)
{
	TArray<FKeyHandle> KeyHandles;
	TArray<FKeyPosition> SelectedKeyPositions;

	TArray<FKeyPosition> SmoothedPositions;

	// Preparing new keys for derivative
	TArray<FKeyPosition> DerivativeKeyPositions;
	TArray<FKeyAttributes> DerivativeKeyAttributes;

	const TMap<FCurveModelID, FKeyHandleSet>* KeysToOperateOn = &InKeysToOperateOn;

	for (const TTuple<FCurveModelID, FKeyHandleSet>& Pair : *KeysToOperateOn)
	{
		FCurveModel* Curve = InCurveEditor->FindCurve(Pair.Key);

		if (!Curve)
		{
			continue;
		}

		// Look at our Input Snap rate to determine how far apart the baked keys are.
		FFrameRate BakeRate = InCurveEditor->GetCurveSnapMetrics(Pair.Key).InputSnapRate;

		KeyHandles.Reset(Pair.Value.Num());
		KeyHandles.Append(Pair.Value.AsArray().GetData(), Pair.Value.Num());

		// Get all the selected keys
		SelectedKeyPositions.SetNum(KeyHandles.Num());
		Curve->GetKeyPositions(KeyHandles, SelectedKeyPositions);


		SmoothedPositions = ApplyGaussianBlur(SelectedKeyPositions, BlurSigma, BlurRadius);
		if (SmoothedPositions.Num() == 0) { return; }


		for (int32 i = 1; i < SmoothedPositions.Num(); ++i)
		{
			double TimeDiff = SmoothedPositions[i].InputValue - SmoothedPositions[i - 1].InputValue;
			if (TimeDiff == 0) { TimeDiff = 0.001; }

			double DerivativeValue = (SmoothedPositions[i].OutputValue - SmoothedPositions[i - 1].OutputValue) / TimeDiff;
			double MidTime = (SmoothedPositions[i].InputValue + SmoothedPositions[i - 1].InputValue) / 2.0;

			DerivativeKeyPositions.Add(FKeyPosition(MidTime, DerivativeValue));
			DerivativeKeyAttributes.Add(FKeyAttributes().SetInterpMode(RCIM_Linear).SetTangentMode(RCTM_Auto));
		}

		if (DerivativeKeyPositions.Num() > 0)
		{
			Curve->Modify();
			Curve->RemoveKeys(KeyHandles);
			Curve->AddKeys(DerivativeKeyPositions, DerivativeKeyAttributes);
		}

	}

}

bool UGaussianDerivativeFilter::CanApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor)
{
    return InCurveEditor->GetSelection().Count() > 0;
}

TArray<FKeyPosition> UGaussianDerivativeFilter::ApplyGaussianBlur(TArray<FKeyPosition> KeyPositions, float Sigma, float Radius)
{
	TArray<FKeyPosition> Result;
	const int32 NumKeys = KeyPositions.Num();
	Result.Reserve(NumKeys);

	// Prepare Gaussian weights
	TArray<float> Weights;
	float WeightSum = 0.0f;
	for (int i = -Radius; i <= Radius; ++i)
	{
		float Weight = FMath::Exp(-0.5f * FMath::Square(i / Sigma));
		Weights.Add(Weight);
		WeightSum += Weight;
	}

	// Apply Gaussian blur
	for (int i = 0; i < NumKeys; ++i)
	{
		float BlurredValue = 0.0f;
		for (int j = -Radius; j <= Radius; ++j)
		{
			int Index = FMath::Clamp(i + j, 0, NumKeys - 1);
			BlurredValue += KeyPositions[Index].OutputValue * Weights[j + Radius];
		}
		BlurredValue /= WeightSum;
		Result.Add(FKeyPosition(KeyPositions[i].InputValue, BlurredValue));
	}

	return Result;
}

TArray<FKeyPosition> UGaussianDerivativeFilter::ComputeDerivative(const TArray<FKeyPosition>& KeyPositions)
{
	return KeyPositions;
}
