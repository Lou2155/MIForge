// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
struct FMIForgeTextureInfo;
struct FMIForgeTextureSet;

class FMIForgeTextureSetBuilder
{
public:
    TArray<FMIForgeTextureSet> BuildTextureSets(const TArray<FMIForgeTextureInfo>& TexInfos) const;
};
