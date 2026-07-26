// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "Generation/MIForgeDecalParameterApplier.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "MaterialEditingLibrary.h"
#include "MIForgeUtilities.h"

bool FMIForgeDecalParameterApplier::Apply(
    UMaterialInstanceConstant* MaterialInstance,
    const FMIForgeTextureSet& TextureSet,
    const FMIForgeGenerationOptions& Options,
    FText& OutError) const
{
	OutError = FText::GetEmpty();

	if (!IsValid(MaterialInstance))
	{
		OutError =
			FText::FromString(TEXT("Material instance is invalid."));
		return false;
	}

	const FMIForgeMaterialPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetDecal();

	auto ApplyRequiredTexture =
		[MaterialInstance, &TextureSet, &Definition, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName) -> bool
		{
			const FMIForgeTextureBinding* Binding =
				Definition.FindTextureBinding(TextureType);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No material parameter mapping configured for %s."),
						DisplayName));
				return false;
			}

			const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(TextureType);
			UTexture* Texture = TextureInfo && TextureInfo->AssetData.IsValid()
				? Cast<UTexture>(TextureInfo->AssetData.GetAsset())
				: nullptr;

			if (!Texture)
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("%s texture not found or invalid for set: %s"),
						DisplayName,
						*TextureSet.SetName));
				return false;
			}

			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
				MaterialInstance,
				Binding->ParameterName,
				Texture);
			return true;
		};

	if (!ApplyRequiredTexture(EMIForgeTextureType::Albedo, TEXT("Albedo")))
	{
		return false;
	}


	auto ApplyOptionalTexture =
		[MaterialInstance, &TextureSet, &Definition, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName,
			bool bRequested,
			bool& bApplied) -> bool
		{
			bApplied = false;
			if (!bRequested)
			{
				return true;
			}

			const FMIForgeTextureBinding* Binding =
				Definition.FindTextureBinding(TextureType);
			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No material parameter mapping configured for %s."),
						DisplayName));
				return false;
			}

			const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(TextureType);
			UTexture* Texture = TextureInfo && TextureInfo->AssetData.IsValid()
				? Cast<UTexture>(TextureInfo->AssetData.GetAsset())
				: nullptr;

			if (!Texture)
			{
				MIForgeUtilities::PrintLog(
					FString::Printf(
						TEXT("%s texture not found or invalid for set: %s, Generated without %s"),
						DisplayName,
						*TextureSet.SetName,
						DisplayName),
					ELogVerbosity::Warning);
				return true;
			}

			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
				MaterialInstance,
				Binding->ParameterName,
				Texture);
			bApplied = true;
			return true;
	};

	bool bNormalApplied = false;
	bool bORMApplied = false;

	if (!ApplyOptionalTexture(
		EMIForgeTextureType::Normal,
		TEXT("Normal"),
		Options.bUseDecalNormal,
		bNormalApplied) ||
		!ApplyOptionalTexture(
			EMIForgeTextureType::ORM,
			TEXT("ORM"),
			Options.bUseDecalORM,
			bORMApplied))
	{
		return false;
	}

	const FMIForgeStaticSwitchBinding* NormalSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseDecalNormal);
	const FMIForgeStaticSwitchBinding* ORMSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseDecalORM);
	const FMIForgeStaticSwitchBinding* OrientationMaskSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseOrientationMask);

	if (!NormalSwitch || NormalSwitch->ParameterName.IsNone() ||
		!ORMSwitch || ORMSwitch->ParameterName.IsNone() ||
		!OrientationMaskSwitch || OrientationMaskSwitch->ParameterName.IsNone())
	{
		OutError = FText::FromString(
			TEXT("Decal preset has an invalid static switch mapping."));
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		NormalSwitch->ParameterName,
		bNormalApplied);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		ORMSwitch->ParameterName,
		bORMApplied);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		OrientationMaskSwitch->ParameterName,
		Options.bUseOrientationMask);

	return true;
}