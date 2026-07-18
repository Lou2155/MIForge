// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"



class FMIForgeMaterialInstanceGenerator
{
public:
	FMIForgeGenerationResult GenerateMaterialInstances(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, const FMIForgeGenerationOptions& Options) const;

	FMIForgeGenerationResult GenerateVertexPaintMaterialInstance(
		const FMIForgeVertexPaintLayerStack& LayerStack,
		const FMIForgeVertexPaintGenerationOptions& Options
	) const;

};


