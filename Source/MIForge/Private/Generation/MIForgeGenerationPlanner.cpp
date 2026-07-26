// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MIForgeGenerationPlanner.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "MaterialEditingLibrary.h"
#include "MIForgeValidator.h"
#include "Engine/Texture.h"

FMIForgeMaterialGenerationPlan FMIForgeGenerationPlanner::PlanMaterialGeneration(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const
{
	FMIForgeMaterialGenerationPlan Plan;

	// 1. Find definition.
	const FMIForgeMaterialPresetDefinition* Definition = FMIForgePresetDefinitions::FindMaterialPreset(Options.Preset);

	if (!Definition)
	{
		Plan.FailedPlanningCount = TextureSets.Num();
		Plan.Messages.Add(
			FText::FromString(TEXT("Unsupported generation preset.")));
		return Plan;
	}

	// 2. Validate target path once.
	FText TargetPathError;

	if (!ValidateTargetPath(Options.TargetPath, TargetPathError))
	{
		Plan.FailedPlanningCount = TextureSets.Num();
		Plan.Messages.Add(TargetPathError);
		return Plan;
	}

	// 3. Load parent once.
	UMaterialInterface* ParentMaterial = Cast<UMaterialInterface>(Definition->ParentMaterialPath.TryLoad());

	if (!IsValid(ParentMaterial))
	{
		Plan.FailedPlanningCount = TextureSets.Num();
		Plan.Messages.Add(
			FText::FromString(
				FString::Printf(
					TEXT("Failed to load parent material: %s"),
					*Definition->ParentMaterialPath.ToString())));
		return Plan;
	}

	// 4. Read parent parameters once.
	TArray<FName> ParentTextureParameterNames;
	UMaterialEditingLibrary::GetTextureParameterNames(ParentMaterial, ParentTextureParameterNames);

	TArray<FName> ParentStaticSwitchNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(ParentMaterial, ParentStaticSwitchNames);

	// 5. Validate every texture set.
	for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : TextureSets)
	{
		if (!TextureSet.IsValid())
		{
			Plan.FailedPlanningCount++;
			Plan.Messages.Add(
				FText::FromString(TEXT("Failed invalid texture set.")));
			continue;
		}

		FText ItemError;
		bool bValid = false;

		switch (Options.Preset)
		{
		case EMIForgeGenerationPreset::Standard:
			bValid = ValidateStandardItem(
				*TextureSet,
				*Definition,
				ParentTextureParameterNames,
				ParentStaticSwitchNames,
				Options,
				ItemError);
			break;

		case EMIForgeGenerationPreset::RGBMask:
			bValid = ValidateRGBMaskItem(
				*TextureSet,
				*Definition,
				ParentTextureParameterNames,
				ParentStaticSwitchNames,
				Options,
				ItemError);
			break;
		case EMIForgeGenerationPreset::Decal:
			bValid = ValidateDecalItem(
				*TextureSet,
				*Definition,
				ParentTextureParameterNames,
				ParentStaticSwitchNames,
				Options,
				ItemError);
			break;

		default:
			ItemError =
				FText::FromString(TEXT("Unsupported generation preset."));
			break;
		}

		if (!bValid)
		{
			Plan.FailedPlanningCount++;

			Plan.Messages.Add(
				FText::FromString(
					FString::Printf(
						TEXT("Cannot generate set '%s': %s"),
						*TextureSet->SetName,
						*ItemError.ToString())));

			continue;
		}

		FMIForgePlannedMaterialItem Item;
		
		Item.ParentMaterial = ParentMaterial;
		Item.TextureSet = TextureSet;
		Item.Options = Options;

		FString DesiredAssetName;

		if (Options.Preset == EMIForgeGenerationPreset::Decal)
		{
			DesiredAssetName = FString::Printf(
				TEXT("MI_Decal_%s"),
				*TextureSet->SetName);
		}
		else if (Options.Preset == EMIForgeGenerationPreset::RGBMask)
		{
			DesiredAssetName = FString::Printf(
				TEXT("MI_RGB_%s"),
				*TextureSet->SetName);
		}
		else
		{
			DesiredAssetName = FString::Printf(
				TEXT("MI_%s"),
				*TextureSet->SetName);
		}


		const FString PackageName = FString::Printf(
			TEXT("%s/%s"),
			*Options.TargetPath,
			*DesiredAssetName);

		if (!FPackageName::IsValidLongPackageName(PackageName))
		{
			Plan.FailedPlanningCount++;
			Plan.Messages.Add(
				FText::FromString(
					FString::Printf(
						TEXT("Invalid output package name for set '%s'."),
						*TextureSet->SetName)));
			continue;
		}

		Item.DesiredAssetName = DesiredAssetName;
		Plan.Items.Add(MoveTemp(Item));

	}

	return Plan;
}

FMIForgeVertexPaintGenerationPlan FMIForgeGenerationPlanner::PlanVertexPaintGeneration(const FMIForgeVertexPaintLayerStack& LayerStack, const FMIForgeVertexPaintGenerationOptions& Options) const
{
	FMIForgeVertexPaintGenerationPlan Plan;

	FText Error;

	if (!ValidateTargetPath(Options.TargetPath, Error))
	{
		Plan.Messages.Add(Error);
		return Plan;
	}

	const FMIForgeVertexPaintPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetVertexPaint();

	UMaterialInterface* ParentMaterial =
		Cast<UMaterialInterface>(
			Definition.ParentMaterialPath.TryLoad());

	if (!IsValid(ParentMaterial))
	{
		Plan.Messages.Add(
			FText::FromString(
				FString::Printf(
					TEXT("Failed to load parent material: %s"),
					*Definition.ParentMaterialPath.ToString())));
		return Plan;
	}

	if (!ValidateVertexPaintPlan(
		LayerStack,
		Definition,
		ParentMaterial,
		Error))
	{
		Plan.Messages.Add(Error);
		return Plan;
	}

	if (!LayerStack.BaseLayer.AssignedTextureSet.IsValid())
	{
		Plan.Messages.Add(
			FText::FromString(
				TEXT("Vertex Paint Base layer is not assigned.")));
		return Plan;
	}

	const FString RequestedName =
		Options.MaterialInstanceName.IsEmpty()
		? LayerStack.BaseLayer.AssignedTextureSet->SetName
		: Options.MaterialInstanceName;

	const FString FinalAssetName =
		RequestedName.StartsWith(TEXT("MI_VP_")) ||
		RequestedName.StartsWith(TEXT("MI_"))
		? RequestedName
		: FString::Printf(
			TEXT("MI_VP_%s"),
			*RequestedName);

	Plan.DesiredAssetName = FinalAssetName;

	const FString PackageName = FString::Printf(
		TEXT("%s/%s"),
		*Options.TargetPath,
		*FinalAssetName);

	if (!FPackageName::IsValidLongPackageName(PackageName))
	{
		Plan.Messages.Add(
			FText::FromString(
				FString::Printf(
					TEXT("Invalid output package name: '%s'."),
					*Plan.DesiredAssetName)));
		return Plan;
	}

	Plan.ParentMaterial = ParentMaterial;
	Plan.LayerStack = LayerStack;
	Plan.Options = Options;
	Plan.bValid = true;

	return Plan;
}

bool FMIForgeGenerationPlanner::ValidateTargetPath(const FString& TargetPath, FText& OutError) const
{
	OutError = FText::GetEmpty();

	if(TargetPath.IsEmpty())
	{
		OutError = FText::FromString(TEXT("Target path is empty."));
		return false;
	}
	if (TargetPath != TEXT("/Game") &&
		!TargetPath.StartsWith(TEXT("/Game/")))
	{
		OutError =
			FText::FromString(TEXT("Target path must be inside /Game."));
		return false;
	}

	return true;
}

bool FMIForgeGenerationPlanner::ValidateStandardItem(const FMIForgeTextureSet& TextureSet, const FMIForgeMaterialPresetDefinition& Definition, const TArray<FName>& ParentTextureParameterNames, const TArray<FName>& ParentStaticSwitchNames, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();

	auto ValidateTextureMapping =
		[&Definition, &ParentTextureParameterNames, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeTextureBinding* Binding =
				Definition.FindTextureBinding(TextureType);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No texture parameter mapping is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentTextureParameterNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s texture parameter '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
	};

	auto ValidateRequiredTexture =
		[&TextureSet, &OutError, &ValidateTextureMapping](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			if (!ValidateTextureMapping(TextureType, DisplayName))
			{
				return false;
			}

			const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(TextureType);

			if (!TextureInfo || !TextureInfo->AssetData.IsValid())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture is missing for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			UTexture* Texture =
				Cast<UTexture>(TextureInfo->AssetData.GetAsset());

			if (!Texture)
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture could not be loaded for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			return true;
	};

	auto ValidateStaticSwitch =
		[&Definition, &ParentStaticSwitchNames, &OutError](
			EMIForgePresetOptions Option,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeStaticSwitchBinding* Binding =
				Definition.FindStaticSwitchBinding(Option);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No static bool parameter is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s static bool '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
	};

	// Required texture assets and mappings.
	if (!ValidateRequiredTexture(EMIForgeTextureType::Albedo, TEXT("Albedo")) ||
		!ValidateRequiredTexture(EMIForgeTextureType::Normal, TEXT("Normal")) ||
		!ValidateRequiredTexture(EMIForgeTextureType::ORM, TEXT("ORM")))
	{
		return false;
	}

	// Optional texture mappings are required only when the user requested them.
	// The optional texture asset itself may still be absent; that remains a warning.
	if (Options.bUseEmissive &&
		!ValidateTextureMapping(EMIForgeTextureType::Emissive, TEXT("Emissive")))
	{
		return false;
	}

	if (Options.bUseDetailNormal &&
		!ValidateTextureMapping(EMIForgeTextureType::DetailNormal, TEXT("Detail Normal")))
	{
		return false;
	}

	if (!ValidateStaticSwitch(
		EMIForgePresetOptions::UseEmissiveTexture,
		TEXT("Emissive")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::UseDetailNormalTexture,
			TEXT("Detail Normal")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::UseTriplanar,
			TEXT("Triplanar")))
	{
		return false;
	}

	return true;

}

bool FMIForgeGenerationPlanner::ValidateRGBMaskItem(const FMIForgeTextureSet& TextureSet, const FMIForgeMaterialPresetDefinition& Definition, const TArray<FName>& ParentTextureParameterNames, const TArray<FName>& ParentStaticSwitchNames, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();

	auto ValidateTextureMapping =
		[&Definition, &ParentTextureParameterNames, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeTextureBinding* Binding =
				Definition.FindTextureBinding(TextureType);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No texture parameter mapping is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentTextureParameterNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s texture parameter '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
		};

	auto ValidateRequiredTexture =
		[&TextureSet, &OutError, &ValidateTextureMapping](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			if (!ValidateTextureMapping(TextureType, DisplayName))
			{
				return false;
			}

			const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(TextureType);

			if (!TextureInfo || !TextureInfo->AssetData.IsValid())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture is missing for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			UTexture* Texture =
				Cast<UTexture>(TextureInfo->AssetData.GetAsset());

			if (!Texture)
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture could not be loaded for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			return true;
		};

	auto ValidateStaticSwitch =
		[&Definition, &ParentStaticSwitchNames, &OutError](
			EMIForgePresetOptions Option,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeStaticSwitchBinding* Binding =
				Definition.FindStaticSwitchBinding(Option);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No static bool parameter is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s static bool '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
		};

	// Required texture assets and mappings.
	if (!ValidateRequiredTexture(EMIForgeTextureType::Albedo, TEXT("Albedo")) ||
		!ValidateRequiredTexture(EMIForgeTextureType::Normal, TEXT("Normal")) ||
		!ValidateRequiredTexture(EMIForgeTextureType::RGB, TEXT("RGB")))
	{
		return false;
	}

	if (Options.bUseBaseORMTexture &&
		!ValidateRequiredTexture(EMIForgeTextureType::ORM, TEXT("ORM")))
	{
		return false;
	}

	// Optional texture mappings are required only when the user requested them.
	// The optional texture asset itself may still be absent; that remains a warning.

	if (Options.bUseDetailNormalTextureRGB &&
		!ValidateTextureMapping(EMIForgeTextureType::DetailNormal, TEXT("Detail Normal")))
	{
		return false;
	}

	// Your generator should explicitly set these static bools true or false.
	if (!ValidateStaticSwitch(
		EMIForgePresetOptions::UseBaseORM,
		TEXT("Base ORM")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::EnableEmissiveChannel,
			TEXT("Emissive Channel")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::UseDetailNormalTexture,
			TEXT("Detail Normal"))
		)
	{
		return false;
	}

	return true;
}

bool FMIForgeGenerationPlanner::ValidateVertexPaintPlan(const FMIForgeVertexPaintLayerStack& LayerStack, const FMIForgeVertexPaintPresetDefinition& Definition, UMaterialInterface* ParentMaterial, FText& OutError) const
{
	OutError = FText::GetEmpty();

	if (!IsValid(ParentMaterial))
	{
		OutError = FText::FromString(
			TEXT("Vertex Paint parent material is invalid."));
		return false;
	}

	const FMIForgeVertexPaintLayerStackValidationResult LogicalResult =
		FMIForgeValidator().ValidateVertexPaintLayerStack(
			LayerStack,
			true);

	if (!LogicalResult.bCanGenerate)
	{
		OutError = FText::FromString(
			TEXT("Vertex Paint layer stack does not satisfy preset rules."));
		return false;
	}

	TArray<FName> ParentTextureParameterNames;
	UMaterialEditingLibrary::GetTextureParameterNames(
		ParentMaterial,
		ParentTextureParameterNames);

	TArray<FName> ParentStaticSwitchNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(
		ParentMaterial,
		ParentStaticSwitchNames);

	const bool bLayerGEnabled =
		LayerStack.LayerG.AssignedTextureSet.IsValid();
	const bool bLayerBEnabled =
		LayerStack.LayerB.AssignedTextureSet.IsValid();

	for (const FMIForgeVertexPaintLayerSlot* Slot : LayerStack.GetSlots())
	{
		if (!Slot)
		{
			OutError = FText::FromString(
				TEXT("Vertex Paint layer stack contains an invalid slot."));
			return false;
		}

		const FMIForgeVertexPaintLayerDefinition* LayerDefinition =
			Definition.FindLayer(Slot->Layer);

		if (!LayerDefinition)
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("No Vertex Paint preset definition is configured for layer '%s'."),
					*Slot->DisplayName));
			return false;
		}

		if (LayerDefinition->bRequired &&
			!Slot->AssignedTextureSet.IsValid())
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("Required Vertex Paint layer '%s' is not assigned."),
					*LayerDefinition->DisplayName));
			return false;
		}

		if (!LayerDefinition->bRequired)
		{
			if (LayerDefinition->EnabledSwitchParameter.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No enable switch is configured for Vertex Paint layer '%s'."),
						*LayerDefinition->DisplayName));
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(
				LayerDefinition->EnabledSwitchParameter))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the enable switch '%s' for Vertex Paint layer '%s'."),
						*LayerDefinition->EnabledSwitchParameter.ToString(),
						*LayerDefinition->DisplayName));
				return false;
			}
		}

		if (!Slot->AssignedTextureSet.IsValid())
		{
			continue;
		}

		const FMIForgeTextureSet& TextureSet =
			*Slot->AssignedTextureSet;

		auto ValidateTextureParameter =
			[&ParentTextureParameterNames,
			 LayerDefinition,
			 Slot,
			 &OutError](
				EMIForgeTextureType TextureType,
				const TCHAR* DisplayName) -> bool
			{
				const FName* ParameterName =
					LayerDefinition->TextureParameters.Find(TextureType);

				if (!ParameterName || ParameterName->IsNone())
				{
					OutError = FText::FromString(
						FString::Printf(
							TEXT("No %s parameter mapping is configured for Vertex Paint layer '%s'."),
							DisplayName,
							*Slot->DisplayName));
					return false;
				}

				if (!ParentTextureParameterNames.Contains(*ParameterName))
				{
					OutError = FText::FromString(
						FString::Printf(
							TEXT("Parent material does not contain the %s parameter '%s' for Vertex Paint layer '%s'."),
							DisplayName,
							*ParameterName->ToString(),
							*Slot->DisplayName));
					return false;
				}

				return true;
			};

		if (!ValidateTextureParameter(
				EMIForgeTextureType::Albedo,
				TEXT("Albedo")) ||
			!ValidateTextureParameter(
				EMIForgeTextureType::Normal,
				TEXT("Normal")) ||
			!ValidateTextureParameter(
				EMIForgeTextureType::ORM,
				TEXT("ORM")))
		{
			return false;
		}

		const FMIForgeTextureInfo* AlbedoInfo =
			TextureSet.Textures.Find(EMIForgeTextureType::Albedo);

		if (!AlbedoInfo || !AlbedoInfo->AssetData.IsValid())
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("Required Albedo texture is missing for Vertex Paint layer '%s'."),
					*Slot->DisplayName));
			return false;
		}

		UTexture* AlbedoTexture =
			Cast<UTexture>(AlbedoInfo->AssetData.GetAsset());

		if (!IsValid(AlbedoTexture))
		{
			OutError = FText::FromString(
				FString::Printf(
					TEXT("Required Albedo texture could not be loaded for Vertex Paint layer '%s'."),
					*Slot->DisplayName));
			return false;
		}

		const bool bNeedsHeightParameter =
			Slot->Layer == EMIForgeVertexPaintLayer::Base ||
			(Slot->Layer == EMIForgeVertexPaintLayer::LayerR &&
			 bLayerGEnabled) ||
			(Slot->Layer == EMIForgeVertexPaintLayer::LayerG &&
			 bLayerBEnabled);

		if (bNeedsHeightParameter &&
			!ValidateTextureParameter(
				EMIForgeTextureType::Height,
				TEXT("Height")))
		{
			return false;
		}
	}

	return true;
}

bool FMIForgeGenerationPlanner::ValidateDecalItem(const FMIForgeTextureSet& TextureSet, const FMIForgeMaterialPresetDefinition& Definition, const TArray<FName>& ParentTextureParameterNames, const TArray<FName>& ParentStaticSwitchNames, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();

	auto ValidateTextureMapping =
		[&Definition, &ParentTextureParameterNames, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeTextureBinding* Binding =
				Definition.FindTextureBinding(TextureType);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No texture parameter mapping is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentTextureParameterNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s texture parameter '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
		};

	auto ValidateRequiredTexture =
		[&TextureSet, &OutError, &ValidateTextureMapping](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			if (!ValidateTextureMapping(TextureType, DisplayName))
			{
				return false;
			}

			const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(TextureType);

			if (!TextureInfo || !TextureInfo->AssetData.IsValid())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture is missing for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			UTexture* Texture =
				Cast<UTexture>(TextureInfo->AssetData.GetAsset());

			if (!Texture)
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Required %s texture could not be loaded for set '%s'."),
						DisplayName,
						*TextureSet.SetName
					)
				);
				return false;
			}

			return true;
		};

	auto ValidateStaticSwitch =
		[&Definition, &ParentStaticSwitchNames, &OutError](
			EMIForgePresetOptions Option,
			const TCHAR* DisplayName
			) -> bool
		{
			const FMIForgeStaticSwitchBinding* Binding =
				Definition.FindStaticSwitchBinding(Option);

			if (!Binding || Binding->ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No static bool parameter is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(Binding->ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s static bool '%s'."),
						DisplayName,
						*Binding->ParameterName.ToString()
					)
				);
				return false;
			}

			return true;
		};

	// Required texture assets and mappings.
	if (!ValidateRequiredTexture(EMIForgeTextureType::Albedo, TEXT("Albedo")))
	{
		return false;
	}

	// Optional texture mappings are required only when the user requested them.
	// The optional texture asset itself may still be absent; that remains a warning.
	if (Options.bUseDecalNormal &&
		!ValidateTextureMapping(EMIForgeTextureType::Normal, TEXT("Normal")))
	{
		return false;
	}

	if (Options.bUseDecalORM &&
		!ValidateTextureMapping(EMIForgeTextureType::ORM, TEXT("ORM")))
	{
		return false;
	}

	if (!ValidateStaticSwitch(
		EMIForgePresetOptions::UseDecalNormal,
		TEXT("Decal Normal")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::UseDecalORM,
			TEXT("Decal ORM")) ||
		!ValidateStaticSwitch(
			EMIForgePresetOptions::UseOrientationMask,
			TEXT("Orientation Mask")))
	{
		return false;
	}

	return true;
}
