// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

class UMaterialInterface;

struct FMIForgePlannedMaterialItem
{
    FString DesiredAssetName;

    UMaterialInterface* ParentMaterial = nullptr;

    TSharedPtr<const FMIForgeTextureSet> TextureSet;

    FMIForgeGenerationOptions Options;
};

struct FMIForgeMaterialGenerationPlan
{
    TArray<FMIForgePlannedMaterialItem> Items;

    int32 FailedPlanningCount = 0;
    TArray<FText> Messages;
};

struct FMIForgeVertexPaintGenerationPlan
{
    bool bValid = false;

    FString DesiredAssetName;

    UMaterialInterface* ParentMaterial = nullptr;

    FMIForgeVertexPaintLayerStack LayerStack;
    FMIForgeVertexPaintGenerationOptions Options;

    TArray<FText> Messages;
};