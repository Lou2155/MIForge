// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeVertexPaintLayerStackPanel.h"

#include "AssetThumbnail.h"
#include "MIForgeTextureSetDropTarget.h"
#include "MIForgeUtilities.h"
#include "PopupWindowCreator.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

void SMIForgeVertexPaintLayerStackPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;
	AssetThumbnailPool = MakeShared<FAssetThumbnailPool>(64);
	RecipeManager.LoadRecipesFromDisk();
	RefreshRecipeOptions();

	ViewModel->OnVertexPaintChanged.AddSP(
		SharedThis(this),
		&SMIForgeVertexPaintLayerStackPanel::HandleVertexPaintChanged);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2.f).HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Layer Stack")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2.f).HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Assign Selected")))
				.OnClicked_Lambda([this]()
				{
					FText Error;
					if (!ViewModel->AssignSelectedTextureSetsToVertexLayers(Error))
					{
						MIForgeUtilities::PrintNotification(Error.ToString());
						return FReply::Unhandled();
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2.f).HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Clear All")))
				.OnClicked_Lambda([this]()
				{
					ViewModel->ClearAllVertexPaintLayers();
					return FReply::Handled();
				})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			CreateLayerSlotWidget(EMIForgeVertexPaintLayer::Base)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			CreateLayerSlotWidget(EMIForgeVertexPaintLayer::LayerR)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			CreateLayerSlotWidget(EMIForgeVertexPaintLayer::LayerG)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			CreateLayerSlotWidget(EMIForgeVertexPaintLayer::LayerB)
		]
		+ SVerticalBox::Slot().FillHeight(4.f)
		[
			SNew(SSpacer)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.f).HAlign(HAlign_Center)
		[
			CreateRecipeSection()
		]
	];
}

TSharedRef<SWidget> SMIForgeVertexPaintLayerStackPanel::CreateLayerSlotWidget(EMIForgeVertexPaintLayer Layer)
{
	TSharedPtr<SBox>* ThumbnailBox = nullptr;
	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base: ThumbnailBox = &BaseLayerThumbnailBox; break;
	case EMIForgeVertexPaintLayer::LayerR: ThumbnailBox = &LayerRThumbnailBox; break;
	case EMIForgeVertexPaintLayer::LayerG: ThumbnailBox = &LayerGThumbnailBox; break;
	case EMIForgeVertexPaintLayer::LayerB: ThumbnailBox = &LayerBThumbnailBox; break;
	}

	static const FSlateRoundedBoxBrush LayerSlotBorderBrush(
		FLinearColor(0.03f, 0.03f, 0.03f, 1.f),
		5.f,
		FLinearColor(0.20f, 0.20f, 0.20f, 1.f),
		1.f);
	static const FSlateRoundedBoxBrush ThumbnailBorderBrush(
		FLinearColor(0.f, 0.f, 0.f, 0.f),
		5.f,
		FLinearColor(0.20f, 0.20f, 0.20f, 1.f),
		0.5f);

	return SNew(SMIForgeTextureSetDropTarget)
		.Layer(Layer)
		.BorderImage(&LayerSlotBorderBrush)
		.Padding(6.f)
		.OnTextureSetDropped_Lambda(
			[this](const TSharedPtr<FMIForgeTextureSet>& DroppedSet, const EMIForgeVertexPaintLayer& DroppedLayer)
			{
				return ViewModel->AssignTextureSetToVertexLayer(DroppedLayer, DroppedSet)
					? FReply::Handled()
					: FReply::Unhandled();
			})
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
					.Text_Lambda([this, Layer]()
					{
						const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);
						return Slot
							? FText::FromString(Slot->DisplayName)
							: FText::FromString(TEXT("Unknown Layer"));
					})
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)[SNullWidget::NullWidget]
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("X")))
					.OnClicked_Lambda([this, Layer]()
					{
						ViewModel->ClearVertexPaintLayer(Layer);
						return FReply::Handled();
					})
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.f, 0.f, 8.f, 0.f).VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(&ThumbnailBorderBrush)
					[
						SAssignNew(*ThumbnailBox, SBox)
						.WidthOverride(64.f)
						.HeightOverride(64.f)
						[
							CreateLayerThumbnailWidget(Layer)
						]
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this, Layer]()
						{
							const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);
							return Slot && Slot->AssignedTextureSet.IsValid()
								? FText::FromString(TEXT("[") + Slot->AssignedTextureSet->SetName + TEXT("]"))
								: FText::FromString(TEXT("[Empty]"));
						})
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this, Layer]()
						{
							const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);
							return Slot && Slot->IsAssigned()
								? FText::FromString(Slot->GetAddedTextureTypeText())
								: GetLayerStatusText(Layer);
						})
						.ColorAndOpacity_Lambda([this, Layer]() { return GetLayerStatusColor(Layer); })
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this, Layer]()
						{
							const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);
							return Slot && Slot->IsAssigned()
								? FText::FromString(Slot->GetTextureSizeText())
								: FText::GetEmpty();
						})
					]
				]
			]
		];
}

TSharedRef<SWidget> SMIForgeVertexPaintLayerStackPanel::CreateRecipeSection()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).Padding(4.f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Save Recipe")))
			.OnClicked_Lambda([this]()
			{
				const FString InitialName = CurrentRecipeOption.IsValid() ? *CurrentRecipeOption : FString();
				PopupWindowCreator::OpenTextInputPopup(
					FText::FromString(TEXT("Save Recipe")),
					FText::FromString(TEXT("Recipe Name")),
					InitialName,
					[this](const FString& RecipeName)
					{
						if (RecipeName.IsEmpty())
						{
							MIForgeUtilities::PrintWindow(TEXT("Please enter a recipe name."), EAppMsgType::Ok);
							return;
						}
						RecipeManager.SaveRecipe(RecipeName, ViewModel->GetVertexPaintLayerStack());
						CurrentRecipeOption = MakeShared<FString>(RecipeName);
						RefreshRecipeOptions();
					});
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).Padding(4.f)
		[
			SAssignNew(RecipeComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&RecipeOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")));
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type)
			{
				if (!Option.IsValid())
				{
					return;
				}
				CurrentRecipeOption = Option;
				CurrentRecipeOptionText->SetText(FText::FromString(*Option));
				FMIForgeVertexPaintLayerStack LoadedStack = ViewModel->GetVertexPaintLayerStack();
				if (RecipeManager.LoadRecipe(*Option, LoadedStack))
				{
					ViewModel->SetVertexPaintLayerStack(LoadedStack);
				}
			})
			.Content()
			[
				SAssignNew(CurrentRecipeOptionText, STextBlock)
				.Text(FText::FromString(TEXT("Select Recipe")))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).Padding(4.f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Delete Recipe")))
			.OnClicked_Lambda([this]()
			{
				if (!CurrentRecipeOption.IsValid())
				{
					MIForgeUtilities::PrintWindow(TEXT("Please select a recipe to delete."), EAppMsgType::Ok);
					return FReply::Handled();
				}
				if (MIForgeUtilities::PrintWindow(
					FString::Printf(TEXT("Are you sure you want to delete the recipe '%s'?"), **CurrentRecipeOption),
					EAppMsgType::YesNo) == EAppReturnType::Yes)
				{
					RecipeManager.DeleteRecipe(*CurrentRecipeOption);
					CurrentRecipeOption.Reset();
					RefreshRecipeOptions();
				}
				return FReply::Handled();
			})
		];
}

void SMIForgeVertexPaintLayerStackPanel::RefreshRecipeOptions()
{
	RecipeOptions.Empty();
	for (const FMIForgeVertexPaintRecipe& Recipe : RecipeManager.GetRecipes())
	{
		RecipeOptions.Add(MakeShared<FString>(Recipe.RecipeName));
	}

	bool bCurrentRecipeStillExists = false;
	if (CurrentRecipeOption.IsValid())
	{
		for (const TSharedPtr<FString>& RecipeOption : RecipeOptions)
		{
			if (RecipeOption.IsValid() && *RecipeOption == *CurrentRecipeOption)
			{
				CurrentRecipeOption = RecipeOption;
				bCurrentRecipeStillExists = true;
				break;
			}
		}
	}
	if (!bCurrentRecipeStillExists)
	{
		CurrentRecipeOption.Reset();
	}
	if (CurrentRecipeOptionText.IsValid())
	{
		CurrentRecipeOptionText->SetText(CurrentRecipeOption.IsValid()
			? FText::FromString(*CurrentRecipeOption)
			: FText::FromString(TEXT("Select Recipe")));
	}
	if (RecipeComboBox.IsValid())
	{
		RecipeComboBox->RefreshOptions();
	}
}

TSharedRef<SWidget> SMIForgeVertexPaintLayerStackPanel::CreateLayerThumbnailWidget(EMIForgeVertexPaintLayer Layer)
{
	const FAssetData* Asset = GetLayerThumbnailAsset(Layer);
	if (!Asset || !AssetThumbnailPool.IsValid())
	{
		return SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("[ preview ]")))
			]
		];
	}

	TSharedPtr<FAssetThumbnail> Thumbnail = MakeShared<FAssetThumbnail>(*Asset, 64, 64, AssetThumbnailPool);
	return SNew(SBox).WidthOverride(64.f).HeightOverride(64.f)[Thumbnail->MakeThumbnailWidget()];
}

FText SMIForgeVertexPaintLayerStackPanel::GetLayerStatusText(EMIForgeVertexPaintLayer Layer) const
{
	for (const FMIForgeVertexPaintLayerValidationResult& Result : ViewModel->GetVertexPaintValidationResult().LayerResults)
	{
		if (Result.Layer != Layer)
		{
			continue;
		}
		switch (Result.Status)
		{
		case EMIForgeVertexPaintLayerStatus::Empty:
			return FText::FromString(Result.bRequired ? TEXT("Required") : TEXT("Optional"));
		case EMIForgeVertexPaintLayerStatus::Valid: return FText::FromString(TEXT("Valid"));
		case EMIForgeVertexPaintLayerStatus::Warning: return FText::FromString(TEXT("Warning"));
		case EMIForgeVertexPaintLayerStatus::Error: return FText::FromString(TEXT("Missing"));
		}
	}
	return FText::FromString(TEXT("Unknown"));
}

FSlateColor SMIForgeVertexPaintLayerStackPanel::GetLayerStatusColor(EMIForgeVertexPaintLayer Layer) const
{
	for (const FMIForgeVertexPaintLayerValidationResult& Result : ViewModel->GetVertexPaintValidationResult().LayerResults)
	{
		if (Result.Layer != Layer)
		{
			continue;
		}
		switch (Result.Status)
		{
		case EMIForgeVertexPaintLayerStatus::Valid: return FSlateColor(FLinearColor(0.2f, 0.8f, 0.35f, 1.f));
		case EMIForgeVertexPaintLayerStatus::Warning: return FSlateColor(FLinearColor(1.f, 0.65f, 0.1f, 1.f));
		case EMIForgeVertexPaintLayerStatus::Error: return FSlateColor(FLinearColor(0.9f, 0.15f, 0.12f, 1.f));
		case EMIForgeVertexPaintLayerStatus::Empty:
		default: return FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.f));
		}
	}
	return FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.f));
}

TSharedPtr<SBox> SMIForgeVertexPaintLayerStackPanel::GetLayerThumbnailBox(EMIForgeVertexPaintLayer Layer) const
{
	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base: return BaseLayerThumbnailBox;
	case EMIForgeVertexPaintLayer::LayerR: return LayerRThumbnailBox;
	case EMIForgeVertexPaintLayer::LayerG: return LayerGThumbnailBox;
	case EMIForgeVertexPaintLayer::LayerB: return LayerBThumbnailBox;
	}
	return nullptr;
}

void SMIForgeVertexPaintLayerStackPanel::RefreshLayerThumbnail(EMIForgeVertexPaintLayer Layer)
{
	const TSharedPtr<SBox> Box = GetLayerThumbnailBox(Layer);
	if (Box.IsValid())
	{
		Box->SetContent(CreateLayerThumbnailWidget(Layer));
	}
}

void SMIForgeVertexPaintLayerStackPanel::HandleVertexPaintChanged()
{
	RefreshLayerThumbnail(EMIForgeVertexPaintLayer::Base);
	RefreshLayerThumbnail(EMIForgeVertexPaintLayer::LayerR);
	RefreshLayerThumbnail(EMIForgeVertexPaintLayer::LayerG);
	RefreshLayerThumbnail(EMIForgeVertexPaintLayer::LayerB);
}

const FAssetData* SMIForgeVertexPaintLayerStackPanel::GetLayerThumbnailAsset(EMIForgeVertexPaintLayer Layer) const
{
	const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);
	if (!Slot || !Slot->AssignedTextureSet.IsValid())
	{
		return nullptr;
	}
	const FMIForgeTextureInfo* Albedo = Slot->AssignedTextureSet->Textures.Find(EMIForgeTextureType::Albedo);
	return Albedo && Albedo->AssetData.IsValid() ? &Albedo->AssetData : nullptr;
}
