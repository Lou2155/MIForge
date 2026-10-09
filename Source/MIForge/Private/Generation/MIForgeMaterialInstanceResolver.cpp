// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "Generation/MIForgeMaterialInstanceResolver.h"

#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/PackageName.h"

FMIForgeMaterialInstanceResolution FMIForgeMaterialInstanceResolver::Resolve(const FMIForgeMaterialInstanceTarget& Target, IAssetTools& AssetTools) const
{
	FMIForgeMaterialInstanceResolution Resolution;
	auto Fail = [&Resolution](const FString& Message)
	{
		Resolution.Action = EMIForgeGenerationAction::Failed;
		Resolution.Message = FText::FromString(Message);
		return Resolution;
	};

	if (Target.AssetName.IsEmpty())
	{
		return Fail(TEXT("Material instance name is empty."));
	}
	if (Target.TargetPath.IsEmpty())
	{
		return Fail(TEXT("Target path is empty."));
	}
	if (!IsValid(Target.ParentMaterial))
	{
		return Fail(TEXT("Parent material is invalid."));
	}

	FString FinalAssetName = Target.AssetName;
	FString FinalPackagePath = Target.TargetPath;
	const FString BasePackageName = Target.TargetPath / Target.AssetName;
	const FString ObjectPath = BasePackageName + TEXT(".") + Target.AssetName;

	// Registry removal does not mean that the UObject name or package file is free.
	UObject* ExistingAsset = StaticFindObject(UObject::StaticClass(), nullptr, *ObjectPath);
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	const FAssetData AssetData = Registry.GetAssetByObjectPath(FSoftObjectPath(ObjectPath));
	const bool bPackageOnDisk = FPackageName::DoesPackageExist(BasePackageName);
	const bool bExists = ExistingAsset || AssetData.IsValid() || bPackageOnDisk;

	if (bExists && Target.IfMIExists == EIfMIExistsOption::CreateUnique)
	{
		FString UniquePackageName;
		AssetTools.CreateUniqueAssetName(BasePackageName, TEXT("_01"), UniquePackageName, FinalAssetName);
		FinalPackagePath = FPackageName::GetLongPackagePath(UniquePackageName);
	}
	else if (bExists)
	{
		if (!ExistingAsset && AssetData.IsValid())
		{
			ExistingAsset = AssetData.GetAsset();
		}
		if (!ExistingAsset && bPackageOnDisk)
		{
			ExistingAsset = LoadObject<UObject>(nullptr, *ObjectPath);
		}
		UMaterialInstanceConstant* ExistingMIC = Cast<UMaterialInstanceConstant>(ExistingAsset);
		if (!IsValid(ExistingMIC) || !ExistingMIC->IsAsset())
		{
			return Fail(FString::Printf(
				TEXT("The target name is occupied by an invalid or non-material asset: %s. Creation was stopped to preserve undo history."),
				*ObjectPath));
		}

		switch (Target.IfMIExists)
		{
		case EIfMIExistsOption::Skip:
			Resolution.Action = EMIForgeGenerationAction::Skipped;
			Resolution.Message = FText::FromString(FString::Printf(TEXT("Skipped existing MI: %s"), *ObjectPath));
			return Resolution;

		case EIfMIExistsOption::Overwrite:
			Resolution.MaterialInstance = ExistingMIC;
			Resolution.Action = EMIForgeGenerationAction::Updated;
			return Resolution;

		default:
			return Fail(TEXT("Unsupported existing material instance policy."));
		}
	}

	const FString FinalPackageName = FinalPackagePath / FinalAssetName;
	const FString FinalObjectPath = FinalPackageName + TEXT(".") + FinalAssetName;
	if (StaticFindObject(UObject::StaticClass(), nullptr, *FinalObjectPath) ||
		FPackageName::DoesPackageExist(FinalPackageName) ||
		Registry.GetAssetByObjectPath(FSoftObjectPath(FinalObjectPath)).IsValid())
	{
		return Fail(FString::Printf(TEXT("Material instance target is still occupied: %s"), *FinalObjectPath));
	}

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = Target.ParentMaterial;
	UMaterialInstanceConstant* NewMIC = nullptr;
	{
		// MIForge's undo record owns creation. UE's automatic creation record would
		// mark this MI as garbage on undo, before we can park it for redo.
		TGuardValue<ITransaction*> CreationTransactionGuard(GUndo, nullptr);
		NewMIC = Cast<UMaterialInstanceConstant>(
			AssetTools.CreateAsset(FinalAssetName, FinalPackagePath, UMaterialInstanceConstant::StaticClass(), Factory));
	}
	if (!NewMIC)
	{
		return Fail(FString::Printf(TEXT("Failed to create material instance: %s"), *FinalAssetName));
	}
	Resolution.MaterialInstance = NewMIC;
	Resolution.Action = EMIForgeGenerationAction::Created;
	return Resolution;
}
