// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Editor/TransBuffer.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture2D.h"
#include "Generation/MIForgeGenerationCoordinator.h"
#include "Generation/MIForgeMaterialInstanceResolver.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MIForgeGenerationUndoRecord.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "ScopedTransaction.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

namespace MIForgeGenerationUndoSpec
{
	class FUndoContext
	{
	public:
		FUndoContext()
			: PreviousTransactor(GEditor->Trans)
			, TestTransactor(NewObject<UTransBuffer>())
			, Registry(FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get())
			, TargetPath(TEXT("/Game/MIForgeAutomation/Undo_") + FGuid::NewGuid().ToString(EGuidFormats::Digits))
		{
			UMIForgeGenerationUndoRecord::FlushPendingAssetChanges();
			TestTransactor->Initialize(32 * 1024 * 1024);
			GEditor->Trans = TestTransactor.Get();
			RemovedHandle = Registry.OnAssetRemoved().AddLambda([this](const FAssetData& Asset)
			{
				if (Asset.PackagePath.ToString() == TargetPath)
				{
					RemovedPaths.Add(Asset.GetObjectPathString());
				}
			});
		}

		~FUndoContext()
		{
			UMIForgeGenerationUndoRecord::FlushPendingAssetChanges();
			Registry.OnAssetRemoved().Remove(RemovedHandle);
			// Delete only this test's GUID folder, using the isolated transaction buffer.
			UEditorAssetLibrary::DeleteDirectory(TargetPath);
			TestTransactor->Reset(FText::FromString(TEXT("MIForge undo test complete")));
			GEditor->Trans = PreviousTransactor.Get();
		}

		FMIForgeMaterialGenerationRequest Request(const TArray<FString>& SetNames = { TEXT("UndoA") })
		{
			FMIForgeMaterialGenerationRequest Result;
			Result.Options.Preset = EMIForgeGenerationPreset::Decal;
			Result.Options.TargetPath = TargetPath;
			Result.Options.IfMIExists = EIfMIExistsOption::Skip;
			UTexture2D* Texture = LoadObject<UTexture2D>(
				nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
			for (const FString& Name : SetNames)
			{
				TSharedPtr<FMIForgeTextureSet> Set = MakeShared<FMIForgeTextureSet>();
				Set->SetName = Name;
				FMIForgeTextureInfo Info;
				Info.AssetData = FAssetData(Texture);
				Info.TextureType = EMIForgeTextureType::Albedo;
				Set->Textures.Add(EMIForgeTextureType::Albedo, Info);
				Result.TextureSets.Add(Set);
			}
			return Result;
		}

		FString Path(const FString& SetName = TEXT("UndoA")) const
		{
			const FString AssetName = TEXT("MI_Decal_") + SetName;
			return TargetPath / AssetName + TEXT(".") + AssetName;
		}

		bool Exists(const FString& SetName = TEXT("UndoA")) const
		{
			return Registry.GetAssetByObjectPath(FSoftObjectPath(Path(SetName))).IsValid();
		}

		TStrongObjectPtr<UTransactor> PreviousTransactor;
		TStrongObjectPtr<UTransBuffer> TestTransactor;
		IAssetRegistry& Registry;
		FString TargetPath;
		TSet<FString> RemovedPaths;
		FDelegateHandle RemovedHandle;
	};
}

DEFINE_SPEC(
	FMIForgeGenerationUndoSpec,
	"MIForge.Integration.GenerationUndo",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FMIForgeGenerationUndoSpec::Define()
{
	using namespace MIForgeGenerationUndoSpec;

	It("should remove browser entries and recreate the same name repeatedly", [this]()
	{
		FUndoContext Context;
		for (int32 Iteration = 0; Iteration < 3; ++Iteration)
		{
			const FMIForgeGenerationOutcome Outcome =
				FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Context.Request());
			TestEqual(TEXT("Created without an overwrite dialog"), Outcome.Result.CreatedCount, 1);
			TestEqual(TEXT("No generation failures"), Outcome.Result.FailedCount, 0);
			if (Outcome.Result.CreatedAssets.Num() != 1)
			{
				return;
			}
			TestTrue(TEXT("Undo succeeds"), GEditor->UndoTransaction());
			FTSTicker::GetCoreTicker().Tick(0.0f);
			TestFalse(TEXT("Asset is absent without navigating folders"), Context.Exists());
			TestTrue(TEXT("Browser removal notification uses original path"), Context.RemovedPaths.Contains(Context.Path()));
			TestNull(TEXT("Original UObject name is free"), StaticFindObject(UObject::StaticClass(), nullptr, *Context.Path()));
		}
	});

	It("should restore the same MI and its parameters through repeated undo redo and GC", [this]()
	{
		FUndoContext Context;
		const FMIForgeGenerationOutcome Outcome =
			FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Context.Request());
		TestEqual(TEXT("Created count"), Outcome.Result.CreatedCount, 1);
		if (Outcome.Result.CreatedAssets.Num() != 1)
		{
			return;
		}
		TWeakObjectPtr<UMaterialInstanceConstant> MI = Cast<UMaterialInstanceConstant>(Outcome.Result.CreatedAssets[0]);
		for (int32 Iteration = 0; Iteration < 3; ++Iteration)
		{
			TestTrue(TEXT("Undo succeeds"), GEditor->UndoTransaction());
			TestTrue(TEXT("Deferred undo succeeds"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
			TestTrue(TEXT("Undo record retains the actual MI across GC"), MI.IsValid());
			TestFalse(TEXT("Undone asset absent"), Context.Exists());
			TestTrue(TEXT("Redo succeeds"), GEditor->RedoTransaction());
			TestTrue(TEXT("Deferred redo succeeds"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
			TestTrue(TEXT("Redone asset registered"), Context.Exists());
			TestTrue(TEXT("Same object returns to original path"),
				StaticFindObject(UObject::StaticClass(), nullptr, *Context.Path()) == MI.Get());
			if (MI.IsValid())
			{
				TestNotNull(TEXT("Albedo assignment restored"),
					UMaterialEditingLibrary::GetMaterialInstanceTextureParameterValue(MI.Get(), TEXT("Albedo")));
				TestNotNull(TEXT("Parent restored"), MI->Parent.Get());
			}
		}
	});

	It("should flush pending undo before immediate regeneration discards redo history", [this]()
	{
		FUndoContext Context;
		FMIForgeGenerationCoordinator Coordinator;
		TestEqual(TEXT("Initial creation"), Coordinator.ExecuteMaterialGeneration(Context.Request()).Result.CreatedCount, 1);
		TestTrue(TEXT("Undo succeeds"), GEditor->UndoTransaction());
		TestEqual(TEXT("Recreation before ticker"), Coordinator.ExecuteMaterialGeneration(Context.Request()).Result.CreatedCount, 1);
		TestTrue(TEXT("Second creation still undoable"), GEditor->UndoTransaction());
		FTSTicker::GetCoreTicker().Tick(0.0f);
		TestFalse(TEXT("Second creation removed"), Context.Exists());
	});

	It("should coalesce rapid undo redo and support multiple batches", [this]()
	{
		FUndoContext Context;
		FMIForgeGenerationCoordinator Coordinator;
		TestEqual(TEXT("First batch"), Coordinator.ExecuteMaterialGeneration(Context.Request({ TEXT("UndoA"), TEXT("UndoB") })).Result.CreatedCount, 2);
		TestEqual(TEXT("Second batch"), Coordinator.ExecuteMaterialGeneration(Context.Request({ TEXT("UndoC") })).Result.CreatedCount, 1);
		TestTrue(TEXT("Undo second batch"), GEditor->UndoTransaction());
		TestTrue(TEXT("Redo before ticker"), GEditor->RedoTransaction());
		FTSTicker::GetCoreTicker().Tick(0.0f);
		TestTrue(TEXT("Quick redo preserves asset"), Context.Exists(TEXT("UndoC")));
		TestTrue(TEXT("Undo second batch again"), GEditor->UndoTransaction());
		TestTrue(TEXT("Undo first batch before ticker"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush both records"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestFalse(TEXT("A absent"), Context.Exists(TEXT("UndoA")));
		TestFalse(TEXT("B absent"), Context.Exists(TEXT("UndoB")));
		TestFalse(TEXT("C absent"), Context.Exists(TEXT("UndoC")));
		TestTrue(TEXT("Redo first batch"), GEditor->RedoTransaction());
		TestTrue(TEXT("Redo second batch"), GEditor->RedoTransaction());
		TestTrue(TEXT("Flush both redos"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestTrue(TEXT("A restored"), Context.Exists(TEXT("UndoA")));
		TestTrue(TEXT("B restored"), Context.Exists(TEXT("UndoB")));
		TestTrue(TEXT("C restored"), Context.Exists(TEXT("UndoC")));
	});

	It("should undo and redo a saved MI without rediscovering it on disk", [this]()
	{
		FUndoContext Context;
		const FMIForgeGenerationOutcome Outcome =
			FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Context.Request());
		TestEqual(TEXT("Created count"), Outcome.Result.CreatedCount, 1);
		if (Outcome.Result.CreatedAssets.Num() != 1)
		{
			return;
		}
		UObject* MI = Outcome.Result.CreatedAssets[0];
		TestTrue(TEXT("Save MI"), UEditorAssetLibrary::SaveLoadedAsset(MI, false));
		const FString Filename = FPackageName::LongPackageNameToFilename(
			MI->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
		Context.Registry.ScanFilesSynchronous({ Filename }, true);
		TArray<uint8> SavedContents;
		TestTrue(TEXT("Saved bytes readable"), FFileHelper::LoadFileToArray(SavedContents, *Filename));
		TestTrue(TEXT("Undo saved asset"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush saved undo"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestFalse(TEXT("File removed while undone"), IFileManager::Get().FileExists(*Filename));
		Context.Registry.ScanPathsSynchronous({ Context.TargetPath }, true);
		TestFalse(TEXT("Scan cannot resurrect saved asset"), Context.Exists());
		TestTrue(TEXT("Redo saved asset"), GEditor->RedoTransaction());
		TestTrue(TEXT("Flush saved redo"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TArray<uint8> RestoredContents;
		TestTrue(TEXT("File restored"), FFileHelper::LoadFileToArray(RestoredContents, *Filename));
		TestTrue(TEXT("Disk contents restored exactly"), RestoredContents == SavedContents);
		TestTrue(TEXT("Restored asset visible"), Context.Exists());
	});

	It("should preserve a read-only saved file and retry undo after it becomes writable", [this]()
	{
		FUndoContext Context;
		const FMIForgeGenerationOutcome Outcome =
			FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Context.Request());
		if (!TestEqual(TEXT("Created count"), Outcome.Result.CreatedCount, 1))
		{
			return;
		}
		UObject* MI = Outcome.Result.CreatedAssets[0];
		TestTrue(TEXT("Save MI"), UEditorAssetLibrary::SaveLoadedAsset(MI, false));
		const FString Filename = FPackageName::LongPackageNameToFilename(
			MI->GetPackage()->GetName(), FPackageName::GetAssetPackageExtension());
		TArray<uint8> Before;
		FFileHelper::LoadFileToArray(Before, *Filename);
		TestTrue(TEXT("Set read-only"), FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Filename, true));
		TestTrue(TEXT("Undo transaction"), GEditor->UndoTransaction());
		AddExpectedError(TEXT("Cannot back up MI file for undo:"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse(TEXT("Reports failure rather than deleting read-only file"),
			UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TArray<uint8> After;
		TestTrue(TEXT("File retained"), FFileHelper::LoadFileToArray(After, *Filename));
		TestTrue(TEXT("File unchanged"), Before == After);
		TestTrue(TEXT("Make writable"), FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Filename, false));
		TestTrue(TEXT("Failed undo can be retried"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestFalse(TEXT("Retry releases the asset name"), Context.Exists());
		TestTrue(TEXT("Redo after retry"), GEditor->RedoTransaction());
		TestTrue(TEXT("Restore succeeds"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestTrue(TEXT("MI restored"), Context.Exists());
	});

	It("should keep ordinary overwrite undo independent of creation undo", [this]()
	{
		FUndoContext Context;
		FMIForgeGenerationCoordinator Coordinator;
		const FMIForgeGenerationOutcome Created = Coordinator.ExecuteMaterialGeneration(Context.Request());
		if (!TestEqual(TEXT("Initial creation"), Created.Result.CreatedCount, 1))
		{
			return;
		}
		UMaterialInstanceConstant* MI = Cast<UMaterialInstanceConstant>(Created.Result.CreatedAssets[0]);
		FMIForgeMaterialGenerationRequest Request = Context.Request();
		Request.Options.IfMIExists = EIfMIExistsOption::Overwrite;
		Request.Options.bUseOrientationMask = true;
		TestEqual(TEXT("Updated count"), Coordinator.ExecuteMaterialGeneration(Request).Result.UpdatedCount, 1);
		TestTrue(TEXT("Overwrite applied"), UMaterialEditingLibrary::GetMaterialInstanceStaticSwitchParameterValue(MI, TEXT("UseOrientationMask?")));
		TestTrue(TEXT("Undo overwrite"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestTrue(TEXT("Original MI still exists"), Context.Exists());
		TestFalse(TEXT("Original parameters restored"), UMaterialEditingLibrary::GetMaterialInstanceStaticSwitchParameterValue(MI, TEXT("UseOrientationMask?")));
		TestTrue(TEXT("Undo original creation"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush creation undo"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestFalse(TEXT("Original creation now removed"), Context.Exists());
	});

	It("should reject an unregistered name collision without opening the engine overwrite dialog", [this]()
	{
		FUndoContext Context;
		const FString PackageName = Context.TargetPath / TEXT("Hidden");
		UPackage* Package = CreatePackage(*PackageName);
		TStrongObjectPtr<UMaterialInstanceConstant> Hidden(NewObject<UMaterialInstanceConstant>(Package, TEXT("Hidden"), RF_Transient));
		FMIForgeMaterialInstanceTarget Target;
		Target.TargetPath = Context.TargetPath;
		Target.AssetName = TEXT("Hidden");
		Target.ParentMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MIForge/MasterMaterialPresets/MM_Decal.MM_Decal"));
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		const FMIForgeMaterialInstanceResolution Resolution = FMIForgeMaterialInstanceResolver().Resolve(Target, AssetTools);
		TestTrue(TEXT("Collision rejected"), Resolution.Action == EMIForgeGenerationAction::Failed);
		TestTrue(TEXT("Reason identifies occupied name"), Resolution.Message.ToString().Contains(TEXT("occupied")));
		TestTrue(TEXT("Existing object unchanged"), IsValid(Hidden.Get()));
	});

	It("should not rename or overwrite another object occupying the redo target", [this]()
	{
		FUndoContext Context;
		const FMIForgeGenerationOutcome Outcome =
			FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Context.Request());
		if (!TestEqual(TEXT("Created count"), Outcome.Result.CreatedCount, 1))
		{
			return;
		}
		UMaterialInstanceConstant* MI = Cast<UMaterialInstanceConstant>(Outcome.Result.CreatedAssets[0]);
		UPackage* OriginalPackage = MI->GetPackage();
		const FName OriginalName = MI->GetFName();
		TestTrue(TEXT("Undo creation"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush undo"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		// Simulate another tool creating an object without recording a transaction.
		TStrongObjectPtr<UMaterialInstanceConstant> Occupant(
			NewObject<UMaterialInstanceConstant>(OriginalPackage, OriginalName, RF_Public | RF_Standalone));
		Context.Registry.AssetCreated(Occupant.Get());
		TestTrue(TEXT("Redo transaction"), GEditor->RedoTransaction());
		AddExpectedError(TEXT("Cannot restore MI undo state: name is occupied:"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse(TEXT("Conflicting restoration rejected"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestEqual(TEXT("Other object's original path preserved"), Occupant->GetPathName(), Context.Path());
		TestTrue(TEXT("Original MI remains parked"), MI->GetOuter() == GetTransientPackage());
		Context.Registry.AssetDeleted(Occupant.Get());
		Occupant->ClearFlags(RF_Public | RF_Standalone);
		Occupant->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
		TestTrue(TEXT("Restore can retry after conflict clears"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestEqual(TEXT("Original MI restored"), MI->GetPathName(), Context.Path());
	});

	It("should undo recorded material names without an MI prefix", [this]()
	{
		FUndoContext Context;
		const FString ObjectPath = Context.TargetPath / TEXT("Custom.Custom");
		{
			FScopedTransaction Transaction(FText::FromString(TEXT("Custom MI")));
			FMIForgeMaterialInstanceTarget Target;
			Target.TargetPath = Context.TargetPath;
			Target.AssetName = TEXT("Custom");
			Target.ParentMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MIForge/MasterMaterialPresets/MM_Decal.MM_Decal"));
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
			const FMIForgeMaterialInstanceResolution Resolution = FMIForgeMaterialInstanceResolver().Resolve(Target, AssetTools);
			if (!TestNotNull(TEXT("Custom MI created"), Resolution.MaterialInstance))
			{
				return;
			}
			Resolution.MaterialInstance->SetFlags(RF_Transactional);
			UMIForgeGenerationUndoRecord* Record = NewObject<UMIForgeGenerationUndoRecord>(GetTransientPackage(), NAME_None, RF_Transient);
			Record->SetFlags(RF_Transactional);
			Record->Initialize({ Resolution.MaterialInstance });
		}
		TestTrue(TEXT("Undo custom MI"), GEditor->UndoTransaction());
		TestTrue(TEXT("Flush custom undo"), UMIForgeGenerationUndoRecord::FlushPendingAssetChanges());
		TestFalse(TEXT("No prefix required for removal"), Context.Registry.GetAssetByObjectPath(FSoftObjectPath(ObjectPath)).IsValid());
		TestNull(TEXT("Custom name freed"), StaticFindObject(UObject::StaticClass(), nullptr, *ObjectPath));
	});
}

#endif
