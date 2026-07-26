// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeMaterialInstanceGenerator.h"
#include "MIForgeTypes.h"

#include "Generation/MIForgeGenerationPlanner.h"
#include "Generation/MIForgeGenerationExecutor.h"

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const
{
	const FMIForgeMaterialGenerationPlan Plan =
		FMIForgeGenerationPlanner().PlanMaterialGeneration(
			TextureSets,
			Options);

	return FMIForgeGenerationExecutor().Execute(Plan);
}

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateVertexPaintMaterialInstance(const FMIForgeVertexPaintLayerStack& LayerStack, const FMIForgeVertexPaintGenerationOptions& Options) const
{
	const FMIForgeVertexPaintGenerationPlan Plan =
		FMIForgeGenerationPlanner().PlanVertexPaintGeneration(
			LayerStack,
			Options);

	return FMIForgeGenerationExecutor().Execute(Plan);

}
