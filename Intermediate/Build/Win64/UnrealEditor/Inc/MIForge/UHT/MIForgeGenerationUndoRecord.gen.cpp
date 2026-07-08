// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MIForgeGenerationUndoRecord.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeMIForgeGenerationUndoRecord() {}

// ********** Begin Cross Module References ********************************************************
COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FSoftObjectPath();
MIFORGE_API UClass* Z_Construct_UClass_UMIForgeGenerationUndoRecord();
MIFORGE_API UClass* Z_Construct_UClass_UMIForgeGenerationUndoRecord_NoRegister();
UPackage* Z_Construct_UPackage__Script_MIForge();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UMIForgeGenerationUndoRecord *********************************************
void UMIForgeGenerationUndoRecord::StaticRegisterNativesUMIForgeGenerationUndoRecord()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord;
UClass* UMIForgeGenerationUndoRecord::GetPrivateStaticClass()
{
	using TClass = UMIForgeGenerationUndoRecord;
	if (!Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("MIForgeGenerationUndoRecord"),
			Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.InnerSingleton,
			StaticRegisterNativesUMIForgeGenerationUndoRecord,
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
	return Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.InnerSingleton;
}
UClass* Z_Construct_UClass_UMIForgeGenerationUndoRecord_NoRegister()
{
	return UMIForgeGenerationUndoRecord::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "MIForgeGenerationUndoRecord.h" },
		{ "ModuleRelativePath", "Public/MIForgeGenerationUndoRecord.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bAssetsShouldExist_MetaData[] = {
		{ "ModuleRelativePath", "Public/MIForgeGenerationUndoRecord.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CreatedAssetPaths_MetaData[] = {
		{ "ModuleRelativePath", "Public/MIForgeGenerationUndoRecord.h" },
	};
#endif // WITH_METADATA
	static void NewProp_bAssetsShouldExist_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bAssetsShouldExist;
	static const UECodeGen_Private::FStructPropertyParams NewProp_CreatedAssetPaths_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_CreatedAssetPaths;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UMIForgeGenerationUndoRecord>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
void Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_bAssetsShouldExist_SetBit(void* Obj)
{
	((UMIForgeGenerationUndoRecord*)Obj)->bAssetsShouldExist = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_bAssetsShouldExist = { "bAssetsShouldExist", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UMIForgeGenerationUndoRecord), &Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_bAssetsShouldExist_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bAssetsShouldExist_MetaData), NewProp_bAssetsShouldExist_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_CreatedAssetPaths_Inner = { "CreatedAssetPaths", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UScriptStruct_FSoftObjectPath, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_CreatedAssetPaths = { "CreatedAssetPaths", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMIForgeGenerationUndoRecord, CreatedAssetPaths), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CreatedAssetPaths_MetaData), NewProp_CreatedAssetPaths_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_bAssetsShouldExist,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_CreatedAssetPaths_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::NewProp_CreatedAssetPaths,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UObject,
	(UObject* (*)())Z_Construct_UPackage__Script_MIForge,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::ClassParams = {
	&UMIForgeGenerationUndoRecord::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::PropPointers),
	0,
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::Class_MetaDataParams), Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UMIForgeGenerationUndoRecord()
{
	if (!Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.OuterSingleton, Z_Construct_UClass_UMIForgeGenerationUndoRecord_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord.OuterSingleton;
}
UMIForgeGenerationUndoRecord::UMIForgeGenerationUndoRecord(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UMIForgeGenerationUndoRecord);
UMIForgeGenerationUndoRecord::~UMIForgeGenerationUndoRecord() {}
// ********** End Class UMIForgeGenerationUndoRecord ***********************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeGenerationUndoRecord_h__Script_MIForge_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UMIForgeGenerationUndoRecord, UMIForgeGenerationUndoRecord::StaticClass, TEXT("UMIForgeGenerationUndoRecord"), &Z_Registration_Info_UClass_UMIForgeGenerationUndoRecord, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UMIForgeGenerationUndoRecord), 16091834U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeGenerationUndoRecord_h__Script_MIForge_379203871(TEXT("/Script/MIForge"),
	Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeGenerationUndoRecord_h__Script_MIForge_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeGenerationUndoRecord_h__Script_MIForge_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
