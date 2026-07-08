// Fill out your copyright notice in the Description page of Project Settings.


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