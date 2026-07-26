// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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
