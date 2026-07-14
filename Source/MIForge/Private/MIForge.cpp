// Copyright Epic Games, Inc. All Rights Reserved.

#include "MIForge.h"
#include "ContentBrowserModule.h"
#include "MainTabWidget.h"
//#include "Framework/Docking/TabManager.h"
#include "MIForgeStyle.h"
#include "MIForgeUtilities.h"
#include "Selection.h"
#include "GameFramework/Actor.h"
#include "MIForgeBatchAdjustMIParams/MIForgeSelectionCollector.h"
#include "MIForgeBatchAdjustMIParams/SMIForgeParentMtlGroupPicker.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParamModelBuilder.h"
#include "MIForgeBatchAdjustMIParams/SMIForgeBatchParameterEditor.h"

#define LOCTEXT_NAMESPACE "FMIForgeModule"

const FName FMIForgeModule::MIForgeMainTabId = FName("MIForgeMainTab");

const FName FMIForgeModule::MIForgeBatchParameterEditorTabId =
FName("MIForgeBatchParameterEditor");

void FMIForgeModule::StartupModule()
{	
	FMIForgeStyle::Initialize();
	InitializeMenuExtension();
	RegisterActorContextMenu();
	RegisterTabSpawner();
	RegisterBatchParameterEditorTabSpawner();
}

void FMIForgeModule::ShutdownModule()
{	
	// TODO:
	// Store delegate handle and unregister
	// ContentBrowser menu extension in Shutdown.
	// (Not necessary for v1.0)

	UnregisterBatchParameterEditorTabSpawner();
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
		FSlateIcon(FMIForgeStyle::GetStyleSetName(), "MIForgeLogo.Compact"),
		FExecuteAction::CreateLambda([this]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(MIForgeMainTabId);
			}
		)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Fix Texture Compression"),
		FText::FromString("Fix the texture compression settings in the folder."),
		FSlateIcon(FMIForgeStyle::GetStyleSetName(), "MIForgeLogo.Compact"),
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
		.SetIcon(FSlateIcon(FMIForgeStyle::GetStyleSetName(), "MIForgeLogo.Compact"))
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

void FMIForgeModule::RegisterBatchParameterEditorTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MIForgeBatchParameterEditorTabId,
		FOnSpawnTab::CreateRaw(this, &FMIForgeModule::OnSpawnBatchParameterEditorTab))
		.SetDisplayName(FText::FromString("MIForge Batch Parameter Editor"))
		.SetIcon(FSlateIcon(FMIForgeStyle::GetStyleSetName(), "MIForgeLogo.Compact"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMIForgeModule::UnregisterBatchParameterEditorTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(
		MIForgeBatchParameterEditorTabId
	);
}

TSharedRef<SDockTab> FMIForgeModule::OnSpawnBatchParameterEditorTab(const FSpawnTabArgs& SpawnTabArgs)
{	
	TSharedPtr<SDockTab> BatchTab;

	SAssignNew(BatchTab, SDockTab)
		.TabRole(ETabRole::NomadTab);

	BatchTab->SetContent(
		SNew(SMIForgeBatchParameterEditor)
		.ParameterModel(PendingBatchParameterModel)
		.OwnerTab(BatchTab)
	);

	return BatchTab.ToSharedRef();
}

void FMIForgeModule::RegisterActorContextMenu()
{
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
			FToolMenuSection& Section = Menu->AddSection("MIForge", FText::FromString("MIForge"), FToolMenuInsert(TEXT("ActorGeneral"), EToolMenuInsertType::After));
			Section.AddMenuEntry(
				"MIForge_BatchAdjustMIParameters",
				FText::FromString(TEXT("MIForge: Batch Adjust MI Parameters")),
				FText::FromString(TEXT("Batch edit Material Instance parameters from selected viewport actors.")),
				FSlateIcon(FMIForgeStyle::GetStyleSetName(), "MIForgeLogo.Compact"),
				FUIAction(
					FExecuteAction::CreateRaw(
						this,
						&FMIForgeModule::OpenBatchParameterAdjusterFromSelection
					)
				)
			);
		})
	);
}

namespace
{
	static void LogChosenParentGroup(const FMIForgeMaterialParentGroup& ChosenGroup)
	{
		const FMIForgeBatchParameterModel Model =
			FMIForgeBatchParamModelBuilder::BuildFromParentGroup(ChosenGroup);

		MIForgeUtilities::PrintNotification(
			FString::Printf(
				TEXT("Parameters found: Scalar %d | Vector %d | Texture %d | Switch %d"),
				Model.ScalarCount,
				Model.VectorCount,
				Model.TextureCount,
				Model.StaticSwitchCount
			),
			8.0f
		);
	}

}

void FMIForgeModule::OpenBatchParameterEditorTab(
	const FMIForgeMaterialParentGroup& ChosenGroup
)
{
	PendingBatchParameterModel = FMIForgeBatchParamModelBuilder::BuildFromParentGroup(ChosenGroup);

	TSharedPtr<SDockTab> BatchTab = FGlobalTabmanager::Get()->TryInvokeTab(
		MIForgeBatchParameterEditorTabId
	);

	if(BatchTab.IsValid())
	{
		BatchTab->SetContent(
			SNew(SMIForgeBatchParameterEditor)
			.ParameterModel(PendingBatchParameterModel)
			.OwnerTab(BatchTab)
		);
	}

}

void FMIForgeModule::OpenBatchParameterAdjusterFromSelection()
{
	const FMIForgeMaterialSlotCollectionResult Result = MIForgeSelectionCollector::CollectFromCurrentEditorSelection();

	if (Result.ParentGroups.Num() == 0)
	{
		MIForgeUtilities::PrintWindow(
			TEXT("No editable Material Instance Constant slots found in the selected actors."),
			EAppMsgType::Ok
		);
		return;
	}

	if (Result.ParentGroups.Num() == 1)
	{
		OpenBatchParameterEditorTab(Result.ParentGroups[0]);

		// Open batch parameter editor directly.
		return;
	}

	TSharedPtr<SWindow> PickerWindow;

	SAssignNew(PickerWindow, SWindow)
		.Title(FText::FromString(TEXT("MIForge - Select Parent Material")))
		.ClientSize(FVector2D(680.f, 360.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::FixedSize);

	PickerWindow->SetContent(
		SNew(SMIForgeParentMtlGroupPicker)
		.ParentGroups(Result.ParentGroups)
		.OwnerWindow(PickerWindow)
		.OnGroupChosen(FMIForgeOnParentMaterialGroupChosen::CreateLambda(
			[this](const FMIForgeMaterialParentGroup& ChosenGroup)
			{
				OpenBatchParameterEditorTab(ChosenGroup);
			}
		))
	);

	FSlateApplication::Get().AddModalWindow(PickerWindow.ToSharedRef(), nullptr);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMIForgeModule, MIForge)