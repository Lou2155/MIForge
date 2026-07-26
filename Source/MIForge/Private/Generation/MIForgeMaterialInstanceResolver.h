// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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