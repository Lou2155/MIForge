// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MIForgeSettings.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeMIForgeSettings() {}

// ********** Begin Cross Module References ********************************************************
DEVELOPERSETTINGS_API UClass* Z_Construct_UClass_UDeveloperSettings();
ENGINE_API UEnum* Z_Construct_UEnum_Engine_TextureCompressionSettings();
MIFORGE_API UClass* Z_Construct_UClass_UMIForgeSettings();
MIFORGE_API UClass* Z_Construct_UClass_UMIForgeSettings_NoRegister();
MIFORGE_API UEnum* Z_Construct_UEnum_MIForge_EMIForgeTextureType();
MIFORGE_API UScriptStruct* Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings();
MIFORGE_API UScriptStruct* Z_Construct_UScriptStruct_FTextureExtension();
UPackage* Z_Construct_UPackage__Script_MIForge();
// ********** End Cross Module References **********************************************************

// ********** Begin ScriptStruct FTextureExtension *************************************************
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FTextureExtension;
class UScriptStruct* FTextureExtension::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FTextureExtension.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FTextureExtension.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FTextureExtension, (UObject*)Z_Construct_UPackage__Script_MIForge(), TEXT("TextureExtension"));
	}
	return Z_Registration_Info_UScriptStruct_FTextureExtension.OuterSingleton;
}
struct Z_Construct_UScriptStruct_FTextureExtension_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Extension_MetaData[] = {
		{ "Category", "Texture Extension" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_Extension_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_Extension;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FTextureExtension>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FTextureExtension_Statics::NewProp_Extension_Inner = { "Extension", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FTextureExtension_Statics::NewProp_Extension = { "Extension", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FTextureExtension, Extension), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Extension_MetaData), NewProp_Extension_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FTextureExtension_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FTextureExtension_Statics::NewProp_Extension_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FTextureExtension_Statics::NewProp_Extension,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FTextureExtension_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FTextureExtension_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_MIForge,
	nullptr,
	&NewStructOps,
	"TextureExtension",
	Z_Construct_UScriptStruct_FTextureExtension_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FTextureExtension_Statics::PropPointers),
	sizeof(FTextureExtension),
	alignof(FTextureExtension),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FTextureExtension_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FTextureExtension_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FTextureExtension()
{
	if (!Z_Registration_Info_UScriptStruct_FTextureExtension.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FTextureExtension.InnerSingleton, Z_Construct_UScriptStruct_FTextureExtension_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_FTextureExtension.InnerSingleton;
}
// ********** End ScriptStruct FTextureExtension ***************************************************

// ********** Begin ScriptStruct FMIForgeTextureCompressionSettings ********************************
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings;
class UScriptStruct* FMIForgeTextureCompressionSettings::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings, (UObject*)Z_Construct_UPackage__Script_MIForge(), TEXT("MIForgeTextureCompressionSettings"));
	}
	return Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.OuterSingleton;
}
struct Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CompressionSettings_MetaData[] = {
		{ "Category", "Texture Compression" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bSRGB_MetaData[] = {
		{ "Category", "Texture Compression" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FBytePropertyParams NewProp_CompressionSettings;
	static void NewProp_bSRGB_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bSRGB;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FMIForgeTextureCompressionSettings>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_CompressionSettings = { "CompressionSettings", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMIForgeTextureCompressionSettings, CompressionSettings), Z_Construct_UEnum_Engine_TextureCompressionSettings, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CompressionSettings_MetaData), NewProp_CompressionSettings_MetaData) }; // 2502711249
void Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_bSRGB_SetBit(void* Obj)
{
	((FMIForgeTextureCompressionSettings*)Obj)->bSRGB = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_bSRGB = { "bSRGB", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FMIForgeTextureCompressionSettings), &Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_bSRGB_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bSRGB_MetaData), NewProp_bSRGB_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_CompressionSettings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewProp_bSRGB,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_MIForge,
	nullptr,
	&NewStructOps,
	"MIForgeTextureCompressionSettings",
	Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::PropPointers),
	sizeof(FMIForgeTextureCompressionSettings),
	alignof(FMIForgeTextureCompressionSettings),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings()
{
	if (!Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.InnerSingleton, Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings.InnerSingleton;
}
// ********** End ScriptStruct FMIForgeTextureCompressionSettings **********************************

// ********** Begin Class UMIForgeSettings *********************************************************
void UMIForgeSettings::StaticRegisterNativesUMIForgeSettings()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UMIForgeSettings;
UClass* UMIForgeSettings::GetPrivateStaticClass()
{
	using TClass = UMIForgeSettings;
	if (!Z_Registration_Info_UClass_UMIForgeSettings.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("MIForgeSettings"),
			Z_Registration_Info_UClass_UMIForgeSettings.InnerSingleton,
			StaticRegisterNativesUMIForgeSettings,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_UMIForgeSettings.InnerSingleton;
}
UClass* Z_Construct_UClass_UMIForgeSettings_NoRegister()
{
	return UMIForgeSettings::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UMIForgeSettings_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "MIForgeSettings.h" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_TextureExtensions_MetaData[] = {
		{ "Category", "Texture Extensions" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_TextureCompressionSettingsMap_MetaData[] = {
		{ "Category", "Texture Compression" },
		{ "ModuleRelativePath", "Public/MIForgeSettings.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_TextureExtensions_ValueProp;
	static const UECodeGen_Private::FStrPropertyParams NewProp_TextureExtensions_Key_KeyProp;
	static const UECodeGen_Private::FMapPropertyParams NewProp_TextureExtensions;
	static const UECodeGen_Private::FStructPropertyParams NewProp_TextureCompressionSettingsMap_ValueProp;
	static const UECodeGen_Private::FBytePropertyParams NewProp_TextureCompressionSettingsMap_Key_KeyProp_Underlying;
	static const UECodeGen_Private::FEnumPropertyParams NewProp_TextureCompressionSettingsMap_Key_KeyProp;
	static const UECodeGen_Private::FMapPropertyParams NewProp_TextureCompressionSettingsMap;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UMIForgeSettings>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions_ValueProp = { "TextureExtensions", nullptr, (EPropertyFlags)0x0000000000004001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 1, Z_Construct_UScriptStruct_FTextureExtension, METADATA_PARAMS(0, nullptr) }; // 3319986910
const UECodeGen_Private::FStrPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions_Key_KeyProp = { "TextureExtensions_Key", nullptr, (EPropertyFlags)0x0000000000004001, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FMapPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions = { "TextureExtensions", nullptr, (EPropertyFlags)0x0010000000004001, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMIForgeSettings, TextureExtensions), EMapPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_TextureExtensions_MetaData), NewProp_TextureExtensions_MetaData) }; // 3319986910
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_ValueProp = { "TextureCompressionSettingsMap", nullptr, (EPropertyFlags)0x0000000000004001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 1, Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings, METADATA_PARAMS(0, nullptr) }; // 756354263
const UECodeGen_Private::FBytePropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_Key_KeyProp_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FEnumPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_Key_KeyProp = { "TextureCompressionSettingsMap_Key", nullptr, (EPropertyFlags)0x0000000000004001, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UEnum_MIForge_EMIForgeTextureType, METADATA_PARAMS(0, nullptr) }; // 2923088621
const UECodeGen_Private::FMapPropertyParams Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap = { "TextureCompressionSettingsMap", nullptr, (EPropertyFlags)0x0010000000004001, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMIForgeSettings, TextureCompressionSettingsMap), EMapPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_TextureCompressionSettingsMap_MetaData), NewProp_TextureCompressionSettingsMap_MetaData) }; // 2923088621 756354263
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UMIForgeSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions_ValueProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions_Key_KeyProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureExtensions,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_ValueProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_Key_KeyProp_Underlying,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap_Key_KeyProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeSettings_Statics::NewProp_TextureCompressionSettingsMap,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeSettings_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UMIForgeSettings_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UDeveloperSettings,
	(UObject* (*)())Z_Construct_UPackage__Script_MIForge,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeSettings_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UMIForgeSettings_Statics::ClassParams = {
	&UMIForgeSettings::StaticClass,
	"MIForge",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UMIForgeSettings_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeSettings_Statics::PropPointers),
	0,
	0x001000A6u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeSettings_Statics::Class_MetaDataParams), Z_Construct_UClass_UMIForgeSettings_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UMIForgeSettings()
{
	if (!Z_Registration_Info_UClass_UMIForgeSettings.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UMIForgeSettings.OuterSingleton, Z_Construct_UClass_UMIForgeSettings_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UMIForgeSettings.OuterSingleton;
}
UMIForgeSettings::UMIForgeSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UMIForgeSettings);
UMIForgeSettings::~UMIForgeSettings() {}
// ********** End Class UMIForgeSettings ***********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FTextureExtension::StaticStruct, Z_Construct_UScriptStruct_FTextureExtension_Statics::NewStructOps, TEXT("TextureExtension"), &Z_Registration_Info_UScriptStruct_FTextureExtension, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FTextureExtension), 3319986910U) },
		{ FMIForgeTextureCompressionSettings::StaticStruct, Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics::NewStructOps, TEXT("MIForgeTextureCompressionSettings"), &Z_Registration_Info_UScriptStruct_FMIForgeTextureCompressionSettings, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMIForgeTextureCompressionSettings), 756354263U) },
	};
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UMIForgeSettings, UMIForgeSettings::StaticClass, TEXT("UMIForgeSettings"), &Z_Registration_Info_UClass_UMIForgeSettings, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UMIForgeSettings), 301546310U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_2544668673(TEXT("/Script/MIForge"),
	Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_Statics::ClassInfo),
	Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h__Script_MIForge_Statics::ScriptStructInfo),
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
