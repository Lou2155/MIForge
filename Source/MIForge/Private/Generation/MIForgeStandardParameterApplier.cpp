// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MIForgeStandardParameterApplier.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "MaterialEditingLibrary.h"
#include "MIForgeUtilities.h"

bool FMIForgeStandardParameterApplier::Apply(
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
		FMIForgePresetDefinitions::GetStandard();

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
		!ApplyRequiredTexture(EMIForgeTextureType::ORM, TEXT("ORM")))
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

	bool bEmissiveApplied = false;
	bool bDetailNormalApplied = false;
	if (!ApplyOptionalTexture(
		EMIForgeTextureType::Emissive,
		TEXT("Emissive"),
		Options.bUseEmissive,
		bEmissiveApplied) ||
		!ApplyOptionalTexture(
			EMIForgeTextureType::DetailNormal,
			TEXT("Detail Normal"),
			Options.bUseDetailNormal,
			bDetailNormalApplied))
	{
		return false;
	}

	const FMIForgeStaticSwitchBinding* EmissiveSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseEmissiveTexture);
	const FMIForgeStaticSwitchBinding* DetailNormalSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseDetailNormalTexture);
	const FMIForgeStaticSwitchBinding* TriplanarSwitch =
		Definition.FindStaticSwitchBinding(
			EMIForgePresetOptions::UseTriplanar);

	if (!EmissiveSwitch || EmissiveSwitch->ParameterName.IsNone() ||
		!DetailNormalSwitch || DetailNormalSwitch->ParameterName.IsNone() ||
		!TriplanarSwitch || TriplanarSwitch->ParameterName.IsNone())
	{
		OutError = FText::FromString(
			TEXT("Standard preset has an invalid static switch mapping."));
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		EmissiveSwitch->ParameterName,
		bEmissiveApplied);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		DetailNormalSwitch->ParameterName,
		bDetailNormalApplied);
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstance,
		TriplanarSwitch->ParameterName,
		Options.bUseTriplanar);

	return true;
}

