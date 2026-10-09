// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeGenerationUndoRecord.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/TransactionObjectEvent.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY_STATIC(LogMIForgeUndo, Log, All);

namespace MIForgeUndo
{
	TArray<TWeakObjectPtr<UMIForgeGenerationUndoRecord>> PendingRecords;

	constexpr EObjectFlags AssetFlags = RF_Public | RF_Standalone | RF_Transient;
	constexpr ERenameFlags RenameFlags = REN_DontCreateRedirectors | REN_NonTransactional | REN_DoNotDirty;

	bool RestoreFiles(const TArray<FMIForgeUndoPackageFile>& Files)
	{
		// Never overwrite files produced by another operation while this MI was undone.
		TArray<int32> MissingFiles;
		int32 FileIndex = 0;
		for (const FMIForgeUndoPackageFile& File : Files)
		{
			if (IFileManager::Get().FileExists(*File.Filename))
			{
				TArray<uint8> ExistingContents;
				if (!FFileHelper::LoadFileToArray(ExistingContents, *File.Filename) || ExistingContents != File.Contents)
				{
					UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot restore MI: a different file already exists: %s"), *File.Filename);
					return false;
				}
			}
			else
			{
				MissingFiles.Add(FileIndex);
			}
			++FileIndex;
		}

		for (int32 Index = 0; Index < MissingFiles.Num(); ++Index)
		{
			const FMIForgeUndoPackageFile& File = Files[MissingFiles[Index]];
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(File.Filename), true);
			if (!FFileHelper::SaveArrayToFile(File.Contents, *File.Filename))
			{
				for (int32 WrittenIndex = 0; WrittenIndex <= Index; ++WrittenIndex)
				{
					IFileManager::Get().Delete(*Files[MissingFiles[WrittenIndex]].Filename, false, false, true);
				}
				UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot restore MI file: %s"), *File.Filename);
				return false;
			}
		}
		return true;
	}

	bool RetireFiles(FMIForgeCreatedAssetUndoState& State)
	{
		// A previous I/O failure may have left a partial retirement; recover it first.
		if (!RestoreFiles(State.SavedFiles))
		{
			return false;
		}
		State.SavedFiles.Reset();
		FString Filename;
		if (!FPackageName::DoesPackageExist(State.OriginalPackage->GetName(), &Filename))
		{
			return true;
		}

		TArray<FMIForgeUndoPackageFile> Files;
		for (const TCHAR* Extension : { TEXT("uasset"), TEXT("uexp"), TEXT("ubulk"), TEXT("uptnl") })
		{
			const FString PartFilename = FPaths::ChangeExtension(Filename, Extension);
			if (!IFileManager::Get().FileExists(*PartFilename))
			{
				continue;
			}
			FMIForgeUndoPackageFile& File = Files.AddDefaulted_GetRef();
			File.Filename = PartFilename;
			if (IFileManager::Get().IsReadOnly(*PartFilename) ||
				!FFileHelper::LoadFileToArray(File.Contents, *PartFilename))
			{
				UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot back up MI file for undo: %s"), *PartFilename);
				return false;
			}
		}

		State.SavedFiles = MoveTemp(Files);
		for (const FMIForgeUndoPackageFile& File : State.SavedFiles)
		{
			if (!IFileManager::Get().Delete(*File.Filename, true, false, true))
			{
				UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot remove MI file for undo: %s"), *File.Filename);
				if (RestoreFiles(State.SavedFiles))
				{
					State.SavedFiles.Reset();
				}
				return false;
			}
		}
		return true;
	}
}

void UMIForgeGenerationUndoRecord::Initialize(const TArray<UObject*>& CreatedAssets)
{
	for (UObject* Asset : CreatedAssets)
	{
		UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(Asset);
		if (!IsValid(MaterialInstance) || !MaterialInstance->IsAsset())
		{
			continue;
		}
		FMIForgeCreatedAssetUndoState& State = Assets.AddDefaulted_GetRef();
		State.MaterialInstance = MaterialInstance;
		State.OriginalPackage = MaterialInstance->GetPackage();
		State.OriginalName = MaterialInstance->GetFName();
		State.OriginalAssetFlags = MaterialInstance->GetFlags() & MIForgeUndo::AssetFlags;
	}

	if (!Assets.IsEmpty())
	{
		Modify();
		bAssetsShouldExist = true;
	}
}

void UMIForgeGenerationUndoRecord::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);
	if (TransactionEvent.GetEventType() != ETransactionObjectEventType::UndoRedo || bSyncQueued)
	{
		return;
	}

	bSyncQueued = true;
	MIForgeUndo::PendingRecords.AddUnique(this);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[WeakRecord = TWeakObjectPtr<UMIForgeGenerationUndoRecord>(this)](float)
		{
			if (GIsTransacting)
			{
				return true;
			}
			if (WeakRecord.IsValid() && WeakRecord->bSyncQueued)
			{
				FlushPendingAssetChanges();
			}
			return false;
		}));
}

bool UMIForgeGenerationUndoRecord::FlushPendingAssetChanges()
{
	check(IsInGameThread());
	if (GIsTransacting)
	{
		return false;
	}
	TArray<TWeakObjectPtr<UMIForgeGenerationUndoRecord>> Records = MoveTemp(MIForgeUndo::PendingRecords);
	MIForgeUndo::PendingRecords.Reset();
	bool bSucceeded = true;
	for (const TWeakObjectPtr<UMIForgeGenerationUndoRecord>& WeakRecord : Records)
	{
		if (UMIForgeGenerationUndoRecord* Record = WeakRecord.Get())
		{
			Record->bSyncQueued = false;
			if (!Record->SynchronizeAssets())
			{
				// Keep a failed operation available for retry, without ticking indefinitely.
				MIForgeUndo::PendingRecords.AddUnique(Record);
				bSucceeded = false;
			}
		}
	}
	return bSucceeded;
}

bool UMIForgeGenerationUndoRecord::SynchronizeAssets()
{
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	bool bSucceeded = true;
	for (FMIForgeCreatedAssetUndoState& State : Assets)
	{
		UMaterialInstanceConstant* MI = State.MaterialInstance;
		if (!IsValid(MI) || !IsValid(State.OriginalPackage))
		{
			UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot restore MI undo state: recorded object is invalid."));
			bSucceeded = false;
			continue;
		}

		// Transaction deserialization may itself restore an object's name and outer.
		const FString OriginalName = State.OriginalName.ToString();
		if (bAssetsShouldExist || State.bPublished)
		{
			UObject* Occupant = StaticFindObjectFast(UObject::StaticClass(), State.OriginalPackage, State.OriginalName);
			if ((Occupant && Occupant != MI) ||
				!MI->Rename(*OriginalName, State.OriginalPackage, MIForgeUndo::RenameFlags | REN_Test))
			{
				UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot restore MI undo state: name is occupied: %s.%s"),
					*State.OriginalPackage->GetName(), *OriginalName);
				bSucceeded = false;
				continue;
			}
		}

		if (bAssetsShouldExist)
		{
			if (!MIForgeUndo::RestoreFiles(State.SavedFiles))
			{
				bSucceeded = false;
				continue;
			}
			MI->Rename(*OriginalName, State.OriginalPackage, MIForgeUndo::RenameFlags);
			MI->ClearFlags(MIForgeUndo::AssetFlags);
			MI->SetFlags(State.OriginalAssetFlags);
			if (!State.bPublished)
			{
				Registry.AssetCreated(MI);
			}
			State.SavedFiles.Reset();
			State.bPublished = true;
			MI->MarkPackageDirty();
			continue;
		}

		if (State.bPublished)
		{
			if (!UPackage::IsEmptyPackage(State.OriginalPackage, MI))
			{
				UE_LOG(LogMIForgeUndo, Error, TEXT("Cannot undo MI creation: package contains other assets: %s"),
					*State.OriginalPackage->GetName());
				bSucceeded = false;
				continue;
			}
			if (GEditor)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(MI);
				GEditor->GetSelectedObjects()->Deselect(MI);
			}
			if (!MIForgeUndo::RetireFiles(State))
			{
				bSucceeded = false;
				continue;
			}
			MI->Rename(*OriginalName, State.OriginalPackage, MIForgeUndo::RenameFlags);
			MI->ClearFlags(MIForgeUndo::AssetFlags);
			MI->SetFlags(State.OriginalAssetFlags);
			// AssetDeleted requires IsAsset() and the original path to notify the browser.
			Registry.AssetDeleted(MI);
		}

		MI->ClearFlags(RF_Public | RF_Standalone);
		MI->SetFlags(RF_Transient);
		if (MI->GetOuter() != GetTransientPackage())
		{
			const FName ParkedName = MakeUniqueObjectName(GetTransientPackage(), MI->GetClass(), State.OriginalName);
			MI->Rename(*ParkedName.ToString(), GetTransientPackage(), MIForgeUndo::RenameFlags);
		}
		if (State.bPublished && UPackage::IsEmptyPackage(State.OriginalPackage))
		{
			Registry.PackageDeleted(State.OriginalPackage);
			State.OriginalPackage->SetDirtyFlag(false);
		}
		State.bPublished = false;
	}
	return bSucceeded;
}
