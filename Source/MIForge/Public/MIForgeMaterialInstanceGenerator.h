// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
class IAssetTools;

class FMIForgeMaterialInstanceGenerator
{
public:
	FMIForgeGenerationResult GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const;

	FMIForgeGenerationResult GenerateVertexPaintMaterialInstance(
		const FMIForgeVertexPaintLayerStack& LayerStack,
		const FMIForgeVertexPaintGenerationOptions& Options
	) const;

private:

	bool ApplyTexturesToMaterialInstance(
		UMaterialInstanceConstant* MaterialInstanceConstant,
		const FMIForgeTextureSet& TextureSet,
		const FMIForgeGenerationOptions& Options, 
		FText& OutError
	) const;

	bool ValidateGenerationInputs(
		const FMIForgeTextureSet& TextureSet,
		UMaterialInterface* ParentMaterial,
		const FMIForgeGenerationOptions& Options,
		FText& OutError
	) const;

	bool ValidateStandardGenerationInputs(
		const FMIForgeTextureSet& TextureSet,
		UMaterialInterface* ParentMaterial,
		const FMIForgeGenerationOptions& Options,
		FText& OutError
	) const;

	bool ValidateRGBGenerationInputs(
		const FMIForgeTextureSet& TextureSet,
		UMaterialInterface* ParentMaterial,
		const FMIForgeGenerationOptions& Options,
		FText& OutError
	) const;


};


