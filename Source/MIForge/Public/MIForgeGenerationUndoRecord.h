// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MIForgeGenerationUndoRecord.generated.h"

class UMaterialInstanceConstant;
class UPackage;

struct FMIForgeUndoPackageFile
{
	FString Filename;
	TArray<uint8> Contents;
};

USTRUCT()
struct FMIForgeCreatedAssetUndoState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UMaterialInstanceConstant> MaterialInstance;

	UPROPERTY()
	TObjectPtr<UPackage> OriginalPackage;

	FName OriginalName;
	EObjectFlags OriginalAssetFlags = RF_NoFlags;
	bool bPublished = true;
	TArray<FMIForgeUndoPackageFile> SavedFiles;
};

UCLASS()
class MIFORGE_API UMIForgeGenerationUndoRecord : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(const TArray<UObject*>& CreatedAssets);

	// Finish deferred undo work before a new transaction discards redo history.
	static bool FlushPendingAssetChanges();

	virtual void PostTransacted(
		const FTransactionObjectEvent& TransactionEvent
	) override;

private:
	UPROPERTY()
	bool bAssetsShouldExist = false;

	// Preserve object identity and file backups across both transaction directions.
	UPROPERTY(Transient, NonTransactional)
	TArray<FMIForgeCreatedAssetUndoState> Assets;

	bool bSyncQueued = false;
	bool SynchronizeAssets();
};
