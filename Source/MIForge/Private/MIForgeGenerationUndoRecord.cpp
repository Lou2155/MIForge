// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeGenerationUndoRecord.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Containers/Ticker.h"
#include "Misc/TransactionObjectEvent.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"

void UMIForgeGenerationUndoRecord::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);

	if (TransactionEvent.GetEventType() != ETransactionObjectEventType::UndoRedo)
	{
		return;
	}

	if (bAssetsShouldExist)
	{
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

				if (UndoRecord->bAssetsShouldExist || !UndoRecord->bDeleteQueued) //If redo happened, bAssetsShouldExist == true, so skip deletion.|| If another path already cancelled the queued delete, skip deletion.
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
					{
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
