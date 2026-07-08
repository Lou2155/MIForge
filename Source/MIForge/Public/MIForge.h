// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMIForgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ContentBrowserMenu

	void InitializeMenuExtension();
	TSharedRef<FExtender> CustomMenuExtender(const TArray<FString>& SelectedPaths);	
	void MenuEntry(class FMenuBuilder& MenuBuilder);

#pragma endregion

#pragma region RegisterTab
	 void RegisterTabSpawner();
	 void UnregisterTabSpawner();
	 TSharedRef<SDockTab> OnSpawnMIForgeMainTab(const FSpawnTabArgs& SpawnTabArgs);

#pragma endregion

private:
	TArray<FString> SelectedFolderPaths;
	
public:
	// Centralized IDs
	static const FName MIForgeMainTabId;
};
