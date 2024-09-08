


#include "CurveEditorDerivativeFilter.h"

#include "CurveEditor.h"
#include "Curves/RealCurve.h"

void UCurveEditorDerivativeFilter::ApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor, const TMap<FCurveModelID, FKeyHandleSet>& InKeysToOperateOn, TMap<FCurveModelID, FKeyHandleSet>& OutKeysToSelect)
{
	TArray<FKeyHandle> KeyHandles;
	TArray<FKeyPosition> SelectedKeyPositions;

	TArray<FKeyPosition> NewKeyPositions;
	TArray<FKeyAttributes> NewKeyAttributes;

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



		for (int32 i = 1; i < SelectedKeyPositions.Num(); ++i)
		{
			double TimeDiff = SelectedKeyPositions[i].InputValue - SelectedKeyPositions[i - 1].InputValue;
			if (TimeDiff == 0) { TimeDiff = 0.001; }

			double DerivativeValue = (SelectedKeyPositions[i].OutputValue - SelectedKeyPositions[i - 1].OutputValue) / TimeDiff;
			double MidTime = (SelectedKeyPositions[i].InputValue + SelectedKeyPositions[i - 1].InputValue) / 2.0;

			DerivativeKeyPositions.Add(FKeyPosition(MidTime, DerivativeValue));
			DerivativeKeyAttributes.Add(FKeyAttributes().SetInterpMode(RCIM_Linear).SetTangentMode(RCTM_Auto));
		}

		if (DerivativeKeyPositions.Num() > 0)
		{
			// Remove old keys and apply new derivative keys
			Curve->Modify();
			Curve->RemoveKeys(KeyHandles);
			Curve->AddKeys(DerivativeKeyPositions, DerivativeKeyAttributes);
		}

	}
}

bool UCurveEditorDerivativeFilter::CanApplyFilter_Impl(TSharedRef<FCurveEditor> InCurveEditor)
{
    return InCurveEditor->GetSelection().Count() > 0;
}
