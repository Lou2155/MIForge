// Fill out your copyright notice in the Description page of Project Settings.


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
