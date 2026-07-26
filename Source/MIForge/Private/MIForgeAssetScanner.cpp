// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeAssetScanner.h"
#include "EditorAssetLibrary.h"

TArray<FAssetData> FMIForgeAssetScanner::FindTexturesInFolders(const TArray<FString>& FolderPaths) const
{
    TArray<FAssetData> TextureAssets;

    for (const FString& FolderPath : FolderPaths)
    {
        TArray<FString> AssetPaths = UEditorAssetLibrary::ListAssets(FolderPath);

        TextureAssets.Reserve(TextureAssets.Num() + AssetPaths.Num());

        for (const FString& AssetPath : AssetPaths)
        {
            FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPath);

            if (!AssetData.IsValid())
            {
                continue;
            }

            UClass* AssetClass = AssetData.GetClass();
            if (AssetClass == nullptr || !AssetClass->IsChildOf(UTexture::StaticClass()))
            {
                continue;
            }

            TextureAssets.Add(MoveTemp(AssetData));
        }
    }

    return TextureAssets;
}
