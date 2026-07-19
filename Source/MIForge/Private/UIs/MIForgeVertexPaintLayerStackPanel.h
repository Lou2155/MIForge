// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "MIForgeVertexPaintRecipeManager.h"
#include "Widgets/SCompoundWidget.h"

class FAssetThumbnailPool;
class FMIForgeMainTabViewModel;
class SBox;
class STextBlock;
template <typename OptionType> class SComboBox;

class SMIForgeVertexPaintLayerStackPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeVertexPaintLayerStackPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateLayerSlotWidget(EMIForgeVertexPaintLayer Layer);
	TSharedRef<SWidget> CreateLayerThumbnailWidget(EMIForgeVertexPaintLayer Layer);
	TSharedRef<SWidget> CreateRecipeSection();

	void RefreshRecipeOptions();
	void HandleVertexPaintChanged();
	void RefreshLayerThumbnail(EMIForgeVertexPaintLayer Layer);

	FText GetLayerStatusText(EMIForgeVertexPaintLayer Layer) const;
	FSlateColor GetLayerStatusColor(EMIForgeVertexPaintLayer Layer) const;
	TSharedPtr<SBox> GetLayerThumbnailBox(EMIForgeVertexPaintLayer Layer) const;
	const FAssetData* GetLayerThumbnailAsset(EMIForgeVertexPaintLayer Layer) const;

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	TSharedPtr<FAssetThumbnailPool> AssetThumbnailPool;
	TSharedPtr<SBox> BaseLayerThumbnailBox;
	TSharedPtr<SBox> LayerRThumbnailBox;
	TSharedPtr<SBox> LayerGThumbnailBox;
	TSharedPtr<SBox> LayerBThumbnailBox;

	FMIForgeVertexPaintRecipeManager RecipeManager;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> RecipeComboBox;
	TArray<TSharedPtr<FString>> RecipeOptions;
	TSharedPtr<FString> CurrentRecipeOption;
	TSharedPtr<STextBlock> CurrentRecipeOptionText;
};
