// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Generation/MIForgeGenerationPlan.h"

struct FMIForgeMaterialPresetDefinition;
struct FMIForgeVertexPaintPresetDefinition;

class FMIForgeGenerationPlanner
{
public:
    FMIForgeMaterialGenerationPlan PlanMaterialGeneration(
        const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets,
        const FMIForgeGenerationOptions& Options) const;

    FMIForgeVertexPaintGenerationPlan PlanVertexPaintGeneration(
        const FMIForgeVertexPaintLayerStack& LayerStack,
        const FMIForgeVertexPaintGenerationOptions& Options) const;

private:
    bool ValidateTargetPath(
        const FString& TargetPath,
        FText& OutError) const;

    bool ValidateStandardItem(
        const FMIForgeTextureSet& TextureSet,
        const FMIForgeMaterialPresetDefinition& Definition,
        const TArray<FName>& ParentTextureParameterNames,
        const TArray<FName>& ParentStaticSwitchNames,
        const FMIForgeGenerationOptions& Options,
        FText& OutError) const;

    bool ValidateRGBMaskItem(
        const FMIForgeTextureSet& TextureSet,
        const FMIForgeMaterialPresetDefinition& Definition,
        const TArray<FName>& ParentTextureParameterNames,
        const TArray<FName>& ParentStaticSwitchNames,
        const FMIForgeGenerationOptions& Options,
        FText& OutError) const;

    bool ValidateVertexPaintPlan(
        const FMIForgeVertexPaintLayerStack& LayerStack,
        const FMIForgeVertexPaintPresetDefinition& Definition,
        UMaterialInterface* ParentMaterial,
        FText& OutError) const;

    bool ValidateDecalItem(
        const FMIForgeTextureSet& TextureSet,
        const FMIForgeMaterialPresetDefinition& Definition,
        const TArray<FName>& ParentTextureParameterNames,
        const TArray<FName>& ParentStaticSwitchNames,
        const FMIForgeGenerationOptions& Options,
		FText& OutError) const;
	
};
