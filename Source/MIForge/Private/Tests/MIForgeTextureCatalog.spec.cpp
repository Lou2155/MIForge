// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Catalog/MIForgeTextureCatalog.h"

DEFINE_SPEC(
	FMIForgeTextureCatalogSpec,
	"MIForge.Unit.TextureCatalog",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeTextureCatalogSpec::Define()
{
	Describe("Lifecycle", [this]()
	{
		It("should preserve folder scope and publish a complete refresh", [this]()
		{
			const TSharedRef<FMIForgeTextureCatalog> Catalog =
				MakeShared<FMIForgeTextureCatalog>();
			TArray<FString> EventOrder;

			Catalog->OnRefreshStarted.AddLambda(
				[&EventOrder]()
				{
					EventOrder.Add(TEXT("Started"));
				});
			Catalog->OnCatalogChanged.AddLambda(
				[&EventOrder]()
				{
					EventOrder.Add(TEXT("Changed"));
				});

			const TArray<FString> FolderPaths = {
				TEXT("/Game/MIForgeAutomation/CatalogDoesNotExist")
			};
			Catalog->Initialize(FolderPaths);

			TestEqual(TEXT("Folder count"), Catalog->GetFolderPaths().Num(), 1);
			TestEqual(
				TEXT("Stored folder"),
				Catalog->GetFolderPaths()[0],
				FolderPaths[0]);
			TestEqual(TEXT("Event count"), EventOrder.Num(), 2);
			if (EventOrder.Num() == 2)
			{
				TestEqual(TEXT("Refresh starts first"), EventOrder[0], FString(TEXT("Started")));
				TestEqual(TEXT("Catalog changes last"), EventOrder[1], FString(TEXT("Changed")));
			}

			Catalog->Shutdown();
		});

		It("should emit one start and one change notification for a direct refresh", [this]()
		{
			const TSharedRef<FMIForgeTextureCatalog> Catalog =
				MakeShared<FMIForgeTextureCatalog>();
			Catalog->Initialize({});

			int32 RefreshStartedCount = 0;
			int32 CatalogChangedCount = 0;
			Catalog->OnRefreshStarted.AddLambda(
				[&RefreshStartedCount]()
				{
					++RefreshStartedCount;
				});
			Catalog->OnCatalogChanged.AddLambda(
				[&CatalogChangedCount]()
				{
					++CatalogChangedCount;
				});

			Catalog->Refresh();

			TestEqual(TEXT("Refresh-start notifications"), RefreshStartedCount, 1);
			TestEqual(TEXT("Catalog-changed notifications"), CatalogChangedCount, 1);

			Catalog->Shutdown();
		});

		It("should allow shutdown to be called more than once", [this]()
		{
			const TSharedRef<FMIForgeTextureCatalog> Catalog =
				MakeShared<FMIForgeTextureCatalog>();
			Catalog->Initialize({});

			Catalog->Shutdown();
			Catalog->Shutdown();

			TestTrue(TEXT("Catalog remains valid"), Catalog->GetFolderPaths().IsEmpty());
		});
	});
}

#endif
