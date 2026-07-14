// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "MIForgeSettings.h"

#ifdef MIFORGE_MIForgeSettings_generated_h
#error "MIForgeSettings.generated.h already included, missing '#pragma once' in MIForgeSettings.h"
#endif
#define MIFORGE_MIForgeSettings_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin ScriptStruct FTextureExtension *************************************************
#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_14_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FTextureExtension_Statics; \
	MIFORGE_API static class UScriptStruct* StaticStruct();


struct FTextureExtension;
// ********** End ScriptStruct FTextureExtension ***************************************************

// ********** Begin ScriptStruct FMIForgeTextureCompressionSettings ********************************
#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_23_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FMIForgeTextureCompressionSettings_Statics; \
	MIFORGE_API static class UScriptStruct* StaticStruct();


struct FMIForgeTextureCompressionSettings;
// ********** End ScriptStruct FMIForgeTextureCompressionSettings **********************************

// ********** Begin Class UMIForgeSettings *********************************************************
MIFORGE_API UClass* Z_Construct_UClass_UMIForgeSettings_NoRegister();

#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_34_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUMIForgeSettings(); \
	friend struct Z_Construct_UClass_UMIForgeSettings_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend MIFORGE_API UClass* Z_Construct_UClass_UMIForgeSettings_NoRegister(); \
public: \
	DECLARE_CLASS2(UMIForgeSettings, UDeveloperSettings, COMPILED_IN_FLAGS(0 | CLASS_DefaultConfig | CLASS_Config), CASTCLASS_None, TEXT("/Script/MIForge"), Z_Construct_UClass_UMIForgeSettings_NoRegister) \
	DECLARE_SERIALIZER(UMIForgeSettings) \
	static const TCHAR* StaticConfigName() {return TEXT("MIForge");} \



#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_34_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UMIForgeSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UMIForgeSettings(UMIForgeSettings&&) = delete; \
	UMIForgeSettings(const UMIForgeSettings&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UMIForgeSettings); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UMIForgeSettings); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UMIForgeSettings) \
	NO_API virtual ~UMIForgeSettings();


#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_31_PROLOG
#define FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_34_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_34_INCLASS_NO_PURE_DECLS \
	FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h_34_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UMIForgeSettings;

// ********** End Class UMIForgeSettings ***********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeSettings_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
