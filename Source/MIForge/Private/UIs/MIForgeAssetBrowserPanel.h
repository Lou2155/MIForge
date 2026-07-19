// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"

class FMIForgeMainTabViewModel;
class FMIForgeTextureCatalog;

class SMIForgeAssetBrowserPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeAssetBrowserPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
		SLATE_ARGUMENT(TSharedPtr<FMIForgeTextureCatalog>, TextureCatalog)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SMIForgeAssetBrowserPanel();

private:
	void SelectTexturesInSet(const FMIForgeTextureSet& TextureSet);
	void UnselectTexturesInSet(const FMIForgeTextureSet& TextureSet);
	void HandleTextureSetCheckStateChanged(
		const TSharedPtr<FMIForgeTextureSet>& TextureSet,
		ECheckBoxState CheckState);

	void RefreshFilteredTextures();
	void RefreshFilteredTextureSets();
	void RefreshListViews();

	void HandleTextureCatalogRefreshStarted();
	void HandleTextureCatalogChanged();
	void HandlePresetChanged(EMIForgeGenerationPreset NewPreset);
	void HandleGenerationOptionsChanged();
	void HandleSelectionChanged();

	TSharedPtr<SWidget> GenerateRightClickMenuWidget();
	TSharedRef<SWidget> ListViewSwitcher();
	TSharedRef<SWidget> IndividualModeListView();
	TSharedRef<SWidget> TextureSetModeListView();
	TSharedPtr<SHeaderRow> SetupHeaderRow();
	TSharedPtr<SHeaderRow> SetupTexSetHeaderRow();

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	TSharedPtr<FMIForgeTextureCatalog> TextureCatalog;

	TSharedPtr<SInlineEditableTextBlock> TexturePathTextBlock;
	TSharedPtr<STextBlock> CurrentFilterComboBoxSelectedOptionText;

	TArray<TSharedPtr<FString>> ActiveFilterOptions;
	TSharedPtr<FString> CurrentFilterOption;

	TArray<TSharedPtr<FString>> TextureFilterOptions;
	TSharedPtr<FString> CurrentTextureFilterOption;

	TArray<TSharedPtr<FString>> TextureSetFilterOptions;
	TSharedPtr<FString> CurrentTextureSetFilterOption;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> FilterComboBox;
	FText CurrentSearchText;

	TSharedPtr<SListView<TSharedPtr<FMIForgeTextureInfo>>> TexListView;
	TSharedPtr<SListView<TSharedPtr<FMIForgeTextureSet>>> TexSetListView;
	TSharedPtr<SWidgetSwitcher> ViewModeSwitcher;

	TArray<TSharedPtr<FMIForgeTextureInfo>> FilteredTextureListItems;
	TArray<TSharedPtr<FMIForgeTextureSet>> FilteredTextureSetListItems;
};
