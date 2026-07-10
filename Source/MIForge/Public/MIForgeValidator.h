// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

class FMIForgeValidator
{
public:
	FMIForgeTextureSetValidationResult ValidateStandardSet(
        const FMIForgeTextureSet& TextureSet,
        bool bUseEmissive,
        bool bUseDetailNormal,
        bool bIgnoreUnrecognizedTextures
    ) const;

    FMIForgeTextureSetValidationResult ValidateRGBSet(
        const FMIForgeTextureSet& TextureSet,
        bool bUseBaseORMTexture,
        bool bEnableEmissiveChannel,
        bool bUseDetailNormalTextureRGB,
        bool bIgnoreUnrecognizedTextures
	) const;

    FMIForgeVertexPaintLayerStackValidationResult ValidateVertexPaintLayerStack(
        const FMIForgeVertexPaintLayerStack& LayerStack,
        bool bIgnoreUnrecognizedTextures
    ) const;

    FMIForgeTextureSetValidationResult ValidateVertexPaintSet(
        const FMIForgeTextureSet& TextureSet,
        bool bIgnoreUnrecognizedTextures
	) const;

    FMIForgeValidationSummary BuildSummaryFromTextureSets(
        const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets,
        TFunctionRef<FMIForgeTextureSetValidationResult(const FMIForgeTextureSet&)> ValidateSet
    ) const;

    FMIForgeValidationSummary BuildStandardSummaryFromTextureSets(
        const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets,
        bool bUseEmissive,
        bool bUseDetailNormal,
        bool bIgnoreUnrecognizedTextures
    ) const;

    FMIForgeValidationSummary BuildRGBSummaryFromTextureSets(
        const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets,
        bool bUseBaseORMTexture,
        bool bEnableEmissiveChannel,
        bool bUseDetailNormalTextureRGB,
        bool bIgnoreUnrecognizedTextures
	) const;

    FMIForgeVertexPaintValidationSummary BuildVertexPaintLayerStackSummary(
        const FMIForgeVertexPaintLayerStackValidationResult& LayerStackResult
	) const;

    FMIForgeValidationSummary BuildSummaryFromTextures(
        const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures,
        TFunctionRef<FMIForgeValidationSummary(const TArray<TSharedPtr<FMIForgeTextureSet>>&)> BuildSummaryFromTextureSets
    ) const;

    FMIForgeValidationSummary BuildStandardSummaryFromTextures(
        const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures,
        bool bUseEmissive,
        bool bUseDetailNormal,
        bool bIgnoreUnrecognizedTextures
	) const;

    FMIForgeValidationSummary BuildRGBSummaryFromTextures(
        const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures,
        bool bUseBaseORMTexture,
        bool bEnableEmissiveChannel,
        bool bUseDetailNormalTextureRGB,
		bool bIgnoreUnrecognizedTextures
	) const;

    enum class EMIForgeTextureSetStatus : uint8
    {
        Ready,
        Warning,
        Error
    };

   

    EMIForgeTextureSetStatus GetStandardSetStatus(
        const FMIForgeTextureSet& TextureSet,
        bool bUseEmissive,
        bool bUseDetailNormal, 
		bool bIgnoreUnrecognizedTextures
    ) const
    {
        const FMIForgeTextureSetValidationResult Result =
            ValidateStandardSet(TextureSet, bUseEmissive, bUseDetailNormal, bIgnoreUnrecognizedTextures);

        if (!Result.bCanGenerate)
        {
            return EMIForgeTextureSetStatus::Error;
        }

        if (Result.MissingOptionalTextures.Num() > 0 ||
            Result.UnrecognizedTextures.Num() > 0)
        {
            return EMIForgeTextureSetStatus::Warning;
        }

        return EMIForgeTextureSetStatus::Ready;
    }

    EMIForgeTextureSetStatus GetRGBSetStatus(
        const FMIForgeTextureSet& TextureSet,
        bool bUseBaseORMTexture,
        bool bEnableEmissiveChannel,
		bool bUseDetailNormalTextureRGB,
		bool bIgnoreUnrecognizedTextures
        ) const
    {
        const FMIForgeTextureSetValidationResult Result =
            ValidateRGBSet(TextureSet, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures);
        if (!Result.bCanGenerate)
        {
            return EMIForgeTextureSetStatus::Error;
        }
        if (Result.MissingOptionalTextures.Num() > 0 ||
            Result.UnrecognizedTextures.Num() > 0)
        {
            return EMIForgeTextureSetStatus::Warning;
        }
        return EMIForgeTextureSetStatus::Ready;
	}

    EMIForgeTextureSetStatus GetVertexPaintSetStatus(
        const FMIForgeTextureSet& TextureSet,
		bool bIgnoreUnrecognizedTextures
    ) const
    {
        const FMIForgeTextureSetValidationResult Result =
            ValidateVertexPaintSet(TextureSet, bIgnoreUnrecognizedTextures);

        if (!Result.bCanGenerate)
        {
            return EMIForgeTextureSetStatus::Error;
        }

        if (Result.MissingOptionalTextures.Num() > 0 ||
            Result.UnrecognizedTextures.Num() > 0)
        {
            return EMIForgeTextureSetStatus::Warning;
        }

        return EMIForgeTextureSetStatus::Ready;
	}

};
