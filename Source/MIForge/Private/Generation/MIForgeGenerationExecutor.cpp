// Fill out your copyright notice in the Description page of Project Settings.


#include "Generation/MIForgeGenerationExecutor.h"

#include "Generation/MIForgeMaterialInstanceResolver.h"
#include "Generation/MIForgeStandardParameterApplier.h"
#include "Generation/MIForgeRGBMaskParameterApplier.h"
#include "Generation/MIForgeVPParameterApplier.h"
#include "Generation/MIForgeDecalParameterApplier.h"

#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ObjectTools.h"
#include "MIForgeUtilities.h"


FMIForgeGenerationResult FMIForgeGenerationExecutor::Execute(const FMIForgeMaterialGenerationPlan& Plan) const
{
	FMIForgeGenerationResult Result;

	Result.FailedCount = Plan.FailedPlanningCount;
	Result.Messages = Plan.Messages;

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(
			TEXT("AssetTools"));

	IAssetTools& AssetTools = AssetToolsModule.Get();

	for (const FMIForgePlannedMaterialItem& Item : Plan.Items)
	{
		const FMIForgeTextureSet& TextureSet = *Item.TextureSet;

		FMIForgeMaterialInstanceTarget Target;
		Target.AssetName = Item.DesiredAssetName;
		Target.TargetPath = Item.Options.TargetPath;
		Target.IfMIExists = Item.Options.IfMIExists;
		Target.ParentMaterial = Item.ParentMaterial;

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

			PrepareForMutation(Resolution.MaterialInstance, Item.ParentMaterial, Resolution.Action);
			break;

		case EMIForgeGenerationAction::Created:

			if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
			{
				Result.FailedCount++;
				continue;
			}

			PrepareForMutation(Resolution.MaterialInstance, Target.ParentMaterial, Resolution.Action);

			Result.CreatedAssets.AddUnique(Resolution.MaterialInstance);
			break;
		}

		const bool bWasCreated =
			Resolution.Action == EMIForgeGenerationAction::Created;

		FText ApplyError;

		if (ApplyMaterialParameters(
			Resolution.MaterialInstance,
			Item,
			ApplyError
		))
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("Applied textures to MI: %s"), *TextureSet.SetName)
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
			if (bWasCreated)
			{
				CleanupFailedCreatedAsset(Resolution.MaterialInstance, Result);
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
							*TextureSet.SetName)));
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

FMIForgeGenerationResult FMIForgeGenerationExecutor::Execute(const FMIForgeVertexPaintGenerationPlan& Plan) const
{
	FMIForgeGenerationResult Result;
	Result.Messages = Plan.Messages;

	if (!Plan.bValid)
	{
		Result.FailedCount = 1;
		return Result;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();


	FMIForgeMaterialInstanceTarget Target;

	Target.AssetName = Plan.DesiredAssetName;
	Target.TargetPath = Plan.Options.TargetPath;
	Target.IfMIExists = Plan.Options.IfMIExists;
	Target.ParentMaterial = Plan.ParentMaterial;

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
		PrepareForMutation(Resolution.MaterialInstance, Target.ParentMaterial, Resolution.Action);
		break;

	case EMIForgeGenerationAction::Created:

		if (!Resolution.MaterialInstance || !IsValid(Resolution.MaterialInstance))
		{
			Result.FailedCount = 1;
			return Result;
		}

		PrepareForMutation(Resolution.MaterialInstance, Target.ParentMaterial, Resolution.Action);

		Result.CreatedAssets.AddUnique(Resolution.MaterialInstance);
		break;
	}
	const bool bWasCreated =
		Resolution.Action == EMIForgeGenerationAction::Created;

	FText ApplyError;

	if (!FMIForgeVPParameterApplier().Apply(
		Resolution.MaterialInstance,
		Plan.LayerStack,
		ApplyError))
	{
		if (bWasCreated)
		{
			CleanupFailedCreatedAsset(
				Resolution.MaterialInstance,
				Result);
		}

		Result.FailedCount++;

		if (!ApplyError.IsEmpty())
		{
			Result.Messages.Add(ApplyError);
		}

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

bool FMIForgeGenerationExecutor::ApplyMaterialParameters(UMaterialInstanceConstant* MaterialInstance, const FMIForgePlannedMaterialItem& Item, FText& OutError) const
{
	switch (Item.Options.Preset)
	{
	case EMIForgeGenerationPreset::Standard:
		return FMIForgeStandardParameterApplier().Apply(
			MaterialInstance,
			*Item.TextureSet,
			Item.Options,
			OutError);

	case EMIForgeGenerationPreset::RGBMask:
		return FMIForgeRGBMaskParameterApplier().Apply(
			MaterialInstance,
			*Item.TextureSet,
			Item.Options,
			OutError);

	case EMIForgeGenerationPreset::Decal:
		return FMIForgeDecalParameterApplier().Apply(
			MaterialInstance,
			*Item.TextureSet,
			Item.Options,
			OutError);
	default:
		OutError = FText::FromString(
			TEXT("Unsupported generation preset."));
		return false;
	}
}

void FMIForgeGenerationExecutor::PrepareForMutation(UMaterialInstanceConstant* MaterialInstance, UMaterialInterface* ParentMaterial, EMIForgeGenerationAction Action) const
{
	MaterialInstance->SetFlags(RF_Transactional);
	MaterialInstance->Modify();

	if (Action == EMIForgeGenerationAction::Updated)
	{
		UMaterialEditingLibrary::ClearAllMaterialInstanceParameters(
			MaterialInstance);

		UMaterialEditingLibrary::SetMaterialInstanceParent(
			MaterialInstance,
			ParentMaterial);
	}
}

void FMIForgeGenerationExecutor::CleanupFailedCreatedAsset(UMaterialInstanceConstant* MaterialInstance, FMIForgeGenerationResult& Result) const
{
	if (!MaterialInstance ||
		!IsValid(MaterialInstance) ||
		MaterialInstance->IsUnreachable())
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TSet<FString> UniquePathsToScan;
	TArray<UObject*> ObjectsToDelete;

	ObjectsToDelete.Add(MaterialInstance);

	const FString PackageName = MaterialInstance->GetPackage()->GetName();
	UniquePathsToScan.Add(FPackageName::GetLongPackagePath(PackageName));

	for (UObject* Object : ObjectsToDelete)
	{
		if (Object && IsValid(Object) && !Object->IsUnreachable())
		{
			AssetRegistry.AssetDeleted(Object);
		}
	}

	Result.CreatedAssets.Remove(MaterialInstance);

	ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

	if (UniquePathsToScan.Num() > 0)
	{
		const TArray<FString> PathsToScan = UniquePathsToScan.Array();

		AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
	}
}
