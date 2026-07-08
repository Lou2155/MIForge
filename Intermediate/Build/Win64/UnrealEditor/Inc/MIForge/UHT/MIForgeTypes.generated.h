// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "MIForgeTypes.h"

#ifdef MIFORGE_MIForgeTypes_generated_h
#error "MIForgeTypes.generated.h already included, missing '#pragma once' in MIForgeTypes.h"
#endif
#define MIFORGE_MIForgeTypes_generated_h

#include "Templates/IsUEnumClass.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ReflectedTypeAccessors.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_UE5_Projects_MIForge_PluginDev_Plugins_MIForge_Source_MIForge_Public_MIForgeTypes_h

// ********** Begin Enum EMIForgeTextureType *******************************************************
#define FOREACH_ENUM_EMIFORGETEXTURETYPE(op) \
	op(EMIForgeTextureType::Unknown) \
	op(EMIForgeTextureType::Albedo) \
	op(EMIForgeTextureType::Normal) \
	op(EMIForgeTextureType::ORM) \
	op(EMIForgeTextureType::Emissive) \
	op(EMIForgeTextureType::DetailNormal) \
	op(EMIForgeTextureType::RGB) \
	op(EMIForgeTextureType::Height) 

enum class EMIForgeTextureType : uint8;
template<> struct TIsUEnumClass<EMIForgeTextureType> { enum { Value = true }; };
template<> MIFORGE_API UEnum* StaticEnum<EMIForgeTextureType>();
// ********** End Enum EMIForgeTextureType *********************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
