// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "Generation/MIForgeGenerationCoordinator.h"
#include "ScopedTransaction.h"
#include "MIForgeGenerationUndoRecord.h"
#include "MIForgeUtilities.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "MIForgeMaterialInstanceGenerator.h"
#include "Containers/Ticker.h"

#define LOCTEXT_NAMESPACE "MIForgeGenerationCoordinator"

FMIForgeGenerationOutcome FMIForgeGenerationCoordinator::ExecuteMaterialGeneration(const FMIForgeMaterialGenerationRequest& Request) const
{
	FMIForgeMaterialInstanceGenerator Generator;

	return ExecuteGeneration(
		Request.Options.TargetPath,
		LOCTEXT(
			"GenerateMaterialInstances",
			"Generate Material Instances"),
		[&Generator, &Request]()
		{
			return Generator.GenerateMaterialInstances(Request.TextureSets, Request.Options);
		}
	);
}

FMIForgeGenerationOutcome FMIForgeGenerationCoordinator::ExecuteVertexPaintGeneration(const FMIForgeVertexPaintGenerationRequest& Request) const
{
	FMIForgeMaterialInstanceGenerator Generator;

	return ExecuteGeneration(
		Request.Options.TargetPath,
		LOCTEXT(
			"GenerateVertexPaintMI",
			"Generate Vertex Paint Material Instance"),
		[&Generator, &Request]()
		{
			return Generator.GenerateVertexPaintMaterialInstance(Request.LayerStack, Request.Options);
		}
	);
}

FMIForgeGenerationOutcome FMIForgeGenerationCoordinator::ExecuteGeneration(const FString& TargetPath, const FText& TransactionText, TFunctionRef<FMIForgeGenerationResult()> Generate) const
{
	FMIForgeGenerationOutcome Outcome;
	if (!UMIForgeGenerationUndoRecord::FlushPendingAssetChanges())
	{
		Outcome.Result.FailedCount = 1;
		Outcome.SummaryText = LOCTEXT("PendingUndoFailed", "Could not finish the previous MIForge undo. Check the Output Log before generating again.");
		Outcome.Result.Messages.Add(Outcome.SummaryText);
		return Outcome;
	}

	{
		FScopedTransaction Transaction(TransactionText);

		Outcome.Result = Generate();

		RecordCreatedAssetsForUndo(Outcome.Result.CreatedAssets);

		if (!Outcome.HasChanged())
		{
			Transaction.Cancel();
		}
	}

	LogMessages(Outcome.Result);

	if (Outcome.HasChanged())
	{
		QueueContentBrowserNavigation(TargetPath);
	}

	Outcome.SummaryText = BuildSummaryText(Outcome.Result);

	return Outcome;
}

void FMIForgeGenerationCoordinator::RecordCreatedAssetsForUndo(const TArray<UObject*>& CreatedAssets) const
{
	if (CreatedAssets.IsEmpty())
	{
		return;
	}

	UMIForgeGenerationUndoRecord* UndoRecord = 
		NewObject<UMIForgeGenerationUndoRecord>(GetTransientPackage(), 
			NAME_None, 
			RF_Transient);

	// Do not register the record's own construction as an undoable object deletion.
	UndoRecord->SetFlags(RF_Transactional);
	UndoRecord->Initialize(CreatedAssets);

}

void FMIForgeGenerationCoordinator::LogMessages(const FMIForgeGenerationResult& Result) const
{
	for (const FText& Message : Result.Messages)
	{
		FString MessageString = Message.ToString();
		MIForgeUtilities::PrintLog(MessageString, ELogVerbosity::Error);
	}
}

void FMIForgeGenerationCoordinator::QueueContentBrowserNavigation(const FString& TargetPath) const
{
	
	// Defer folder navigation to next frame for stability
	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([TargetPath](float) -> bool
		{
			FContentBrowserModule& ContentBrowserModule =
				FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

			TArray<FString> FoldersToSync;
			FoldersToSync.Add(TargetPath);

			// Navigate to folder - this is 100% crash-proof
			ContentBrowserModule.Get().SyncBrowserToFolders(FoldersToSync, false, false);

			// Optional: Log success
			//UE_LOG(LogTemp, Log, TEXT("Navigated Content Browser to: %s"), *TargetFolderPath);

			return false; // Single execution
		}),
		0.3f // Small delay for stability
		);
	
}

FText FMIForgeGenerationCoordinator::BuildSummaryText(const FMIForgeGenerationResult& Result) const
{
	return FText::FromString(FString::Printf(TEXT("Material Instances Created: %d, Updated: %d, Skipped: %d, Failed: %d."),
		Result.CreatedCount, Result.UpdatedCount, Result.SkippedCount, Result.FailedCount));
}

#undef LOCTEXT_NAMESPACE
