// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeValidator.h"
#include "MIForgeTypes.h"
#include "MIForgeUtilities.h"
#include "MIForgeTextureSetBuilder.h"

FMIForgeTextureSetValidationResult FMIForgeValidator::ValidateStandardSet(const FMIForgeTextureSet& TextureSet, bool bUseEmissive, bool bUseDetailNormal, bool bIgnoreUnrecognizedTextures) const
{
	FMIForgeTextureSetValidationResult Result;
	Result.SetName = TextureSet.SetName;

	auto Require = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.bCanGenerate = false;
				Result.MissingRequiredTextures.Add({
					TEXT(""),
					Type,
					true
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				true
				});
		};

	auto Optional = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.MissingOptionalTextures.Add({
					TEXT(""),
					Type,
					false
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				false
				});
		};

	Require(EMIForgeTextureType::Albedo);
	Require(EMIForgeTextureType::Normal);
	Require(EMIForgeTextureType::ORM);

	if (bUseEmissive)
	{
		Optional(EMIForgeTextureType::Emissive);
	}

	if (bUseDetailNormal)
	{
		Optional(EMIForgeTextureType::DetailNormal);
	}

	if (!bIgnoreUnrecognizedTextures)
	{
		for (const FMIForgeTextureInfo& UnknownTexture : TextureSet.UnrecognizedTextures)
		{
			Result.UnrecognizedTextures.Add({
				UnknownTexture.AssetName,
				EMIForgeTextureType::Unknown,
				false
				});
		}

		for (const TPair<EMIForgeTextureType, FMIForgeTextureInfo>& TexturePair : TextureSet.Textures)
		{
			const FMIForgeTextureInfo& TextureInfo = TexturePair.Value;

			if (TextureInfo.TextureType == EMIForgeTextureType::Unknown ||
				TextureInfo.TextureType == EMIForgeTextureType::RGB ||
				TextureInfo.TextureType == EMIForgeTextureType::Height)
			{
				Result.UnrecognizedTextures.Add({
					TextureInfo.AssetName,
					TextureInfo.TextureType,
					false
					});
			}
		}
	}


	return Result;

}

FMIForgeTextureSetValidationResult FMIForgeValidator::ValidateRGBSet(const FMIForgeTextureSet& TextureSet, bool bUseBaseORMTexture, bool bEnableEmissiveChannel, bool bUseDetailNormalTextureRGB, bool bIgnoreUnrecognizedTextures) const
{
	FMIForgeTextureSetValidationResult Result;
	Result.SetName = TextureSet.SetName;

	auto Require = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.bCanGenerate = false;
				Result.MissingRequiredTextures.Add({
					TEXT(""),
					Type,
					true
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				true
				});
		};

	auto Optional = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.MissingOptionalTextures.Add({
					TEXT(""),
					Type,
					false
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				false
				});
		};
	Require(EMIForgeTextureType::Albedo);
	Require(EMIForgeTextureType::Normal);
	Require(EMIForgeTextureType::RGB);

	if(bUseBaseORMTexture)
	{
		Require(EMIForgeTextureType::ORM);
	}

	if(bEnableEmissiveChannel)
	{
		Optional(EMIForgeTextureType::Emissive);
	}

	if(bUseDetailNormalTextureRGB)
	{
		Optional(EMIForgeTextureType::DetailNormal);
	}

	if(!bIgnoreUnrecognizedTextures)
	{
		for (const FMIForgeTextureInfo& UnknownTexture : TextureSet.UnrecognizedTextures)
		{
			Result.UnrecognizedTextures.Add({
				UnknownTexture.AssetName,
				EMIForgeTextureType::Unknown,
				false
				});
		}

		for (const TPair<EMIForgeTextureType, FMIForgeTextureInfo>& TexturePair : TextureSet.Textures)
		{
			const FMIForgeTextureInfo& TextureInfo = TexturePair.Value;

			if (TextureInfo.TextureType == EMIForgeTextureType::Unknown ||
				TextureInfo.TextureType == EMIForgeTextureType::Height)
			{
				Result.UnrecognizedTextures.Add({
					TextureInfo.AssetName,
					TextureInfo.TextureType,
					false
					});
			}
		}
	}

	return Result;
}

FMIForgeVertexPaintLayerStackValidationResult FMIForgeValidator::ValidateVertexPaintLayerStack(const FMIForgeVertexPaintLayerStack& LayerStack, bool bIgnoreUnrecognizedTextures) const
{
	FMIForgeVertexPaintLayerStackValidationResult Result;
	bool bHasBlockingError = false;

	for (const FMIForgeVertexPaintLayerSlot* Slot : LayerStack.GetSlots())
	{
		if (!Slot)
		{
			continue;
		}

		FMIForgeVertexPaintLayerValidationResult OneLayerResult;
		OneLayerResult.Layer = Slot->Layer;
		OneLayerResult.DisplayName = Slot->DisplayName;
		OneLayerResult.bRequired = Slot->bRequired;

		if (!Slot->AssignedTextureSet.IsValid())
		{
			OneLayerResult.AssignedSetName = TEXT("Empty");
			OneLayerResult.Status = EMIForgeVertexPaintLayerStatus::Empty;

			if (Slot->bRequired)
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
			ValidateVertexPaintSet(
				TextureSet,
				bIgnoreUnrecognizedTextures
			);

		OneLayerResult.MissingRequiredTextures =
			SetResult.MissingRequiredTextures;

		OneLayerResult.MissingOptionalTextures =
			SetResult.MissingOptionalTextures;

		OneLayerResult.UnrecognizedTextures =
			SetResult.UnrecognizedTextures;

		if (Slot->Layer == EMIForgeVertexPaintLayer::LayerB)
		{
			OneLayerResult.MissingOptionalTextures.RemoveAll(
				[](const FMIForgeTextureRequirement& Requirement)
				{
					return Requirement.TextureType == EMIForgeTextureType::Height;
				}
			);
		}

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
	FMIForgeTextureSetValidationResult Result;
	Result.SetName = TextureSet.SetName;

	auto Require = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.bCanGenerate = false;
				Result.MissingRequiredTextures.Add({
					TEXT(""),
					Type,
					true
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				true
				});
		};

	auto Optional = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type);

			if (!TextureInfo)
			{
				Result.MissingOptionalTextures.Add({
					TEXT(""),
					Type,
					false
					});
				return;
			}

			Result.SuccessfulAppliedTextures.Add({
				TextureInfo->AssetName,
				Type,
				false
				});
		};

	Require(EMIForgeTextureType::Albedo);
	Optional(EMIForgeTextureType::Normal);
	Optional(EMIForgeTextureType::ORM);
	Optional(EMIForgeTextureType::Height);
	

	if (!bIgnoreUnrecognizedTextures)
	{
		for (const FMIForgeTextureInfo& UnknownTexture : TextureSet.UnrecognizedTextures)
		{
			Result.UnrecognizedTextures.Add({
				UnknownTexture.AssetName,
				EMIForgeTextureType::Unknown,
				false
				});
		}

		for (const TPair<EMIForgeTextureType, FMIForgeTextureInfo>& TexturePair : TextureSet.Textures)
		{
			const FMIForgeTextureInfo& TextureInfo = TexturePair.Value;

			if (TextureInfo.TextureType == EMIForgeTextureType::Unknown ||
				TextureInfo.TextureType == EMIForgeTextureType::RGB ||
				TextureInfo.TextureType == EMIForgeTextureType::DetailNormal ||
				TextureInfo.TextureType == EMIForgeTextureType::Emissive  )
			{
				Result.UnrecognizedTextures.Add({
					TextureInfo.AssetName,
					TextureInfo.TextureType,
					false
					});
			}
		}
	}


	return Result;
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
