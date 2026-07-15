// Fill out your copyright notice in the Description page of Project Settings.


#include "Presets/MIForgePresetDefinitions.h"

const FMIForgeMaterialPresetDefinition& FMIForgePresetDefinitions::GetStandard()
{
	static const FMIForgeMaterialPresetDefinition Definition = []()
	{
		FMIForgeMaterialPresetDefinition Result;
		Result.Preset = EMIForgeGenerationPreset::Standard;
		Result.ParentMaterialPath = FSoftObjectPath(TEXT("/MIForge/MasterMaterialPresets/MM_Standard.MM_Standard"));
		Result.TextureBindings = {
			{ EMIForgeTextureType::Albedo, FName(TEXT("Albedo")), EMIForgeRequirement::Required },
			{ EMIForgeTextureType::Normal, FName(TEXT("Normal")), EMIForgeRequirement::Required },
			{ EMIForgeTextureType::ORM, FName(TEXT("ORM")), EMIForgeRequirement::Required },
			{ EMIForgeTextureType::Emissive, FName(TEXT("Emissive")), EMIForgeRequirement::Optional, EMIForgePresetOptions::UseEmissiveTexture },
			{ EMIForgeTextureType::DetailNormal, FName(TEXT("Detail Normal")), EMIForgeRequirement::Optional, EMIForgePresetOptions::UseDetailNormalTexture }
		};
		Result.StaticSwitchBindings = {
			{ FName(TEXT("UseTriplanar?")), EMIForgePresetOptions::UseTriplanar },
			{ FName(TEXT("UseEmissiveTex?")), EMIForgePresetOptions::UseEmissiveTexture },
			{ FName(TEXT("UseDetailNormal?")), EMIForgePresetOptions::UseDetailNormalTexture}
		};

		return Result;
	}();

	return Definition;

}

const FMIForgeMaterialPresetDefinition& FMIForgePresetDefinitions::GetRGBMask()
{
	static const FMIForgeMaterialPresetDefinition Definition = []()
		{
			FMIForgeMaterialPresetDefinition Result;
			Result.Preset = EMIForgeGenerationPreset::RGBMask;
			Result.ParentMaterialPath = FSoftObjectPath(TEXT("/MIForge/MasterMaterialPresets/MM_RGBmasking.MM_RGBmasking"));
			Result.TextureBindings = {
				{ EMIForgeTextureType::Albedo, FName(TEXT("Albedo")), EMIForgeRequirement::Required },
				{ EMIForgeTextureType::Normal, FName(TEXT("Base Normal")), EMIForgeRequirement::Required },
				{ EMIForgeTextureType::ORM, FName(TEXT("ORM")), EMIForgeRequirement::Required, EMIForgePresetOptions::UseBaseORM },
				{ EMIForgeTextureType::RGB, FName(TEXT("RGB_Mask")), EMIForgeRequirement::Required },
				{ EMIForgeTextureType::DetailNormal, FName(TEXT("Detail Normal")), EMIForgeRequirement::Optional, EMIForgePresetOptions::UseDetailNormalTexture }
			};
			Result.StaticSwitchBindings = {
				{ FName(TEXT("UseBaseORM?")), EMIForgePresetOptions::UseBaseORM },
				{ FName(TEXT("EnableEmissiveChannel?")), EMIForgePresetOptions::EnableEmissiveChannel },
				{ FName(TEXT("UseDetailNormal?")), EMIForgePresetOptions::UseDetailNormalTexture}
			};
			return Result;
		}();
	return Definition;
}

const FMIForgeVertexPaintPresetDefinition& FMIForgePresetDefinitions::GetVertexPaint()
{	

	static const FMIForgeVertexPaintPresetDefinition Definition = []()
		{	
			TMap<EMIForgeTextureType, FName> BaseLayerParameterNames;
			BaseLayerParameterNames.Add(EMIForgeTextureType::Albedo, FName(TEXT("Layer01_Albedo")));
			BaseLayerParameterNames.Add(EMIForgeTextureType::Normal, FName(TEXT("Layer01_Normal")));
			BaseLayerParameterNames.Add(EMIForgeTextureType::ORM, FName(TEXT("Layer01_ORM")));
			BaseLayerParameterNames.Add(EMIForgeTextureType::Height, FName(TEXT("Layer01_Height")));

			TMap<EMIForgeTextureType, FName> LayerRParameterNames;
			LayerRParameterNames.Add(EMIForgeTextureType::Albedo, FName(TEXT("Layer02_Albedo")));
			LayerRParameterNames.Add(EMIForgeTextureType::Normal, FName(TEXT("Layer02_Normal")));
			LayerRParameterNames.Add(EMIForgeTextureType::ORM, FName(TEXT("Layer02_ORM")));
			LayerRParameterNames.Add(EMIForgeTextureType::Height, FName(TEXT("Layer02_Height")));

			TMap<EMIForgeTextureType, FName> LayerGParameterNames;
			LayerGParameterNames.Add(EMIForgeTextureType::Albedo, FName(TEXT("Layer03_Albedo")));
			LayerGParameterNames.Add(EMIForgeTextureType::Normal, FName(TEXT("Layer03_Normal")));
			LayerGParameterNames.Add(EMIForgeTextureType::ORM, FName(TEXT("Layer03_ORM")));
			LayerGParameterNames.Add(EMIForgeTextureType::Height, FName(TEXT("Layer03_Height")));

			TMap<EMIForgeTextureType, FName> LayerBParameterNames;
			LayerBParameterNames.Add(EMIForgeTextureType::Albedo, FName(TEXT("Layer04_Albedo")));
			LayerBParameterNames.Add(EMIForgeTextureType::Normal, FName(TEXT("Layer04_Normal")));
			LayerBParameterNames.Add(EMIForgeTextureType::ORM, FName(TEXT("Layer04_ORM")));

			FMIForgeVertexPaintPresetDefinition Result;
			Result.ParentMaterialPath = FSoftObjectPath(TEXT("/MIForge/MasterMaterialPresets/MM_VertexPainting.MM_VertexPainting"));
			Result.Layers =
			{
				{EMIForgeVertexPaintLayer::Base, FString(TEXT("Base Layer")), FString(TEXT("Base")), true, BaseLayerParameterNames},
				{EMIForgeVertexPaintLayer::LayerR, FString(TEXT("Layer 01 / R")), FString(TEXT("R")), true, LayerRParameterNames},
				{EMIForgeVertexPaintLayer::LayerG, FString(TEXT("Layer 02 / G")), FString(TEXT("G")), false, LayerGParameterNames, FName(TEXT("Enable G Channel?"))},
				{EMIForgeVertexPaintLayer::LayerB, FString(TEXT("Layer 03 / B")), FString(TEXT("B")), false, LayerBParameterNames, FName(TEXT("Enable B Channel?"))}
			};

			return Result;
		}();
	return Definition;
}
