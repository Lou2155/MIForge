// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
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
