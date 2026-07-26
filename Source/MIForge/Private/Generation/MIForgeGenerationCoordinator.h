// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

struct FMIForgeMaterialGenerationRequest
{
	TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets;
	FMIForgeGenerationOptions Options;
};

struct FMIForgeVertexPaintGenerationRequest
{
	FMIForgeVertexPaintLayerStack LayerStack;
	FMIForgeVertexPaintGenerationOptions Options;
};

struct FMIForgeGenerationOutcome
{
	FMIForgeGenerationResult Result;
	FText SummaryText;

	bool HasChanged() const
	{
		return Result.CreatedCount > 0 || Result.UpdatedCount > 0 ;
	}
};

class FMIForgeGenerationCoordinator
{
public:
	FMIForgeGenerationOutcome ExecuteMaterialGeneration(const FMIForgeMaterialGenerationRequest& Request) const;
	FMIForgeGenerationOutcome ExecuteVertexPaintGeneration(const FMIForgeVertexPaintGenerationRequest& Request) const;
private:
	FMIForgeGenerationOutcome ExecuteGeneration(
		const FString& TargetPath,
		const FText& TransactionText,
		TFunctionRef<FMIForgeGenerationResult()> Generate
	) const;

	void RecordCreatedAssetsForUndo(
		const TArray<UObject*>& CreatedAssets) const;

	void LogMessages(
		const FMIForgeGenerationResult& Result) const;

	void QueueContentBrowserNavigation(
		const FString& TargetPath) const;

	FText BuildSummaryText(
		const FMIForgeGenerationResult& Result) const;
};
