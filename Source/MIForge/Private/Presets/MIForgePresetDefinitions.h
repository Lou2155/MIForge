// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

enum class EMIForgePresetOptions : uint8
{
	None,
	UseEmissiveTexture,
	UseDetailNormalTexture,
	UseTriplanar,
	UseBaseORM,
	EnableEmissiveChannel,
	UseDecalNormal,
	UseDecalORM,
	UseOrientationMask
};

enum class EMIForgeRequirement : uint8
{
	Required,
	Optional
};

struct FMIForgeTextureBinding
{
	EMIForgeTextureType TextureType;
	FName ParameterName;
	EMIForgeRequirement Requirement;
	EMIForgePresetOptions PresetOption = EMIForgePresetOptions::None;

};

struct FMIForgeStaticSwitchBinding
{
	FName ParameterName;
	EMIForgePresetOptions PresetOption;
};

struct FMIForgeMaterialPresetDefinition
{
	EMIForgeGenerationPreset Preset;
	FSoftObjectPath ParentMaterialPath;

	TArray<FMIForgeTextureBinding> TextureBindings;
	TArray<FMIForgeStaticSwitchBinding> StaticSwitchBindings;

	const FMIForgeTextureBinding* FindTextureBinding(
		EMIForgeTextureType TextureType) const
	{
		return TextureBindings.FindByPredicate(
			[TextureType](const FMIForgeTextureBinding& Binding)
			{
				return Binding.TextureType == TextureType;
			});
	}

	const FMIForgeStaticSwitchBinding* FindStaticSwitchBinding(
		EMIForgePresetOptions Option) const
	{
		return StaticSwitchBindings.FindByPredicate(
			[Option](const FMIForgeStaticSwitchBinding& Binding)
			{
				return Binding.PresetOption == Option;
			});
	}
};

struct FMIForgeVertexPaintLayerDefinition
{
	EMIForgeVertexPaintLayer Layer;

	FString DisplayName;
	FString ChannelName;

	bool bRequired = false;

	TMap<EMIForgeTextureType, FName> TextureParameters;
	FName EnabledSwitchParameter;
};

struct FMIForgeVertexPaintPresetDefinition
{
	FSoftObjectPath ParentMaterialPath;
	TArray<FMIForgeVertexPaintLayerDefinition> Layers;

	const FMIForgeVertexPaintLayerDefinition* FindLayer(
		EMIForgeVertexPaintLayer Layer) const
	{
		return Layers.FindByPredicate(
			[Layer](const FMIForgeVertexPaintLayerDefinition& Definition)
			{
				return Definition.Layer == Layer;
			});
	}

};

class FMIForgePresetDefinitions
{
public:
	static const FMIForgeMaterialPresetDefinition& GetStandard();
	static const FMIForgeMaterialPresetDefinition& GetRGBMask();
	static const FMIForgeVertexPaintPresetDefinition& GetVertexPaint();
	static const FMIForgeMaterialPresetDefinition& GetDecal();

	static const FMIForgeMaterialPresetDefinition* FindMaterialPreset(EMIForgeGenerationPreset Preset);
};

