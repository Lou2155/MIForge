// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"
class MIForgeSelectionCollector
{
public:
	static FMIForgeMaterialSlotCollectionResult CollectFromCurrentEditorSelection();

	static UMaterialInterface* ResolveRootParentMaterial(UMaterialInstanceConstant* MaterialInstance);
};
