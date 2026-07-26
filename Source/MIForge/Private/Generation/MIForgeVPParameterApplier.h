// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

class UMaterialInstanceConstant;

class FMIForgeVPParameterApplier
{
public:
    bool Apply(
        UMaterialInstanceConstant* MaterialInstance,
        const FMIForgeVertexPaintLayerStack& LayerStack,
        FText& OutError) const;

private:
    bool ApplyLayer(
        UMaterialInstanceConstant* MaterialInstance,
        const FMIForgeVertexPaintLayerSlot& LayerSlot,
        bool bLayerGEnabled,
        bool bLayerBEnabled,
        FText& OutError) const;

    bool ApplyTexture(
        UMaterialInstanceConstant* MaterialInstance,
        const FMIForgeVertexPaintLayerSlot& LayerSlot,
        EMIForgeTextureType TextureType,
        const TCHAR* DisplayName,
        bool bRequired,
        FText& OutError) const;
};
