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

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const
{	
	FMIForgeGenerationResult Result;
	FString ParentMaterialPath = Options.MaterialInstanceParentPath.ToString();

	UMaterialInterface* ParentMaterial = Cast<UMaterialInterface>(Options.MaterialInstanceParentPath.TryLoad());
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

		FMIForgeMaterialInstanceResolution Resolution =
			ResolveMaterialInstance(*TextureSet, ParentMaterial, Options, AssetTools);

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

		
		if(ApplyTexturesToMaterialInstance(
			Resolution.MaterialInstance,
			*TextureSet,
			Options
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
			Result.Messages.Add(FText::FromString(
				FString::Printf(TEXT("Failed to apply textures to MI: %s"), *TextureSet->SetName)
			));
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

FMIForgeMaterialInstanceResolution FMIForgeMaterialInstanceGenerator::ResolveMaterialInstance(const FMIForgeTextureSet& TextureSet, UMaterialInterface* ParentMaterial, const FMIForgeGenerationOptions& Options, IAssetTools& AssetTools) const
{
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();

	Factory->InitialParent = ParentMaterial;

	const FString AssetName = FString::Printf(TEXT("MI_%s"), *TextureSet.SetName);

	const FString BasePackageName = FString::Printf(
		TEXT("%s/%s"),
		*Options.TargetPath,
		*AssetName
	);

	const FString ObjectPath = FString::Printf(
		TEXT("%s.%s"),
		*BasePackageName,
		*AssetName
	);

	// Check multiple sources and validate object state
	bool bAssetAlreadyExists = false;
	UMaterialInstanceConstant* ExistingValidMIC = nullptr;

	// Step 1: Try to load the asset
	UObject* ExistingAsset = UEditorAssetLibrary::LoadAsset(ObjectPath);
	
	if (ExistingAsset && IsValid(ExistingAsset) && !ExistingAsset->IsUnreachable())
	{
		ExistingValidMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
		if (ExistingValidMIC && IsValid(ExistingValidMIC))
		{
			bAssetAlreadyExists = true;
		}
	}

	// Step 2: If not loaded, check registry and disk
	if (!bAssetAlreadyExists)
	{
		// Check asset registry
		if (UEditorAssetLibrary::DoesAssetExist(ObjectPath))
		{
			// Verify file exists on disk
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(
				BasePackageName, 
				PackageFilename, 
				FPackageName::GetAssetPackageExtension()))
			{
				if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*PackageFilename))
				{
					// File exists but object might be invalid - try loading again
					ExistingAsset = UEditorAssetLibrary::LoadAsset(ObjectPath);
					if (ExistingAsset && IsValid(ExistingAsset) && !ExistingAsset->IsUnreachable())
					{
						ExistingValidMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
						if (ExistingValidMIC && IsValid(ExistingValidMIC))
						{
							bAssetAlreadyExists = true;
						}
					}
				}
			}
		}
	}

	FMIForgeMaterialInstanceResolution Resolution;

	FString FinalAssetName = AssetName;
	FString FinalPackagePath = Options.TargetPath;

	if (bAssetAlreadyExists && ExistingValidMIC)
	{
		switch (Options.IfMIExists)
		{
		case EIfMIExistsOption::Skip:
			Resolution.Action = EMIForgeGenerationAction::Skipped;
			Resolution.Message = FText::FromString(
				FString::Printf(TEXT("Skipped existing MI: %s"), *ObjectPath)
			);
			return Resolution;

		case EIfMIExistsOption::Overwrite:
		{
			// Validate object is safe to modify
			if (!IsValid(ExistingValidMIC) || ExistingValidMIC->IsUnreachable())
			{
				Resolution.Action = EMIForgeGenerationAction::Failed;
				Resolution.Message = FText::FromString(
					FString::Printf(TEXT("Existing MI is invalid or pending deletion: %s"), *ObjectPath)
				);
				return Resolution;
			}

			Resolution.MaterialInstance = ExistingValidMIC;
			Resolution.Action = EMIForgeGenerationAction::Updated;
			return Resolution;
		}

		case EIfMIExistsOption::CreateUnique:
		{
			FString UniquePackageName;
			FString UniqueAssetName;

			AssetTools.CreateUniqueAssetName(
				BasePackageName,
				TEXT("_01"),
				UniquePackageName,
				UniqueAssetName
			);

			FinalAssetName = UniqueAssetName;
			FinalPackagePath = FPackageName::GetLongPackagePath(UniquePackageName);
			break;
		}
		}
	}

	// Create new asset
	UMaterialInstanceConstant* NewMIC =
		Cast<UMaterialInstanceConstant>(
			AssetTools.CreateAsset(
				FinalAssetName,
				FinalPackagePath,
				UMaterialInstanceConstant::StaticClass(),
				Factory
			)
		);

	if (!NewMIC)
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message = FText::FromString(
			FString::Printf(TEXT("Failed to create MI for: %s"), *TextureSet.SetName)
		);
		return Resolution;
	}

	Resolution.MaterialInstance = NewMIC;
	Resolution.Action = EMIForgeGenerationAction::Created;
	return Resolution;
}

bool FMIForgeMaterialInstanceGenerator::ApplyTexturesToMaterialInstance(UMaterialInstanceConstant* MaterialInstanceConstant, const FMIForgeTextureSet& TextureSet, const FMIForgeGenerationOptions& Options) const
{
	switch(Options.Preset)
	{
		case EMIForgeGenerationPreset::Standard:
			return ApplyTexturesToStandardMaterialInstance(MaterialInstanceConstant, TextureSet, Options);
		case EMIForgeGenerationPreset::RGBMask:
			return ApplyTexturesToRGBMaterialInstance(MaterialInstanceConstant, TextureSet, Options);
	}
	return false;
}

bool FMIForgeMaterialInstanceGenerator::ApplyTexturesToStandardMaterialInstance(UMaterialInstanceConstant* MaterialInstanceConstant, const FMIForgeTextureSet& TextureSet, const FMIForgeGenerationOptions& Options) const
{
	const FMIForgeTextureInfo* Albedo = TextureSet.Textures.Find(EMIForgeTextureType::Albedo);
	const FName* ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::Albedo);
	UTexture* Texture = Albedo && Albedo->AssetData.IsValid()
		? Cast<UTexture>(Albedo->AssetData.GetAsset())
		: nullptr;

	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("Albedo texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for Albedo."),
			ELogVerbosity::Error
		);
		return false;
	}
	
	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture);

	const FMIForgeTextureInfo* Normal = TextureSet.Textures.Find(EMIForgeTextureType::Normal);
	ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::Normal);
	Texture = Normal && Normal->AssetData.IsValid()
		? Cast<UTexture>(Normal->AssetData.GetAsset())
		: nullptr;
	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("Normal texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for Normal."),
			ELogVerbosity::Error
		);
		return false;
	}
	
	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture);

	const FMIForgeTextureInfo* ORM = TextureSet.Textures.Find(EMIForgeTextureType::ORM);
	ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::ORM);
	Texture = ORM && ORM->AssetData.IsValid()
		? Cast<UTexture>(ORM->AssetData.GetAsset())
		: nullptr;
	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("ORM texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for ORM."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture
	);

	bool bEmissiveApplied = false;

	if (Options.bUseEmissive)
	{
		const FMIForgeTextureInfo* Emissive =
			TextureSet.Textures.Find(EMIForgeTextureType::Emissive);

		const FName* TextureParameterName =
			Options.TextureParameterNames.Find(EMIForgeTextureType::Emissive);

		if (!TextureParameterName || TextureParameterName->IsNone())
		{
			MIForgeUtilities::PrintLog(
				TEXT("No material parameter mapping configured for Emissive."),
				ELogVerbosity::Error
			);
			return false;
		}

		Texture = Emissive && Emissive->AssetData.IsValid()
			? Cast<UTexture>(Emissive->AssetData.GetAsset())
			: nullptr;

		if (!Texture)
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("Emissive texture not found or invalid for set: %s, Generated without Emissive"), *TextureSet.SetName), ELogVerbosity::Warning
			);

			
		}
		else
		{
				UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
					MaterialInstanceConstant,
					*TextureParameterName,
					Texture
				);
				bEmissiveApplied = true;
		}
	}

	// This runs whether the user enabled Emissive or not.
	if (Options.EmissiveParameterName.IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No static bool parameter configured for Emissive."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstanceConstant,
		Options.EmissiveParameterName,
		bEmissiveApplied
	);
	
	bool bDetailNormalApplied = false;

	if (Options.bUseDetailNormal)
	{
		const FMIForgeTextureInfo* DetailNormal =
			TextureSet.Textures.Find(EMIForgeTextureType::DetailNormal);

		const FName* TextureParameterName =
			Options.TextureParameterNames.Find(EMIForgeTextureType::DetailNormal);

		if (!TextureParameterName || TextureParameterName->IsNone())
		{
			MIForgeUtilities::PrintLog(
				TEXT("No material parameter mapping configured for Detail Normal."),
				ELogVerbosity::Error
			);
			return false;
		}

		Texture = DetailNormal && DetailNormal->AssetData.IsValid()
			? Cast<UTexture>(DetailNormal->AssetData.GetAsset())
			: nullptr;

		if (!Texture)
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("DetailNormal texture not found or invalid for set: %s, Generated without DetailNormal"), *TextureSet.SetName), ELogVerbosity::Warning
			);


		}
		else
		{
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
				MaterialInstanceConstant,
				*TextureParameterName,
				Texture
				);
			bDetailNormalApplied = true;
		}
	}

	// This runs whether the user enabled DetailNormal or not.
	if (Options.DetailNormalParameterName.IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No static bool parameter configured for DetailNormal."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstanceConstant,
		Options.DetailNormalParameterName,
		bDetailNormalApplied
		);
	

	//Triplanar
	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstanceConstant,
		Options.TriplanarParameterName,
		Options.bUseTriplanar
	);

	return true;
}

bool FMIForgeMaterialInstanceGenerator::ApplyTexturesToRGBMaterialInstance(UMaterialInstanceConstant* MaterialInstanceConstant, const FMIForgeTextureSet& TextureSet, const FMIForgeGenerationOptions& Options) const
{
	const FMIForgeTextureInfo* Albedo = TextureSet.Textures.Find(EMIForgeTextureType::Albedo);
	const FName* ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::Albedo);
	UTexture* Texture = Albedo && Albedo->AssetData.IsValid()
		? Cast<UTexture>(Albedo->AssetData.GetAsset())
		: nullptr;

	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("Albedo texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for Albedo."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture);

	const FMIForgeTextureInfo* Normal = TextureSet.Textures.Find(EMIForgeTextureType::Normal);
	ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::Normal);
	Texture = Normal && Normal->AssetData.IsValid()
		? Cast<UTexture>(Normal->AssetData.GetAsset())
		: nullptr;
	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("Normal texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for Normal."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture);

	const FMIForgeTextureInfo* RGB = TextureSet.Textures.Find(EMIForgeTextureType::RGB);
	ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::RGB);
	Texture = RGB && RGB->AssetData.IsValid()
		? Cast<UTexture>(RGB->AssetData.GetAsset())
		: nullptr;

	if (!Texture)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(TEXT("RGB texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
		);
		return false;
	}
	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No material parameter mapping configured for RGB."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
		MaterialInstanceConstant,
		*ParameterName,
		Texture
	);

	bool bBaseORMApplied = false;

	if (Options.bUseBaseORMTexture)
	{
		const FMIForgeTextureInfo* ORM = TextureSet.Textures.Find(EMIForgeTextureType::ORM);
		ParameterName = Options.TextureParameterNames.Find(EMIForgeTextureType::ORM);
		Texture = ORM && ORM->AssetData.IsValid()
			? Cast<UTexture>(ORM->AssetData.GetAsset())
			: nullptr;
		if (!Texture)
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("ORM texture not found or invalid for set: %s"), *TextureSet.SetName), ELogVerbosity::Error
			);
			return false;
		}
		if (!ParameterName || ParameterName->IsNone())
		{
			MIForgeUtilities::PrintLog(
				TEXT("No material parameter mapping configured for ORM."),
				ELogVerbosity::Error
			);
			return false;
		}

		UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
							MaterialInstanceConstant,
							*ParameterName,
							Texture
		);

		bBaseORMApplied = true;
	}

	if (Options.BaseORMParameterName.IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No static bool parameter configured for Base ORM."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstanceConstant,
		Options.BaseORMParameterName,
		bBaseORMApplied
	);

	bool bDetailNormalApplied = false;

	if (Options.bUseDetailNormalTextureRGB)
	{
		const FMIForgeTextureInfo* DetailNormal =
			TextureSet.Textures.Find(EMIForgeTextureType::DetailNormal);

		const FName* TextureParameterName =
			Options.TextureParameterNames.Find(EMIForgeTextureType::DetailNormal);

		if (!TextureParameterName || TextureParameterName->IsNone())
		{
			MIForgeUtilities::PrintLog(
				TEXT("No material parameter mapping configured for Detail Normal."),
				ELogVerbosity::Error
			);
			return false;
		}

		Texture = DetailNormal && DetailNormal->AssetData.IsValid()
			? Cast<UTexture>(DetailNormal->AssetData.GetAsset())
			: nullptr;

		if (!Texture)
		{
			MIForgeUtilities::PrintLog(
				FString::Printf(TEXT("DetailNormal texture not found or invalid for set: %s, Generated without DetailNormal"), *TextureSet.SetName), ELogVerbosity::Warning
			);


		}
		else
		{
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(
				MaterialInstanceConstant,
				*TextureParameterName,
				Texture
			);

			bDetailNormalApplied = true;
		}
	}

	// This runs whether the user enabled DetailNormal or not.
	if (Options.DetailNormalParameterName.IsNone())
	{
		MIForgeUtilities::PrintLog(
			TEXT("No static bool parameter configured for DetailNormal."),
			ELogVerbosity::Error
		);
		return false;
	}

	UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
		MaterialInstanceConstant,
		Options.DetailNormalParameterName,
		bDetailNormalApplied
	);


	return true;
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
		[&Options, &ParentTextureParameterNames, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			const FName* ParameterName =
				Options.TextureParameterNames.Find(TextureType);

			if (!ParameterName || ParameterName->IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No texture parameter mapping is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentTextureParameterNames.Contains(*ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s texture parameter '%s'."),
						DisplayName,
						*ParameterName->ToString()
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
		[&ParentStaticSwitchNames, &OutError](
			const FName& ParameterName,
			const TCHAR* DisplayName
			) -> bool
		{
			if (ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No static bool parameter is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s static bool '%s'."),
						DisplayName,
						*ParameterName.ToString()
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
	if (!ValidateStaticSwitch(Options.EmissiveParameterName, TEXT("Emissive")) ||
		!ValidateStaticSwitch(Options.DetailNormalParameterName, TEXT("Detail Normal")) ||
		!ValidateStaticSwitch(Options.TriplanarParameterName, TEXT("Triplanar")))
	{
		return false;
	}

	return true;
}

bool FMIForgeMaterialInstanceGenerator::ValidateRGBGenerationInputs(const FMIForgeTextureSet& TextureSet, UMaterialInterface* ParentMaterial, const FMIForgeGenerationOptions& Options, FText& OutError) const
{
	OutError = FText::GetEmpty();

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
		[&Options, &ParentTextureParameterNames, &OutError](
			EMIForgeTextureType TextureType,
			const TCHAR* DisplayName
			) -> bool
		{
			const FName* ParameterName =
				Options.TextureParameterNames.Find(TextureType);

			if (!ParameterName || ParameterName->IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No texture parameter mapping is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentTextureParameterNames.Contains(*ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s texture parameter '%s'."),
						DisplayName,
						*ParameterName->ToString()
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
		[&ParentStaticSwitchNames, &OutError](
			const FName& ParameterName,
			const TCHAR* DisplayName
			) -> bool
		{
			if (ParameterName.IsNone())
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("No static bool parameter is configured for %s."),
						DisplayName
					)
				);
				return false;
			}

			if (!ParentStaticSwitchNames.Contains(ParameterName))
			{
				OutError = FText::FromString(
					FString::Printf(
						TEXT("Parent material does not contain the %s static bool '%s'."),
						DisplayName,
						*ParameterName.ToString()
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
	if (!ValidateStaticSwitch(Options.BaseORMParameterName, TEXT("Base ORM")) ||
		!ValidateStaticSwitch(Options.EmissiveChannelParameterName, TEXT("Emissive Channel")) ||
		!ValidateStaticSwitch(Options.DetailNormalParameterName, TEXT("Detail Normal"))
		)
	{
		return false;
	}

	return true;
}

FMIForgeGenerationResult FMIForgeMaterialInstanceGenerator::GenerateVertexPaintMaterialInstance(const FMIForgeVertexPaintLayerStack& LayerStack, const FMIForgeVertexPaintGenerationOptions& Options) const
{
	FMIForgeGenerationResult Result;
	FString ParentMaterialPath = Options.MaterialInstanceParentPath.ToString();

	UMaterialInterface* ParentMaterial = Cast<UMaterialInterface>(Options.MaterialInstanceParentPath.TryLoad());
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

	FMIForgeMaterialInstanceResolution Resolution;
	if (Options.MaterialInstanceName.IsEmpty())
	{
		Resolution =
			ResolveMaterialInstanceByName(LayerStack.BaseLayer.AssignedTextureSet->SetName, ParentMaterial, Options.TargetPath, Options.IfMIExists, AssetTools);
	}
	else
	{
		 Resolution =
			ResolveMaterialInstanceByName(Options.MaterialInstanceName, ParentMaterial, Options.TargetPath, Options.IfMIExists, AssetTools);
	}

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

	const bool bLayerGEnabled =
		LayerStack.LayerG.AssignedTextureSet.IsValid();

	const bool bLayerBEnabled =
		LayerStack.LayerB.AssignedTextureSet.IsValid();

	FText OutError;

	if (!ApplyVertexPaintLayerTextures(
		Resolution.MaterialInstance,
		LayerStack.BaseLayer,
		Options,
		bLayerGEnabled,
		bLayerBEnabled,
		OutError
	))
	{
		if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance))
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Resolution.MaterialInstance);

			ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

			Result.CreatedAssets.Remove(Resolution.MaterialInstance);
		}
		Result.Messages.Add(OutError);
		Result.FailedCount++;
		Result.Messages.Add(FText::FromString(
			FString::Printf(TEXT("Failed to apply textures to Layer: %s"), *LayerStack.BaseLayer.DisplayName)
		));
		return Result;
	}

	if (!ApplyVertexPaintLayerTextures(
		Resolution.MaterialInstance,
		LayerStack.LayerR,
		Options,
		bLayerGEnabled,
		bLayerBEnabled,
		OutError
	))
	{
		if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance))
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Resolution.MaterialInstance);

			ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

			Result.CreatedAssets.Remove(Resolution.MaterialInstance);
		}
		Result.Messages.Add(OutError);
		Result.FailedCount++;
		Result.Messages.Add(FText::FromString(
			FString::Printf(TEXT("Failed to apply textures to Layer: %s"), *LayerStack.LayerR.DisplayName)
		));
		return Result;
	}

	if (!ApplyVertexPaintLayerTextures(
		Resolution.MaterialInstance,
		LayerStack.LayerG,
		Options,
		bLayerGEnabled,
		bLayerBEnabled,
		OutError
	))
	{
		if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance))
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Resolution.MaterialInstance);

			ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

			Result.CreatedAssets.Remove(Resolution.MaterialInstance);
		}
		Result.Messages.Add(OutError);
		Result.FailedCount++;
		Result.Messages.Add(FText::FromString(
			FString::Printf(TEXT("Failed to apply textures to Layer: %s"), *LayerStack.LayerG.DisplayName)
		));
		return Result;
	}

	if (!ApplyVertexPaintLayerTextures(
		Resolution.MaterialInstance,
		LayerStack.LayerB,
		Options,
		bLayerGEnabled,
		bLayerBEnabled,
		OutError
	))
	{
		if (bWasCreated && Resolution.MaterialInstance && IsValid(Resolution.MaterialInstance))
		{
			TArray<UObject*> ObjectsToDelete;
			ObjectsToDelete.Add(Resolution.MaterialInstance);

			ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

			Result.CreatedAssets.Remove(Resolution.MaterialInstance);
		}
		Result.Messages.Add(OutError);
		Result.FailedCount++;
		Result.Messages.Add(FText::FromString(
			FString::Printf(TEXT("Failed to apply textures to Layer: %s"), *LayerStack.LayerB.DisplayName)
		));
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

FMIForgeMaterialInstanceResolution FMIForgeMaterialInstanceGenerator::ResolveMaterialInstanceByName(const FString& AssetName, UMaterialInterface* ParentMaterial, const FString& TargetPath, EIfMIExistsOption IfMIExists, IAssetTools& AssetTools) const
{
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();

	Factory->InitialParent = ParentMaterial;

	const FString FinalBaseAssetName =
		AssetName.StartsWith(TEXT("MI_VP_")) || AssetName.StartsWith(TEXT("MI_"))
		? AssetName
		: FString::Printf(TEXT("MI_VP_%s"), *AssetName);

	const FString BasePackageName = FString::Printf(
		TEXT("%s/%s"),
		*TargetPath,
		*FinalBaseAssetName
	);

	const FString ObjectPath = FString::Printf(
		TEXT("%s.%s"),
		*BasePackageName,
		*FinalBaseAssetName
	);

	bool bAssetAlreadyExists = false;
	UMaterialInstanceConstant* ExistingValidMIC = nullptr;

	// load the asset if it exists
	UObject* ExistingAsset = UEditorAssetLibrary::LoadAsset(ObjectPath);

	if (ExistingAsset && IsValid(ExistingAsset) && !ExistingAsset->IsUnreachable())
	{
		ExistingValidMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
		if (ExistingValidMIC && IsValid(ExistingValidMIC))
		{
			bAssetAlreadyExists = true;
		}
	}

	// If not loaded, check registry and disk
	if (!bAssetAlreadyExists)
	{
		//Check asset registry
		if (UEditorAssetLibrary::DoesAssetExist(ObjectPath))
		{
			//verify file exists on disk
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(BasePackageName, PackageFilename, FPackageName::GetAssetPackageExtension()))
			{
				if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*PackageFilename))
				{
					//File exists but object might be invalid - try to load again
					ExistingAsset = UEditorAssetLibrary::LoadAsset(ObjectPath);
					if (ExistingAsset && IsValid(ExistingAsset) && !ExistingAsset->IsUnreachable())
					{
						ExistingValidMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
						if (ExistingValidMIC && IsValid(ExistingValidMIC))
						{
							bAssetAlreadyExists = true;
						}
					}
				}
			}
		}
	}

	FMIForgeMaterialInstanceResolution Resolution;

	FString FinalAssetName = FinalBaseAssetName;
	FString FinalPackagePath = TargetPath;

	if (bAssetAlreadyExists && ExistingAsset)
	{
		switch (IfMIExists)
		{
			case EIfMIExistsOption::Skip:
			{
				Resolution.Action = EMIForgeGenerationAction::Skipped;
				Resolution.Message = FText::FromString(FString::Printf(TEXT("Skipped existing MI: %s"), *ObjectPath)
				);
				return Resolution;
			}
			case EIfMIExistsOption::Overwrite:
			{
				// Validate object is safe to modify
				if (!IsValid(ExistingValidMIC) || ExistingValidMIC->IsUnreachable())
				{
					Resolution.Action = EMIForgeGenerationAction::Failed;
					Resolution.Message = FText::FromString(FString::Printf(TEXT("Existing MI is invalid or pending deletion: %s"), *ObjectPath));
					return Resolution;
				}

				Resolution.MaterialInstance = ExistingValidMIC;
				Resolution.Action = EMIForgeGenerationAction::Updated;
				return Resolution;

			}
			case EIfMIExistsOption::CreateUnique:
			{
				FString UniquePackageName;
				FString UniqueAssetName;

				AssetTools.CreateUniqueAssetName(
					BasePackageName,
					TEXT("_01"),
					UniquePackageName,
					UniqueAssetName
				);

				FinalAssetName = UniqueAssetName;
				FinalPackagePath = FPackageName::GetLongPackagePath(UniquePackageName);
				break;

			}
		}
	}

	// Create new MIC
	UMaterialInstanceConstant* NewMIC =
		Cast<UMaterialInstanceConstant>(
			AssetTools.CreateAsset(
				FinalAssetName,
				FinalPackagePath,
				UMaterialInstanceConstant::StaticClass(),
				Factory
			)
		);
	if (!NewMIC)
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message = FText::FromString(
			FString::Printf(TEXT("Failed to create MI for: %s"), *AssetName)
		);
		return Resolution;
	}

	Resolution.MaterialInstance = NewMIC;
	Resolution.Action = EMIForgeGenerationAction::Created;
	return Resolution;
}

bool FMIForgeMaterialInstanceGenerator::ApplyVertexPaintLayerTextures(UMaterialInstanceConstant* MaterialInstanceConstant, const FMIForgeVertexPaintLayerSlot& LayerSlot, const FMIForgeVertexPaintGenerationOptions& Options, bool bLayerGEnabled,
	bool bLayerBEnabled, FText& OutError) const
{
	if (!MaterialInstanceConstant)
	{
		OutError = FText::FromString(TEXT("Material instance is invalid."));
		return false;
	}

	const bool bIsOptionalLayer =
		LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerG ||
		LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerB;

	const bool bLayerAssigned = LayerSlot.AssignedTextureSet.IsValid();

	// Optional empty layers should not block generation.
	if (bIsOptionalLayer && !bLayerAssigned)
	{
		if (const FName* EnabledParameterName =
			Options.LayerEnabledParameterNames.Find(LayerSlot.Layer))
		{
			UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(
				MaterialInstanceConstant,
				*EnabledParameterName,
				false
			);
		}

		return true;
	}

	// Required layers must have Albedo.
	if (!ApplyVertexPaintTexture(
		MaterialInstanceConstant,
		LayerSlot,
		Options,
		EMIForgeTextureType::Albedo,
		TEXT("Albedo"),
		true
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

	// Optional textures. Missing ones warn and continue.
	ApplyVertexPaintTexture(
		MaterialInstanceConstant,
		LayerSlot,
		Options,
		EMIForgeTextureType::Normal,
		TEXT("Normal"),
		false
	);

	ApplyVertexPaintTexture(
		MaterialInstanceConstant,
		LayerSlot,
		Options,
		EMIForgeTextureType::ORM,
		TEXT("ORM"),
		false
	);

	const bool bShouldApplyHeight =
		(LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerR && bLayerGEnabled) ||
		(LayerSlot.Layer == EMIForgeVertexPaintLayer::LayerG && bLayerBEnabled);

	if(LayerSlot.Layer == EMIForgeVertexPaintLayer::Base)
	{
		ApplyVertexPaintTexture(
			MaterialInstanceConstant,
			LayerSlot,
			Options,
			EMIForgeTextureType::Height,
			TEXT("Height"),
			false
		);
	}
	else if (bShouldApplyHeight)
	{
		ApplyVertexPaintTexture(
			MaterialInstanceConstant,
			LayerSlot,
			Options,
			EMIForgeTextureType::Height,
			TEXT("Height"),
			false
		);
	}

	// Enable optional layer switches deterministically.
	if (bIsOptionalLayer)
	{
		const FName* EnabledParameterName =
			Options.LayerEnabledParameterNames.Find(LayerSlot.Layer);

		if (!EnabledParameterName || EnabledParameterName->IsNone())
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
			MaterialInstanceConstant,
			*EnabledParameterName,
			true
		);
	}


	return true;
}

bool FMIForgeMaterialInstanceGenerator::ApplyVertexPaintTexture(UMaterialInstanceConstant* MaterialInstanceConstant, const FMIForgeVertexPaintLayerSlot& LayerSlot, const FMIForgeVertexPaintGenerationOptions& Options, EMIForgeTextureType TextureType, const TCHAR* DisplayName, bool bRequired)const
{	
	if (!MaterialInstanceConstant)
	{
		MIForgeUtilities::PrintLog(
			TEXT("Material instance is invalid."),
			ELogVerbosity::Error
		);
		return false;
	}

	if (!LayerSlot.AssignedTextureSet.IsValid())
	{
		const ELogVerbosity::Type Verbosity =
			bRequired ? ELogVerbosity::Error : ELogVerbosity::Warning;

		MIForgeUtilities::PrintLog(
			FString::Printf(
				TEXT("%s has no assigned texture set."),
				*LayerSlot.DisplayName
			),
			Verbosity
		);

		return !bRequired;
	}

	const FMIForgeTextureSet& TextureSet = *LayerSlot.AssignedTextureSet;

	const TMap<EMIForgeTextureType, FName>* ParameterNameMap =
		Options.LayerTextureParameterNames.Find(LayerSlot.Layer);

	if (!ParameterNameMap)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(
				TEXT("No texture parameter mapping configured for %s."),
				*LayerSlot.DisplayName
			),
			ELogVerbosity::Error
		);
		return false;
	}

	const FName* ParameterName = ParameterNameMap->Find(TextureType);

	if (!ParameterName || ParameterName->IsNone())
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(
				TEXT("No material parameter mapping configured for %s %s."),
				*LayerSlot.DisplayName,
				DisplayName
			),
			ELogVerbosity::Error
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
		MaterialInstanceConstant,
		*ParameterName,
		Texture
	);

	/*if (!bApplied)
	{
		MIForgeUtilities::PrintLog(
			FString::Printf(
				TEXT("Failed to assign %s texture parameter '%s' for %s."),
				DisplayName,
				*ParameterName->ToString(),
				*LayerSlot.DisplayName
			),
			ELogVerbosity::Error
		);

		return false;
	}*/

	return true;

}

