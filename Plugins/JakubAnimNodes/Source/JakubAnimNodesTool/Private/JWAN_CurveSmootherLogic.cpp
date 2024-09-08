

#include "JWAN_CurveSmootherLogic.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"


float InterpToConstant(float Current, float Target, float DeltaTime, float InterpSpeed)
{
	float Distance = Target - Current;
	float Step = InterpSpeed * DeltaTime;
	if (FMath::Abs(Distance) <= Step)
	{
		return Target;
	}
	return Current + FMath::Sign(Distance) * Step;
}


void FAnimNode_CurveSmoother::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	//Initialize_AnyThread(Context);
	SourcePose.Initialize(Context);
	if (IsValid(AnimInst)==false)
	{ AnimInst = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject()); }

	for (int i = 0; i < SmoothingData.Num(); i++)
	{
		if (AnimInst)
		{
			float CurrentValue = 0.0;
			const bool CurveValid = AnimInst->GetCurveValue(SmoothingData[i].TargetCurveName, CurrentValue);
			if (CurveValid == true)
			{ 
				InterpValues.Add(CurrentValue); PrevCurvesValue.Add(CurrentValue); InterpValuesLayer2.Add(CurrentValue);
			}
			else
			{ 
				InterpValues.Add(0.0); PrevCurvesValue.Add(0.0); InterpValuesLayer2.Add(0.0);
			}
		}
		else
		{ 
			InterpValues.Add(0.0); PrevCurvesValue.Add(0.0); InterpValuesLayer2.Add(0.0);
		}

		SpeedValues.Add(0.0);
		ElapsedTimes.Add(-1.0);
		
	}

}

void FAnimNode_CurveSmoother::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	//CacheBones_AnyThread(Context);
	SourcePose.CacheBones(Context);
	if (IsValid(AnimInst)==false)
	{ AnimInst = Cast<UAnimInstance>(Context.AnimInstanceProxy->GetAnimInstanceObject()); }
}

void FAnimNode_CurveSmoother::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	FPoseContext SourceData(Output);
	SourcePose.Evaluate(SourceData);
	Output = SourceData;

	float dt = 0.005;

	if (AnimInst)
	{
		dt = AnimInst->GetDeltaSeconds();
	}

	if (InterpValues.Num() == SmoothingData.Num())
	{
		for (int i = 0; i < SmoothingData.Num(); i++)
		{
			const float InCurrentValue = InterpValues[i];
			const float NewValue = Output.Curve.Get(SmoothingData[i].TargetCurveName);
			
			//Calculate Curve Value Changing Speed
			if (SmoothingData[i].ReducePeakChange == true)
			{
				if (SpeedValues.IsValidIndex(i))
				{
					SpeedValues[i] = abs((NewValue - PrevCurvesValue[i]) / dt);
					PrevCurvesValue[i] = NewValue;

					const float CurrentSpeed = SpeedValues[i];

					if (CurrentSpeed > SmoothingData[i].PeakSpeedThreshold)
					{
						ElapsedTimes[i] = 0.0; // Enable Timer
					}


					if (ElapsedTimes[i] > -0.1) //Check The Timer is enabled
					{
						if (ElapsedTimes[i] < SmoothingData[i].InterpingTime)
						{
							ElapsedTimes[i] = ElapsedTimes[i] + dt; // Add delta to timer

							float UseNewValue = FMath::Lerp<float>(InterpValuesLayer2[i], NewValue, SmoothingData[i].PeakInterpSpeed * dt);
							InterpValuesLayer2[i] = UseNewValue;
							Output.Curve.Set(SmoothingData[i].TargetCurveName, UseNewValue);

						}
						else
						{
							ElapsedTimes[i] = -1.0; // Disable Timer
						}
					}
					else
					{
						InterpValuesLayer2[i] = NewValue;
					}

				}
			}

			if (SmoothingData[i].InterpSpeed > 0.0)
			{
				InterpValues[i] = ProcessCurveOperation(Output, SmoothingData[i].TargetCurveName, InCurrentValue, NewValue, SmoothingData[i], dt);
			}
		}
	}
}

float FAnimNode_CurveSmoother::ProcessCurveOperation(FPoseContext& Output, const FName& CurveName, float CurrentValue, float NewValue, FCurveSmootherSetting Settings, float dt)
{
	float UseNewValue = CurrentValue;

	float UseAlpha = FMath::Clamp(InternalBlendAlpha, 0.f, 1.f);

	if (Settings.UseConstantInterp == false)
	{
		UseNewValue = FMath::Lerp(NewValue, FMath::Lerp<float>(CurrentValue, NewValue, Settings.InterpSpeed * dt),UseAlpha);
	}
	else
	{
		UseNewValue = FMath::Lerp(NewValue, InterpToConstant(CurrentValue, NewValue, dt, Settings.InterpSpeed), UseAlpha);
	}

	Output.Curve.Set(CurveName, UseNewValue);

	return UseNewValue;
}


void FAnimNode_CurveSmoother::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	// Run update on input pose nodes
	SourcePose.Update(Context);

	InternalBlendAlpha = 0.f;
	switch (AlphaInputType)
	{
	case EAnimAlphaInputType::Float:
		InternalBlendAlpha = AlphaScaleBias.ApplyTo(AlphaScaleBiasClamp.ApplyTo(Alpha, Context.GetDeltaTime()));
		break;
	case EAnimAlphaInputType::Bool:
		InternalBlendAlpha = AlphaScaleBias.ApplyTo(AlphaScaleBiasClamp.ApplyTo(Alpha, Context.GetDeltaTime()));
		break;
	case EAnimAlphaInputType::Curve:
		if (AnimInst)
		{ 
			InternalBlendAlpha = AlphaScaleBiasClamp.ApplyTo(AnimInst->GetCurveValue(AlphaCurveName), Context.GetDeltaTime()); 
		}
		break;
	};

	// Make sure Alpha is clamped between 0 and 1.
	InternalBlendAlpha = FMath::Clamp<float>(InternalBlendAlpha, 0.f, 1.f);

	// Evaluate any BP logic plugged into this node
	GetEvaluateGraphExposedInputs().Execute(Context);
}



