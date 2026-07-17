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
	/*UMaterialInstanceConstant* CreateOrGetMaterialInstance(
		const FMIForgeTextureSet& TextureSet,
		UMaterialInterface* ParentMaterial,
		const FMIForgeGenerationOptions& Options,
		FMIForgeGenerationResult& Result) const;*/

	FMIForgeMaterialInstanceResolution ResolveMaterialInstance(
		const FMIForgeTextureSet& TextureSet,
		UMaterialInterface* ParentMaterial,
		const FMIForgeGenerationOptions& Options,
		IAssetTools& AssetTools
	) const;

	bool ApplyTexturesToMaterialInstance(
		UMaterialInstanceConstant* MaterialInstanceConstant,
		const FMIForgeTextureSet& TextureSet,
		const FMIForgeGenerationOptions& Options
	) const;

	bool ApplyTexturesToStandardMaterialInstance(
		UMaterialInstanceConstant* MaterialInstanceConstant,
		const FMIForgeTextureSet& TextureSet,
		const FMIForgeGenerationOptions& Options
	) const;

	bool ApplyTexturesToRGBMaterialInstance(
		UMaterialInstanceConstant* MaterialInstanceConstant,
		const FMIForgeTextureSet& TextureSet,
		const FMIForgeGenerationOptions& Options
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

	

	FMIForgeMaterialInstanceResolution ResolveMaterialInstanceByName(
		const FString& AssetName,
		UMaterialInterface* ParentMaterial,
		const FString& TargetPath,
		EIfMIExistsOption IfMIExists,
		IAssetTools& AssetTools
	) const;

	bool ApplyVertexPaintLayerTextures(
		UMaterialInstanceConstant* MaterialInstance,
		const FMIForgeVertexPaintLayerSlot& LayerSlot,
		bool bLayerGEnabled,
		bool bLayerBEnabled,
		FText& OutError
	) const;

	bool ApplyVertexPaintTexture(
		UMaterialInstanceConstant* MaterialInstanceConstant,
		const FMIForgeVertexPaintLayerSlot& LayerSlot,
		EMIForgeTextureType TextureType,
		const TCHAR* DisplayName,
		bool bRequired

	) const;
};


