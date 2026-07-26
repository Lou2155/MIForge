// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"
class MIForgeSelectionCollector
{
public:
	static FMIForgeMaterialSlotCollectionResult CollectFromCurrentEditorSelection();

	static UMaterialInterface* ResolveRootParentMaterial(UMaterialInstanceConstant* MaterialInstance);
};
