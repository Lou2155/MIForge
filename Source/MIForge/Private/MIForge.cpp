// Copyright Epic Games, Inc. All Rights Reserved.

#include "MIForge.h"
#include "ContentBrowserModule.h"
#include "MainTabWidget.h"
//#include "Framework/Docking/TabManager.h"
#include "MIForgeStyle.h"
#include "MIForgeUtilities.h"

#define LOCTEXT_NAMESPACE "FMIForgeModule"

const FName FMIForgeModule::MIForgeMainTabId = FName("MIForgeMainTab");

void FMIForgeModule::StartupModule()
{	
	FMIForgeStyle::Initialize();
	InitializeMenuExtension();
	RegisterTabSpawner();
}

void FMIForgeModule::ShutdownModule()
{	
	// TODO:
	// Store delegate handle and unregister
	// ContentBrowser menu extension in Shutdown.
	// (Not necessary for v1.0)
	UnregisterTabSpawner();
	FMIForgeStyle::Shutdown();
}

void FMIForgeModule::InitializeMenuExtension()
{
	// add right-click menu entry to content browser

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	TArray<FContentBrowserMenuExtender_SelectedPaths>& MenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//Delegate for menu extender, will be called when right-clicking in the content browser
	MenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FMIForgeModule::CustomMenuExtender));
}

TSharedRef<FExtender> FMIForgeModule::CustomMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShared<FExtender>();

	MenuExtender->AddMenuExtension(
		FName("AddToFavorites"),
		EExtensionHook::After,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FMIForgeModule::MenuEntry)
	);

	SelectedFolderPaths = SelectedPaths;

	return MenuExtender;
}

void FMIForgeModule::MenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Open MIForge Tab"),
		FText::FromString("Open the MIForge tab to manage your assets."),
		FSlateIcon(FMIForgeStyle::GetStyleSetName(), "Panel.FolderSelection"),
		FExecuteAction::CreateLambda([this]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(MIForgeMainTabId);
			}
		)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Fix Texture Compression"),
		FText::FromString("Fix the texture compression settings in the folder."),
		FSlateIcon(),
		FExecuteAction::CreateLambda([this]()
			{
				// Call the function to fix texture compression settings
				MIForgeUtilities::FixTextureCompressionInSelectedFolders(SelectedFolderPaths);
			}
		)
	);
}

void FMIForgeModule::RegisterTabSpawner() {
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MIForgeMainTabId, 
		FOnSpawnTab::CreateRaw(this, &FMIForgeModule::OnSpawnMIForgeMainTab))
		.SetDisplayName(FText::FromString("MIForge"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMIForgeModule::UnregisterTabSpawner() {
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MIForgeMainTabId);
}

TSharedRef<SDockTab> FMIForgeModule::OnSpawnMIForgeMainTab(const FSpawnTabArgs& SpawnTabArgs) {

	TSharedRef<SDockTab> MIForgeMainTab =
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	MIForgeMainTab->SetContent(
		SNew(SBox)
		.MinDesiredWidth(1136.f)
		.MinDesiredHeight(728.f)
		[
			SNew(SMainTabWidget)
				.SelectedFolderPaths(SelectedFolderPaths)
		]
	);
	return MIForgeMainTab;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMIForgeModule, MIForge)