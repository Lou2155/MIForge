// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeValidator.h"
#include "MIForgeTypes.h"
#include "MIForgeUtilities.h"
#include "MIForgeTextureSetBuilder.h"
#include "Presets/MIForgePresetDefinitions.h"


namespace
{
    struct FMIForgeTextureValidationRule
    {
        EMIForgeTextureType TextureType =
            EMIForgeTextureType::Unknown;

        EMIForgeRequirement Requirement =
            EMIForgeRequirement::Optional;

        EMIForgePresetOptions PresetOption =
            EMIForgePresetOptions::None;
    };

    struct FMIForgeValidationContext
    {
        TSet<EMIForgePresetOptions> EnabledOptions;

        bool IsOptionEnabled(
            EMIForgePresetOptions Option) const
        {
            return Option == EMIForgePresetOptions::None ||
                EnabledOptions.Contains(Option);
        }
    };

	static TArray<FMIForgeTextureValidationRule> BuildMaterialValidationRules(
		const FMIForgeMaterialPresetDefinition& Definition)
	{
		TArray<FMIForgeTextureValidationRule> Rules;
		Rules.Reserve(Definition.TextureBindings.Num());

		for (const FMIForgeTextureBinding& Binding : Definition.TextureBindings)
		{
			Rules.Add({
				Binding.TextureType,
				Binding.Requirement,
				Binding.PresetOption
				});
		}
		return Rules;
	}

	static TArray<FMIForgeTextureValidationRule> BuildVertexPaintLayerValidationRules(
		const FMIForgeVertexPaintLayerDefinition& LayerDefinition)
	{
		TArray<FMIForgeTextureValidationRule> Rules;

		const EMIForgeTextureType OrderedTypes[] = {
			EMIForgeTextureType::Albedo,
			EMIForgeTextureType::Normal,
			EMIForgeTextureType::ORM,
			EMIForgeTextureType::Height
		};

		for (EMIForgeTextureType TextureType : OrderedTypes)
		{
			if (!LayerDefinition.TextureParameters.Contains(TextureType))
			{
				continue;
			}

			Rules.Add({
				TextureType,
				TextureType == EMIForgeTextureType::Albedo
					? EMIForgeRequirement::Required
					: EMIForgeRequirement::Optional,
				EMIForgePresetOptions::None
				});
		}

		return Rules;
	}


	static FMIForgeTextureSetValidationResult
		ValidateTextureSetByRules(
			const FMIForgeTextureSet& TextureSet,
			const TArray<FMIForgeTextureValidationRule>& Rules,
			const FMIForgeValidationContext& Context,
			bool bIgnoreUnrecognizedTextures)
	{	
		FMIForgeTextureSetValidationResult Result;
		Result.SetName = TextureSet.SetName;
		Result.bCanGenerate = true;

		TSet<EMIForgeTextureType> SupportedTypes;
		SupportedTypes.Reserve(Rules.Num());

		for (const FMIForgeTextureValidationRule& Rule : Rules)
		{	
			// Add before checking whether the rule is active.
			// An inactive rule is still a supported texture type.

			SupportedTypes.Add(Rule.TextureType);

			const bool bRuleActive = Context.IsOptionEnabled(Rule.PresetOption);

			if (!bRuleActive)
			{
				continue;
			}

			const bool bRequired = Rule.Requirement == EMIForgeRequirement::Required;

			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Rule.TextureType);

			if (TextureInfo)
			{
				Result.SuccessfulAppliedTextures.Add({
					TextureInfo->AssetName,
					Rule.TextureType,
					bRequired
					});

				continue;
			}

			FMIForgeTextureRequirement MissingTexture = {
				TEXT(""),
				Rule.TextureType,
				bRequired
			};

			if (bRequired)
			{
				Result.bCanGenerate = false;
				Result.MissingRequiredTextures.Add(MissingTexture);
			}
			else
			{
				Result.MissingOptionalTextures.Add(MissingTexture);
			}
		}


		if (!bIgnoreUnrecognizedTextures)
		{	
			// Textures that were already classified as unknown.
			for (const FMIForgeTextureInfo& UnknownTexture : TextureSet.UnrecognizedTextures)
			{
				Result.UnrecognizedTextures.Add({
					UnknownTexture.AssetName,
					EMIForgeTextureType::Unknown,
					false
				});
			}

			// Recognized MIForge texture types that are unsupported
			// by this particular preset.
			for (const TPair<EMIForgeTextureType, FMIForgeTextureInfo>& TexturePair : TextureSet.Textures)
			{
				if (SupportedTypes.Contains(TexturePair.Key))
				{
					continue;
				}

				Result.UnrecognizedTextures.Add({
					TexturePair.Value.AssetName,
					TexturePair.Key,
					false
					});
			}
		}

		return Result;

	}

}


FMIForgeTextureSetValidationResult FMIForgeValidator::ValidateStandardSet(const FMIForgeTextureSet& TextureSet, bool bUseEmissive, bool bUseDetailNormal, bool bIgnoreUnrecognizedTextures) const
{
	FMIForgeValidationContext Context;

	if (bUseEmissive)
	{
		Context.EnabledOptions.Add(
			EMIForgePresetOptions::UseEmissiveTexture);
	}

	if (bUseDetailNormal)
	{
		Context.EnabledOptions.Add(
			EMIForgePresetOptions::UseDetailNormalTexture);
	}

	return ValidateTextureSetByRules(
		TextureSet,
		BuildMaterialValidationRules(
			FMIForgePresetDefinitions::GetStandard()),
		Context,
		bIgnoreUnrecognizedTextures);

}

FMIForgeTextureSetValidationResult FMIForgeValidator::ValidateRGBSet(const FMIForgeTextureSet& TextureSet, bool bUseBaseORMTexture, bool bEnableEmissiveChannel, bool bUseDetailNormalTextureRGB, bool bIgnoreUnrecognizedTextures) const
{	
	FMIForgeValidationContext Context;

	if (bUseBaseORMTexture)
	{
		Context.EnabledOptions.Add(
			EMIForgePresetOptions::UseBaseORM);
	}

	if (bUseDetailNormalTextureRGB)
	{
		Context.EnabledOptions.Add(
			EMIForgePresetOptions::UseDetailNormalTexture);
	}

	return ValidateTextureSetByRules(
		TextureSet,
		BuildMaterialValidationRules(
			FMIForgePresetDefinitions::GetRGBMask()),
		Context,
		bIgnoreUnrecognizedTextures);
}

FMIForgeVertexPaintLayerStackValidationResult FMIForgeValidator::ValidateVertexPaintLayerStack(const FMIForgeVertexPaintLayerStack& LayerStack, bool bIgnoreUnrecognizedTextures) const
{
	FMIForgeVertexPaintLayerStackValidationResult Result;

	const FMIForgeVertexPaintPresetDefinition& Definition = FMIForgePresetDefinitions::GetVertexPaint();
	
	bool bHasBlockingError = false;

	for (const FMIForgeVertexPaintLayerSlot* Slot : LayerStack.GetSlots())
	{
		if (!Slot)
		{
			continue;
		}

		const FMIForgeVertexPaintLayerDefinition* LayerDefinition =
			Definition.FindLayer(Slot->Layer);

		if (!LayerDefinition)
		{
			bHasBlockingError = true;
			Result.bCanGenerate = false;
			return Result;
		}

		FMIForgeVertexPaintLayerValidationResult OneLayerResult;

		
		OneLayerResult.Layer = LayerDefinition->Layer;
		OneLayerResult.DisplayName = LayerDefinition->DisplayName;
		OneLayerResult.bRequired = LayerDefinition->bRequired;

		if (!Slot->AssignedTextureSet.IsValid())
		{
			OneLayerResult.AssignedSetName = TEXT("Empty");
			OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Empty;

			if (OneLayerResult.bRequired)
			{
				OneLayerResult.bCanGenerate = false;
				OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Error;

				OneLayerResult.MissingRequiredTextures.Add({
					TEXT(""),
					EMIForgeTextureType::Unknown,
					true
					});

				bHasBlockingError = true;
			}

			Result.LayerResults.Add(MoveTemp(OneLayerResult));
			continue;
		}

		const FMIForgeTextureSet& TextureSet = *Slot->AssignedTextureSet;
		OneLayerResult.AssignedSetName = TextureSet.SetName;
		Result.AssignedLayerCount++;

		const FMIForgeTextureSetValidationResult SetResult =
			ValidateTextureSetByRules(
				TextureSet,
				BuildVertexPaintLayerValidationRules(*LayerDefinition),
				FMIForgeValidationContext(),
				bIgnoreUnrecognizedTextures);

		OneLayerResult.MissingRequiredTextures =
			SetResult.MissingRequiredTextures;

		OneLayerResult.MissingOptionalTextures =
			SetResult.MissingOptionalTextures;

		OneLayerResult.UnrecognizedTextures =
			SetResult.UnrecognizedTextures;


		OneLayerResult.bCanGenerate =
			OneLayerResult.MissingRequiredTextures.Num() == 0;

		if (!OneLayerResult.bCanGenerate)
		{
			OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Error;
			bHasBlockingError = true;
		}
		else if (
			OneLayerResult.MissingOptionalTextures.Num() > 0 ||
			OneLayerResult.UnrecognizedTextures.Num() > 0
			)
		{
			OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Warning;
		}
		else
		{
			OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Valid;
		}

		Result.LayerResults.Add(MoveTemp(OneLayerResult));

	}

	bool bBaseAndRLayerAssigned = true;
	for(FMIForgeVertexPaintLayerValidationResult& lr : Result.LayerResults)
	{
		if (lr.Layer == EMIForgeVertexPaintLayer::Base)
		{
			if(lr.Status == EMIForgeVertexPaintLayerStatus::Error)
			{
				bBaseAndRLayerAssigned = false;
			}
		}
		if (lr.Layer == EMIForgeVertexPaintLayer::LayerR)
		{
			if (lr.Status == EMIForgeVertexPaintLayerStatus::Error)
			{
				bBaseAndRLayerAssigned = false;
			}
		}
	}

	Result.bCanGenerate =
		!bHasBlockingError &&
		bBaseAndRLayerAssigned;

	return Result;

}

FMIForgeTextureSetValidationResult FMIForgeValidator::ValidateVertexPaintSet(const FMIForgeTextureSet& TextureSet, bool bIgnoreUnrecognizedTextures) const
{	
	const FMIForgeVertexPaintPresetDefinition& Definition =
		FMIForgePresetDefinitions::GetVertexPaint();

	const FMIForgeVertexPaintLayerDefinition* BaseDefinition =
		Definition.FindLayer(EMIForgeVertexPaintLayer::Base);


	return ValidateTextureSetByRules(
		TextureSet,
		BuildVertexPaintLayerValidationRules(
			*BaseDefinition),
		FMIForgeValidationContext(),
		bIgnoreUnrecognizedTextures);
}

FMIForgeValidationSummary FMIForgeValidator::BuildSummaryFromTextureSets(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, TFunctionRef<FMIForgeTextureSetValidationResult(const FMIForgeTextureSet&)> ValidateSet) const
{
	FMIForgeValidationSummary Summary;
	Summary.TotalSets = TextureSets.Num();

	for (const TSharedPtr<FMIForgeTextureSet>& ts : TextureSets) {
	
		if (!ts.IsValid())
		{
			MIForgeUtilities::PrintNotification(TEXT("Invalid Texture Set found, skipping validation"));
			continue;
			
		}

		FMIForgeTextureSetValidationResult SetResult = ValidateSet(*ts);

		Summary.MissingRequiredTextureCount += SetResult.MissingRequiredTextures.Num();
		Summary.MissingOptionalTextureCount += SetResult.MissingOptionalTextures.Num();
		Summary.UnrecognizedTextureCount += SetResult.UnrecognizedTextures.Num();

		if (SetResult.bCanGenerate)
		{
			Summary.ReadyToCreateCount++;
		}
		else
		{
			Summary.SetsWithErrors++;
		}

		if (SetResult.MissingOptionalTextures.Num() > 0)
		{
			Summary.SetsWithWarnings++;
		}

		Summary.SetResults.Add(MoveTemp(SetResult));

	}

	return Summary;
}

FMIForgeValidationSummary FMIForgeValidator::BuildStandardSummaryFromTextureSets(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, bool bUseEmissive, bool bUseDetailNormal, bool bIgnoreUnrecognizedTextures) const
{
	return BuildSummaryFromTextureSets(TextureSets, [this, bUseEmissive, bUseDetailNormal, bIgnoreUnrecognizedTextures](const FMIForgeTextureSet& Set)
		{
			return ValidateStandardSet(Set, bUseEmissive, bUseDetailNormal, bIgnoreUnrecognizedTextures);
		});
}

FMIForgeValidationSummary FMIForgeValidator::BuildRGBSummaryFromTextureSets(const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets, bool bUseBaseORMTexture, bool bEnableEmissiveChannel, bool bUseDetailNormalTextureRGB, bool bIgnoreUnrecognizedTextures) const
{
	return BuildSummaryFromTextureSets(TextureSets, [this, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures](const FMIForgeTextureSet& Set)
		{
			return ValidateRGBSet(Set, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures);
		});
}

FMIForgeVertexPaintValidationSummary FMIForgeValidator::BuildVertexPaintLayerStackSummary(const FMIForgeVertexPaintLayerStackValidationResult& LayerStackResult) const
{	
	FMIForgeVertexPaintValidationSummary Summary;

	Summary.AssignedLayerCount = LayerStackResult.AssignedLayerCount;
	Summary.bCanGenerate = LayerStackResult.bCanGenerate;
	Summary.LayerResults = LayerStackResult.LayerResults;

	for(const FMIForgeVertexPaintLayerValidationResult& lr : LayerStackResult.LayerResults)
	{	
		Summary.MissingRequiredTextureCount += lr.MissingRequiredTextures.Num();
		Summary.MissingOptionalTextureCount += lr.MissingOptionalTextures.Num();
		Summary.UnrecognizedTextureCount += lr.UnrecognizedTextures.Num();

		switch (lr.Layer)
		{
		case EMIForgeVertexPaintLayer::Base:
			Summary.BaseStatus = lr.Status;
			if (lr.Status == EMIForgeVertexPaintLayerStatus::Error && lr.AssignedSetName == TEXT("Empty"))
			{
				Summary.BaseStatus = EMIForgeVertexPaintLayerStatus::Empty;
			}
			break;

		case EMIForgeVertexPaintLayer::LayerR:
			Summary.LayerRStatus = lr.Status;
			if (lr.Status == EMIForgeVertexPaintLayerStatus::Error && lr.AssignedSetName == TEXT("Empty"))
			{
				Summary.LayerRStatus = EMIForgeVertexPaintLayerStatus::Empty;
			}
			break;

		case EMIForgeVertexPaintLayer::LayerG:
			Summary.LayerGStatus = lr.Status;
			break;

		case EMIForgeVertexPaintLayer::LayerB:
			Summary.LayerBStatus = lr.Status;
			break;
		}

	}
	return Summary;
}

FMIForgeValidationSummary FMIForgeValidator::BuildSummaryFromTextures(
	const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures,
	TFunctionRef<FMIForgeValidationSummary(const TArray<TSharedPtr<FMIForgeTextureSet>>&)> BuildSummaryFromTextureSets
) const
{
	TArray<FMIForgeTextureInfo> RecognizedTextures;

	for (const TSharedPtr<FMIForgeTextureInfo>& Texture : SelectedTextures)
	{
		if (!Texture.IsValid())
		{
			continue;
		}

		if (Texture->TextureType == EMIForgeTextureType::Unknown)
		{
			// Unknown selected textures are validation warnings/counts,
			// but they should not become generation input.
			continue;
		}

		RecognizedTextures.Add(*Texture);
	}

	FMIForgeTextureSetBuilder SetBuilder;
	TArray<FMIForgeTextureSet> RawTextureSets =
		SetBuilder.BuildTextureSets(RecognizedTextures);

	TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets;
	TextureSets.Reserve(RawTextureSets.Num());

	for (FMIForgeTextureSet& Set : RawTextureSets)
	{
		TextureSets.Add(MakeShared<FMIForgeTextureSet>(MoveTemp(Set)));
	}

	FMIForgeValidationSummary Summary =
		BuildSummaryFromTextureSets(TextureSets);

	for (const TSharedPtr<FMIForgeTextureInfo>& Texture : SelectedTextures)
	{
		if (Texture.IsValid() &&
			Texture->TextureType == EMIForgeTextureType::Unknown)
		{
			Summary.UnrecognizedTextureCount++;
		}
	}

	return Summary;
}

FMIForgeValidationSummary FMIForgeValidator::BuildStandardSummaryFromTextures(const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures, bool bUseEmissive, bool bUseDetailNormal, bool bIgnoreUnrecognizedTextures) const
{
	return BuildSummaryFromTextures(SelectedTextures, [this, bUseEmissive, bUseDetailNormal, bIgnoreUnrecognizedTextures](const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets)
		{
			return BuildStandardSummaryFromTextureSets(TextureSets, bUseEmissive, bUseDetailNormal, bIgnoreUnrecognizedTextures);
		});
}

FMIForgeValidationSummary FMIForgeValidator::BuildRGBSummaryFromTextures(const TArray<TSharedPtr<FMIForgeTextureInfo>>& SelectedTextures, bool bUseBaseORMTexture, bool bEnableEmissiveChannel, bool bUseDetailNormalTextureRGB, bool bIgnoreUnrecognizedTextures) const
{
	return BuildSummaryFromTextures(SelectedTextures, [this, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures](const TArray<TSharedPtr<FMIForgeTextureSet>>& TextureSets)
		{
			return BuildRGBSummaryFromTextureSets(TextureSets, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures);
		});
}
