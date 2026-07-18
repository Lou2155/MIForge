// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

class IAssetTools;
class UMaterialInterface;

struct FMIForgeMaterialInstanceTarget
{
    FString TargetPath;
    FString AssetName;

    EIfMIExistsOption IfMIExists =
        EIfMIExistsOption::Skip;

    UMaterialInterface* ParentMaterial = nullptr;
};

class FMIForgeMaterialInstanceResolver
{
public:
    FMIForgeMaterialInstanceResolution Resolve(
        const FMIForgeMaterialInstanceTarget& Target,
        IAssetTools& AssetTools) const;
};