// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EMIForgeTextureType : uint8
{
    Unknown      UMETA(DisplayName = "Unknown"),
    Albedo       UMETA(DisplayName = "Albedo"),
    Normal       UMETA(DisplayName = "Normal"),
    ORM          UMETA(DisplayName = "ORM"),
    Emissive     UMETA(DisplayName = "Emissive"),
    DetailNormal UMETA(DisplayName = "Detail Normal"),
    RGB          UMETA(DisplayName = "RGB Mask"),
	Height       UMETA(DisplayName = "Height")
};

struct FMIForgeTextureInfo
{
    FAssetData AssetData;
    FString AssetName;
    FString PackagePath;
    FString ObjectPath;
    EMIForgeTextureType TextureType = EMIForgeTextureType::Unknown;
    FString MatchedSuffix;
    FString BaseName;

    FIntPoint TextureSize = FIntPoint::ZeroValue;
    FString TextureSizeText;
};

struct FMIForgeTextureSet
{
    FString SetName;

    TMap<EMIForgeTextureType, FMIForgeTextureInfo> Textures;
    TArray<FMIForgeTextureInfo> UnrecognizedTextures;

    bool HasTexture(EMIForgeTextureType Type) const
    {
        return Textures.Contains(Type);
    }

    /*const FMIForgeTextureInfo* FindTextureInfo(EMIForgeTextureType Type) const
    {
        return Textures.Find(Type);
    }*/

    const FString GetTextureSizeText() const
    {
        FIntPoint ReferenceSize(0, 0);
        bool bHaveReference = false;
		FString SizeText = TEXT("-");

        for (const auto& TexturePair : Textures)
        {
            const FMIForgeTextureInfo& TexInfo = TexturePair.Value;
            if (!TexInfo.TextureSizeText.IsEmpty())
            {
                if (!bHaveReference)
                {
                    ReferenceSize = TexInfo.TextureSize;
                    bHaveReference = true;
                }
                else if (TexInfo.TextureSize != ReferenceSize)
                {
                    SizeText = TEXT("Size Varies");
                    return SizeText;
                }
            }
        }
		return bHaveReference ? FString::Printf(TEXT("%dx%d"), ReferenceSize.X, ReferenceSize.Y) : SizeText;
	}

    int UnrecognizedTextureCount() const
    {
        int Count = 0;
        for (const auto& TexturePair : Textures)
        {
            if (TexturePair.Value.TextureType == EMIForgeTextureType::Unknown)
            {
                Count++;
            }
        }
        return Count;
	}

    bool IsValid() const
    {
        return Textures.Num() > 0;
	}

};

struct FMIForgeTextureRequirement
{   
	FString TextureName;
    EMIForgeTextureType TextureType = EMIForgeTextureType::Unknown;
    bool bRequired = false;
};

struct FMIForgeTextureSetValidationResult
{
    FString SetName;
    bool bCanGenerate = true;

	TArray<FMIForgeTextureRequirement> SuccessfulAppliedTextures;
    TArray<FMIForgeTextureRequirement> MissingRequiredTextures;
    TArray<FMIForgeTextureRequirement> MissingOptionalTextures;
	TArray<FMIForgeTextureRequirement> UnrecognizedTextures;
    TArray<FText> Messages;
};

struct FMIForgeValidationSummary
{
    int32 TotalSets = 0;
    int32 ReadyToCreateCount = 0;

    int32 SetsWithErrors = 0;
    int32 SetsWithWarnings = 0;

    int32 MissingRequiredTextureCount = 0;
    int32 MissingOptionalTextureCount = 0;
	int32 UnrecognizedTextureCount = 0;

    TArray<FMIForgeTextureSetValidationResult> SetResults;
};



enum class EIfMIExistsOption : uint8
{
    Skip = 0,
    Overwrite,
    CreateUnique
};

enum class EMIForgeGenerationAction : uint8
{
    Created,
    Updated,
    Skipped,
    Failed
};

enum class EMIForgeGenerationPreset : uint8
{
    Standard,
    RGBMask,
    VertexPainting
};

struct FMIForgeGenerationOptions
{
    EMIForgeGenerationPreset Preset = EMIForgeGenerationPreset::Standard;

    FString TargetPath;
	FSoftObjectPath MaterialInstanceParentPath;

    EIfMIExistsOption IfMIExists = EIfMIExistsOption::Skip;

    //standard
    bool bUseEmissive = false;
    bool bUseDetailNormal = false;
    bool bUseTriplanar = false;
	//rgb mask
    bool bUseBaseORMTexture = true;
    bool bEnableEmissiveChannel = false;
    bool bUseDetailNormalTextureRGB = false;

	TMap<EMIForgeTextureType, FName> TextureParameterNames; // Map of texture type to parameter name in the material instance

	//static switch parameter names
    FName TriplanarParameterName = FName(TEXT("UseTriplanar?"));
    FName EmissiveParameterName = FName(TEXT("UseEmissiveTex?"));
	FName DetailNormalParameterName = FName(TEXT("UseDetailNormal?"));

	FName BaseORMParameterName = FName(TEXT("UseBaseORM?"));
	FName EmissiveChannelParameterName = FName(TEXT("EnableEmissiveChannel?"));
    
};

struct FMIForgeGenerationResult
{
    int32 CreatedCount = 0;
    int32 SkippedCount = 0;
    int32 FailedCount = 0;

    int32 UpdatedCount = 0;

    TArray<UObject*> CreatedAssets;
	TArray<UObject*> AffectedAssets;
    TArray<FText> Messages;
};

struct FMIForgeMaterialInstanceResolution
{
    UMaterialInstanceConstant* MaterialInstance = nullptr;
    EMIForgeGenerationAction Action = EMIForgeGenerationAction::Failed;
    FText Message;
};

enum class EMIForgeVertexPaintLayer : uint8
{
    Base,
    LayerR,
    LayerG,
    LayerB
};

enum class EMIForgeVertexPaintLayerStatus : uint8
{
    Empty,
    Valid,
    Warning,
    Error
};

struct FMIForgeVertexPaintLayerSlot
{
    EMIForgeVertexPaintLayer Layer = EMIForgeVertexPaintLayer::Base;

    FString DisplayName;
    FString ChannelName;

    bool bRequired = false;

    TSharedPtr<FMIForgeTextureSet> AssignedTextureSet;

    bool IsAssigned() const
    {
        return AssignedTextureSet.IsValid();
    }

    FString GetAddedTextureTypeText()
    {   
		FString AddedTypesText;

        if (!AssignedTextureSet.IsValid())
        {
            return TEXT("");
        }

        TArray<FString> AddedTypes;

        if (AssignedTextureSet->Textures.Contains(EMIForgeTextureType::Albedo))
        {
            AddedTypes.Add(TEXT("Albedo"));
        }

        if (AssignedTextureSet->Textures.Contains(EMIForgeTextureType::Normal))
        {
            AddedTypes.Add(TEXT("Normal"));
        }

        if (AssignedTextureSet->Textures.Contains(EMIForgeTextureType::ORM))
        {
            AddedTypes.Add(TEXT("ORM"));
        }

        if (AssignedTextureSet->Textures.Contains(EMIForgeTextureType::Height))
        {
            AddedTypes.Add(TEXT("Height"));
        }

        return FString::Join(AddedTypes, TEXT(" / "));
	}

    FString GetTextureSizeText() const
    {
		FString SizeText = TEXT("-");
        
        if (AssignedTextureSet.IsValid())
        {
            SizeText = AssignedTextureSet->GetTextureSizeText();
        }
		return SizeText;
	}
 //   EMIForgeVertexPaintLayerStatus GetStatus() const
 //   {
 //       if (!IsAssigned())
 //       {
 //           return bRequired ? EMIForgeVertexPaintLayerStatus::Error : EMIForgeVertexPaintLayerStatus::Empty;
 //       }
 //       // Check for missing required textures
 //       if (AssignedTextureSet->Textures.Num() == 0)
 //       {
 //           return EMIForgeVertexPaintLayerStatus::Warning;
 //       }
 //       return EMIForgeVertexPaintLayerStatus::Valid;
	//}
};

struct FMIForgeVertexPaintLayerStack
{
    FMIForgeVertexPaintLayerSlot BaseLayer;
    FMIForgeVertexPaintLayerSlot LayerR;
    FMIForgeVertexPaintLayerSlot LayerG;
    FMIForgeVertexPaintLayerSlot LayerB;

    TArray<FMIForgeVertexPaintLayerSlot*> GetSlots()
    {
        return {
            &BaseLayer,
            &LayerR,
            &LayerG,
            &LayerB
        };
	}

    TArray<const FMIForgeVertexPaintLayerSlot*> GetSlots() const
    {
        return {
            &BaseLayer,
            &LayerR,
            &LayerG,
            &LayerB
        };
    }
};

struct FMIForgeVertexPaintLayerValidationResult
{
    EMIForgeVertexPaintLayer Layer = EMIForgeVertexPaintLayer::Base;

    FString DisplayName;
    FString AssignedSetName;

    bool bRequired = false;
	bool bCanGenerate = true;

    EMIForgeVertexPaintLayerStatus Status =
        EMIForgeVertexPaintLayerStatus::Empty;

    TArray<FMIForgeTextureRequirement> MissingRequiredTextures;
    TArray<FMIForgeTextureRequirement> MissingOptionalTextures;
	TArray<FMIForgeTextureRequirement> UnrecognizedTextures;
};

struct FMIForgeVertexPaintLayerStackValidationResult
{
    bool bCanGenerate = false;
    int32 AssignedLayerCount = 0;

    TArray<FMIForgeVertexPaintLayerValidationResult> LayerResults;
};

struct FMIForgeVertexPaintValidationSummary
{   
    int32 AssignedLayerCount = 0;
    int32 MissingRequiredTextureCount = 0;
    int32 MissingOptionalTextureCount = 0;
    int32 UnrecognizedTextureCount = 0;

    EMIForgeVertexPaintLayerStatus BaseStatus = EMIForgeVertexPaintLayerStatus::Empty;
   /* int32 BaseLayerMissingRequiredTextureCount = 0;
    int32 BaseLayerMissingOptionalTextureCount = 0;*/

    EMIForgeVertexPaintLayerStatus LayerRStatus = EMIForgeVertexPaintLayerStatus::Empty;
    /*int32 LayerRMissingRequiredTextureCount = 0;
    int32 LayerRMissingOptionalTextureCount = 0;*/

    EMIForgeVertexPaintLayerStatus LayerGStatus = EMIForgeVertexPaintLayerStatus::Empty;
   /* int32 LayerGMissingRequiredTextureCount = 0;
    int32 LayerGMissingOptionalTextureCount = 0;*/

    EMIForgeVertexPaintLayerStatus LayerBStatus = EMIForgeVertexPaintLayerStatus::Empty;
  /*  int32 LayerBMissingRequiredTextureCount = 0;
    int32 LayerBMissingOptionalTextureCount = 0;*/

    bool bCanGenerate = false;
    TArray<FMIForgeVertexPaintLayerValidationResult> LayerResults;
};

struct FMIForgeVertexPaintGenerationOptions
{
    FString TargetPath;
    FSoftObjectPath MaterialInstanceParentPath;
    EIfMIExistsOption IfMIExists = EIfMIExistsOption::Skip;

    FString MaterialInstanceName;

    TMap<EMIForgeVertexPaintLayer, TMap<EMIForgeTextureType, FName>> LayerTextureParameterNames;
    TMap<EMIForgeVertexPaintLayer, FName> LayerEnabledParameterNames;
};

struct FMIForgeVertexPaintRecipeLayer
{
    FString SetName;

    TMap<EMIForgeTextureType, FSoftObjectPath> TexturePaths;
};

struct FMIForgeVertexPaintRecipe
{
    FString RecipeName;

    FMIForgeVertexPaintRecipeLayer LayerR;
    FMIForgeVertexPaintRecipeLayer LayerG;
    FMIForgeVertexPaintRecipeLayer LayerB;
};