// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"

enum class EMIForgeTextureType : uint8;
struct FMIForgeTextureInfo;


class FMIForgeTextureClassifier
{
public:
	FMIForgeTextureInfo ClassifyTexture(const FAssetData& AssetData) const;

	TArray<FMIForgeTextureInfo> ClassifyTextures(const TArray<FAssetData>& Assets) const;

	EMIForgeTextureType TextureTypeFromName(const FString& TypeName) const;

private:
	void FillTextureSize(const FAssetData& AssetData, FMIForgeTextureInfo& Info) const;
};