// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MIForgeTypes.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeMIForgeTypes() {}

// ********** Begin Cross Module References ********************************************************
MIFORGE_API UEnum* Z_Construct_UEnum_MIForge_EMIForgeTextureType();
UPackage* Z_Construct_UPackage__Script_MIForge();
// ********** End Cross Module References **********************************************************

// ********** Begin Enum EMIForgeTextureType *******************************************************
static FEnumRegistrationInfo Z_Registration_Info_UEnum_EMIForgeTextureType;
static UEnum* EMIForgeTextureType_StaticEnum()
{
	if (!Z_Registration_Info_UEnum_EMIForgeTextureType.OuterSingleton)
	{
		Z_Registration_Info_UEnum_EMIForgeTextureType.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_MIForge_EMIForgeTextureType, (UObject*)Z_Construct_UPackage__Script_MIForge(), TEXT("EMIForgeTextureType"));
	}
	return Z_Registration_Info_UEnum_EMIForgeTextureType.OuterSingleton;
}
template<> MIFORGE_API UEnum* StaticEnum<EMIForgeTextureType>()
{
	return EMIForgeTextureType_StaticEnum();
}
struct Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
		{ "Albedo.DisplayName", "Albedo" },
		{ "Albedo.Name", "EMIForgeTextureType::Albedo" },
		{ "BlueprintType", "true" },
		{ "DetailNormal.DisplayName", "Detail Normal" },
		{ "DetailNormal.Name", "EMIForgeTextureType::DetailNormal" },
		{ "Emissive.DisplayName", "Emissive" },
		{ "Emissive.Name", "EMIForgeTextureType::Emissive" },
		{ "Height.DisplayName", "Height" },
		{ "Height.Name", "EMIForgeTextureType::Height" },
		{ "ModuleRelativePath", "Public/MIForgeTypes.h" },
		{ "Normal.DisplayName", "Normal" },
		{ "Normal.Name", "EMIForgeTextureType::Normal" },
		{ "ORM.DisplayName", "ORM" },
		{ "ORM.Name", "EMIForgeTextureType::ORM" },
		{ "RGB.DisplayName", "RGB Mask" },
		{ "RGB.Name", "EMIForgeTextureType::RGB" },
		{ "Unknown.DisplayName", "Unknown" },
		{ "Unknown.Name", "EMIForgeTextureType::Unknown" },
	};
#endif // WITH_METADATA
	static constexpr UECodeGen_Private::FEnumeratorParam Enumerators[] = {
		{ "EMIForgeTextureType::Unknown", (int64)EMIForgeTextureType::Unknown },
		{ "EMIForgeTextureType::Albedo", (int64)EMIForgeTextureType::Albedo },
		{ "EMIForgeTextureType::Normal", (int64)EMIForgeTextureType::Normal },
		{ "EMIForgeTextureType::ORM", (int64)EMIForgeTextureType::ORM },
		{ "EMIForgeTextureType::Emissive", (int64)EMIForgeTextureType::Emissive },
		{ "EMIForgeTextureType::DetailNormal", (int64)EMIForgeTextureType::DetailNormal },
		{ "EMIForgeTextureType::RGB", (int64)EMIForgeTextureType::RGB },
		{ "EMIForgeTextureType::Height", (int64)EMIForgeTextureType::Height },
	};
	static const UECodeGen_Private::FEnumParams EnumParams;
};
const UECodeGen_Private::FEnumParams Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::EnumParams = {
	(UObject*(*)())Z_Construct_UPackage__Script_MIForge,
	nullptr,
	"EMIForgeTextureType",
	"EMIForgeTextureType",
	Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::Enumerators,
	RF_Public|RF_Transient|RF_MarkAsNative,
	UE_ARRAY_COUNT(Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::Enumerators),
	EEnumFlags::None,
	(uint8)UEnum::ECppForm::EnumClass,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::Enum_MetaDataParams), Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::Enum_MetaDataParams)
};
UEnum* Z_Construct_UEnum_MIForge_EMIForgeTextureType()
{
	if (!Z_Registration_Info_UEnum_EMIForgeTextureType.InnerSingleton)
	{
		UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_EMIForgeTextureType.InnerSingleton, Z_Construct_UEnum_MIForge_EMIForgeTextureType_Statics::EnumParams);
	}
	return Z_Registration_Info_UEnum_EMIForgeTextureType.InnerSingleton;
}
// ********** End Enum EMIForgeTextureType *********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeTypes_h__Script_MIForge_Statics
{
	static constexpr FEnumRegisterCompiledInInfo EnumInfo[] = {
		{ EMIForgeTextureType_StaticEnum, TEXT("EMIForgeTextureType"), &Z_Registration_Info_UEnum_EMIForgeTextureType, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 2923088621U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeTypes_h__Script_MIForge_413610744(TEXT("/Script/MIForge"),
	nullptr, 0,
	nullptr, 0,
	Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeTypes_h__Script_MIForge_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeTypes_h__Script_MIForge_Statics::EnumInfo));
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
