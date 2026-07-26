// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FOnPathSelected, const FString&)

class MIForgeUtilities
{
public:
	static TArray<TSharedPtr<FAssetData>> GetTexturesFromSelectedFolders(const TArray<FString>& FoldersPath);

	static void CreatePathSelector(TSharedRef<SWidget> ParentWidget, FOnPathSelected OnPathSelected);  

	static TArray<TSharedPtr<FString>> GetFilterOptions();

	static TArray<TSharedPtr<FAssetData>> FilterTextures(const TArray<TSharedPtr<FAssetData>>& AllAssetsData, const TSharedPtr<FString>& TextureType);

	static void PrintDebug(const FString& message, const FColor& color = FColor::Cyan, const float& duration = 3);
	static void PrintLog(const FString& message, const ELogVerbosity::Type& type = ELogVerbosity::Log);
	static EAppReturnType::Type PrintWindow(const FString& message, const EAppMsgType::Type& msgType = EAppMsgType::YesNo);
	static void PrintNotification(const FString& message, const float& duration = 3);

	static void ActivateContentBrowserTabAndSyncToAssets(const TArray<FAssetData>& AssetDatas);

	static void FixTextureCompressionInSelectedFolders(TArray<FString> SelectedFolderPaths);

	static TSharedRef<SWidget> WithRowSeparator(TSharedRef<SWidget> Content);

	static void SafelyDeleteAssets(UObject* ObjectToDelete, bool bWasCreated, TArray<UObject*> CreatedAssets);

	static void CloseOpenAssetEditors(const TArray<UObject*>& ObjectsToDelete);

private:
	//TextureCompressionSettings GetExpectedCompression(EMIForgeTextureType Type);
};
