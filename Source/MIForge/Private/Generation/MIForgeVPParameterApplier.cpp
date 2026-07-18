// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MIForgeVPParameterApplier.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "MaterialEditingLibrary.h"
#include "MIForgeUtilities.h"


bool FMIForgeVPParameterApplier::Apply(UMaterialInstanceConstant* MaterialInstance, const FMIForgeVertexPaintLayerStack& LayerStack, FText& OutError) const
{
	OutError = FText::GetEmpty();

	if (!IsValid(MaterialInstance))
	{
		OutError =
			FText::FromString(TEXT("Material instance is invalid."));
		return false;
	}

	const bool bLayerGEnabled =
		LayerStack.LayerG.AssignedTextureSet.IsValid();

	const bool bLayerBEnabled =
		LayerStack.LayerB.AssignedTextureSet.IsValid();

	for (const FMIForgeVertexPaintLayerSlot* Slot :
		LayerStack.GetSlots())
	{
		if (!Slot)
		{
			OutError =
				FText::FromString(TEXT("Invalid Vertex Paint layer."));
			return false;
		}

		if (!ApplyLayer(
			MaterialInstance,
			*Slot,
			bLayerGEnabled,
			bLayerBEnabled,
			OutError))
		{
			if (OutError.IsEmpty())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Failed to apply Vertex Paint layer: %s"),
						*Slot->DisplayName));
			}

			return false;
		}
	}

	return true;
}

bool FMIForgeVPParameterApplier::ApplyLayer(UMaterialInstanceConstant* MaterialInstance, const FMIForgeVertexPaintLayerSlot& LayerSlot, bool bLayerGEnabled, bool bLayerBEnabled, FText& OutError) const
{
	if (!MaterialInstance)
	{
		OutError = FText::FromString(TEXT("Material instance is invalid."));
		return false;
	}

	const FMIForgeVertexPaintPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetVertexPaint();
	const FMIForgeVertexPaintLayerDefinition* LayerDefinition =
		Definition.FindLayer(LayerSlot.Layer);

	if (!LayerDefinition)
	{
		OutError = FText::FromString(
			FString::Printf(
				TEXT("No preset definition configured for %s."),
				*LayerSlot.DisplayName));
		return false;
	}

	const bool bIsOptionalLayer = !LayerDefinition->bRequired;

	const bool bLayerAssigned = LayerSlot.AssignedTextureSet.IsValid();

	// Optional empty layers should not block generation.
	if (bIsOptionalLayer && !bLayerAssigned)
	{
		if (LayerDefinition->EnabledSwitchParameter.IsNone())
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("No static bool parameter configured for %s."),
					*LayerSlot.DisplayName));
			return false;
		}

		UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
			MaterialInstance,
			LayerDefinition->EnabledSwitchParameter,
			false);

		return true;
	}

	// Required layers must have Albedo.
	if (!ApplyTexture(
		MaterialInstance,
		LayerSlot,
		EMIForgeTextureType::Albedo,
		TEXT("Albedo"),
		true,
		OutError
	))
	{
		OutError = FText::FromString(
			FString::Printf(
				TEXT("Failed to apply required Albedo texture for %s."),
				*LayerSlot.DisplayName
			)
		);
		return false;
	}

	if (!ApplyTexture(
		MaterialInstance,
		LayerSlot,
		EMIForgeTextureType::Normal,
		TEXT("Normal"),
		false,
		OutError))
	{
		return false;
	}

	if (!ApplyTexture(
		MaterialInstance,
		LayerSlot,
		EMIForgeTextureType::ORM,
		TEXT("ORM"),
		false,
		OutError))
	{
		return false;
	}

	const bool bShouldApplyHeight =
		(LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerR && bLayerGEnabled) ||
		(LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerG && bLayerBEnabled);

	if (LayerSlot.Layer == EMIForgeVertexPaintLayer::Base)
	{
		if (!ApplyTexture(
			MaterialInstance,
			LayerSlot,
			EMIForgeTextureType::Height,
			TEXT("Height"),
			false,
			OutError))
		{
			return false;
		}
	}
	else if (bShouldApplyHeight)
	{
		if (!ApplyTexture(
			MaterialInstance,
			LayerSlot,
			EMIForgeTextureType::Height,
			TEXT("Height"),
			false,
			OutError))
		{
			return false;
		}
	}

	// Enable optional layer switches deterministically.
	if (bIsOptionalLayer)
	{
		if (LayerDefinition->EnabledSwitchParameter.IsNone())
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("No static bool parameter configured for %s."),
					*LayerSlot.DisplayName
				)
			);
			return false;
		}

		UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
			MaterialInstance,
			LayerDefinition->EnabledSwitchParameter,
			true
		);
	}


	return true;
}

bool FMIForgeVPParameterApplier::ApplyTexture(UMaterialInstanceConstant* MaterialInstance, const FMIForgeVertexPaintLayerSlot& LayerSlot, EMIForgeTextureType TextureType, const TCHAR* DisplayName, bool bRequired, FText& OutError) const
{
	if (!MaterialInstance)
	{
		OutError = FText::FromString(
			TEXT("Material instance is invalid.")
		);
		return false;
	}

	if (!LayerSlot.AssignedTextureSet.IsValid())
	{
		if(bRequired)
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("%s has no assigned texture set."),
					*LayerSlot.DisplayName
				)
			);
		}
		else
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(
					TEXT("%s has no assigned texture set."),
					*LayerSlot.DisplayName
				),
				ELogVerbosity::Warning
			);
		}

		return !bRequired;
	}

	const FMIForgeTextureSet& TextureSet = *LayerSlot.AssignedTextureSet;

	const FMIForgeVertexPaintPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetVertexPaint();

	const FMIForgeVertexPaintLayerDefinition* LayerDefinition =
		Definition.FindLayer(LayerSlot.Layer);

	if (!LayerDefinition)
	{
		OutError = FText::FromString(
			FString::Printf(
				TEXT("No texture parameter mapping configured for %s."),
				*LayerSlot.DisplayName
			)
		);
		return false;
	}

	const FName* ParameterName =
		LayerDefinition->TextureParameters.Find(TextureType);

	if (!ParameterName || ParameterName->IsNone())
	{
		OutError = FText::FromString(
			FString::Printf(
				TEXT("No material parameter mapping configured for %s %s."),
				*LayerSlot.DisplayName,
				DisplayName
			)
		);
		return false;
	}

	const FMIForgeTextureInfo* TextureInfo =
		TextureSet.Textures.Find(TextureType);

	UTexture* Texture = TextureInfo && TextureInfo->AssetData.IsValid()
		? Cast<UTexture>(TextureInfo->AssetData.GetAsset())
		: nullptr;

	if (!Texture)

	{
		const ELogVerbosity::Type Verbosity =
			bRequired ? ELogVerbosity::Error : ELogVerbosity::Warning;

		MIForgeUtilities::PrintLog(
			FString::Printf(
				TEXT("%s texture not found or invalid for %s set: %s"),
				DisplayName,
				*LayerSlot.DisplayName,
				*TextureSet.SetName
			),
			Verbosity
		);

		return !bRequired;
	}


	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstance,
		*ParameterName,
		Texture
	);

	return true;
}
