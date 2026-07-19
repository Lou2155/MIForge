// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "MIForgeVertexPaintRecipeManager.h"

class FMIForgeMainTabViewModel;
class FMIForgeTextureCatalog;

class SMainTabWidget : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SMainTabWidget) {}
		SLATE_ARGUMENT(TArray<FString>, SelectedFolderPaths)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	~SMainTabWidget();

	void RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer Layer);


private:
	TSharedRef<SWidget> Page1();
	TSharedRef<SWidget> PresetComboBox();
	TSharedRef<SWidget> PresetPannelSwitcher();
	TSharedRef<SWidget> RightContentWidget();

	TSharedRef<SWidget> StandardPresetPannel();
	TSharedRef<SWidget> RGBmaskingPresetPannel();
	TSharedRef<SWidget> VertexPaintingPresetPannel();

	TSharedRef<SWidget> VertexPaintingLayerStackPanel();

	TSharedRef<SWidget> ValidationSummaryText();
	TSharedRef<SWidget> VertexPaintValidationSummaryText();
	TSharedRef<SWidget> ValidationSummarySection();
	void GenerateStandardMIButton(TSharedRef<SVerticalBox> Container);

	TSharedRef<SWidget> VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer Layer);

	void TargetFolderSection(TSharedRef<SVerticalBox> Container);

	void StandardMIOptionSection(TSharedRef<SVerticalBox> Container);

	void RGBmaskingMIOptionSection(TSharedRef<SVerticalBox> Container);
	void GenerateRGBmaskingMIButton(TSharedRef<SVerticalBox> Container);

	void VertexPaintGenerateNameTextBox(TSharedRef<SVerticalBox> Container);
	void VertexPaintingMIOptionSection(TSharedRef<SVerticalBox> Container);
	void GenerateVertexPaintMIButton(TSharedRef<SVerticalBox> Container);

	void VertexPaintRecipeSection(TSharedRef<SVerticalBox> Container);
	void RefreshVertexPaintRecipeOptions();


	TSharedRef<SWidget> IfMIExistsOptionBlock();

	TArray<TSharedPtr<FMIForgeTextureInfo>> ResolveTexturesForSet(const FMIForgeTextureSet& TextureSet) const;

	FText GetValidationSummaryText() const;
	FText GetMIReadyCount() const;
	
	TSharedRef<SWidget> CreateStandardValidationDetailsWidget();
	TSharedRef<SWidget> CreateRGBmaskingValidationDetailsWidget();
	TSharedRef<SWidget> CreateVertexPaintLayerThumbnailWidget(EMIForgeVertexPaintLayer Layer);

	FText GetVertexPaintLayerStatusText(EMIForgeVertexPaintLayer Layer) const;
	FText GetVertexPaintValidationSummaryText() const;
	FSlateColor GetVertexPaintLayerStatusColor(EMIForgeVertexPaintLayer Layer) const;

	TSharedPtr<SBox> GetVertexPaintLayerThumbnailBox(EMIForgeVertexPaintLayer Layer) const;
	
	const FAssetData* GetVertexPaintLayerThumbnailAsset(EMIForgeVertexPaintLayer Layer) const;

private:
	TSharedPtr<SWidgetSwitcher> PresetPannelSwitcher0;
	//TSharedPtr<SEditableTextBox> TargetPathInputBox;

	TSharedPtr<STextBlock> CurrentMaxLayersComboBoxSelectedOptionText;
	TSharedPtr<FString> CurrentMaxLayersOption;
	

	TArray<TSharedPtr<FMIForgeTextureInfo>> ResolvedTextures;

	TSharedPtr<FAssetThumbnailPool> AssetThumbnailPool;
	TSharedPtr<SBox> BaseLayerThumbnailBox;
	TSharedPtr<SBox> LayerRThumbnailBox;
	TSharedPtr<SBox> LayerGThumbnailBox;
	TSharedPtr<SBox> LayerBThumbnailBox;

	FString CurrentVertexPaintMIName;

	FMIForgeVertexPaintRecipeManager VertexPaintRecipeManager;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> VertexPaintRecipeComboBox;
	TArray<TSharedPtr<FString>> VertexPaintRecipeOptions;
	TSharedPtr<FString> CurrentVertexPaintRecipeOption;

	TSharedPtr<STextBlock> CurrentVertexPaintRecipeOptionText;

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	TSharedPtr<FMIForgeTextureCatalog> TextureCatalog;

	TArray<TSharedPtr<EMIForgeGenerationPreset>>
		PresetOptions;

	void HandlePresetChanged(
		EMIForgeGenerationPreset NewPreset);

	void HandleVertexPaintChanged();
	
};
