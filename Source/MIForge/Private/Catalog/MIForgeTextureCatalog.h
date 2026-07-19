// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "MIForgeTypes.h"

struct FAssetData;

DECLARE_MULTICAST_DELEGATE(FMIForgeOnTextureCatalogRefreshStarted);
DECLARE_MULTICAST_DELEGATE(FMIForgeOnTextureCatalogChanged);

class FMIForgeTextureCatalog
	: public TSharedFromThis<FMIForgeTextureCatalog>
{
public:
	~FMIForgeTextureCatalog();

	void Initialize(const TArray<FString>& InFolderPaths);
	void Shutdown();

	void SetFolderPaths(const TArray<FString>& InFolderPaths);
	const TArray<FString>& GetFolderPaths() const;

	void Refresh();
	void QueueRefresh();

	const TArray<TSharedPtr<FMIForgeTextureInfo>>& GetTextures() const;
	const TArray<TSharedPtr<FMIForgeTextureSet>>& GetTextureSets() const;

	TSharedPtr<FMIForgeTextureInfo> FindTextureListItem(
		const FMIForgeTextureInfo& TextureInfo) const;

	FMIForgeOnTextureCatalogRefreshStarted OnRefreshStarted;
	FMIForgeOnTextureCatalogChanged OnCatalogChanged;

private:
	void BindAssetRegistry();
	void UnbindAssetRegistry();
	void HandleAssetChanged(const FAssetData& AssetData);
	bool IsAssetRelevant(const FAssetData& AssetData) const;

	TArray<FString> FolderPaths;
	TArray<TSharedPtr<FMIForgeTextureInfo>> TextureListItems;
	TArray<TSharedPtr<FMIForgeTextureSet>> TextureSetListItems;

	FDelegateHandle AddAssetHandle;
	FDelegateHandle RemoveAssetHandle;

	bool bRefreshQueued = false;
	FTSTicker::FDelegateHandle RefreshTickerHandle;
};
