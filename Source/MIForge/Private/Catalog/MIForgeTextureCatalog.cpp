// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.


#include "Catalog/MIForgeTextureCatalog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "MIForgeAssetScanner.h"
#include "MIForgeTextureClassifier.h"
#include "MIForgeTextureSetBuilder.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"

FMIForgeTextureCatalog::~FMIForgeTextureCatalog()
{
	Shutdown();
}

void FMIForgeTextureCatalog::Initialize(
	const TArray<FString>& InFolderPaths)
{
	FolderPaths = InFolderPaths;
	Refresh();
	BindAssetRegistry();
}

void FMIForgeTextureCatalog::Shutdown()
{
	UnbindAssetRegistry();

	if (RefreshTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RefreshTickerHandle);
		RefreshTickerHandle.Reset();
	}

	bRefreshQueued = false;
}

void FMIForgeTextureCatalog::SetFolderPaths(
	const TArray<FString>& InFolderPaths)
{
	FolderPaths = InFolderPaths;
}

const TArray<FString>& FMIForgeTextureCatalog::GetFolderPaths() const
{
	return FolderPaths;
}

void FMIForgeTextureCatalog::Refresh()
{
	OnRefreshStarted.Broadcast();

	FMIForgeAssetScanner Scanner;
	FMIForgeTextureClassifier Classifier;
	FMIForgeTextureSetBuilder SetBuilder;

	TArray<FAssetData> FoundTextures =
		Scanner.FindTexturesInFolders(FolderPaths);
	TArray<FMIForgeTextureInfo> ClassifiedTextureInfos =
		Classifier.ClassifyTextures(FoundTextures);
	TArray<FMIForgeTextureSet> TextureSets =
		SetBuilder.BuildTextureSets(ClassifiedTextureInfos);

	TextureListItems.Empty();
	TextureListItems.Reserve(ClassifiedTextureInfos.Num());
	for (FMIForgeTextureInfo& TextureInfo : ClassifiedTextureInfos)
	{
		TextureListItems.Add(
			MakeShared<FMIForgeTextureInfo>(MoveTemp(TextureInfo)));
	}

	TextureSetListItems.Empty();
	TextureSetListItems.Reserve(TextureSets.Num());
	for (FMIForgeTextureSet& TextureSet : TextureSets)
	{
		TextureSetListItems.Add(
			MakeShared<FMIForgeTextureSet>(MoveTemp(TextureSet)));
	}

	OnCatalogChanged.Broadcast();
}

void FMIForgeTextureCatalog::QueueRefresh()
{
	check(IsInGameThread());

	if (bRefreshQueued)
	{
		return;
	}

	bRefreshQueued = true;
	TWeakPtr<FMIForgeTextureCatalog> WeakCatalog = AsShared();

	RefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakCatalog](float)
			{
				if (const TSharedPtr<FMIForgeTextureCatalog> Catalog =
					WeakCatalog.Pin())
				{
					Catalog->bRefreshQueued = false;
					Catalog->RefreshTickerHandle.Reset();
					Catalog->Refresh();
				}

				return false;
			}),
		0.0f);
}

const TArray<TSharedPtr<FMIForgeTextureInfo>>&
FMIForgeTextureCatalog::GetTextures() const
{
	return TextureListItems;
}

const TArray<TSharedPtr<FMIForgeTextureSet>>&
FMIForgeTextureCatalog::GetTextureSets() const
{
	return TextureSetListItems;
}

TSharedPtr<FMIForgeTextureInfo>
FMIForgeTextureCatalog::FindTextureListItem(
	const FMIForgeTextureInfo& TextureInfo) const
{
	const FString TargetPath = TextureInfo.ObjectPath;

	for (const TSharedPtr<FMIForgeTextureInfo>& Item : TextureListItems)
	{
		if (Item.IsValid() && Item->ObjectPath == TargetPath)
		{
			return Item;
		}
	}

	return nullptr;
}

void FMIForgeTextureCatalog::BindAssetRegistry()
{
	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
			FName("AssetRegistry"));

	TWeakPtr<FMIForgeTextureCatalog> WeakCatalog = AsShared();

	AddAssetHandle = AssetRegistry.Get().OnAssetAdded().AddLambda(
		[WeakCatalog](const FAssetData& AssetData)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakCatalog, AssetData]()
				{
					if (const TSharedPtr<FMIForgeTextureCatalog> Catalog =
						WeakCatalog.Pin())
					{
						Catalog->HandleAssetChanged(AssetData);
					}
				});
		});

	RemoveAssetHandle = AssetRegistry.Get().OnAssetRemoved().AddLambda(
		[WeakCatalog](const FAssetData& AssetData)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakCatalog, AssetData]()
				{
					if (const TSharedPtr<FMIForgeTextureCatalog> Catalog =
						WeakCatalog.Pin())
					{
						Catalog->HandleAssetChanged(AssetData);
					}
				});
		});
}

void FMIForgeTextureCatalog::UnbindAssetRegistry()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		FAssetRegistryModule& AssetRegistry =
			FModuleManager::GetModuleChecked<FAssetRegistryModule>(
				"AssetRegistry");

		if (AddAssetHandle.IsValid())
		{
			AssetRegistry.Get().OnAssetAdded().Remove(AddAssetHandle);
			AddAssetHandle.Reset();
		}

		if (RemoveAssetHandle.IsValid())
		{
			AssetRegistry.Get().OnAssetRemoved().Remove(RemoveAssetHandle);
			RemoveAssetHandle.Reset();
		}
	}
	else
	{
		AddAssetHandle.Reset();
		RemoveAssetHandle.Reset();
	}
}

void FMIForgeTextureCatalog::HandleAssetChanged(
	const FAssetData& AssetData)
{
	if (IsAssetRelevant(AssetData))
	{
		QueueRefresh();
	}
}

bool FMIForgeTextureCatalog::IsAssetRelevant(
	const FAssetData& AssetData) const
{
	const FString AssetPackageFolder =
		FPackageName::GetLongPackagePath(AssetData.PackageName.ToString());

	for (const FString& Folder : FolderPaths)
	{
		if (!Folder.IsEmpty() &&
			(AssetPackageFolder == Folder ||
				AssetPackageFolder.StartsWith(Folder + TEXT("/"))))
		{
			return true;
		}
	}

	return false;
}

