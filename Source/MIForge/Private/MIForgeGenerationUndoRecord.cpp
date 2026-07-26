// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeGenerationUndoRecord.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Containers/Ticker.h"
#include "Misc/TransactionObjectEvent.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

void UMIForgeGenerationUndoRecord::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);

	if (TransactionEvent.GetEventType() != ETransactionObjectEventType::UndoRedo)
	{
		return;
	}

	if (bAssetsShouldExist)
	{
		bDeleteQueued = false;  //redo is allowed to happen before the deferred undo-delete runs, but we have to cancel that pending delete to avoid deleting the assets that were just re-created.
		return;
	}

	//Improvement Start: queue deletion to next tick to avoid crash when undoing multiple times in a row.
	if (bDeleteQueued) //bDeleteQueued prevents multiple deferred delete callbacks from being queued for the same undo record.
	{
		return;
	}

	bDeleteQueued = true;

	TWeakObjectPtr<UMIForgeGenerationUndoRecord> WeakUndoRecord(this);

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakUndoRecord](float)
			{	
				UMIForgeGenerationUndoRecord* UndoRecord = WeakUndoRecord.Get();
				if (!UndoRecord)
				{
					return false;
				}

				if (UndoRecord->bAssetsShouldExist) //If redo happened, bAssetsShouldExist == true, so skip deletion.
				{
					UndoRecord->bDeleteQueued = false; //redo is allowed to happen before the deferred undo-delete runs, but we have to cancel that pending delete to avoid deleting the assets that were just re-created.
					return false;
				}

				if (!UndoRecord->bDeleteQueued) //If another path already cancelled the queued delete, skip deletion.
				{
					return false;
				}

				UndoRecord->bDeleteQueued = false;

				FAssetRegistryModule& AssetRegistryModule =
					FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

				IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

				TArray<UObject*> ObjectsToDelete;
				TSet<FString> UniquePathsToScan;

				for (const FSoftObjectPath& Path : UndoRecord->CreatedAssetPaths)
				{
					UObject* Asset = nullptr;
					FString PackagePathToScan;

					// Prefer Asset Registry lookup first.
					const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(
						FSoftObjectPath(Path.GetAssetPathString())
					);

					if (AssetData.IsValid())
					{	// Asset is valid and registered in the Asset Registry, so we can safely get it.
						Asset = AssetData.GetAsset();

						PackagePathToScan = FPackageName::GetLongPackagePath(
							AssetData.PackageName.ToString()
						);
					}
					else
					{
						// Fallback: resolve loaded object from soft path.
						Asset = Path.ResolveObject();

						const FString AssetPathString = Path.GetAssetPathString();

						FString PackageName;
						FString AssetName;

						if (AssetPathString.Split(TEXT("."), &PackageName, &AssetName))
						{
							PackagePathToScan = FPackageName::GetLongPackagePath(PackageName);
						}
						else
						{
							PackagePathToScan = FPackageName::GetLongPackagePath(AssetPathString);
						}
					}

					if (!Asset || !IsValid(Asset) || Asset->IsUnreachable())
					{
						continue;
					}

					UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(Asset);
					if (!MIC)
					{
						continue;
					}

					if (!MIC->GetName().StartsWith(TEXT("MI_")))
					{
						continue;
					}

					ObjectsToDelete.AddUnique(MIC);

					if (!PackagePathToScan.IsEmpty())
					{
						UniquePathsToScan.Add(PackagePathToScan);
					}
				}

				if (ObjectsToDelete.Num() > 0)
				{	
					

					// Important:
					// Notify Asset Registry before DeleteObjectsUnchecked.
					// In my case, skipping this will cause crash / stale registry state.
					for (UObject* Object : ObjectsToDelete)
					{
						if (Object && IsValid(Object) && !Object->IsUnreachable())
						{
							AssetRegistry.AssetDeleted(Object);
						}
					}

					ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);
				}

				if (UniquePathsToScan.Num() > 0)
				{
					const TArray<FString> PathsToScan = UniquePathsToScan.Array();

					AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
				}

				return false; //only run once
			}),
		0.0f
	);
}
