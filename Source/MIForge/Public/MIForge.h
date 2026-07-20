// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"

class FMIForgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ContentBrowserMenu

	void InitializeMenuExtension();
	void UnregisterMenuExtension();
	TSharedRef<FExtender> CustomMenuExtender(const TArray<FString>& SelectedPaths);	
	void MenuEntry(class FMenuBuilder& MenuBuilder);

#pragma endregion

#pragma region RegisterTab
	 void RegisterTabSpawner();
	 void UnregisterTabSpawner();
	 TSharedRef<SDockTab> OnSpawnMIForgeMainTab(const FSpawnTabArgs& SpawnTabArgs);


	 void RegisterBatchParameterEditorTabSpawner();
	 void UnregisterBatchParameterEditorTabSpawner();

	 TSharedRef<SDockTab> OnSpawnBatchParameterEditorTab(
		 const FSpawnTabArgs& SpawnTabArgs
	 );

#pragma endregion

#pragma region BatchMIParamsAdjustor

	 void RegisterActorContextMenu();

	 void ExtendActorContextMenu();
	 void UnregisterActorContextMenu();
	 
#pragma endregion
private:
	TArray<FString> SelectedFolderPaths;

	FDelegateHandle ContentBrowserPathExtenderHandle;

	void OpenBatchParameterAdjusterFromSelection();

	void OpenBatchParameterEditorTab(
		const FMIForgeMaterialParentGroup& ChosenGroup
	);
	FMIForgeBatchParameterModel PendingBatchParameterModel;

	
public:
	// Centralized IDs
	static const FName MIForgeMainTabId;

	static const FName MIForgeBatchParameterEditorTabId;
};
