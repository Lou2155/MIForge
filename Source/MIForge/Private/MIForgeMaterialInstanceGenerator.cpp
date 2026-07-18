// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeMaterialInstanceGenerator.h"
#include "MIForgeTypes.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h" 
#include "MaterialEditingLibrary.h"
#include "MIForgeUtilities.h"
#include "EditorAssetLibrary.h"
#include "HAL/PlatformFileManager.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "Generation/MIForgeMaterialInstanceResolver.h"

#include "Generation/MIForgeStandardParameterApplier.h"
#include "Generation/MIForgeRGBMaskParameterApplier.h"
#include "Generation/MIForgeVPParameterApplier.h"

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const
{
	FMIForgeGenerationResult Result;

	const FMIForgeMaterialPresetDefinition* Definition = FMIForgePresetDefinitions::FindMaterialPreset(Options.Preset);

	if (!Definition)
	{
		Result.FailedCount = TextureSets.Num();
		Result.Messages.Add(
			FText::FromString(
				TEXT("Unsupported generation preset.")));
		return Result;
	}

	FString ParentMaterialPath = Definition->ParentMaterialPath.ToString();

	UMaterialInterface* ParentMaterial = Cast<UMaterialInterface>(Definition->ParentMaterialPath.TryLoad());
	if(!ParentMaterial)
	{
		Result.Messages.Add(FText::FromString("Failed to load parent material: " + ParentMaterialPath));
		Result.FailedCount = TextureSets.Num();
		return Result;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();

	for (TSharedPtr<FMIForgeTextureSet> TextureSet : TextureSets) {

		if (!TextureSet.IsValid())
		{
			Result.FailedCount++;
			Result.Messages.Add(FText::FromString(TEXT("Failed invalid texture set.")));
			continue;
		}
	
		FText PreflightError;

		if (!ValidateGenerationInputs(
			*TextureSet,
			ParentMaterial,
			Options,
			PreflightError
		))
		{
			Result.FailedCount++;
			Result.Messages.Add(PreflightError);
			continue;
		}

		FMIForgeMaterialInstanceTarget Target;
		Target.AssetName = FString::Printf(
			TEXT("MI_%s"),
			*TextureSet->SetName);
		Target.TargetPath = Options.TargetPath;
		Target.IfMIExists = Options.IfMIExists;
		Target.ParentMaterial = ParentMaterial;

		const FMIForgeMaterialInstanceResolution Resolution =
			FMIForgeMaterialInstanceResolver().Resolve(
				Target,
				AssetTools);

		switch (Resolution.Action)
		{
		case EMIForgeGenerationAction::Skipped:
			Result.SkippedCount++;
			Result.Messages.Add(Resolution.Message);
			continue;

		case EMIForgeGenerationAction::Failed:
			Result.FailedCount++;
			Result.Messages.Add(Resolution.Message);
			continue;

		case EMIForgeGenerationAction::Updated:

			if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
			{
				Result.FailedCount++;
				continue;
			}

			Resolution.MaterialInstance->SetFlags(RF_Transactional);
			Resolution.MaterialInstance->Modify();

			UMaterialEditingLibrary::ClearAllMaterialInstanceParameters(
				Resolution.MaterialInstance
			);
			UMaterialEditingLibrary::SetMaterialInstanceParent(
				Resolution.MaterialInstance,
				ParentMaterial
			);
			break;

		case EMIForgeGenerationAction::Created:

			if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
			{
				Result.FailedCount++;
				continue;
			}

			Resolution.MaterialInstance->SetFlags(RF_Transactional);
			Resolution.MaterialInstance->Modify();

			Result.CreatedAssets.AddUnique(Resolution.MaterialInstance);
			break;
		}

		const bool bWasCreated =
			Resolution.Action == EMIForgeGenerationAction::Created;

		FText ApplyError;
		
		if(ApplyTexturesToMaterialInstance(
			Resolution.MaterialInstance,
			*TextureSet,
			Options,
			ApplyError
		))
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("Applied textures to MI: %s"), *TextureSet->SetName)
			);
			Resolution.MaterialInstance->MarkPackageDirty();
			/*if (!UEditorAssetLibrary::SaveLoadedAsset(Resolution.MaterialInstance, false))  //this is commented out because it prevents transactional undo/redo from working properly. The user can save the asset manually if they want to.
			{
				Result.FailedCount++;
				Result.Messages.Add(FText::FromString(
					FString::Printf(TEXT("Could not save MI: %s"), *TextureSet->SetName)
				));
				continue;
			}*/
		}
		else
		{	
			if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance) && !Resolution.MaterialInstance->IsUnreachable())
			{
				

				FAssetRegistryModule& AssetRegistryModule =
					FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

				IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

				TSet<FString> UniquePathsToScan;
				TArray<UObject*> ObjectsToDelete;

				ObjectsToDelete.Add(Resolution.MaterialInstance);

				const FString PackageName = Resolution.MaterialInstance->GetPackage()->GetName();
				UniquePathsToScan.Add(FPackageName::GetLongPackagePath(PackageName));

				for(UObject* Object : ObjectsToDelete)
				{
					if (Object && IsValid(Object) && !Object->IsUnreachable())
					{
						AssetRegistry.AssetDeleted(Object);
					}
				}

				Result.CreatedAssets.Remove(Resolution.MaterialInstance);

				ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

				if (UniquePathsToScan.Num() > 0)
				{
					const TArray<FString> PathsToScan = UniquePathsToScan.Array();

					AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
				}

				
			}

			Result.FailedCount++;
			if (!ApplyError.IsEmpty())
			{
				Result.Messages.Add(ApplyError);
			}
			else
			{
				Result.Messages.Add(
					FText::FromString(
						FString::Printf(
							TEXT("Failed to apply textures to MI: %s"),
							*TextureSet->SetName)));
			}
			continue;
		}

		if (Resolution.Action == EMIForgeGenerationAction::Created)
		{
			Result.CreatedCount++;
		}
		else
		{
			Result.UpdatedCount++;
		}
		
		Result.AffectedAssets.AddUnique(Resolution.MaterialInstance);
	}

	return Result;
}

bool FMIForgeMaterialInstanceGenerator::ApplyTexturesToMaterialInstance(UMaterialInstanceConstant* MaterialInstance, const FMIForgeTextureSet& TextureSet, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	switch(Options.Preset)
	{
		case EMIForgeGenerationPreset::Standard:
			return FMIForgeStandardParameterApplier().Apply(
				MaterialInstance,
				TextureSet,
				Options,
				OutError);

		case EMIForgeGenerationPreset::RGBMask:
			return FMIForgeRGBMaskParameterApplier().Apply(
				MaterialInstance,
				TextureSet,
				Options,
				OutError);

		default:
			OutError =
				FText::FromString(TEXT("Unsupported generation preset."));
			return false;
	}
}

bool FMIForgeMaterialInstanceGenerator::ValidateGenerationInputs(const FMIForgeTextureSet& TextureSet, UMaterialInterface* ParentMaterial, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	switch (Options.Preset)
	{
		case EMIForgeGenerationPreset::Standard:
			return ValidateStandardGenerationInputs(TextureSet, ParentMaterial, Options, OutError);
		case EMIForgeGenerationPreset::RGBMask:
			return ValidateRGBGenerationInputs(TextureSet, ParentMaterial, Options, OutError);
			
	}

	OutError = FText::FromString(TEXT("Unsupported generation preset."));
	return false;
}

bool FMIForgeMaterialInstanceGenerator::ValidateStandardGenerationInputs(const FMIForgeTextureSet& TextureSet, UMaterialInterface* ParentMaterial, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();
	const FMIForgeMaterialPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetStandard();

	if (!ParentMaterial)
	{
		OutError = FText::FromString(TEXT("Parent material is invalid."));
		return false;
	}

	if (Options.TargetPath.IsEmpty() ||
		(Options.TargetPath != TEXT("/Game") &&
			!Options.TargetPath.StartsWith(TEXT("/Game/"))))
	{
		OutError = FText::FromString(TEXT("Target path must be inside /Game."));
		return false;
	}

	TArray<FName> ParentTextureParameterNames;
	UMaterialEditingLibrary::GetTextureParameterNames(
		ParentMaterial,
		ParentTextureParameterNames
	);

	TArray<FName> ParentStaticSwitchNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(
		ParentMaterial,
		ParentStaticSwitchNames
	);

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

	// Your generator should explicitly set these static bools true or false.
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

bool FMIForgeMaterialInstanceGenerator::ValidateRGBGenerationInputs(const FMIForgeTextureSet& TextureSet, UMaterialInterface* ParentMaterial, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();
	const FMIForgeMaterialPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetRGBMask();

	if (!ParentMaterial)
	{
		OutError = FText::FromString(TEXT("Parent material is invalid."));
		return false;
	}

	if (Options.TargetPath.IsEmpty() ||
		(Options.TargetPath != TEXT("/Game") &&
			!Options.TargetPath.StartsWith(TEXT("/Game/"))))
	{
		OutError = FText::FromString(TEXT("Target path must be inside /Game."));
		return false;
	}

	TArray<FName> ParentTextureParameterNames;
	UMaterialEditingLibrary::GetTextureParameterNames(
		ParentMaterial,
		ParentTextureParameterNames
	);

	TArray<FName> ParentStaticSwitchNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(
		ParentMaterial,
		ParentStaticSwitchNames
	);

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

	if(Options.bUseBaseORMTexture &&
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

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateVertexPaintMaterialInstance(const FMIForgeVertexPaintLayerStack& LayerStack, const FMIForgeVertexPaintGenerationOptions& Options) const
{
	FMIForgeGenerationResult Result;

	const FMIForgeVertexPaintPresetDefinition& Definition = FMIForgePresetDefinitions::GetVertexPaint();

	FString ParentMaterialPath = Definition.ParentMaterialPath.ToString();

	UMaterialInterface* ParentMaterial = Cast<UMaterialInterface>(Definition.ParentMaterialPath.TryLoad());
	if (!ParentMaterial)
	{
		Result.Messages.Add(FText::FromString("Failed to load parent material: " + ParentMaterialPath));
		Result.FailedCount = 1;
		return Result;
	}
	if(LayerStack.GetSlots().Num() == 0)
	{
		Result.Messages.Add(FText::FromString("No slots found in the layer stack."));
		Result.FailedCount = 1;
		return Result;
	}
	for(const FMIForgeVertexPaintLayerSlot* Slot : LayerStack.GetSlots())
	{
		if (!Slot)
		{
			Result.Messages.Add(FText::FromString("Invalid slot found in the layer stack."));
			Result.FailedCount = 1;
			return Result;
		}
		if (Slot->Layer == EMIForgeVertexPaintLayer::Base || Slot->Layer == EMIForgeVertexPaintLayer::LayerR)
		{
			if (!Slot->IsAssigned())
			{
				Result.Messages.Add(FText::FromString("Invalid texture set for slot: " + Slot->ChannelName));
				Result.FailedCount = 1;
				return Result;
			}
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();


	FMIForgeMaterialInstanceTarget Target;

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
	
	Target.AssetName = FinalAssetName;
	Target.TargetPath = Options.TargetPath;
	Target.IfMIExists = Options.IfMIExists;
	Target.ParentMaterial = ParentMaterial;

	const FMIForgeMaterialInstanceResolution Resolution =
			FMIForgeMaterialInstanceResolver().Resolve(
				Target,
				AssetTools);
	
	switch (Resolution.Action)
	{
	case EMIForgeGenerationAction::Skipped:
		Result.SkippedCount++;
		Result.Messages.Add(Resolution.Message);
		return Result;

	case EMIForgeGenerationAction::Failed:
		Result.FailedCount++;
		Result.Messages.Add(Resolution.Message);
		return Result;

	case EMIForgeGenerationAction::Updated:

		if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
		{
			Result.FailedCount = 1;
			return Result;
		}

		Resolution.MaterialInstance->SetFlags(RF_Transactional);
		Resolution.MaterialInstance->Modify();

		UMaterialEditingLibrary::ClearAllMaterialInstanceParameters(
			Resolution.MaterialInstance
		);
		UMaterialEditingLibrary::SetMaterialInstanceParent(
			Resolution.MaterialInstance,
			ParentMaterial
		);
		break;

	case EMIForgeGenerationAction::Created:

		if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
		{
			Result.FailedCount = 1;
			return Result;
		}

		Result.CreatedAssets.AddUnique(Resolution.MaterialInstance);
		break;
	}
	const bool bWasCreated =
		Resolution.Action == EMIForgeGenerationAction::Created;

	FText ApplyError;

	if (!FMIForgeVPParameterApplier().Apply(
		Resolution.MaterialInstance,
		LayerStack,
		ApplyError))
	{
		// Keep existing newly-created-asset cleanup here.
		if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance))
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Resolution.MaterialInstance);

			ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

			Result.CreatedAssets.Remove(Resolution.MaterialInstance);
		}

		if (!ApplyError.IsEmpty())
		{
			Result.Messages.Add(ApplyError);
		}

		Result.FailedCount++;
		return Result;
	}

	if (Resolution.Action == EMIForgeGenerationAction::Created)
	{	
		Resolution.MaterialInstance->MarkPackageDirty();
		Result.CreatedCount++;
	}
	else
	{
		Resolution.MaterialInstance->MarkPackageDirty();	
		Result.UpdatedCount++;
	}

	Result.AffectedAssets.AddUnique(Resolution.MaterialInstance);

	return Result;

}
