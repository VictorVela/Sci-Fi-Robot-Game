// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrajectoryGeneratorBPLibrary.h"
#include "TrajectoryGenerator.h"
#include "Misc/PackageName.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/NoExportTypes.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Animation/AnimationAsset.h"
#include "EditorFramework/AssetImportData.h"
#include "Animation/AnimSequence.h"
#include "EditorFramework/AssetImportData.h"

UTrajectoryGeneratorBPLibrary::UTrajectoryGeneratorBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float UTrajectoryGeneratorBPLibrary::TrajectoryGeneratorSampleFunction(float Param)
{
	return 0;
}

bool UTrajectoryGeneratorBPLibrary::AddKeyToCurveVector(UCurveVector* CurveVector, float Time, FVector Value, TEnumAsByte<ERichCurveInterpMode> InterpMode)
{
    if (!CurveVector)
    {
        return false;
    }
    // Dodaj klucze dla X, Y, Z z odpowiednim czasem i wartoœci¹
    FKeyHandle KeyHandleX = CurveVector->FloatCurves[0].AddKey(Time, Value.X);
    FKeyHandle KeyHandleY = CurveVector->FloatCurves[1].AddKey(Time, Value.Y);
    FKeyHandle KeyHandleZ = CurveVector->FloatCurves[2].AddKey(Time, Value.Z);

    // Ustaw interpolacjê dla kluczy
    CurveVector->FloatCurves[0].SetKeyInterpMode(KeyHandleX, InterpMode);
    CurveVector->FloatCurves[1].SetKeyInterpMode(KeyHandleY, InterpMode);
    CurveVector->FloatCurves[2].SetKeyInterpMode(KeyHandleZ, InterpMode);

    // Zapisz zmiany
    CurveVector->PostEditChange();
    return true;
}

bool UTrajectoryGeneratorBPLibrary::AddKeyToCurveFloat(UCurveFloat* CurveFloat, float Time, float Value, TEnumAsByte<ERichCurveInterpMode> InterpMode)
{
    if (!CurveFloat)
    {
        return false;
    }
    FKeyHandle KeyHandle = CurveFloat->FloatCurve.AddKey(Time, Value);
    CurveFloat->FloatCurve.SetKeyInterpMode(KeyHandle, InterpMode);
    CurveFloat->FloatCurve.SetKeyTangentMode(KeyHandle, ERichCurveTangentMode::RCTM_Auto);
    // Zapisz zmiany
    CurveFloat->PostEditChange();
    return true;
}

void UTrajectoryGeneratorBPLibrary::ClearAllKeysFromCurveFloat(UCurveFloat* CurveFloat)
{
    if (!CurveFloat)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurveFloat is null"));
        return;
    }
    CurveFloat->FloatCurve.Reset();
    CurveFloat->PostEditChange();
}

void UTrajectoryGeneratorBPLibrary::ClearAllKeysFromCurveVector(UCurveVector* CurveVector)
{
    if (!CurveVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurveVector is null"));
        return;
    }

    for (int32 i = 0; i < 3; i++)
    {
        CurveVector->FloatCurves[i].Reset();
    }
    CurveVector->PostEditChange();
}


UCurveVector* UTrajectoryGeneratorBPLibrary::CreateCurveVectorAsset(const FString& AssetName, const FString& AssetPath)
{
    FString PackageName = AssetPath / AssetName;
    FString FullPackageName = FPackageName::ObjectPathToPackageName(PackageName);
    FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

    UPackage* Package = CreatePackage(*FullPackageName);
    if (!Package)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create package: %s"), *FullPackageName);
        return nullptr;
    }

    UCurveVector* NewCurveVector = NewObject<UCurveVector>(Package, *AssetName, RF_Public | RF_Standalone);
    if (!NewCurveVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create CurveVector asset: %s"), *AssetName);
        return nullptr;
    }

    FAssetRegistryModule::AssetCreated(NewCurveVector);
    Package->MarkPackageDirty();

    FString FilePath = FPackageName::LongPackageNameToFilename(FullPackageName, FPackageName::GetAssetPackageExtension());
    bool bSaved = UPackage::SavePackage(Package, NewCurveVector, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);

    if (!bSaved)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save CurveVector asset to: %s"), *FilePath);
        return nullptr;
    }

    return NewCurveVector;
}


UCurveFloat* UTrajectoryGeneratorBPLibrary::CreateCurveFloatAsset(const FString& AssetName, const FString& AssetPath)
{
    FString PackageName = AssetPath / AssetName;
    FString FullPackageName = FPackageName::ObjectPathToPackageName(PackageName);
    FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

    UPackage* Package = CreatePackage(*FullPackageName);
    if (!Package)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create package: %s"), *FullPackageName);
        return nullptr;
    }

    UCurveFloat* NewCurveVector = NewObject<UCurveFloat>(Package, *AssetName, RF_Public | RF_Standalone);
    if (!NewCurveVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create CurveFloat asset: %s"), *AssetName);
        return nullptr;
    }

    FAssetRegistryModule::AssetCreated(NewCurveVector);
    Package->MarkPackageDirty();

    FString FilePath = FPackageName::LongPackageNameToFilename(FullPackageName, FPackageName::GetAssetPackageExtension());
    bool bSaved = UPackage::SavePackage(Package, NewCurveVector, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);

    if (!bSaved)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save CurveFloat asset to: %s"), *FilePath);
        return nullptr;
    }

    return NewCurveVector;
}


TArray<UObject*> UTrajectoryGeneratorBPLibrary::LoadAssetsFromPath(TSubclassOf<UObject> AssetClass, const FString& AssetPath)
{
    TArray<UObject*> AssetList;

    if (!AssetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid AssetClass"));
        return AssetList;
    }

    // Ensure the asset path is valid
    if (!AssetPath.StartsWith(TEXT("/Game/")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid AssetPath: %s. AssetPath should start with /Game/"), *AssetPath);
        return AssetList;
    }

    // Get the asset registry module
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Get assets in the specified path
    TArray<FAssetData> AssetDataList;
    AssetRegistry.ScanPathsSynchronous({ AssetPath }, true);
    AssetRegistry.GetAssetsByPath(FName(*AssetPath), AssetDataList, true);

    // Log the number of assets found
    UE_LOG(LogTemp, Log, TEXT("Found %d assets in path %s"), AssetDataList.Num(), *AssetPath);

    // Filter assets by the specified class
    for (const FAssetData& AssetData : AssetDataList)
    {
        UObject* LoadedAsset = AssetData.GetAsset();
        if (LoadedAsset && LoadedAsset->IsA(AssetClass))
        {
            AssetList.Add(LoadedAsset);
            //UE_LOG(LogTemp, Log, TEXT("Loaded asset: %s"), *LoadedAsset->GetName());
        }
    }

    // Log the number of assets loaded
    UE_LOG(LogTemp, Log, TEXT("Loaded %d assets of class %s from path %s"), AssetList.Num(), *AssetClass->GetName(), *AssetPath);

    return AssetList;
}

int32 UTrajectoryGeneratorBPLibrary::GetNumKeyFramesCurveFloat(UCurveFloat* Curve)
{
    if (Curve)
    {
        return Curve->FloatCurve.GetNumKeys();
    }
    return 0;
}

int32 UTrajectoryGeneratorBPLibrary::GetNumKeyFramesCurveVector(UCurveVector* Curve, ECurveVectorAxis Axis)
{
    if (Curve)
    {
        switch (Axis)
        {
        case ECurveVectorAxis::X:
            return Curve->FloatCurves[0].GetNumKeys();
        case ECurveVectorAxis::Y:
            return Curve->FloatCurves[1].GetNumKeys();
        case ECurveVectorAxis::Z:
            return Curve->FloatCurves[2].GetNumKeys();
        }
    }
    return 0;
}

float UTrajectoryGeneratorBPLibrary::GetFloatValueAtKeyFrame(UCurveFloat* Curve, int32 KeyFrameIndex)
{
    if (Curve && Curve->FloatCurve.Keys.IsValidIndex(KeyFrameIndex))
    {
        return Curve->FloatCurve.Keys[KeyFrameIndex].Value;
    }
    return 0.0f;
}

float UTrajectoryGeneratorBPLibrary::GetVectorValueAtKeyFrame(UCurveVector* Curve, int32 KeyFrameIndex, ECurveVectorAxis Axis)
{
    if (Curve)
    {
        switch (Axis)
        {
        case ECurveVectorAxis::X:
            if (Curve->FloatCurves[0].Keys.IsValidIndex(KeyFrameIndex))
            {
                return Curve->FloatCurves[0].Keys[KeyFrameIndex].Value;
            }
            break;
        case ECurveVectorAxis::Y:
            if (Curve->FloatCurves[1].Keys.IsValidIndex(KeyFrameIndex))
            {
                return Curve->FloatCurves[1].Keys[KeyFrameIndex].Value;
            }
            break;
        case ECurveVectorAxis::Z:
            if (Curve->FloatCurves[2].Keys.IsValidIndex(KeyFrameIndex))
            {
                return Curve->FloatCurves[2].Keys[KeyFrameIndex].Value;
            }
            break;
        }
    }
    return 0.0f;
}

bool UTrajectoryGeneratorBPLibrary::RemoveKeyFrameFromCurveFloat(UCurveFloat* Curve, int32 KeyFrameIndex)
{
    if (Curve && Curve->FloatCurve.Keys.IsValidIndex(KeyFrameIndex))
    {
        Curve->FloatCurve.Keys.RemoveAt(KeyFrameIndex);
        return true;
    }
    return false;
}

bool UTrajectoryGeneratorBPLibrary::RemoveKeyFrameFromCurveVector(UCurveVector* Curve, int32 KeyFrameIndex, ECurveVectorAxis Axis)
{
    if (!Curve)
    { return false; }

    switch (Axis)
    {
    case ECurveVectorAxis::X:
        if (Curve->FloatCurves[0].Keys.IsValidIndex(KeyFrameIndex))
        {
            Curve->FloatCurves[0].Keys.RemoveAt(KeyFrameIndex);
            return true;
        }
        break;

    case ECurveVectorAxis::Y:
        if (Curve->FloatCurves[1].Keys.IsValidIndex(KeyFrameIndex))
        {
            Curve->FloatCurves[1].Keys.RemoveAt(KeyFrameIndex);
            return true;
        }
        break;

    case ECurveVectorAxis::Z:
        if (Curve->FloatCurves[2].Keys.IsValidIndex(KeyFrameIndex))
        {
            Curve->FloatCurves[2].Keys.RemoveAt(KeyFrameIndex);
            return true;
        }
        break;
    }
    return false;
}


