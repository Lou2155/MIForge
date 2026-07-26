// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "Generation/MIForgeMaterialInstanceResolver.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "EditorAssetLibrary.h"
#include "Materials/MaterialInstanceConstant.h" 

FMIForgeMaterialInstanceResolution FMIForgeMaterialInstanceResolver::Resolve(const FMIForgeMaterialInstanceTarget& Target, IAssetTools& AssetTools) const
{
	FMIForgeMaterialInstanceResolution Resolution;

	if (Target.AssetName.IsEmpty())
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message =
			FText::FromString(TEXT("Material instance name is empty."));
		return Resolution;
	}

	if (Target.TargetPath.IsEmpty())
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message =
			FText::FromString(TEXT("Target path is empty."));
		return Resolution;
	}

	if (!IsValid(Target.ParentMaterial))
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message =
			FText::FromString(TEXT("Parent material is invalid."));
		return Resolution;
	}

	FString FinalAssetName = Target.AssetName;
	FString FinalPackagePath = Target.TargetPath;

	const FString BasePackageName = FString::Printf(
		TEXT("%s/%s"),
		*Target.TargetPath,
		*Target.AssetName);

	const FString ObjectPath = FString::Printf(
		TEXT("%s.%s"),
		*BasePackageName,
		*Target.AssetName);

	bool bAssetAlreadyExists = false;
	UMaterialInstanceConstant* ExistingMIC = nullptr;

	if (UEditorAssetLibrary::DoesAssetExist(ObjectPath))
	{
		UObject* ExistingAsset =
			UEditorAssetLibrary::LoadAsset(ObjectPath);

		ExistingMIC =
			Cast<UMaterialInstanceConstant>(ExistingAsset);

		if (ExistingMIC && IsValid(ExistingMIC))
		{
			bAssetAlreadyExists = true;

			switch (Target.IfMIExists)
			{
			case EIfMIExistsOption::Skip:
				Resolution.Action =
					EMIForgeGenerationAction::Skipped;
				Resolution.Message = FText::FromString(
					FString::Printf(
						TEXT("Skipped existing MI: %s"),
						*ObjectPath));
				return Resolution;

			case EIfMIExistsOption::Overwrite:
				Resolution.MaterialInstance = ExistingMIC;
				Resolution.Action =
					EMIForgeGenerationAction::Updated;
				return Resolution;

			case EIfMIExistsOption::CreateUnique:
			{
				FString UniquePackageName;
				FString UniqueAssetName;

				AssetTools.CreateUniqueAssetName(
					BasePackageName,
					TEXT("_01"),
					UniquePackageName,
					UniqueAssetName);

				FinalAssetName = UniqueAssetName;
				FinalPackagePath =
					FPackageName::GetLongPackagePath(
						UniquePackageName);
				break;
			}
			}
		}

		// Step 2: If not loaded, check registry and disk
		if (!bAssetAlreadyExists)
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
							ExistingMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
							if (ExistingMIC && IsValid(ExistingMIC))
							{
								bAssetAlreadyExists = true;

								switch (Target.IfMIExists)
								{
								case EIfMIExistsOption::Skip:
									Resolution.Action =
										EMIForgeGenerationAction::Skipped;
									Resolution.Message = FText::FromString(
										FString::Printf(
											TEXT("Skipped existing MI: %s"),
											*ObjectPath));
									return Resolution;

								case EIfMIExistsOption::Overwrite:
									Resolution.MaterialInstance = ExistingMIC;
									Resolution.Action =
										EMIForgeGenerationAction::Updated;
									return Resolution;

								case EIfMIExistsOption::CreateUnique:
								{
									FString UniquePackageName;
									FString UniqueAssetName;

									AssetTools.CreateUniqueAssetName(
										BasePackageName,
										TEXT("_01"),
										UniquePackageName,
										UniqueAssetName);

									FinalAssetName = UniqueAssetName;
									FinalPackagePath =
										FPackageName::GetLongPackagePath(
											UniquePackageName);
									break;
								}
								}
							}
							else
							{
								Resolution.Action = EMIForgeGenerationAction::Failed;
								Resolution.Message = FText::FromString(
									FString::Printf(TEXT("Existing asset is not a valid UMaterialInstanceConstant: %s"), *ObjectPath)
								);
								return Resolution;
							}
						}
					}
				}
			
		}

		
	}

	

	UMaterialInstanceConstantFactoryNew* Factory =
		NewObject<UMaterialInstanceConstantFactoryNew>();

	Factory->InitialParent = Target.ParentMaterial;

	UMaterialInstanceConstant* NewMIC =
		Cast<UMaterialInstanceConstant>(
			AssetTools.CreateAsset(
				FinalAssetName,
				FinalPackagePath,
				UMaterialInstanceConstant::StaticClass(),
				Factory));

	if (!NewMIC)
	{
		Resolution.Action =
			EMIForgeGenerationAction::Failed;
		Resolution.Message = FText::FromString(
			FString::Printf(
				TEXT("Failed to create material instance: %s"),
				*FinalAssetName));
		return Resolution;
	}

	Resolution.MaterialInstance = NewMIC;
	Resolution.Action = EMIForgeGenerationAction::Created;
	return Resolution;
}
