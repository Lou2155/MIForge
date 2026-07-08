// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeMIForge_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_MIForge;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_MIForge()
	{
		if (!Z_Registration_Info_UPackage__Script_MIForge.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/MIForge",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000040,
				0x48DA5B79,
				0x4523D886,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_MIForge.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_MIForge.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_MIForge(Z_Construct_UPackage__Script_MIForge, TEXT("/Script/MIForge"), Z_Registration_Info_UPackage__Script_MIForge, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0x48DA5B79, 0x4523D886));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
