// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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
