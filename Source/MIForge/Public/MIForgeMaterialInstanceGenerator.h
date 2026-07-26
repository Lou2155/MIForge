// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"



class FMIForgeMaterialInstanceGenerator
{
public:
	FMIForgeGenerationResult GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const;

	FMIForgeGenerationResult GenerateVertexPaintMaterialInstance(
		const FMIForgeVertexPaintLayerStack& LayerStack,
		const FMIForgeVertexPaintGenerationOptions& Options
	) const;

};


