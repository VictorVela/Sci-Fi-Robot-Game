

#include "ALS_DamageConfigData.h"

UALS_DamageConfigData::UALS_DamageConfigData()
{
    // Dodaj dwa klucze do krzywej float.
    FKeyHandle Key1Handle = ScalingCurve.GetRichCurve()->AddKey(0.0f, 0.0f);
    FKeyHandle Key2Handle = ScalingCurve.GetRichCurve()->AddKey(1.0f, 1.0f);

    // Ustaw interpolacjê kluczy na liniow¹.
    ScalingCurve.GetRichCurve()->SetKeyInterpMode(Key1Handle, RCIM_Linear);
    ScalingCurve.GetRichCurve()->SetKeyInterpMode(Key2Handle, RCIM_Linear);

    FBoneAndWeight Bone1;
    Bone1.BoneName = TEXT("clavicle_l"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);
    // ----------------------------
    Bone1.BoneName = TEXT("clavicle_r"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);
    // ----------------------------
    Bone1.BoneName = TEXT("head"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);
    // ----------------------------
    Bone1.BoneName = TEXT("neck"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);
    // ----------------------------
    Bone1.BoneName = TEXT("spine_03"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);
    // ----------------------------
    Bone1.BoneName = TEXT("spine_02"); Bone1.Value = 1;
    WeightsPerBones.Add(Bone1);

    WeightPerWeapon.Add(ECALS_WeaponType::None, 1.0f);
    WeightPerWeapon.Add(ECALS_WeaponType::Rifle, 1.0f);
    WeightPerWeapon.Add(ECALS_WeaponType::Pistol, 1.0f);
    WeightPerWeapon.Add(ECALS_WeaponType::Bow, 1.0f);
    WeightPerWeapon.Add(ECALS_WeaponType::Knife, 1.0f);

    ReferenceCurve = ScalingCurve;

    DamageEquation = TEXT("(DamageIN')");
}

UALS_DamageConfigData::~UALS_DamageConfigData()
{
}

float UALS_DamageConfigData::GetScalingCurveValueC(float CurrentHealthPoints, float MaxHealthPoints, FRuntimeFloatCurve Curve1, FRuntimeFloatCurve CurveRef)
{

    float Time = FMath::GetMappedRangeValueClamped(FVector2D(0.0, MaxHealthPoints), FVector2D(0.0, 1.0), CurrentHealthPoints);
    float ValueRef = Curve1.GetRichCurve()->Eval(Time);
    float ValueDif = CurveRef.GetRichCurve()->Eval(Time);

    const float a = ValueRef - ValueDif;

    return FMath::Clamp<float>(1 - a, 0.0, 1000.0);
}

float UALS_DamageConfigData::GetWeightPerBoneC(FName BoneName, TArray<FBoneAndWeight> Data)
{
    if (Data.Num() == 0) { return 1.0; }

    for (FBoneAndWeight D : Data)
    {
        if (D.BoneName == BoneName)
        {
            return D.Value;
        }
    }
    return 1.0;
}
