// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
struct FMIForgeTextureInfo;
struct FMIForgeTextureSet;

class FMIForgeTextureSetBuilder
{
public:
    TArray<FMIForgeTextureSet> BuildTextureSets(const TArray<FMIForgeTextureInfo>& TexInfos) const;
};
