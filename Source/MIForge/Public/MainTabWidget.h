// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "MIForgeVertexPaintRecipeManager.h"


class SMainTabWidget : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SMainTabWidget) {}
		SLATE_ARGUMENT(TArray<FString>, SelectedFolderPaths)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void BindActionsToOnAssetsChanged();
	void UnbindActionsToOnAssetsChanged();

	~SMainTabWidget();

	template<typename T>
	void SelectItem(TArray<T>& SelectedItems, const T& Item)
	{
		SelectedItems.AddUnique(Item);
	}

	template<typename T>
	void UnselectItem(TArray<T>& SelectedItems, const T& Item)
	{
		SelectedItems.Remove(Item);
	}

	template<typename T>
	bool IsItemSelected(const TArray<T>& SelectedItems, const T& Item) const
	{
		return SelectedItems.Contains(Item);
	}

	/*void SelectAssets(TSharedPtr<FMIForgeTextureInfo> AssetData);
	void UnselectAssets(TSharedPtr<FMIForgeTextureInfo> AssetData);
	bool IsAssetSelected(TSharedPtr<FMIForgeTextureInfo> AssetData) const;*/

	void SelectTexture(TSharedPtr<FMIForgeTextureInfo> Item);
	void UnselectTexture(TSharedPtr<FMIForgeTextureInfo> Item);
	bool IsTextureSelected(TSharedPtr<FMIForgeTextureInfo> Item) const;

	void SelectTextureSet(TSharedPtr<FMIForgeTextureSet> Item);
	void UnselectTextureSet(TSharedPtr<FMIForgeTextureSet> Item);
	bool IsTextureSetSelected(TSharedPtr<FMIForgeTextureSet> Item) const;

	void SelectTexturesInSet(const FMIForgeTextureSet& TextureSet);
	void UnselectTexturesInSet(const FMIForgeTextureSet& TextureSet);

	bool bUseEmissiveTextures = false;
	bool bUseDetailNormalTextures = false;
	bool bUseTriplanarProjection = false;

	bool bUseBaseORMTexture = true;
	bool bEnableEmissiveChannel = false;
	bool bUseDetailNormalTextureRGB = false;

	bool bIgnoreUnrecognizedTextures = false;

	TSharedPtr<FString> CurrentPresetOption;

	void RefreshValidationSummary();
	void RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer Layer);

private:
	FDelegateHandle AddAssetHandle;
	FDelegateHandle RemoveAssetHandle;

	bool bRefreshQueued = false;
	FTSTicker::FDelegateHandle RefreshTickerHandle;
	void QueueListRefresh();

	void RefreshFilteredTextures();
	void RefreshFilteredTextureSets();
	void RefreshListViews();
	TSharedPtr<FMIForgeTextureInfo> FindTextureListItem(const FMIForgeTextureInfo& TextureInfo) const;

	TSharedPtr<SWidget> GenerateRightClickMenuWidget();

	TSharedRef<SWidget> Page1();
	TSharedRef<SWidget> PresetComboBox();
	TSharedRef<SWidget> PresetPannelSwitcher();
	TSharedRef<SWidget> ListViewSwitcher();
	TSharedRef<SWidget> IndividualModeListView();
	TSharedRef<SWidget> TextureSetModeListView();
	TSharedPtr<SHeaderRow> SetupHeaderRow();
	TSharedPtr<SHeaderRow> SetupTexSetHeaderRow();
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

	void AssignSelectedTextureSetToVertexLayer(EMIForgeVertexPaintLayer Layer);
	void ClearVertexLayerAssignment(EMIForgeVertexPaintLayer Layer);
	FMIForgeVertexPaintLayerSlot* GetVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer);
	FText GetVertexPaintLayerStatusText(EMIForgeVertexPaintLayer Layer) const;
	FText GetVertexPaintValidationSummaryText() const;
	FSlateColor GetVertexPaintLayerStatusColor(EMIForgeVertexPaintLayer Layer) const;

	TSharedPtr<SBox> GetVertexPaintLayerThumbnailBox(EMIForgeVertexPaintLayer Layer) const;
	
	const FAssetData* GetVertexPaintLayerThumbnailAsset(EMIForgeVertexPaintLayer Layer) const;

private:
	TArray<FString> SelectedFolderPaths;
	TSharedPtr<STextBlock> PresetComboBoxSelectedOptionText;
	
	TSharedPtr<SWidgetSwitcher> PresetPannelSwitcher0;
	//TSharedPtr<SEditableTextBox> TargetPathInputBox;
	FString CurrentTargetPath;

	TSharedPtr<SInlineEditableTextBlock> TexturePathTextBlock;
#pragma region Filter related members

	TSharedPtr<STextBlock> CurrentFilterComboBoxSelectedOptionText;  
	TSharedPtr<STextBlock> CurrentMaxLayersComboBoxSelectedOptionText;

	TArray<TSharedPtr<FString>> ActiveFilterOptions;
	TSharedPtr<FString> CurrentFilterOption;

	TArray<TSharedPtr<FString>> TextureFilterOptions;
	TSharedPtr<FString> CurrentTextureFilterOption;

	TArray<TSharedPtr<FString>> TextureSetFilterOptions;
	TSharedPtr<FString> CurrentTextureSetFilterOption;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> FilterComboBox;

	FText CurrentSearchText;
#pragma endregion
	TSharedPtr<FString> CurrentMaxLayersOption;

	TSharedPtr<SListView<TSharedPtr<FMIForgeTextureInfo>>> TexListView;
	TSharedPtr<SListView<TSharedPtr<FMIForgeTextureSet>>> TexSetListView;
	TSharedPtr<SWidgetSwitcher> ViewModeSwitcher0;

	TArray<TSharedPtr<FMIForgeTextureInfo>> TextureListItems;
	TArray<TSharedPtr<FMIForgeTextureInfo>> FilteredTextureListItems;
	TArray<TSharedPtr<FMIForgeTextureInfo>> SelectedTextureItems;

	TArray<TSharedPtr<FMIForgeTextureSet>> TextureSetListItems;
	TArray<TSharedPtr<FMIForgeTextureSet>> FilteredTextureSetListItems;
	TArray<TSharedPtr<FMIForgeTextureSet>> SelectedTextureSetItems;

	TArray<TSharedPtr<FMIForgeTextureInfo>> ResolvedTextures;

	FMIForgeValidationSummary CurrentValidationSummary;
	FMIForgeVertexPaintLayerStackValidationResult CurrentVertexPaintValidationResult;
	FMIForgeVertexPaintValidationSummary CurrentVertexPaintValidationSummary;



	// Currently selected option (default Skip)
	EIfMIExistsOption CurrentIfMIExistsOption = EIfMIExistsOption::Skip;

	enum class EMIForgeInputMode : uint8
	{
		IndividualTextures,
		TextureSets
	};

	TArray<TSharedPtr<FMIForgeTextureSet>> BuildGenerationTextureSets() const;

	EMIForgeInputMode CurrentInputMode =
		EMIForgeInputMode::IndividualTextures;

	FMIForgeVertexPaintLayerStack VertexPaintLayerStack;

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

	


};
