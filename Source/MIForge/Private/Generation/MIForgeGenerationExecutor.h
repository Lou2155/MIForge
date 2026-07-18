// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Generation/MIForgeGenerationPlan.h"

class UMaterialInstanceConstant;
class UMaterialInterface;

class FMIForgeGenerationExecutor
{
public:
	FMIForgeGenerationResult Execute(
		const FMIForgeMaterialGenerationPlan& Plan) const;

	FMIForgeGenerationResult Execute(
		const FMIForgeVertexPaintGenerationPlan& Plan) const;

private:
	bool ApplyMaterialParameters(
		UMaterialInstanceConstant* MaterialInstance,
		const FMIForgePlannedMaterialItem& Item,
		FText& OutError) const;

	void PrepareForMutation(
		UMaterialInstanceConstant* MaterialInstance,
		UMaterialInterface* ParentMaterial,
		EMIForgeGenerationAction Action) const;

	void CleanupFailedCreatedAsset(
		UMaterialInstanceConstant* MaterialInstance,
		FMIForgeGenerationResult& Result) const;
	
};
