// Fill out your copyright notice in the Description page of Project Settings.


#include "MainTabWidget.h"
#include "MIForgeAssetScanner.h"
#include "MIForgeTypes.h"
#include "MIForgeTextureClassifier.h"
#include "MIForgeTextureSetBuilder.h"
#include "MIForgeUtilities.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "TextureTableRow.h" 
#include "TextureSetTableRow.h"
#include "MIForgeValidator.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "MIForgeStyle.h"
#include "PopupWindowCreator.h"
#include "Widgets/Input/SSearchBox.h"
#include "MIForgeTextureSetDropTarget.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "Generation/MIForgeGenerationCoordinator.h"




#define LOCTEXT_NAMESPACE "MainTabWidget"

namespace
{
	static const FSlateRoundedBoxBrush ListViewBorderBrush(
		FLinearColor(0.f, 0.f, 0.f, 0.f),   // Fill color (transparent)
		5.0f,                                 // Corner radius
		FLinearColor(0.10f, 0.10f, 0.10f, 1.f), // Outline color
		0.5f                                  // Outline thickness
	);

	static FText VertexPaintStatusToText(EMIForgeVertexPaintLayerStatus Status)
	{
		switch (Status)
		{
		case EMIForgeVertexPaintLayerStatus::Valid:
			return FText::FromString(TEXT("Ready"));

		case EMIForgeVertexPaintLayerStatus::Warning:
			return FText::FromString(TEXT("Warning"));

		case EMIForgeVertexPaintLayerStatus::Error:
			return FText::FromString(TEXT("Error"));

		case EMIForgeVertexPaintLayerStatus::Empty:
		default:
			return FText::FromString(TEXT("Empty"));
		}
	}

	static FSlateColor VertexPaintStatusToColor(
		EMIForgeVertexPaintLayerStatus Status
	)
	{
		switch (Status)
		{
		case EMIForgeVertexPaintLayerStatus::Valid:
			return FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.0f));

		case EMIForgeVertexPaintLayerStatus::Warning:
			return FSlateColor(FLinearColor(1.0f, 0.75f, 0.0f, 1.0f));

		case EMIForgeVertexPaintLayerStatus::Error:
			return FSlateColor(FLinearColor(1.f, 0.0f, 0.0f, 1.0f));

		case EMIForgeVertexPaintLayerStatus::Empty:
		default:
			return FSlateColor::UseForeground();
		}
	}
}

void SMainTabWidget::Construct(const FArguments& InArgs) {
	FMIForgeAssetScanner Scanner;
	FMIForgeTextureClassifier Classifier;
	FMIForgeTextureSetBuilder SetBuilder;

	SelectedFolderPaths = InArgs._SelectedFolderPaths;
	TArray<FAssetData> FoundTextures = Scanner.FindTexturesInFolders(InArgs._SelectedFolderPaths);
	TArray<FMIForgeTextureInfo> ClassifiedTextureInfos = Classifier.ClassifyTextures(FoundTextures);
	TArray<FMIForgeTextureSet> TextureSets = SetBuilder.BuildTextureSets(ClassifiedTextureInfos);
	
	TextureListItems.Empty();
	TextureListItems.Reserve(ClassifiedTextureInfos.Num());
	for (FMIForgeTextureInfo& TextureInfo : ClassifiedTextureInfos)
	{
		TextureListItems.Add(MakeShared<FMIForgeTextureInfo>(MoveTemp(TextureInfo)));
	}

	TextureSetListItems.Empty();
	TextureSetListItems.Reserve(TextureSets.Num());
	for (FMIForgeTextureSet& TextureSet : TextureSets)
	{
		TextureSetListItems.Add(MakeShared<FMIForgeTextureSet>(MoveTemp(TextureSet)));
	}

	TextureFilterOptions = MIForgeUtilities::GetFilterOptions();

	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Ready")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Warning")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Error")));

	ActiveFilterOptions = TextureFilterOptions;
	CurrentTextureFilterOption = TextureFilterOptions[0];
	CurrentTextureSetFilterOption = TextureSetFilterOptions[0];
	CurrentFilterOption = CurrentTextureFilterOption;

	CurrentPresetOption = MakeShared<FString>(TEXT("Standard"));
	CurrentMaxLayersOption = MakeShared<FString>(TEXT("2 (R)"));

	RefreshFilteredTextures();
	RefreshFilteredTextureSets();
	
	BindActionsToOnAssetsChanged();

	// Initialize the vertex paint layer stack with default values
#pragma region VertexPaintLayerStackInitialization
	const FMIForgeVertexPaintPresetDefinition&
		VertexPaintDefinition =
		FMIForgePresetDefinitions::GetVertexPaint();

	for (const FMIForgeVertexPaintLayerDefinition& LayerDefinition :
		VertexPaintDefinition.Layers)
	{
		FMIForgeVertexPaintLayerSlot* Slot =
			GetVertexPaintLayerSlot(LayerDefinition.Layer); //get VertexPaintlayerStack.layer

		if (!Slot)
		{
			continue;
		}

		Slot->Layer = LayerDefinition.Layer;
		Slot->DisplayName = LayerDefinition.DisplayName;
		Slot->ChannelName = LayerDefinition.ChannelName;
		Slot->bRequired = LayerDefinition.bRequired;
	}

#pragma endregion

	AssetThumbnailPool = MakeShared<FAssetThumbnailPool>(64);

	VertexPaintRecipeManager.LoadRecipesFromDisk();
	RefreshVertexPaintRecipeOptions();

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					Page1()
				]
		];


	if (!SelectedFolderPaths.IsEmpty()) {
		TexturePathTextBlock->SetText(FText::FromString(SelectedFolderPaths[0]));
	}

	if (CurrentTextureFilterOption.IsValid() && CurrentFilterComboBoxSelectedOptionText.IsValid())
	{
		CurrentFilterComboBoxSelectedOptionText->SetText(FText::FromString(*CurrentTextureFilterOption));
	}

	if (TexListView.IsValid())
	{
		TexListView->RebuildList();
	}
}

void SMainTabWidget::BindActionsToOnAssetsChanged()
{
	//	//when assets are modified, added or removed in the content browser, we want to update the list view to reflect those changes. To do that we subscribe to the asset registry events.
	//you have to unload this module when the widget is destroyed. Otherwise it will acumulate in memory and cause a memory leak.
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));

	TWeakPtr<SMainTabWidget> WeakMainTabWidget = SharedThis(this);

	AddAssetHandle = AssetRegistry.Get().OnAssetAdded().AddLambda([WeakMainTabWidget](const FAssetData& AssetData)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakMainTabWidget, AssetData]()
				{	
					if (TSharedPtr<SMainTabWidget> PinnedTab = WeakMainTabWidget.Pin())
					{
						FString AssetPackageFolder = FPackageName::GetLongPackagePath(AssetData.PackageName.ToString());
						
						bool bRelevant = false;
						for (const FString& Folder : PinnedTab->SelectedFolderPaths)
						{

							if (!Folder.IsEmpty() && (AssetPackageFolder == Folder || AssetPackageFolder.StartsWith(Folder + TEXT("/"))))
							{
								bRelevant = true;
								break;
							}
						}
						if (bRelevant)
						{
							PinnedTab->QueueListRefresh();
						}
					}
				});
		});

	RemoveAssetHandle = AssetRegistry.Get().OnAssetRemoved().AddLambda([WeakMainTabWidget](const FAssetData& AssetData)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakMainTabWidget, AssetData]()
				{
					if (TSharedPtr<SMainTabWidget> PinnedTab = WeakMainTabWidget.Pin())
					{
						FString AssetPackageFolder = FPackageName::GetLongPackagePath(AssetData.PackageName.ToString());

						bool bRelevant = false;
						for (const FString& Folder : PinnedTab->SelectedFolderPaths)
						{

							if (!Folder.IsEmpty() && (AssetPackageFolder == Folder || AssetPackageFolder.StartsWith(Folder + TEXT("/"))))
							{
								bRelevant = true;
								break;
							}
						}
						if (bRelevant)
						{
							PinnedTab->QueueListRefresh();
						}
					}
				});
		});
}

void SMainTabWidget::UnbindActionsToOnAssetsChanged()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		FAssetRegistryModule& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");

		if (AddAssetHandle.IsValid())
		{
			AssetRegistry.Get().OnAssetAdded().Remove(AddAssetHandle);
			AddAssetHandle.Reset();
		}

		if (RemoveAssetHandle.IsValid())
		{
			AssetRegistry.Get().OnAssetRemoved().Remove(RemoveAssetHandle);
			RemoveAssetHandle.Reset();
		}
	}
	else
	{
		// If module isn't loaded, just clear our handle state to avoid double-remove later
		AddAssetHandle.Reset();
		RemoveAssetHandle.Reset();
	}
}

SMainTabWidget::~SMainTabWidget()
{
	UnbindActionsToOnAssetsChanged();

	if (RefreshTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RefreshTickerHandle);
		RefreshTickerHandle.Reset();
	}
}

//void SMainTabWidget::SelectAssets(TSharedPtr<FMIForgeTextureInfo> AssetData)
//{
//	SelectedTextureItems.AddUnique(AssetData);
//}
//
//void SMainTabWidget::UnselectAssets(TSharedPtr<FMIForgeTextureInfo> AssetData)
//{
//	SelectedTextureItems.Remove(AssetData);
//}
//
//bool SMainTabWidget::IsAssetSelected(TSharedPtr<FMIForgeTextureInfo> AssetData) const
//{
//	return SelectedTextureItems.Contains(AssetData);
//}

void SMainTabWidget::SelectTexture(TSharedPtr<FMIForgeTextureInfo> Item)
{
	SelectItem(SelectedTextureItems, Item);
	RefreshValidationSummary();
}

void SMainTabWidget::UnselectTexture(TSharedPtr<FMIForgeTextureInfo> Item)
{
	UnselectItem(SelectedTextureItems, Item);
	RefreshValidationSummary();
}

bool SMainTabWidget::IsTextureSelected(TSharedPtr<FMIForgeTextureInfo> Item) const
{
	return IsItemSelected(SelectedTextureItems, Item);

}

void SMainTabWidget::SelectTextureSet(TSharedPtr<FMIForgeTextureSet> Item)
{
	SelectItem(SelectedTextureSetItems, Item);
	RefreshValidationSummary();
}

void SMainTabWidget::UnselectTextureSet(TSharedPtr<FMIForgeTextureSet> Item)
{
	UnselectItem(SelectedTextureSetItems, Item);
	RefreshValidationSummary();
}

bool SMainTabWidget::IsTextureSetSelected(TSharedPtr<FMIForgeTextureSet> Item) const
{
	return IsItemSelected(SelectedTextureSetItems, Item);
}

void SMainTabWidget::SelectTexturesInSet(const FMIForgeTextureSet& TextureSet)
{
	auto SelectTextureType = [this, &TextureSet](EMIForgeTextureType Type)
		{
			if (const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type))
			{
				TSharedPtr<FMIForgeTextureInfo> ExistingItem = FindTextureListItem(*TextureInfo);

				if (ExistingItem.IsValid())
				{
					SelectedTextureItems.AddUnique(ExistingItem);
				}
			}
		};

	if(CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("Standard"))
	{
		SelectTextureType(EMIForgeTextureType::Albedo);
		SelectTextureType(EMIForgeTextureType::Normal);
		SelectTextureType(EMIForgeTextureType::ORM);

		if (bUseEmissiveTextures)
		{
			SelectTextureType(EMIForgeTextureType::Emissive);
		}

		if (bUseDetailNormalTextures)
		{
			SelectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else if (CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("RGB Masking"))
	{
		SelectTextureType(EMIForgeTextureType::Albedo);
		SelectTextureType(EMIForgeTextureType::Normal);
		SelectTextureType(EMIForgeTextureType::RGB);

		if(bUseBaseORMTexture)
		{
			SelectTextureType(EMIForgeTextureType::ORM);
		}
		
		if(bUseDetailNormalTextureRGB)
		{
			SelectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else 
	{	//Todo: handle other presets here if needed in the future
		if (TexListView.IsValid())
		{
			TexListView->RequestListRefresh();
			RefreshValidationSummary();
		}
	}

	if (TexListView.IsValid())
	{
		TexListView->RequestListRefresh();
		RefreshValidationSummary();
	}
}

void SMainTabWidget::UnselectTexturesInSet(const FMIForgeTextureSet& TextureSet)
{
	auto UnselectTextureType = [this, &TextureSet](EMIForgeTextureType Type)
		{
			if (const FMIForgeTextureInfo* TextureInfo = TextureSet.Textures.Find(Type))
			{
				TSharedPtr<FMIForgeTextureInfo> ExistingItem = FindTextureListItem(*TextureInfo);

				if (ExistingItem.IsValid())
				{
					SelectedTextureItems.Remove(ExistingItem);
				}
			}
		};
	if (CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("Standard"))
	{
		UnselectTextureType(EMIForgeTextureType::Albedo);
		UnselectTextureType(EMIForgeTextureType::Normal);
		UnselectTextureType(EMIForgeTextureType::ORM);

		if (bUseEmissiveTextures)
		{
			UnselectTextureType(EMIForgeTextureType::Emissive);
		}

		if (bUseDetailNormalTextures)
		{
			UnselectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else if(CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("RGB Masking"))
	{
		UnselectTextureType(EMIForgeTextureType::Albedo);
		UnselectTextureType(EMIForgeTextureType::Normal);
		UnselectTextureType(EMIForgeTextureType::RGB);
		if (bUseBaseORMTexture)
		{
			UnselectTextureType(EMIForgeTextureType::ORM);
		}
	
		if (bUseDetailNormalTextureRGB)
		{
			UnselectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else
	{	
		if (TexListView.IsValid())
		{
			TexListView->RequestListRefresh();
			RefreshValidationSummary();
		}
	}

	if (TexListView.IsValid())
	{
		TexListView->RequestListRefresh();
		RefreshValidationSummary();
	}
}

void SMainTabWidget::QueueListRefresh()
{
	check(IsInGameThread());

	if (bRefreshQueued)
	{
		return;
	}

	bRefreshQueued = true;

	TWeakPtr<SMainTabWidget> WeakMainTabWidget = SharedThis(this);

	RefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakMainTabWidget](float)
			{
				if (TSharedPtr<SMainTabWidget> PinnedTab =
					WeakMainTabWidget.Pin())
				{
					PinnedTab->bRefreshQueued = false;
					PinnedTab->RefreshTickerHandle.Reset();
					PinnedTab->RefreshListViews();
				}

				return false; // Run once.
			}),
		0.0f // Next editor tick.
	);
}

void SMainTabWidget::RefreshFilteredTextures()
{
	FilteredTextureListItems.Empty();

	const FString SearchString =
		CurrentSearchText.ToString().TrimStartAndEnd();

	const bool bUseTypeFilter =
		CurrentTextureFilterOption.IsValid() &&
		*CurrentTextureFilterOption != TEXT("All");

	EMIForgeTextureType FilterType = EMIForgeTextureType::Unknown;

	if (bUseTypeFilter)
	{
		FMIForgeTextureClassifier Classifier;
		FilterType = Classifier.TextureTypeFromName(*CurrentTextureFilterOption);
	}

	for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo : TextureListItems)
	{
		if (!TextureInfo.IsValid())
		{
			continue;
		}

		if (bUseTypeFilter && TextureInfo->TextureType != FilterType)
		{
			continue;
		}

		if (!SearchString.IsEmpty())
		{
			const bool bMatchesSearch =
				TextureInfo->AssetName.Contains(SearchString, ESearchCase::IgnoreCase) ||
				TextureInfo->BaseName.Contains(SearchString, ESearchCase::IgnoreCase) ||
				TextureInfo->MatchedSuffix.Contains(SearchString, ESearchCase::IgnoreCase);

			if (!bMatchesSearch)
			{
				continue;
			}
		}

		FilteredTextureListItems.Add(TextureInfo);
	}

	if (TexListView.IsValid())
	{
		TexListView->RequestListRefresh();
	}
}

void SMainTabWidget::RefreshFilteredTextureSets()
{
	FilteredTextureSetListItems.Empty();

	FMIForgeValidator Validator;

	const FString SearchString =
		CurrentSearchText.ToString().TrimStartAndEnd();

	const bool bUseTypeFilter =
		CurrentTextureSetFilterOption.IsValid() &&
		*CurrentTextureSetFilterOption != TEXT("All");
		

		for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : TextureSetListItems)
		{
			if (!TextureSet.IsValid())
			{
				continue;
			}

			if (bUseTypeFilter)
			{
				FMIForgeValidator::EMIForgeTextureSetStatus Status;
				if (CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("RGB Masking"))
				{
					Status = Validator.GetRGBSetStatus(
						*TextureSet,
						bUseBaseORMTexture,
						bEnableEmissiveChannel,
						bUseDetailNormalTextureRGB,
						bIgnoreUnrecognizedTextures
					);
				}
				else
				{
					Status = Validator.GetStandardSetStatus(
						*TextureSet,
						bUseEmissiveTextures,
						bUseDetailNormalTextures,
						bIgnoreUnrecognizedTextures
					);
				}

				const FString& FilterText = *CurrentTextureSetFilterOption;

				const bool bMatches =
					(FilterText == TEXT("Ready") &&
						Status == FMIForgeValidator::EMIForgeTextureSetStatus::Ready) ||
					(FilterText == TEXT("Warning") &&
						Status == FMIForgeValidator::EMIForgeTextureSetStatus::Warning) ||
					(FilterText == TEXT("Error") &&
						Status == FMIForgeValidator::EMIForgeTextureSetStatus::Error);

				if (!bMatches)
				{
					continue;
				}
			}
			if (!SearchString.IsEmpty())
			{
				const bool bMatchesSearch =
					TextureSet->SetName.Contains(SearchString, ESearchCase::IgnoreCase);
					

				if (!bMatchesSearch)
				{
					continue;
				}
			}

			FilteredTextureSetListItems.Add(TextureSet);
		}
	

	if (TexSetListView.IsValid())
	{
		TexSetListView->RequestListRefresh();
	}
}

void SMainTabWidget::RefreshListViews()
{
	if (TexListView.IsValid() && TexSetListView.IsValid())
	{	
		SelectedTextureItems.Empty();
		SelectedTextureSetItems.Empty();

		FMIForgeAssetScanner Scanner;
		FMIForgeTextureClassifier Classifier;
		FMIForgeTextureSetBuilder SetBuilder;

		TArray<FAssetData> FoundTextures = Scanner.FindTexturesInFolders(SelectedFolderPaths);
		TArray<FMIForgeTextureInfo> ClassifiedTextureInfos = Classifier.ClassifyTextures(FoundTextures);
		TArray<FMIForgeTextureSet> TextureSets = SetBuilder.BuildTextureSets(ClassifiedTextureInfos);

		TextureListItems.Empty();
		TextureListItems.Reserve(ClassifiedTextureInfos.Num());
		for (FMIForgeTextureInfo& TextureInfo : ClassifiedTextureInfos)
		{
			TextureListItems.Add(MakeShared<FMIForgeTextureInfo>(MoveTemp(TextureInfo)));
		}

		TextureSetListItems.Empty();
		TextureSetListItems.Reserve(TextureSets.Num());
		for (FMIForgeTextureSet& TextureSet : TextureSets)
		{
			TextureSetListItems.Add(MakeShared<FMIForgeTextureSet>(MoveTemp(TextureSet)));
		}

		RefreshFilteredTextures();
		RefreshFilteredTextureSets();

		TexListView->RebuildList();
	
		TexSetListView->RebuildList();
	}
}

TSharedPtr<FMIForgeTextureInfo> SMainTabWidget::FindTextureListItem(const FMIForgeTextureInfo& TextureInfo) const
{
	const FString TargetPath = TextureInfo.ObjectPath;

	for (const TSharedPtr<FMIForgeTextureInfo>& Item : TextureListItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (Item->ObjectPath == TargetPath)
		{
			return Item;
		}
	}

	return nullptr;
}

TSharedPtr<SWidget> SMainTabWidget::GenerateRightClickMenuWidget()
{
	FMenuBuilder MenuBuilder(true, TSharedPtr<FUICommandList>());

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Browse to Assets")),
		FText::FromString(TEXT("Opens the content browser and selects the assets.")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
			{
				if(CurrentInputMode == EMIForgeInputMode::IndividualTextures)
				{
					if (SelectedTextureItems.Num() > 0) {
						TArray<FAssetData> AssetsToSelect;
						for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo : SelectedTextureItems)
						{
							if (TextureInfo.IsValid())
							{
								FAssetData AssetData = TextureInfo.Get()->AssetData;
								AssetsToSelect.Add(AssetData);
							}
						}
						if (!AssetsToSelect.IsEmpty())
						{
							MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(AssetsToSelect);
						}
					}
					else
					{
						TArray<FAssetData> AssetsToSelect;
						TArray<TSharedPtr<FMIForgeTextureInfo>> AssetsSelected = TexListView->GetSelectedItems();
						for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo : AssetsSelected)
						{
							if (TextureInfo.IsValid())
							{
								FAssetData AssetData = TextureInfo.Get()->AssetData;
								AssetsToSelect.Add(AssetData);
							}
						}
						if (!AssetsToSelect.IsEmpty())
						{
							MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(AssetsToSelect);
						}
						
					}
				}
				else
				{
					if (SelectedTextureSetItems.Num() > 0) {
						TArray<FAssetData> AssetsToSelect;
						for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : SelectedTextureSetItems)
						{
							if (TextureSet.IsValid())
							{
								const FMIForgeTextureSet& Set = *TextureSet;
								for (const auto& Pair : Set.Textures)
								{
									const FMIForgeTextureInfo& TextureInfo = Pair.Value;
									FAssetData AssetData = TextureInfo.AssetData;
									AssetsToSelect.Add(AssetData);
								}
							}
						}
						if (!AssetsToSelect.IsEmpty())
						{
							MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(AssetsToSelect);
						}
					}
					else
					{
						TArray<FAssetData> AssetsToSelect;
						TArray<TSharedPtr<FMIForgeTextureSet>> AssetsSelected = TexSetListView->GetSelectedItems();
						for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : AssetsSelected)
						{
							if (TextureSet.IsValid())
							{
								const FMIForgeTextureSet& Set = *TextureSet;
								for (const auto& Pair : Set.Textures)
								{
									const FMIForgeTextureInfo& TextureInfo = Pair.Value;
									FAssetData AssetData = TextureInfo.AssetData;
									AssetsToSelect.Add(AssetData);
								}
							}
						}
						if (!AssetsToSelect.IsEmpty())
						{
							MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(AssetsToSelect);
						}
					}
				}
			}))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Refresh")),
		FText::FromString(TEXT("Refreshes the list of assets.")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
			{
				RefreshFilteredTextures();
				RefreshFilteredTextureSets();
				TexListView->RebuildList();
				TexSetListView->RebuildList();
			}))
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Clear Selection")),
		FText::FromString(TEXT("Clears the selection of assets.")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
			{
				SelectedTextureItems.Empty();
				SelectedTextureSetItems.Empty();
				TexListView->ClearSelection();
				TexSetListView->ClearSelection();
				RefreshValidationSummary();
			}))
		);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Ignore Unrecognized Textures")),
		FText::FromString(TEXT("Exclude unrecognized and needless textures in the current preset")),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
				{
					if (CurrentInputMode == EMIForgeInputMode::TextureSets)
					{
						bIgnoreUnrecognizedTextures = !bIgnoreUnrecognizedTextures;

						RefreshValidationSummary();
						RefreshFilteredTextureSets();

						if (TexSetListView.IsValid())
						{
							TexSetListView->RequestListRefresh();
						}

					}
				}),
			FCanExecuteAction::CreateLambda([this]()
				{
					return CurrentInputMode == EMIForgeInputMode::TextureSets;
				}),
			FIsActionChecked::CreateLambda([this]()
				{
					return bIgnoreUnrecognizedTextures;
				})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SMainTabWidget::Page1()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(2.5f)
		.Padding(5.0f)
		[
#pragma region "Left"
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					PresetComboBox()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				[
					PresetPannelSwitcher()
				]
				
		]
#pragma endregion
		+ SHorizontalBox::Slot()
		.FillWidth(0.025f)
		.Padding(1.0f)
		[
			SNew(SSeparator)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(5.5f)
		[
#pragma region "Right"
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[	
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(6.0f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT(" Source Texture Folder: ")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))

						]
						
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(6.0f)
						[
							SAssignNew(TexturePathTextBlock, SInlineEditableTextBlock)
							.Text(FText::FromString(TEXT("Please specify a path that includes your textures")))
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked_Lambda([this]() {
								MIForgeUtilities::CreatePathSelector(SharedThis(this), FOnPathSelected::CreateLambda([this](const FString& SelectedPath) {
									SelectedFolderPaths.Empty();
									SelectedTextureSetItems.Empty();
									SelectedFolderPaths.Add(SelectedPath);
									TexturePathTextBlock->SetText(FText::FromString(SelectedPath));
									RefreshValidationSummary();
									QueueListRefresh();
									
									}));
								return FReply::Handled();
									})
								[
									SNew(SImage)
										.Image(FMIForgeStyle::Get().GetBrush("Panel.FolderSelection"))
								]
						]
						
						
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[	
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						.Padding(6.0f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT(" View Mode: ")))
								
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(SSegmentedControl<int32>)
								.IsEnabled_Lambda([this]()
									{
										return !CurrentPresetOption.IsValid() ||
											*CurrentPresetOption != TEXT("Vertex Painting");
									})
								.Value_Lambda([this]()
									{
										return CurrentInputMode == EMIForgeInputMode::TextureSets ? 1 : 0;
									})
								.OnValueChanged_Lambda([this](int32 Val) {

								CurrentInputMode = Val == 1 ? EMIForgeInputMode::TextureSets : EMIForgeInputMode::IndividualTextures;
								ViewModeSwitcher0->SetActiveWidgetIndex(Val);

								if (CurrentInputMode == EMIForgeInputMode::IndividualTextures)
								{
									ActiveFilterOptions = TextureFilterOptions;
									CurrentFilterOption = CurrentTextureFilterOption;
									RefreshFilteredTextures();
								}
								else
								{
									ActiveFilterOptions = TextureSetFilterOptions;
									CurrentFilterOption = CurrentTextureSetFilterOption;
									RefreshFilteredTextureSets();
								}

								if (FilterComboBox.IsValid())
								{
									FilterComboBox->RefreshOptions();
								}

								if (CurrentFilterComboBoxSelectedOptionText.IsValid() &&
									CurrentFilterOption.IsValid())
								{
									CurrentFilterComboBoxSelectedOptionText->SetText(
										FText::FromString(*CurrentFilterOption)
									);
								}

								RefreshValidationSummary();

									})

								+ SSegmentedControl<int32>::Slot(0)
								.Text(LOCTEXT("Individual", "Individual"))

								+ SSegmentedControl<int32>::Slot(1)
								.Text(LOCTEXT("TexSet", "Texture Set"))
						]
						
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNullWidget::NullWidget
						]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							.Padding(6.0f)
							[
								SAssignNew(FilterComboBox, SComboBox<TSharedPtr<FString>>)
									.OptionsSource(&ActiveFilterOptions)
									.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) {

									return SNew(STextBlock)
										.Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("")));

										})
									.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type SelectInfo) {

									if (!Option.IsValid())
									{
										return;
									}

									CurrentFilterOption = Option;

									if (CurrentInputMode == EMIForgeInputMode::IndividualTextures)
									{
										CurrentTextureFilterOption = Option;
										RefreshFilteredTextures();
									}
									else
									{
										CurrentTextureSetFilterOption = Option;
										RefreshFilteredTextureSets();
									}

									CurrentFilterComboBoxSelectedOptionText->SetText(FText::FromString(*Option));
										})

									.Content()
									[
										SAssignNew(CurrentFilterComboBoxSelectedOptionText, STextBlock)
											.Text(FText::FromString(TEXT("All")))
									]

							]

						+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(6.0f)
							[
								SNew(SSearchBox)
									.OnTextChanged_Lambda([this](const FText& InText) {
										CurrentSearchText = InText;
										if (CurrentInputMode == EMIForgeInputMode::IndividualTextures)
										{
											RefreshFilteredTextures();

											if (TexListView.IsValid())
											{
												TexListView->RequestListRefresh();
											}
										}
										else
										{
											RefreshFilteredTextureSets();

											if (TexSetListView.IsValid())
											{
												TexSetListView->RequestListRefresh();
											}
										}
									})	
							]
						
					
				]
				+SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(6.f)
					[	
						
						SNew(SBorder)
							.BorderImage(&ListViewBorderBrush)
							.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.Padding(6.0f)
							[
								SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.HAlign(HAlign_Left)
											.VAlign(VAlign_Center)
											.Padding(8.0f)
											.AutoWidth()
											[
												SNew(STextBlock)

													.Text_Lambda([this]() {
													if (CurrentInputMode == EMIForgeInputMode::IndividualTextures)
													{
														return FText::FromString(FString::Printf(TEXT("Detected Textures: %d/%d"), FilteredTextureListItems.Num(), TextureListItems.Num()));
													}
													else
													{
														return FText::FromString(FString::Printf(TEXT("Detected Texture Sets: %d/%d"), FilteredTextureSetListItems.Num(), TextureSetListItems.Num()));
													}
														})
													.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11.5f))
											]
										+ SHorizontalBox::Slot()
											.FillWidth(1.0f)
											[
												SNullWidget::NullWidget
											]
										+ SHorizontalBox::Slot()
											.HAlign(HAlign_Right)
											.VAlign(VAlign_Center)
											.Padding(2.0f)
											.AutoWidth()
											[
												SNew(SButton)
													.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
													.OnClicked_Lambda([this]() { 
													RefreshListViews();
													return FReply::Handled(); })
													[
														SNew(SImage)
															.Image(FMIForgeStyle::Get().GetBrush("Panel.RefreshButton"))
													]
											]
									]
									+ SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										RightContentWidget()

									]
							]
					]
				
		]
#pragma endregion
	;
}

TSharedRef<SWidget> SMainTabWidget::PresetComboBox()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(3.0f)
		[
			SNew(STextBlock)
			
				.Text(FText::FromString(TEXT("Master Material Preset: ")))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(new TArray<TSharedPtr<FString>>{ MakeShared<FString>(TEXT("Standard")), MakeShared<FString>(TEXT("RGB Masking")), MakeShared<FString>(TEXT("Vertex Painting")) })
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) {
				return SNew(STextBlock).Text(FText::FromString(*InItem));
					})
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type SelectInfo) {

				if (Option.IsValid())
				{
					CurrentPresetOption = Option;
					PresetComboBoxSelectedOptionText->SetText(FText::FromString(*Option));

					if (*CurrentPresetOption == FString("Standard")) {
						PresetPannelSwitcher0->SetActiveWidgetIndex(0);
						TexSetListView->RequestListRefresh();
						RefreshValidationSummary();
					}
					else if (*CurrentPresetOption == FString("RGB Masking")) {
						PresetPannelSwitcher0->SetActiveWidgetIndex(1);
						TexSetListView->RequestListRefresh();
						RefreshValidationSummary();
					}
					else if (*CurrentPresetOption == FString("Vertex Painting")) {
						PresetPannelSwitcher0->SetActiveWidgetIndex(2);

						CurrentInputMode = EMIForgeInputMode::TextureSets;

						if (ViewModeSwitcher0.IsValid())
						{
							ViewModeSwitcher0->SetActiveWidgetIndex(1);
						}

						SelectedTextureItems.Empty();

						ActiveFilterOptions = TextureSetFilterOptions;
						CurrentFilterOption = CurrentTextureSetFilterOption;

						if (FilterComboBox.IsValid())
						{
							FilterComboBox->RefreshOptions();
						}

						if (CurrentFilterComboBoxSelectedOptionText.IsValid() &&
							CurrentFilterOption.IsValid())
						{
							CurrentFilterComboBoxSelectedOptionText->SetText(
								FText::FromString(*CurrentFilterOption)
							);
						}

						RefreshFilteredTextureSets();
						RefreshValidationSummary();

						if (TexSetListView.IsValid())
						{
							TexSetListView->RequestListRefresh();
						}
					}
				}
				
					})
				.Content()
				[
					SAssignNew(PresetComboBoxSelectedOptionText, STextBlock)
						.Text(FText::FromString(TEXT("Select a Preset")))
				]
		];
}

TSharedRef<SWidget> SMainTabWidget::PresetPannelSwitcher()
{
	return SAssignNew(PresetPannelSwitcher0, SWidgetSwitcher)

		+ SWidgetSwitcher::Slot()
		[
			StandardPresetPannel()

		]

		+ SWidgetSwitcher::Slot()
		[
			RGBmaskingPresetPannel()
		]

		+ SWidgetSwitcher::Slot()
		[
			VertexPaintingPresetPannel()
		]
		;

		
}

TSharedRef<SWidget> SMainTabWidget::ListViewSwitcher()
{
	return SAssignNew(ViewModeSwitcher0, SWidgetSwitcher)
		

		+ SWidgetSwitcher::Slot()
		[
			IndividualModeListView()
		]

		+ SWidgetSwitcher::Slot()
		[
			TextureSetModeListView()
		]
	;
}

TSharedRef<SWidget> SMainTabWidget::IndividualModeListView()
{
	return SAssignNew(TexListView, SListView<TSharedPtr<FMIForgeTextureInfo>>)
		.ListItemsSource(&FilteredTextureListItems)
		.OnGenerateRow_Lambda([this](TSharedPtr<FMIForgeTextureInfo> Item, const TSharedRef<STableViewBase>& OwnerTable) {
		return SNew(STextureTableRow, OwnerTable)
			.TextureListItems(Item)
			.ParentTable(SharedThis(this));
		
			})
		.HeaderRow(SetupHeaderRow())
		.OnContextMenuOpening(this, &SMainTabWidget::GenerateRightClickMenuWidget);

		;
}

TSharedRef<SWidget> SMainTabWidget::TextureSetModeListView()
{
	return SAssignNew(TexSetListView, SListView<TSharedPtr<FMIForgeTextureSet>>)
		.ListItemsSource(&FilteredTextureSetListItems)
		.OnGenerateRow_Lambda([this](TSharedPtr<FMIForgeTextureSet> Item, const TSharedRef<STableViewBase>& OwnerTable) {
		return SNew(STextureSetTableRow, OwnerTable)
			.TextureSets(Item)
			.ParentTable(SharedThis(this));

			})
		.HeaderRow(SetupTexSetHeaderRow())
		.OnContextMenuOpening(this, &SMainTabWidget::GenerateRightClickMenuWidget);

		;
}

TSharedPtr<SHeaderRow> SMainTabWidget::SetupHeaderRow()
{
	return SNew(SHeaderRow)
		+ SHeaderRow::Column(FName("Select"))
		.FixedWidth(28.f)
		.HAlignHeader(HAlign_Center) // align the content in the header to the center	
		.HAlignCell(HAlign_Center) // align the content in the cell to the center	
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState) {
				switch (CheckState) {

					// when the checkbox state changes, this lambda function will be called. If the checkbox is checked, all assets will be selected. If the checkbox is unchecked, all assets will be unselected.
				case ECheckBoxState::Checked:
					SelectedTextureItems = FilteredTextureListItems;
					RefreshValidationSummary();
					break;
				case ECheckBoxState::Unchecked:
					SelectedTextureItems.Empty();
					SelectedTextureSetItems.Empty();
					RefreshValidationSummary();
					break;
				}
					})
				.IsChecked_Lambda([this] {
				if (SelectedTextureItems.Num() == 0) {
					return ECheckBoxState::Unchecked;
				}
				else if (SelectedTextureItems.Num() == TextureListItems.Num()) {
					return ECheckBoxState::Checked;
				}
				else {
					return ECheckBoxState::Undetermined; // when some but not all assets are selected, the checkbox will be in an undetermined state
				}
					})
		]

		+ SHeaderRow::Column(FName("AssetName")) // ID
		.FillWidth(0.5f)
		.DefaultLabel(FText::FromString("[Asset Name]")) // name displayed in the header

		+ SHeaderRow::Column(FName("TextureSize")) // ID
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString("[Texture Size]")) // name displayed in the header


		+ SHeaderRow::Column(FName("AssetReferencers"))
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString("[Asset Referencers]"))

		;
}

TSharedPtr<SHeaderRow> SMainTabWidget::SetupTexSetHeaderRow()
{
	return SNew(SHeaderRow)
		+ SHeaderRow::Column(FName("Select"))
		.FixedWidth(28.f)
		.HAlignHeader(HAlign_Center) // align the content in the header to the center	
		.HAlignCell(HAlign_Center) // align the content in the cell to the center	
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState) {
				switch (CheckState) {

					// when the checkbox state changes, this lambda function will be called. If the checkbox is checked, all assets will be selected. If the checkbox is unchecked, all assets will be unselected.
				case ECheckBoxState::Checked:
					SelectedTextureSetItems = FilteredTextureSetListItems;
					for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : FilteredTextureSetListItems)
					{
						SelectTexturesInSet(*TextureSet);
					}
					break;
				case ECheckBoxState::Unchecked:
					SelectedTextureSetItems.Empty();
					for (const TSharedPtr<FMIForgeTextureSet>& TextureSet : FilteredTextureSetListItems)
					{
						UnselectTexturesInSet(*TextureSet);
					}
					break;
				}
					})
				.IsChecked_Lambda([this] {
				if (SelectedTextureSetItems.Num() == 0) {
					return ECheckBoxState::Unchecked;
				}
				else if (SelectedTextureSetItems.Num() == TextureSetListItems.Num()) {
					return ECheckBoxState::Checked;
				}
				else {
					return ECheckBoxState::Undetermined; // when some but not all assets are selected, the checkbox will be in an undetermined state
				}
					})
		]
		+SHeaderRow::Column(FName("Status")) // ID
		.FillWidth_Lambda([this]() {
			if (CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("Vertex Painting"))
			{
				return 0.08f;
			}
			else
			{
				return 0.04f;
			}
			})
		.HAlignCell(HAlign_Center) 
		.VAlignCell(VAlign_Center)
		.HeaderContent()
		[	
			SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
						.Image(FMIForgeStyle::Get().GetBrush("ListView.Row.Info"))
				]
			
		]

		+ SHeaderRow::Column(FName("AssetName")) // ID
		.FillWidth(0.5f)
		.DefaultLabel(FText::FromString("[Asset Name]")) // name displayed in the header

		+ SHeaderRow::Column(FName("TextureSize")) // ID
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString("[Texture Size]")) // name displayed in the header



		;
}

TSharedRef<SWidget> SMainTabWidget::RightContentWidget()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(TAttribute<float>::CreateLambda([this]()
			{
				if (CurrentPresetOption.IsValid() &&
					*CurrentPresetOption == TEXT("Vertex Painting"))
				{
					return 0.5f;
				}

				return 1.0f;
			}))
		.Padding(0.0f, 0.0f, 6.0f, 0.0f)
		[
			ListViewSwitcher()
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		.Padding(6.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBorder)
				.Visibility_Lambda([this]()
					{
						return CurrentPresetOption.IsValid() &&
							*CurrentPresetOption == TEXT("Vertex Painting")
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(6.0f)
				[
					VertexPaintingLayerStackPanel()
				]
		];
}

TSharedRef<SWidget> SMainTabWidget::StandardPresetPannel()
{
	TSharedRef<SVerticalBox> MainContainer = SNew(SVerticalBox);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Generate Standard material instances. (ORM Workflow)")))
				.ColorAndOpacity(FLinearColor(.15f, .15f, .15f, 1.f))
		];

	TargetFolderSection(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SSeparator)
		];

	StandardMIOptionSection(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)

		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Instance Options")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		];

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			IfMIExistsOptionBlock()
		];
	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			ValidationSummarySection()
		];

	GenerateStandardMIButton(MainContainer);

	return MainContainer;
}

TSharedRef<SWidget> SMainTabWidget::RGBmaskingPresetPannel()
{	
	TSharedRef<SVerticalBox> MainContainer = SNew(SVerticalBox);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Generate RGB Mask material instances. (ORM Workflow)")))
				.ColorAndOpacity(FLinearColor(.15f, .15f, .15f, 1.f))
		];

	TargetFolderSection(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SSeparator)
		];

	RGBmaskingMIOptionSection(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)

		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Instance Options")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		];

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			IfMIExistsOptionBlock()
		];
	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			ValidationSummarySection()
		];

	GenerateRGBmaskingMIButton(MainContainer);

	return MainContainer;

}

TSharedRef<SWidget> SMainTabWidget::VertexPaintingPresetPannel()
{
	TSharedRef<SVerticalBox> MainContainer = SNew(SVerticalBox);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Generate Vertex Paint material instances with layer stack.")))
				.ColorAndOpacity(FLinearColor(.15f, .15f, .15f, 1.f))
		];

	TargetFolderSection(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SSeparator)
		];

	VertexPaintGenerateNameTextBox(MainContainer);

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SSeparator)
		];

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)

		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Instance Options")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		];

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			IfMIExistsOptionBlock()
		];

	MainContainer->AddSlot()
		.AutoHeight()
		.Padding(4.f, 10.f, 4.f, 4.f)
		[
			ValidationSummarySection()
		];

	GenerateVertexPaintMIButton(MainContainer);

	return MainContainer;
}

TSharedRef<SWidget> SMainTabWidget::VertexPaintingLayerStackPanel()
{
	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);
	VertexPaintingMIOptionSection(Container);

	Container->AddSlot()
		.FillHeight(1.f)
		[
			SNew(SSpacer)
		];

	VertexPaintRecipeSection(Container);
	return Container;
}

TSharedRef<SWidget> SMainTabWidget::ValidationSummaryText()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Validation Summary:\n")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text_Lambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Ready to create: {0}/{1} MI(s)")),
							CurrentValidationSummary.ReadyToCreateCount,
							CurrentValidationSummary.TotalSets
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						if (CurrentValidationSummary.ReadyToCreateCount == 0)
							return FSlateColor(FLinearColor::Red);
						else if (CurrentValidationSummary.ReadyToCreateCount < CurrentValidationSummary.TotalSets)
							return FSlateColor(FLinearColor::Red);
						else
							return CurrentValidationSummary.ReadyToCreateCount == CurrentValidationSummary.TotalSets
							? FSlateColor(FLinearColor::Green)
							: FSlateColor(FLinearColor::Red);
					})
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text_Lambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Missing required textures: {0}")),
							CurrentValidationSummary.MissingRequiredTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return CurrentValidationSummary.MissingRequiredTextureCount == 0
							? FSlateColor(FLinearColor::Green)
							: FSlateColor(FLinearColor::Red);
					})
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text_Lambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Missing optional textures: {0}")),
							CurrentValidationSummary.MissingOptionalTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return CurrentValidationSummary.MissingOptionalTextureCount == 0
							? FSlateColor(FLinearColor::Green)
							: FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f));
					})
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text_Lambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Unrecognized textures: {0}")),
							CurrentValidationSummary.UnrecognizedTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return CurrentValidationSummary.UnrecognizedTextureCount == 0
							? FSlateColor(FLinearColor::Green)
							: FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f));
					})
		];
}

TSharedRef<SWidget> SMainTabWidget::VertexPaintValidationSummaryText()
{
	auto MakeSummaryLine =
		[](TAttribute<FText> Text, TAttribute<FSlateColor> Color)
		{
			return SNew(STextBlock)
				.Text(Text)
				.ColorAndOpacity(Color)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10.f));
		};

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Vertex Paint Validation Summary: \n")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Ready to generate: {0} \n")),
							CurrentVertexPaintValidationSummary.bCanGenerate
							? FText::FromString(TEXT("Yes"))
							: FText::FromString(TEXT("No"))
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return CurrentVertexPaintValidationSummary.bCanGenerate
							? FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.0f))
							: FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.0f));
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Layers Assigned: {0} / 4")),
							CurrentVertexPaintValidationSummary.AssignedLayerCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return CurrentVertexPaintValidationSummary.AssignedLayerCount < 2
							? FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.0f))
							: FSlateColor::UseForeground();
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Required Missing: {0}")),
							CurrentVertexPaintValidationSummary.MissingRequiredTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return CurrentVertexPaintValidationSummary.MissingRequiredTextureCount > 0
							? FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.0f))
							: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.0f));
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Optional Missing: {0}")),
							CurrentVertexPaintValidationSummary.MissingOptionalTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return CurrentVertexPaintValidationSummary.MissingOptionalTextureCount > 0
							? FSlateColor(FLinearColor(1.0f, 0.75f, 0.0f, 1.0f))
							: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.0f));
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Unrecognized Textures: {0} \n")),
							CurrentVertexPaintValidationSummary.UnrecognizedTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return CurrentVertexPaintValidationSummary.UnrecognizedTextureCount > 0
							? FSlateColor(FLinearColor(1.0f, 0.75f, 0.0f, 1.0f))
							: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.0f));
					})
			)
		]


		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("Base Layer: {0}")),
							VertexPaintStatusToText(CurrentVertexPaintValidationSummary.BaseStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							CurrentVertexPaintValidationSummary.BaseStatus
						);
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("R Layer: {0}")),
							VertexPaintStatusToText(CurrentVertexPaintValidationSummary.LayerRStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							CurrentVertexPaintValidationSummary.LayerRStatus
						);
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("G Layer: {0}")),
							VertexPaintStatusToText(CurrentVertexPaintValidationSummary.LayerGStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							CurrentVertexPaintValidationSummary.LayerGStatus
						);
					})
			)
		]

	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeSummaryLine(
				TAttribute<FText>::CreateLambda([this]()
					{
						return FText::Format(
							FText::FromString(TEXT("B Layer: {0}")),
							VertexPaintStatusToText(CurrentVertexPaintValidationSummary.LayerBStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							CurrentVertexPaintValidationSummary.LayerBStatus
						);
					})
			)
		];
}

TSharedRef<SWidget> SMainTabWidget::ValidationSummarySection()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder")) // use editor style border brush
		.Padding(FMargin(6.0f))
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.f)
				[	
					SNew(SWidgetSwitcher)
						.WidgetIndex_Lambda([this]() -> int32 {
						// Re-evaluates every frame - fully dynamic
						return (CurrentPresetOption.IsValid() &&
							*CurrentPresetOption == TEXT("Vertex Painting")) ? 1 : 0;
							})

						+ SWidgetSwitcher::Slot() // Index 0: Standard + RGB Masking
						[
							ValidationSummaryText()
						]

						+ SWidgetSwitcher::Slot() // Index 1: Vertex Painting
						[
							VertexPaintValidationSummaryText()
						]

				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.f)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "FlatButton")
						.Text(FText::FromString(TEXT("View Details")))
						.OnClicked_Lambda([this]() {
						if(CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("Standard"))
						{
							PopupWindowCreator::OpenPopupWindow(FText::FromString(TEXT("Validation Details")),
								CreateStandardValidationDetailsWidget(),
								FVector2D(560.f, 460.f),
								true
							);
						}
						else if (CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("RGB Masking"))
						{
							PopupWindowCreator::OpenPopupWindow(FText::FromString(TEXT("Validation Details")),
								CreateRGBmaskingValidationDetailsWidget(),
								FVector2D(560.f, 460.f),
								true
							);
						}
						else if(CurrentPresetOption.IsValid() && *CurrentPresetOption == TEXT("Vertex Painting"))
						{
							return FReply::Handled();
						}
						
						return FReply::Handled();
							})
				]

		];
}

void SMainTabWidget::GenerateStandardMIButton(TSharedRef<SVerticalBox> Container)
{	
	Container->AddSlot()
		.FillHeight(12.f)
		.Padding(10.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
				.OnClicked_Lambda([this]() {
				if (CurrentTargetPath.IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}
				if (CurrentValidationSummary.SetsWithErrors > 0)
				{
					MIForgeUtilities::PrintWindow((TEXT("Please fix all the errors before proceeding. (See 'Validation Summary' or click on 'View Details' for more information)")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				FMIForgeMaterialGenerationRequest Request;

				Request.TextureSets = BuildGenerationTextureSets();
				Request.Options.Preset = EMIForgeGenerationPreset::Standard;
				Request.Options.TargetPath = CurrentTargetPath;
				Request.Options.bUseEmissive = bUseEmissiveTextures;
				Request.Options.bUseDetailNormal = bUseDetailNormalTextures;
				Request.Options.bUseTriplanar = bUseTriplanarProjection;
				Request.Options.IfMIExists = CurrentIfMIExistsOption;

				
				const FMIForgeGenerationOutcome Result = FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Request);

				MIForgeUtilities::PrintNotification(Result.SummaryText.ToString(), 5.f);

				return FReply::Handled();
					})

				[
					SNew(STextBlock)
						.Text_Lambda([this]() { return this->GetMIReadyCount(); })
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.0f))
				]
		];
}



TSharedRef<SWidget> SMainTabWidget::VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer Layer)
{
	FMIForgeVertexPaintLayerSlot* Slot = GetVertexPaintLayerSlot(Layer);

	TSharedPtr<SBox>* ThumbnailBoxPtr = nullptr;

	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base:
		ThumbnailBoxPtr = &BaseLayerThumbnailBox;
		break;

	case EMIForgeVertexPaintLayer::LayerR:
		ThumbnailBoxPtr = &LayerRThumbnailBox;
		break;

	case EMIForgeVertexPaintLayer::LayerG:
		ThumbnailBoxPtr = &LayerGThumbnailBox;
		break;

	case EMIForgeVertexPaintLayer::LayerB:
		ThumbnailBoxPtr = &LayerBThumbnailBox;
		break;
	}

	//static const FSlateRoundedBoxBrush LayerSlotBorderBrush(
	//	FLinearColor(0.03f, 0.03f, 0.03f, 1.f), // fill color
	//	5.0f,                                  // corner radius
	//	FLinearColor(0.20f, 0.20f, 0.20f, 1.f), // outline color
	//	1.0f                                   // outline thickness
	//);

	static const FSlateRoundedBoxBrush LayerSlotBorderBrush(
		FLinearColor(0.03f, 0.03f, 0.03f, 1.f),
		5.0f,
		FLinearColor(0.20f, 0.20f, 0.20f, 1.f),
		1.0f
	);

	static const FSlateRoundedBoxBrush ThumbnailBorderBrush(
		FLinearColor(0.f, 0.f, 0.f, 0.f),
		5.0f,
		FLinearColor(0.20f, 0.20f, 0.20f, 1.f),
		0.5f
	);

	return SNew(SMIForgeTextureSetDropTarget)
		.Layer(Layer)
		.BorderImage(&LayerSlotBorderBrush)
		.Padding(6.0f)
		.OnTextureSetDropped_Lambda(
			[this, Layer](const TSharedPtr<FMIForgeTextureSet>& DroppedTextureSet, const EMIForgeVertexPaintLayer& DroppedLayer)
			{
				FMIForgeVertexPaintLayerSlot* LayerSlot = GetVertexPaintLayerSlot(DroppedLayer);

				if (!LayerSlot || !DroppedTextureSet.IsValid())
				{
					return FReply::Unhandled();
				}

				LayerSlot->AssignedTextureSet = DroppedTextureSet;

				RefreshVertexPaintLayerThumbnail(DroppedLayer);
				RefreshValidationSummary();

				return FReply::Handled();

			}
		)
		[	SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
		    .Padding(2.0f, 0.0f, 0.0f, 6.f)
			[	
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.0f))
							.Text_Lambda([this, Layer]() {
							if (FMIForgeVertexPaintLayerSlot* CurrentSlot = GetVertexPaintLayerSlot(Layer))
							{
								return FText::FromString(CurrentSlot->DisplayName);
							}
							return FText::FromString(TEXT("Unknown Layer"));
								})
					]
					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNullWidget::NullWidget
						]
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Right)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("X")))
								.OnClicked_Lambda([this, Layer]() {
								ClearVertexLayerAssignment(Layer);
								return FReply::Handled();
									})
						]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.0f, 0.0f, 8.0f, 0.0f)
						.VAlign(VAlign_Center)
						[	
							SNew(SBorder)
							.BorderImage(&ThumbnailBorderBrush)
							[
							SAssignNew(*ThumbnailBoxPtr, SBox)
								.WidthOverride(64.0f)
								.HeightOverride(64.0f)
								[
									CreateVertexPaintLayerThumbnailWidget(Layer)
								]
							]
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SNew(STextBlock)
										.Text_Lambda([this, Layer]() { 
										if (FMIForgeVertexPaintLayerSlot* CurrentSlot = GetVertexPaintLayerSlot(Layer))
										{
											if (CurrentSlot->AssignedTextureSet.IsValid())
											{
												return FText::FromString("[" +
													CurrentSlot->AssignedTextureSet->SetName + "]");
											}
										}

										return FText::FromString(TEXT("[Empty]"));
								
											})
								]
								+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 2.0f)
								[
									SNew(STextBlock)
										.Text_Lambda([this, Layer]()
											{	
												if (GetVertexPaintLayerSlot(Layer)->IsAssigned())
												{
													return FText::FromString(GetVertexPaintLayerSlot(Layer)->GetAddedTextureTypeText());
												}
												return GetVertexPaintLayerStatusText(Layer);
											})
										.ColorAndOpacity_Lambda([this, Layer]()
											{
												return GetVertexPaintLayerStatusColor(Layer);
											})
								]
								+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.0f, 2.0f)
									[
										SNew(STextBlock)
											.Text_Lambda([this, Layer]()
												{	if (GetVertexPaintLayerSlot(Layer)->IsAssigned())
													{
														return FText::FromString(GetVertexPaintLayerSlot(Layer)->GetTextureSizeText());
													}
													return FText::FromString(TEXT(""));
												})
											
									]
						
						]
						/*+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(4.0f, 0.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SButton)
									.Text(FText::FromString(TEXT("Assign Selected")))
									.OnClicked_Lambda([this, Layer]() {

										AssignSelectedTextureSetToVertexLayer(Layer);
										return FReply::Handled();
									})
							]*/
						
				    ]

		];
}

void SMainTabWidget::TargetFolderSection(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)

		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Target Folder:")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))

		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(1.f)
				[
					SNew(SEditableTextBox)
						.Text_Lambda([this]()
							{
								return FText::FromString(CurrentTargetPath);
							})
						.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
							{
								CurrentTargetPath = NewText.ToString();
							})
						.HintText(FText::FromString(TEXT("/Game/Folder/Subfolder")))


				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("SimpleButton"))
						.OnClicked_Lambda([this]() {
						MIForgeUtilities::CreatePathSelector(SharedThis(this), FOnPathSelected::CreateLambda([this](const FString& SelectedPath) {
							CurrentTargetPath = SelectedPath;
							}));
						return FReply::Handled();
							})
						[
							SNew(SImage)
								.Image(FMIForgeStyle::Get().GetBrush("Panel.FolderSelection"))
						]
				]
		];

}

void SMainTabWidget::StandardMIOptionSection(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)

		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Instance Options")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))

		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bUseEmissiveTextures = (NewState == ECheckBoxState::Checked);
						TexSetListView->RequestListRefresh();
						RefreshFilteredTextureSets();
						RefreshValidationSummary();
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bUseEmissiveTextures ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Use Emissive Texture(s) ")))
				]
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bUseDetailNormalTextures = (NewState == ECheckBoxState::Checked);
						TexSetListView->RequestListRefresh();
						RefreshFilteredTextureSets();
						RefreshValidationSummary();
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bUseDetailNormalTextures ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Use Detail Normal Texture(s) ")))
				]
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bUseTriplanarProjection = (NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bUseTriplanarProjection ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Use Triplanar ")))
				]
		];
}

void SMainTabWidget::RGBmaskingMIOptionSection(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Instance Options")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bUseBaseORMTexture = (NewState == ECheckBoxState::Checked);
						TexSetListView->RequestListRefresh();
						RefreshFilteredTextureSets();
						RefreshValidationSummary();
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bUseBaseORMTexture ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Use Base ORM Texture")))
				]
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bEnableEmissiveChannel = (NewState == ECheckBoxState::Checked);
						TexSetListView->RequestListRefresh();
						RefreshFilteredTextureSets();
						RefreshValidationSummary();
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bEnableEmissiveChannel ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Enable Emissive Channel")))
				]
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						this->bUseDetailNormalTextureRGB = (NewState == ECheckBoxState::Checked);
						TexSetListView->RequestListRefresh();
						RefreshFilteredTextureSets();
						RefreshValidationSummary();
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return this->bUseDetailNormalTextureRGB ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Use Detail Normal Texture")))
				]
		];
}

void SMainTabWidget::GenerateRGBmaskingMIButton(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.FillHeight(12.f)
		.Padding(10.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
				.OnClicked_Lambda([this]() {
				if (CurrentTargetPath.IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}
				if (CurrentValidationSummary.SetsWithErrors > 0)
				{
					MIForgeUtilities::PrintWindow((TEXT("Please fix all the errors before proceeding. (See 'Validation Summary' or click on 'View Details' for more information)")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				FMIForgeMaterialGenerationRequest Request;

				Request.TextureSets = BuildGenerationTextureSets();
				Request.Options.Preset = EMIForgeGenerationPreset::RGBMask;
				Request.Options.TargetPath = CurrentTargetPath;
				Request.Options.bUseBaseORMTexture = bUseBaseORMTexture;
				Request.Options.bEnableEmissiveChannel = bEnableEmissiveChannel;
				Request.Options.bUseDetailNormalTextureRGB =
					bUseDetailNormalTextureRGB;
				Request.Options.IfMIExists = CurrentIfMIExistsOption;

				const FMIForgeGenerationOutcome Outcome =
					FMIForgeGenerationCoordinator()
					.ExecuteMaterialGeneration(Request);

				MIForgeUtilities::PrintNotification(
					Outcome.SummaryText.ToString(),
					5.f);

				return FReply::Handled();
					})
				[
					SNew(STextBlock)
						.Text_Lambda([this]() { return this->GetMIReadyCount(); })
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.0f))
				]
		];
}

void SMainTabWidget::GenerateVertexPaintMIButton(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.FillHeight(12.f)
		.Padding(10.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SecondaryButton")
				.OnClicked_Lambda([this]() {
				if (CurrentTargetPath.IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				RefreshValidationSummary();
				if (!CurrentVertexPaintValidationSummary.bCanGenerate)
				{
					MIForgeUtilities::PrintWindow(
						TEXT("Please fix all Vertex Paint layer errors before proceeding."),
						EAppMsgType::Ok
					);
					return FReply::Handled();
				}

				FMIForgeVertexPaintGenerationRequest Request;
				Request.LayerStack = VertexPaintLayerStack;
				Request.Options.TargetPath = CurrentTargetPath;
				Request.Options.IfMIExists = CurrentIfMIExistsOption;
				

				const FString RequestedMIName =
					CurrentVertexPaintMIName.TrimStartAndEnd();

				if (!RequestedMIName.IsEmpty())
				{
					FText NameError;
					if (!FName::IsValidXName(
						RequestedMIName,
						INVALID_OBJECTNAME_CHARACTERS,
						&NameError
					))
					{
						MIForgeUtilities::PrintWindow(
							FString::Printf(TEXT("Invalid material instance name: %s"), *NameError.ToString()),
							EAppMsgType::Ok
						);
						return FReply::Handled();
					}

					Request.Options.MaterialInstanceName = RequestedMIName;
				}

				const FMIForgeGenerationOutcome Outcome =
					FMIForgeGenerationCoordinator()
					.ExecuteVertexPaintGeneration(Request);

				MIForgeUtilities::PrintNotification(
					Outcome.SummaryText.ToString(),
					5.f);
				

				return FReply::Handled();
					})
					[
						SNew(STextBlock)
							.Text(LOCTEXT("GenerateVPInstance", "Generate VP Instance"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.0f))
					]
		];

}

void SMainTabWidget::VertexPaintRecipeSection(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.Padding(4.f)
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Save Recipe")))
						.OnClicked_Lambda([this]() {
						const FString InitialName =
							CurrentVertexPaintRecipeOption.IsValid()
							? *CurrentVertexPaintRecipeOption
							: FString();

						PopupWindowCreator::OpenTextInputPopup(
							FText::FromString(TEXT("Save Recipe")),
							FText::FromString(TEXT("Recipe Name")),
							InitialName,
							[this](const FString& RecipeName)
							{
								if (RecipeName.IsEmpty())
								{
									MIForgeUtilities::PrintWindow(
										TEXT("Please enter a recipe name."),
										EAppMsgType::Ok
									);
									return;
								}

								VertexPaintRecipeManager.SaveRecipe(
									RecipeName,
									VertexPaintLayerStack
								);

								CurrentVertexPaintRecipeOption =
									MakeShared<FString>(RecipeName);

								RefreshVertexPaintRecipeOptions();
							}
						);
						return FReply::Handled();
							})
						
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.Padding(4.f)
				[
					SAssignNew(VertexPaintRecipeComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&VertexPaintRecipeOptions)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) {
						return SNew(STextBlock)
							.Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("")));
							})
						.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type SelectInfo) {

						if (Option.IsValid())
						{
							CurrentVertexPaintRecipeOption = Option;
							CurrentVertexPaintRecipeOptionText->SetText(FText::FromString(*Option));

							if (VertexPaintRecipeManager.LoadRecipe(*Option, VertexPaintLayerStack))
							{
								RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerR);
								RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerG);
								RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerB);
								RefreshValidationSummary();
							}
						}

							})
						.Content()
						[
							SAssignNew(CurrentVertexPaintRecipeOptionText, STextBlock)
								.Text(FText::FromString(TEXT("Select Recipe")))
						]
				]
				+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(4.f)
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Delete Recipe")))
						.OnClicked_Lambda([this]() {
						if (!CurrentVertexPaintRecipeOption.IsValid())
						{
							MIForgeUtilities::PrintWindow(
								TEXT("Please select a recipe to delete."),
								EAppMsgType::Ok
							);
							return FReply::Handled();
						}

						if(MIForgeUtilities::PrintWindow(
							FString::Printf(TEXT("Are you sure you want to delete the recipe '%s'?"), **CurrentVertexPaintRecipeOption),
							EAppMsgType::YesNo) == EAppReturnType::Yes)
						{
							VertexPaintRecipeManager.DeleteRecipe(*CurrentVertexPaintRecipeOption);
							CurrentVertexPaintRecipeOption.Reset();
							RefreshVertexPaintRecipeOptions();
						}

						return FReply::Handled();
							})
				]

		];

}

void SMainTabWidget::RefreshVertexPaintRecipeOptions()
{
	VertexPaintRecipeOptions.Empty();
	const TArray<FMIForgeVertexPaintRecipe>& SavedRecipes = VertexPaintRecipeManager.GetRecipes();
	for (const FMIForgeVertexPaintRecipe& Recipe : SavedRecipes)
	{
		VertexPaintRecipeOptions.Add(MakeShared<FString>(Recipe.RecipeName));
	}

	bool bCurrentRecipeStillExists = false;

	if (CurrentVertexPaintRecipeOption.IsValid())
	{
		for (const TSharedPtr<FString>& RecipeOption : VertexPaintRecipeOptions)
		{
			if (RecipeOption.IsValid() &&
				*RecipeOption == *CurrentVertexPaintRecipeOption)
			{
				CurrentVertexPaintRecipeOption = RecipeOption;
				bCurrentRecipeStillExists = true;
				break;
			}
		}
	}

	if (!bCurrentRecipeStillExists)
	{
		CurrentVertexPaintRecipeOption.Reset();
	}

	if (CurrentVertexPaintRecipeOptionText.IsValid())
	{
		CurrentVertexPaintRecipeOptionText->SetText(
			CurrentVertexPaintRecipeOption.IsValid()
			? FText::FromString(*CurrentVertexPaintRecipeOption)
			: FText::FromString(TEXT("Select Recipe"))
		);
	}
	if (VertexPaintRecipeComboBox.IsValid())
	{
		VertexPaintRecipeComboBox->RefreshOptions();
	}
}

void SMainTabWidget::VertexPaintGenerateNameTextBox(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Material Instance Name:")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		];
	Container->AddSlot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(SEditableTextBox)
				.HintText(FText::FromString(TEXT("Leave blank to use default name.")))
				.Text_Lambda([this]()
					{
						return FText::FromString(CurrentVertexPaintMIName);
					})
				.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
					{
						CurrentVertexPaintMIName = NewText.ToString().TrimStartAndEnd();
					})
				
				
		];
}

void SMainTabWidget::VertexPaintingMIOptionSection(TSharedRef<SVerticalBox> Container)
{
	Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[	SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.f)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Layer Stack")))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
						]

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNullWidget::NullWidget
						]
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.f)
						.HAlign(HAlign_Right)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Assign Selected")))
								.OnClicked_Lambda([this]() {

								if (SelectedTextureSetItems.Num() == 0)
								{
									MIForgeUtilities::PrintNotification(
										TEXT("Please select at least one texture set to assign.")
									);
									return FReply::Unhandled();
								}

								if (SelectedTextureSetItems.Num() > 4)
								{
									MIForgeUtilities::PrintNotification(
										TEXT("Please select no more than 4 texture sets to assign.")
									);
									return FReply::Unhandled();
								}

								VertexPaintLayerStack.BaseLayer.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerR.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerG.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerB.AssignedTextureSet.Reset();

								VertexPaintLayerStack.BaseLayer.AssignedTextureSet = SelectedTextureSetItems[0];

								if (SelectedTextureSetItems.Num() >= 2)
								{
									VertexPaintLayerStack.LayerR.AssignedTextureSet = SelectedTextureSetItems[1];
								}

								if (SelectedTextureSetItems.Num() >= 3)
								{
									VertexPaintLayerStack.LayerG.AssignedTextureSet = SelectedTextureSetItems[2];
								}

								if (SelectedTextureSetItems.Num() >= 4)
								{
									VertexPaintLayerStack.LayerB.AssignedTextureSet = SelectedTextureSetItems[3];
								}

								RefreshValidationSummary();
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.BaseLayer.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerR.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerG.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerB.Layer);

								return FReply::Handled();
									})
						]
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.f)
						.HAlign(HAlign_Right)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Clear All")))
								.OnClicked_Lambda([this]() {

								VertexPaintLayerStack.BaseLayer.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerR.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerG.AssignedTextureSet.Reset();
								VertexPaintLayerStack.LayerB.AssignedTextureSet.Reset();

								RefreshValidationSummary();
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.BaseLayer.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerR.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerG.Layer);
								RefreshVertexPaintLayerThumbnail(VertexPaintLayerStack.LayerB.Layer);

								return FReply::Handled();
									})
						]
						
				]
			
		];
	/*Container->AddSlot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Max Layers (RGB): ")))
						
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(new TArray<TSharedPtr<FString>>{ MakeShared<FString>(TEXT("2 (R)")), MakeShared<FString>(TEXT("3 (R, G)")), MakeShared<FString>(TEXT("4 (R, G, B)")) })
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) {
						return SNew(STextBlock).Text(FText::FromString(*InItem));
							})
						.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type SelectInfo) {

						if (Option.IsValid())
						{
							CurrentMaxLayersOption = Option;
							CurrentMaxLayersComboBoxSelectedOptionText->SetText(FText::FromString(*Option));
						}
							})
						.Content()
						[
							SAssignNew(CurrentMaxLayersComboBoxSelectedOptionText, STextBlock)
								.Text(FText::FromString(TEXT("2 (R)")))
						]
				]
						
		];*/

	Container->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer::Base)
		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer::LayerR)
		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer::LayerG)
		];

	Container->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			VertexPaintLayerSlotWidget(EMIForgeVertexPaintLayer::LayerB)
		];
}



TSharedRef<SWidget> SMainTabWidget::IfMIExistsOptionBlock()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						if (NewState == ECheckBoxState::Checked)
						{
							this->CurrentIfMIExistsOption = EIfMIExistsOption::Skip;
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (this->CurrentIfMIExistsOption == EIfMIExistsOption::Skip) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Skip")))
				]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						if (NewState == ECheckBoxState::Checked)
						{
							this->CurrentIfMIExistsOption = EIfMIExistsOption::Overwrite;
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (this->CurrentIfMIExistsOption == EIfMIExistsOption::Overwrite) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Overwrite")))
				]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			SNew(SCheckBox)
				.Style(FAppStyle::Get(), "RadioButton")
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
					{
						if (NewState == ECheckBoxState::Checked)
						{
							this->CurrentIfMIExistsOption = EIfMIExistsOption::CreateUnique;
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (this->CurrentIfMIExistsOption == EIfMIExistsOption::CreateUnique) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Create Unique Name")))
				]
		];
}

TArray<TSharedPtr<FMIForgeTextureInfo>> SMainTabWidget::ResolveTexturesForSet(const FMIForgeTextureSet& TextureSet) const
{
	TArray<TSharedPtr<FMIForgeTextureInfo>> Result;

	auto AddTextureIfPresent = [&Result, &TextureSet](EMIForgeTextureType Type)
		{
			if (const FMIForgeTextureInfo* Texture = TextureSet.Textures.Find(Type))
			{
				Result.Add(MakeShared<FMIForgeTextureInfo>(*Texture));
			}
		};

	AddTextureIfPresent(EMIForgeTextureType::Albedo);
	AddTextureIfPresent(EMIForgeTextureType::Normal);
	AddTextureIfPresent(EMIForgeTextureType::ORM);

	if (bUseEmissiveTextures)
	{
		AddTextureIfPresent(EMIForgeTextureType::Emissive);
	}

	if (bUseDetailNormalTextures)
	{
		AddTextureIfPresent(EMIForgeTextureType::DetailNormal);
	}

	return Result;
}

FText SMainTabWidget::GetValidationSummaryText() const
{
	return FText::Format(
		FText::FromString(TEXT("Validation Summary: \n\nReady to create: {0}/{1} MI(s)\nMissing required textures: {2}\nMissing optional textures: {3}\nUnrecognized textures: {4}")),
		CurrentValidationSummary.ReadyToCreateCount,
		CurrentValidationSummary.TotalSets,
		CurrentValidationSummary.MissingRequiredTextureCount,
		CurrentValidationSummary.MissingOptionalTextureCount,
		CurrentValidationSummary.UnrecognizedTextureCount
	);
}

FText SMainTabWidget::GetMIReadyCount() const
{
	return FText::Format(
		FText::FromString(TEXT("Generate Instances ({0})")),
		CurrentValidationSummary.ReadyToCreateCount
	);
}



void SMainTabWidget::RefreshValidationSummary()
{
	FMIForgeValidator Validator;

	if (!CurrentPresetOption.IsValid())
	{
		CurrentValidationSummary = FMIForgeValidationSummary();
		return;
	}

	if (*CurrentPresetOption == TEXT("Standard"))
	{
		if (CurrentInputMode == EMIForgeInputMode::TextureSets)
		{
			CurrentValidationSummary = Validator.BuildStandardSummaryFromTextureSets(
				BuildGenerationTextureSets(),
				bUseEmissiveTextures,
				bUseDetailNormalTextures,
				bIgnoreUnrecognizedTextures
			);
		}
		else
		{
			CurrentValidationSummary = Validator.BuildStandardSummaryFromTextures(
				SelectedTextureItems,
				bUseEmissiveTextures,
				bUseDetailNormalTextures,
				bIgnoreUnrecognizedTextures
			);
		}
	}
	else if (*CurrentPresetOption == TEXT("RGB Masking"))
	{
		if (CurrentInputMode == EMIForgeInputMode::TextureSets)
		{
			CurrentValidationSummary = Validator.BuildRGBSummaryFromTextureSets(
				BuildGenerationTextureSets(),
				bUseBaseORMTexture,
				bEnableEmissiveChannel,
				bUseDetailNormalTextureRGB,
				bIgnoreUnrecognizedTextures
			);
		}
		else
		{
			CurrentValidationSummary = Validator.BuildRGBSummaryFromTextures(
				SelectedTextureItems,
				bUseBaseORMTexture,
				bEnableEmissiveChannel,
				bUseDetailNormalTextureRGB,
				bIgnoreUnrecognizedTextures
			);
		}

	}
	else if (*CurrentPresetOption == TEXT("Vertex Painting"))
	{
		
		CurrentVertexPaintValidationResult = Validator.ValidateVertexPaintLayerStack(VertexPaintLayerStack, bIgnoreUnrecognizedTextures);

		CurrentVertexPaintValidationSummary = Validator.BuildVertexPaintLayerStackSummary(CurrentVertexPaintValidationResult);
	}
}

TSharedRef<SWidget> SMainTabWidget::CreateStandardValidationDetailsWidget()
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	 if(CurrentValidationSummary.SetResults.Num() == 0)
	{
		ScrollBox->AddSlot()
			.Padding(4.f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("No validation results available.")))
		];

		return ScrollBox;
	}

	 auto TextureType2Text = [](EMIForgeTextureType Type) -> FText {
		 switch (Type)
		 {
		 case EMIForgeTextureType::Albedo:
			 return FText::FromString(TEXT("Albedo"));
		 case EMIForgeTextureType::Normal:
			 return FText::FromString(TEXT("Normal"));
		 case EMIForgeTextureType::ORM:
			 return FText::FromString(TEXT("ORM"));
		 case EMIForgeTextureType::Emissive:
			 return FText::FromString(TEXT("Emissive"));
		 case EMIForgeTextureType::DetailNormal:
			 return FText::FromString(TEXT("Detail Normal"));
		 default:
			 return FText::FromString(TEXT("Unknown"));
		 }
		 };

	 auto AddTableRow = [TextureType2Text](
		 TSharedRef<SGridPanel> Grid,
		 int32& RowIndex,
		 const FText& TypeText,
		 const FText& DetailText,
		 const FSlateBrush* Icon
		 )
		 {
			 Grid->AddSlot(0, RowIndex)
				 .Padding(4.f, 3.f)
				 [
					 SNew(STextBlock)
						 .Text(TypeText)
				 ];

			 Grid->AddSlot(1, RowIndex)
				 .Padding(4.f, 3.f)
				 [
					 SNew(STextBlock)
						 .Text(DetailText)
				 ];

			 Grid->AddSlot(2, RowIndex)
				 .Padding(4.f, 3.f)
				 .HAlign(HAlign_Center)
				 .VAlign(VAlign_Center)
				 [
					 SNew(SImage)
						 .Image(Icon)
				 ];

			 RowIndex++;

			 Grid->AddSlot(0, RowIndex)
				 .ColumnSpan(3)
				 .Padding(0.f, 1.f)
				 [
					 SNew(SSeparator)
				 ];

			 RowIndex++;
				 

		 };

	 for (const FMIForgeTextureSetValidationResult& SetResult :
		 CurrentValidationSummary.SetResults)
	 {
		 TSharedRef<SGridPanel> Grid = SNew(SGridPanel);

		 int32 RowIndex = 0;

		 Grid->AddSlot(0, RowIndex)
			 .Padding(4.f)
			 [
				 SNew(STextBlock)
					 .Text(FText::FromString(TEXT("Type")))
					 .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			 ];

		 Grid->AddSlot(1, RowIndex)
			 .Padding(4.f)
			 [
				 SNew(STextBlock)
					 .Text(FText::FromString(TEXT("Texture / Issue")))
					 .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			 ];

		 Grid->AddSlot(2, RowIndex)
			 .Padding(4.f)
			 .HAlign(HAlign_Center)
			 [
					 SNew(STextBlock)
					 .Text(FText::FromString(TEXT("Status")))
					 .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			 ];

		 RowIndex++;

		 Grid->AddSlot(0, RowIndex)
			 .ColumnSpan(3)
			 [
				 SNew(SSeparator)
			 ];

		 RowIndex++;

		 for (const FMIForgeTextureRequirement& Requirement :
			 SetResult.SuccessfulAppliedTextures)
		 {
			 AddTableRow(
				 Grid,
				 RowIndex,
				 TextureType2Text(Requirement.TextureType),
				 FText::FromString(Requirement.TextureName),
				 FMIForgeStyle::Get().GetBrush("ListView.Row.Accept")
			 );
		 }

		 for (const FMIForgeTextureRequirement& Requirement :
			 SetResult.MissingRequiredTextures)
		 {
			 AddTableRow(
				 Grid,
				 RowIndex,
				 TextureType2Text(Requirement.TextureType),
				 FText::FromString(TEXT("Missing required texture")),
				 FMIForgeStyle::Get().GetBrush("ListView.Row.Reject")
			 );
		 }

		 for (const FMIForgeTextureRequirement& Requirement :
			 SetResult.MissingOptionalTextures)
		 {
			 AddTableRow(
				 Grid,
				 RowIndex,
				 TextureType2Text(Requirement.TextureType),
				 FText::FromString(TEXT("Missing optional texture")),
				 FMIForgeStyle::Get().GetBrush("ListView.Row.Warning")
			 );
		 }

		 for (const FMIForgeTextureRequirement& Requirement :
			 SetResult.UnrecognizedTextures)
		 {
			 AddTableRow(
				 Grid,
				 RowIndex,
				 TextureType2Text(Requirement.TextureType),
				 FText::FromString(Requirement.TextureName),
				 FMIForgeStyle::Get().GetBrush("ListView.Row.Warning")
			 );
		 }

		 ScrollBox->AddSlot()
			 .Padding(4.f)
			 [
				 SNew(SBorder)
					 .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					 .Padding(6.f)
					 [
						 SNew(SVerticalBox)

							 + SVerticalBox::Slot()
							 .AutoHeight()
							 .Padding(0.f, 0.f, 0.f, 6.f)
							 [
								 SNew(STextBlock)
									 .Text(FText::FromString(SetResult.SetName))
									 .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
							 ]

							 + SVerticalBox::Slot()
							 .AutoHeight()
							 [
								 Grid
							 ]
					 ]
			 ];
	 }
	 

	return ScrollBox;
}

TSharedRef<SWidget> SMainTabWidget::CreateRGBmaskingValidationDetailsWidget()
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	if (CurrentValidationSummary.SetResults.Num() == 0)
	{
		ScrollBox->AddSlot()
			.Padding(4.f)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("No validation results available.")))
			];

		return ScrollBox;
	}

	auto TextureType2Text = [](EMIForgeTextureType Type) -> FText {
		switch (Type)
		{
		case EMIForgeTextureType::Albedo:
			return FText::FromString(TEXT("Albedo"));
		case EMIForgeTextureType::Normal:
			return FText::FromString(TEXT("Normal"));
		case EMIForgeTextureType::RGB:
			return FText::FromString(TEXT("RGB Mask"));
		case EMIForgeTextureType::ORM:
			return FText::FromString(TEXT("ORM"));
		case EMIForgeTextureType::Emissive:
			return FText::FromString(TEXT("Emissive"));
		case EMIForgeTextureType::DetailNormal:
			return FText::FromString(TEXT("Detail Normal"));
		default:
			return FText::FromString(TEXT("Unknown"));
		}
		};

	auto AddTableRow = [TextureType2Text](
		TSharedRef<SGridPanel> Grid,
		int32& RowIndex,
		const FText& TypeText,
		const FText& DetailText,
		const FSlateBrush* Icon
		)
		{
			Grid->AddSlot(0, RowIndex)
				.Padding(4.f, 3.f)
				[
					SNew(STextBlock)
						.Text(TypeText)
				];

			Grid->AddSlot(1, RowIndex)
				.Padding(4.f, 3.f)
				[
					SNew(STextBlock)
						.Text(DetailText)
				];

			Grid->AddSlot(2, RowIndex)
				.Padding(4.f, 3.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
						.Image(Icon)
				];

			RowIndex++;

			Grid->AddSlot(0, RowIndex)
				.ColumnSpan(3)
				.Padding(0.f, 1.f)
				[
					SNew(SSeparator)
				];

			RowIndex++;


		};

	for (const FMIForgeTextureSetValidationResult& SetResult :
		CurrentValidationSummary.SetResults)
	{
		TSharedRef<SGridPanel> Grid = SNew(SGridPanel);

		int32 RowIndex = 0;

		Grid->AddSlot(0, RowIndex)
			.Padding(4.f)
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Type")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			];

		Grid->AddSlot(1, RowIndex)
			.Padding(4.f)
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Texture / Issue")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			];

		Grid->AddSlot(2, RowIndex)
			.Padding(4.f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Status")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
			];

		RowIndex++;

		Grid->AddSlot(0, RowIndex)
			.ColumnSpan(3)
			[
				SNew(SSeparator)
			];

		RowIndex++;

		for (const FMIForgeTextureRequirement& Requirement :
			SetResult.SuccessfulAppliedTextures)
		{
			AddTableRow(
				Grid,
				RowIndex,
				TextureType2Text(Requirement.TextureType),
				FText::FromString(Requirement.TextureName),
				FMIForgeStyle::Get().GetBrush("ListView.Row.Accept")
			);
		}

		for (const FMIForgeTextureRequirement& Requirement :
			SetResult.MissingRequiredTextures)
		{
			AddTableRow(
				Grid,
				RowIndex,
				TextureType2Text(Requirement.TextureType),
				FText::FromString(TEXT("Missing required texture")),
				FMIForgeStyle::Get().GetBrush("ListView.Row.Reject")
			);
		}

		for (const FMIForgeTextureRequirement& Requirement :
			SetResult.MissingOptionalTextures)
		{
			AddTableRow(
				Grid,
				RowIndex,
				TextureType2Text(Requirement.TextureType),
				FText::FromString(TEXT("Missing optional texture")),
				FMIForgeStyle::Get().GetBrush("ListView.Row.Warning")
			);
		}

		for (const FMIForgeTextureRequirement& Requirement :
			SetResult.UnrecognizedTextures)
		{
			AddTableRow(
				Grid,
				RowIndex,
				TextureType2Text(Requirement.TextureType),
				FText::FromString(Requirement.TextureName),
				FMIForgeStyle::Get().GetBrush("ListView.Row.Warning")
			);
		}

		ScrollBox->AddSlot()
			.Padding(4.f)
			[
				SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(6.f)
					[
						SNew(SVerticalBox)

							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 0.f, 0.f, 6.f)
							[
								SNew(STextBlock)
									.Text(FText::FromString(SetResult.SetName))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
							]

							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								Grid
							]
					]
			];
	}


	return ScrollBox;
}

TSharedRef<SWidget> SMainTabWidget::CreateVertexPaintLayerThumbnailWidget(EMIForgeVertexPaintLayer Layer)
{
	const FAssetData* ThumbnailAsset = GetVertexPaintLayerThumbnailAsset(Layer);

	if (!ThumbnailAsset || !AssetThumbnailPool.IsValid())
	{
		return SNew(SBox)
			.WidthOverride(64.0f)
			.HeightOverride(64.0f)
			[
				SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString(TEXT("[ preview ]")))
					]
			];
	}

	TSharedPtr<FAssetThumbnail> Thumbnail =
		MakeShared<FAssetThumbnail>(*ThumbnailAsset, 64, 64, AssetThumbnailPool);

	return SNew(SBox)
		.WidthOverride(64.0f)
		.HeightOverride(64.0f)
		[
			Thumbnail->MakeThumbnailWidget()
		];
}

void SMainTabWidget::AssignSelectedTextureSetToVertexLayer(EMIForgeVertexPaintLayer Layer)
{	
	if (SelectedTextureSetItems.Num() == 0)
	{
		MIForgeUtilities::PrintNotification(
			TEXT("Please select any texture set to assign.")
		);
		return;
	}

	FMIForgeVertexPaintLayerSlot* Slot = GetVertexPaintLayerSlot(Layer);
	
	if (!Slot)
	{
		return;
	}

	Slot->AssignedTextureSet = SelectedTextureSetItems.Last();

	RefreshValidationSummary();
	RefreshVertexPaintLayerThumbnail(Layer);
}

void SMainTabWidget::ClearVertexLayerAssignment(EMIForgeVertexPaintLayer Layer)
{
	FMIForgeVertexPaintLayerSlot* Slot = GetVertexPaintLayerSlot(Layer);

	if (!Slot)
	{
		return;
	}

	Slot->AssignedTextureSet.Reset();

	RefreshValidationSummary();
	RefreshVertexPaintLayerThumbnail(Layer);
}

FMIForgeVertexPaintLayerSlot* SMainTabWidget::GetVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer)
{	
	switch (Layer)
	{
		case EMIForgeVertexPaintLayer::Base:
			return &VertexPaintLayerStack.BaseLayer;
		case EMIForgeVertexPaintLayer::LayerR:
			return &VertexPaintLayerStack.LayerR;
		case EMIForgeVertexPaintLayer::LayerG:
			return &VertexPaintLayerStack.LayerG;
		case EMIForgeVertexPaintLayer::LayerB:
			return &VertexPaintLayerStack.LayerB;
	}
	return nullptr;
}

FText SMainTabWidget::GetVertexPaintLayerStatusText(EMIForgeVertexPaintLayer Layer) const
{
	for (const FMIForgeVertexPaintLayerValidationResult& Result :
		CurrentVertexPaintValidationResult.LayerResults)
	{
		if (Result.Layer == Layer)
		{
			switch (Result.Status)
			{
			case EMIForgeVertexPaintLayerStatus::Empty:
				return Result.bRequired
					? FText::FromString(TEXT("Required"))
					: FText::FromString(TEXT("Optional"));

			case EMIForgeVertexPaintLayerStatus::Valid:
				return FText::FromString(TEXT("Valid"));

			case EMIForgeVertexPaintLayerStatus::Warning:
				return FText::FromString(TEXT("Warning"));

			case EMIForgeVertexPaintLayerStatus::Error:
				return FText::FromString(TEXT("Missing"));
			}
		}
	}

	return FText::FromString(TEXT("Unknown"));
}

FText SMainTabWidget::GetVertexPaintValidationSummaryText() const
{
	FString SummaryText = TEXT("Vertex Paint Validation:\n\n");

	SummaryText += FString::Printf(
		TEXT("Ready to create: %s\n\n"),
		CurrentVertexPaintValidationSummary.bCanGenerate ? TEXT("Yes") : TEXT("No")
	);

	auto StatusToString = [](EMIForgeVertexPaintLayerStatus Status) -> FString
		{
			switch (Status)
			{
			case EMIForgeVertexPaintLayerStatus::Empty:
				return TEXT("Empty");

			case EMIForgeVertexPaintLayerStatus::Valid:
				return TEXT("Valid");

			case EMIForgeVertexPaintLayerStatus::Warning:
				return TEXT("Warning");

			case EMIForgeVertexPaintLayerStatus::Error:
				return TEXT("Error");
			}

			return TEXT("Unknown");
		};

	for (const FMIForgeVertexPaintLayerValidationResult& LayerResult :
		CurrentVertexPaintValidationSummary.LayerResults)
	{
		SummaryText += FString::Printf(
			TEXT("%s: %s"),
			*LayerResult.DisplayName,
			*StatusToString(LayerResult.Status)
		);

		if (!LayerResult.AssignedSetName.IsEmpty())
		{
			SummaryText += FString::Printf(
				TEXT(" - %s"),
				*LayerResult.AssignedSetName
			);
		}

		if (LayerResult.Status == EMIForgeVertexPaintLayerStatus::Error)
		{
			SummaryText += FString::Printf(
				TEXT(" | Missing required: %d | Missing optional: %d"),
				LayerResult.MissingRequiredTextures.Num(),
				LayerResult.MissingOptionalTextures.Num()
			);
		}
		else if (LayerResult.Status == EMIForgeVertexPaintLayerStatus::Warning)
		{
			SummaryText += FString::Printf(
				TEXT(" | Missing optional: %d"),
				LayerResult.MissingOptionalTextures.Num()
			);
		}

		SummaryText += TEXT("\n");
	}

	return FText::FromString(SummaryText);
}

FSlateColor SMainTabWidget::GetVertexPaintLayerStatusColor(EMIForgeVertexPaintLayer Layer) const
{
	for (const FMIForgeVertexPaintLayerValidationResult& Result :
		CurrentVertexPaintValidationResult.LayerResults)
	{
		if (Result.Layer == Layer)
		{
			switch (Result.Status)
			{
			case EMIForgeVertexPaintLayerStatus::Valid:
				return FSlateColor(FLinearColor(0.2f, 0.8f, 0.35f, 1.0f));

			case EMIForgeVertexPaintLayerStatus::Warning:
				return FSlateColor(FLinearColor(1.0f, 0.65f, 0.1f, 1.0f));

			case EMIForgeVertexPaintLayerStatus::Error:
				return FSlateColor(FLinearColor(0.9f, 0.15f, 0.12f, 1.0f));

			case EMIForgeVertexPaintLayerStatus::Empty:
			default:
				return FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.0f));
			}
		}
	}

	return FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.0f));
}

TSharedPtr<SBox> SMainTabWidget::GetVertexPaintLayerThumbnailBox(EMIForgeVertexPaintLayer Layer) const
{
	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base:
		return BaseLayerThumbnailBox;

	case EMIForgeVertexPaintLayer::LayerR:
		return LayerRThumbnailBox;

	case EMIForgeVertexPaintLayer::LayerG:
		return LayerGThumbnailBox;

	case EMIForgeVertexPaintLayer::LayerB:
		return LayerBThumbnailBox;
	}

	return nullptr;
}

void SMainTabWidget::RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer Layer)
{
	TSharedPtr<SBox> ThumbnailBox = GetVertexPaintLayerThumbnailBox(Layer);

	if (!ThumbnailBox.IsValid())
	{
		return;
	}

	ThumbnailBox->SetContent(
		CreateVertexPaintLayerThumbnailWidget(Layer)
	);
}

const FAssetData* SMainTabWidget::GetVertexPaintLayerThumbnailAsset(EMIForgeVertexPaintLayer Layer) const
{
	const FMIForgeVertexPaintLayerSlot* Slot = nullptr;

	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base:
		Slot = &VertexPaintLayerStack.BaseLayer;
		break;

	case EMIForgeVertexPaintLayer::LayerR:
		Slot = &VertexPaintLayerStack.LayerR;
		break;

	case EMIForgeVertexPaintLayer::LayerG:
		Slot = &VertexPaintLayerStack.LayerG;
		break;

	case EMIForgeVertexPaintLayer::LayerB:
		Slot = &VertexPaintLayerStack.LayerB;
		break;
	}

	if (!Slot || !Slot->AssignedTextureSet.IsValid())
	{
		return nullptr;
	}

	const FMIForgeTextureInfo* Albedo =
		Slot->AssignedTextureSet->Textures.Find(EMIForgeTextureType::Albedo);

	if (!Albedo || !Albedo->AssetData.IsValid())
	{
		return nullptr;
	}

	return &Albedo->AssetData;
}

TArray<TSharedPtr<FMIForgeTextureSet>> SMainTabWidget::BuildGenerationTextureSets() const
{
	if (CurrentInputMode == EMIForgeInputMode::TextureSets)
	{
		return SelectedTextureSetItems;
	}

	TArray<FMIForgeTextureInfo> RawSelectedTextures;
	RawSelectedTextures.Reserve(SelectedTextureItems.Num());

	for (const TSharedPtr<FMIForgeTextureInfo>& Texture :
		SelectedTextureItems)
	{
		if (Texture.IsValid())
		{
			RawSelectedTextures.Add(*Texture);
		}
	}

	FMIForgeTextureSetBuilder SetBuilder;
	TArray<FMIForgeTextureSet> BuiltSets =
		SetBuilder.BuildTextureSets(RawSelectedTextures);

	TArray<TSharedPtr<FMIForgeTextureSet>> Result;
	Result.Reserve(BuiltSets.Num());

	for (FMIForgeTextureSet& Set : BuiltSets)
	{
		Result.Add(MakeShared<FMIForgeTextureSet>(MoveTemp(Set)));
	}

	return Result;
}


#undef LOCTEXT_NAMESPACE
