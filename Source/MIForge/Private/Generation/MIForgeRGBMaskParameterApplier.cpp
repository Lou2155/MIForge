// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MIForgeRGBMaskParameterApplier.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "MaterialEditingLibrary.h"
#include "MIForgeUtilities.h"


bool  FMIForgeRGBMaskParameterApplier::Apply(
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
		FMIForgePresetDefinitions::GetRGBMask();

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

	if (!ApplyRequiredTexture(EMIForgeTextureType::Albedo, TEXT("Albedo")) ||
		!ApplyRequiredTexture(EMIForgeTextureType::Normal, TEXT("Normal")) ||
		!ApplyRequiredTexture(EMIForgeTextureType::RGB, TEXT("RGB")))
	{
		return false;
	}

	bool bBaseORMApplied = false;
	if (Options.bUseBaseORMTexture)
	{
		if (!ApplyRequiredTexture(EMIForgeTextureType::ORM, TEXT("ORM")))
		{
			return false;
		}
		bBaseORMApplied = true;
	}

	bool bDetailNormalApplied = false;
	if (Options.bUseDetailNormalTextureRGB)
	{
		const FMIForgeTextureBinding* Binding =
			Definition.FindTextureBinding(EMIForgeTextureType::DetailNormal);
		if (!Binding || Binding->ParameterName.IsNone())
		{
			OutError = FText::FromString(
				TEXT("No material parameter mapping configured for Detail Normal."));
			return false;
		}

		const FMIForgeTextureInfo* DetailNormal =
			TextureSet.Textures.Find(EMIForgeTextureType::DetailNormal);
		UTexture* Texture = DetailNormal && DetailNormal->AssetData.IsValid()
			? Cast<UTexture>(DetailNormal->AssetData.GetAsset())
			: nullptr;

		if (!Texture)
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(
					TEXT("Detail Normal texture not found or invalid for set: %s, Generated without Detail Normal"),
					*TextureSet.SetName),
				ELogVerbosity::Warning);
		}
		else
		{
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
				MaterialInstance,
				Binding->ParameterName,
				Texture);
			bDetailNormalApplied = true;
		}
	}

	const FMIForgeStaticSwitchBinding* BaseORMSwitch =
		Definition.FindStaticSwitchBinding(EMIForgePresetOptions::UseBaseORM);
	const FMIForgeStaticSwitchBinding* EmissiveChannelSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::EnableEmissiveChannel);
	const FMIForgeStaticSwitchBinding* DetailNormalSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseDetailNormalTexture);

	if (!BaseORMSwitch || BaseORMSwitch->ParameterName.IsNone() ||
		!EmissiveChannelSwitch || EmissiveChannelSwitch->ParameterName.IsNone() ||
		!DetailNormalSwitch || DetailNormalSwitch->ParameterName.IsNone())
	{
		OutError = FText::FromString(
			TEXT("RGB Mask preset has an invalid static switch mapping."));
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		BaseORMSwitch->ParameterName,
		bBaseORMApplied);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		EmissiveChannelSwitch->ParameterName,
		Options.bEnableEmissiveChannel);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		DetailNormalSwitch->ParameterName,
		bDetailNormalApplied);

	return true;
}