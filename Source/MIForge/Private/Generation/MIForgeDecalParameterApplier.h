// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

class UMaterialInstanceConstant;

class FMIForgeDecalParameterApplier
{
public:
    bool Apply(
        UMaterialInstanceConstant* MaterialInstance,
        const FMIForgeTextureSet& TextureSet,
        const FMIForgeGenerationOptions& Options,
        FText& OutError) const;
};
