// Jakub W 2024

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Curves/CurveFloat.h"
#include "ALS_DamageConfigData.generated.h"

USTRUCT(BlueprintType)
struct FBoneAndWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	FBoneAndWeight()
		: BoneName(NAME_None), Value(1.0f)
	{}
};

UENUM(BlueprintType)
enum class ECALS_WeaponType : uint8
{
	None   UMETA(DisplayName = "None"),
	Rifle  UMETA(DisplayName = "Rifle"),
	Pistol UMETA(DisplayName = "Pistol"),
	Bow    UMETA(DisplayName = "Bow"),
	Knife  UMETA(DisplayName = "Knife")
};


UCLASS(Blueprintable, BlueprintType)
class HELPFULFUNCTIONS_API UALS_DamageConfigData : public UObject
{
	GENERATED_BODY()

public:
	UALS_DamageConfigData();
	~UALS_DamageConfigData();

protected:

	/** Default max Health points for character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config", ClampMin = "1", ClampMax = "5000.0", UIMin = "1.0", UIMax = "5000.0"))
	float MaxHealth = 100;

	/** Allows you to introduce a constant scaling of RECEIVED damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config", ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
	float ReceiveDamageScale = 1.0;

	/** It results in the final damage value being reduced by an amount associated with the armor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float ArmorValue = 0.0;

	/**  Is used to introduce non-linear scaling of the damage value. The range on the X and Y axis should always be between 0 and 1.
The X axis represents the mapped minimum and maximum health values. We set the scaling value on the Y axis.
Note: a reference curve is used to calculate the final value, which is always a function of Y = X. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config"))
	FRuntimeFloatCurve ScalingCurve;

	/* Sets the scaling of the 'Input Damage' value depending on the bone hit.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config"))
	TArray<FBoneAndWeight> WeightsPerBones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Damage Config"))
	TMap<ECALS_WeaponType, float> WeightPerWeapon;

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", Category = "Damage System", DisplayName = "Get Scaling Curve Value", Keywords = "Damage System"))
	static float GetScalingCurveValueC(float CurrentHealthPoints, float MaxHealthPoints, FRuntimeFloatCurve Curve1, FRuntimeFloatCurve CurveRef);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", Category = "Damage System", DisplayName = "Get Weight Per Bone", Keywords = "Damage System"))
	static float GetWeightPerBoneC(FName BoneName, TArray<FBoneAndWeight> Data);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Category = "Read Only"))
	FRuntimeFloatCurve ReferenceCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Category = "Read Only"))
	FName DamageEquation;


#if WITH_EDITOR	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		Refresh();
	}
#endif

private:

	bool CheckForNonDefaultWeights()
	{
		for (const auto& Pair : WeightPerWeapon)
		{
			if (Pair.Value != 1.0f)
			{
				return true;
			}
		}
		return false;
	}

	void Refresh()
	{
		ReferenceCurve.GetRichCurve()->Reset();
		FKeyHandle k1 = ReferenceCurve.GetRichCurve()->AddKey(0.0f, 0.0f);
		FKeyHandle k2 = ReferenceCurve.GetRichCurve()->AddKey(1.0f, 1.0f);
		ReferenceCurve.GetRichCurve()->SetKeyInterpMode(k1, RCIM_Linear);
		ReferenceCurve.GetRichCurve()->SetKeyInterpMode(k2, RCIM_Linear);

		FString n1, n2, n3, n4 = TEXT("");
		if (ReceiveDamageScale != 1.0) { n1 = TEXT(" * ReceiveDamageScale"); }

		if (ArmorValue != 0.0) { n2 = TEXT(" - ArmorValue"); }

		for (FBoneAndWeight D : WeightsPerBones)
		{
			if (D.Value != 1.0) { n3 = TEXT(" * WeightsPerBones(BoneName)"); break; }
		}

		if (CheckForNonDefaultWeights() == true) { n4 = TEXT(" * WeightPerWeapon(Type)"); }

		FString outEqu = TEXT("(DamageIN'") + n1 + n3 + n4 + TEXT(")") + n2;
		DamageEquation = FName(*outEqu);
	}


};
