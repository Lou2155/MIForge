// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeTextureSetBuilder.h"
#include "MIForgeTypes.h"

TArray<FMIForgeTextureSet> FMIForgeTextureSetBuilder::BuildTextureSets(const TArray<FMIForgeTextureInfo>& TexInfos) const
{
	TMap<FString, FMIForgeTextureSet> TextureSetsByName;

	for (const FMIForgeTextureInfo& TexInfo : TexInfos)
	{
        if (TexInfo.TextureType == EMIForgeTextureType::Unknown)
        {   
          
            continue;
        }

        if (TexInfo.BaseName.IsEmpty())
        {
            continue;
        }

        FMIForgeTextureSet& TextureSet = TextureSetsByName.FindOrAdd(TexInfo.BaseName);
        TextureSet.SetName = TexInfo.BaseName;

        TextureSet.Textures.Add(TexInfo.TextureType, TexInfo);
	}

	TArray<FMIForgeTextureSet> Result;
	TextureSetsByName.GenerateValueArray(Result);
	return Result;
}