// Fill out your copyright notice in the Description page of Project Settings.

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