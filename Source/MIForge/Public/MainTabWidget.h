// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "Widgets/SCompoundWidget.h"

class FMIForgeMainTabViewModel;
class FMIForgeTextureCatalog;
class SWidgetSwitcher;

class SMainTabWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainTabWidget) {}
		SLATE_ARGUMENT(TArray<FString>, SelectedFolderPaths)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SMainTabWidget();

private:
	TSharedRef<SWidget> BuildMainLayout();
	TSharedRef<SWidget> BuildPresetSelector();
	TSharedRef<SWidget> BuildPresetPanelSwitcher();
	TSharedRef<SWidget> BuildAssetWorkSpace();

	void HandlePresetChanged(EMIForgeGenerationPreset NewPreset);

	TSharedPtr<SWidgetSwitcher> PresetPanelSwitcher;
	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	TSharedPtr<FMIForgeTextureCatalog> TextureCatalog;
	TArray<TSharedPtr<EMIForgeGenerationPreset>> PresetOptions;
};
