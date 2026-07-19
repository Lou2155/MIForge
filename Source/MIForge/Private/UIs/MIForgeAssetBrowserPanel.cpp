// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeAssetBrowserPanel.h"

#include "Catalog/MIForgeTextureCatalog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "MIForgeStyle.h"
#include "MIForgeTextureClassifier.h"
#include "MIForgeUtilities.h"
#include "MIForgeValidator.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "TextureSetTableRow.h"
#include "TextureTableRow.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MIForgeAssetBrowserPanel"

namespace
{
	static const FSlateRoundedBoxBrush ListViewBorderBrush(
		FLinearColor(0.f, 0.f, 0.f, 0.f),
		5.0f,
		FLinearColor(0.10f, 0.10f, 0.10f, 1.f),
		0.5f);
}

void SMIForgeAssetBrowserPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;
	TextureCatalog = InArgs._TextureCatalog;

	TextureFilterOptions = MIForgeUtilities::GetFilterOptions();

	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Ready")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Warning")));
	TextureSetFilterOptions.Add(MakeShared<FString>(TEXT("Error")));

	ActiveFilterOptions = TextureFilterOptions;
	CurrentTextureFilterOption = TextureFilterOptions[0];
	CurrentTextureSetFilterOption = TextureSetFilterOptions[0];
	CurrentFilterOption = CurrentTextureFilterOption;

	ViewModel->OnPresetChanged.AddSP(
		SharedThis(this),
		&SMIForgeAssetBrowserPanel::HandlePresetChanged);
	ViewModel->OnGenerationOptionsChanged.AddSP(
		SharedThis(this),
		&SMIForgeAssetBrowserPanel::HandleGenerationOptionsChanged);
	ViewModel->OnSelectionChanged.AddSP(
		SharedThis(this),
		&SMIForgeAssetBrowserPanel::HandleSelectionChanged);

	TextureCatalog->OnRefreshStarted.AddSP(
		SharedThis(this),
		&SMIForgeAssetBrowserPanel::HandleTextureCatalogRefreshStarted);
	TextureCatalog->OnCatalogChanged.AddSP(
		SharedThis(this),
		&SMIForgeAssetBrowserPanel::HandleTextureCatalogChanged);

	RefreshFilteredTextures();
	RefreshFilteredTextureSets();

	ChildSlot
	[
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
				.Text(FText::FromString(
					TEXT("Please specify a path that includes your textures")))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(
					&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(
						"SimpleButton"))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked_Lambda([this]()
					{
						MIForgeUtilities::CreatePathSelector(
							SharedThis(this),
							FOnPathSelected::CreateLambda(
								[this](const FString& SelectedPath)
								{
									ViewModel->ClearTextureSetSelection();
									TextureCatalog->SetFolderPaths({ SelectedPath });
									TexturePathTextBlock->SetText(
										FText::FromString(SelectedPath));
									TextureCatalog->QueueRefresh();
								}));
						return FReply::Handled();
					})
				[
					SNew(SImage)
					.Image(FMIForgeStyle::Get().GetBrush(
						"Panel.FolderSelection"))
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
						return !ViewModel.IsValid() ||
							ViewModel->GetPreset() !=
							EMIForgeGenerationPreset::VertexPainting;
					})
				.Value_Lambda([this]()
					{
						return ViewModel->GetInputMode() ==
							EMIForgeInputMode::TextureSets ? 1 : 0;
					})
				.OnValueChanged_Lambda([this](int32 Value)
					{
						ViewModel->SetInputMode(
							Value == 1
								? EMIForgeInputMode::TextureSets
								: EMIForgeInputMode::IndividualTextures);
						ViewModeSwitcher->SetActiveWidgetIndex(Value);

						if (ViewModel->GetInputMode() ==
							EMIForgeInputMode::IndividualTextures)
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
								FText::FromString(*CurrentFilterOption));
						}
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
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock)
							.Text(FText::FromString(
								Item.IsValid() ? *Item : TEXT("")));
					})
				.OnSelectionChanged_Lambda(
					[this](TSharedPtr<FString> Option, ESelectInfo::Type)
					{
						if (!Option.IsValid())
						{
							return;
						}

						CurrentFilterOption = Option;

						if (ViewModel->GetInputMode() ==
							EMIForgeInputMode::IndividualTextures)
						{
							CurrentTextureFilterOption = Option;
							RefreshFilteredTextures();
						}
						else
						{
							CurrentTextureSetFilterOption = Option;
							RefreshFilteredTextureSets();
						}

						CurrentFilterComboBoxSelectedOptionText->SetText(
							FText::FromString(*Option));
					})
				.Content()
				[
					SAssignNew(
						CurrentFilterComboBoxSelectedOptionText,
						STextBlock)
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
				.OnTextChanged_Lambda([this](const FText& Text)
					{
						CurrentSearchText = Text;
						if (ViewModel->GetInputMode() ==
							EMIForgeInputMode::IndividualTextures)
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

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(6.0f)
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
						.Text_Lambda([this]()
							{
								if (ViewModel->GetInputMode() ==
									EMIForgeInputMode::IndividualTextures)
								{
									return FText::FromString(FString::Printf(
										TEXT("Detected Textures: %d/%d"),
										FilteredTextureListItems.Num(),
										TextureCatalog->GetTextures().Num()));
								}

								return FText::FromString(FString::Printf(
									TEXT("Detected Texture Sets: %d/%d"),
									FilteredTextureSetListItems.Num(),
									TextureCatalog->GetTextureSets().Num()));
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
						.ButtonStyle(
							&FAppStyle::Get().GetWidgetStyle<FButtonStyle>(
								"SimpleButton"))
						.OnClicked_Lambda([this]()
							{
								RefreshListViews();
								return FReply::Handled();
							})
						[
							SNew(SImage)
							.Image(FMIForgeStyle::Get().GetBrush(
								"Panel.RefreshButton"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(TAttribute<float>::CreateLambda([this]()
						{
							if (ViewModel.IsValid() &&
								ViewModel->GetPreset() ==
								EMIForgeGenerationPreset::VertexPainting)
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
								return ViewModel.IsValid() &&
									ViewModel->GetPreset() ==
									EMIForgeGenerationPreset::VertexPainting
									? EVisibility::Visible
									: EVisibility::Collapsed;
							})
						.BorderImage(FAppStyle::GetBrush(
							"ToolPanel.GroupBorder"))
						.Padding(6.0f)
						[
							InArgs._Content.Widget
						]
					]
				]
			]
		]
	];

	if (!TextureCatalog->GetFolderPaths().IsEmpty())
	{
		TexturePathTextBlock->SetText(
			FText::FromString(TextureCatalog->GetFolderPaths()[0]));
	}

	if (CurrentTextureFilterOption.IsValid() &&
		CurrentFilterComboBoxSelectedOptionText.IsValid())
	{
		CurrentFilterComboBoxSelectedOptionText->SetText(
			FText::FromString(*CurrentTextureFilterOption));
	}

	if (TexListView.IsValid())
	{
		TexListView->RebuildList();
	}
}

SMIForgeAssetBrowserPanel::~SMIForgeAssetBrowserPanel()
{
	if (ViewModel.IsValid())
	{
		ViewModel->OnPresetChanged.RemoveAll(this);
		ViewModel->OnGenerationOptionsChanged.RemoveAll(this);
		ViewModel->OnSelectionChanged.RemoveAll(this);
	}

	if (TextureCatalog.IsValid())
	{
		TextureCatalog->OnRefreshStarted.RemoveAll(this);
		TextureCatalog->OnCatalogChanged.RemoveAll(this);
	}
}

void SMIForgeAssetBrowserPanel::SelectTexturesInSet(
	const FMIForgeTextureSet& TextureSet)
{
	TArray<TSharedPtr<FMIForgeTextureInfo>> NewSelection =
		ViewModel->GetSelectedTextures();

	auto SelectTextureType =
		[this, &TextureSet, &NewSelection](EMIForgeTextureType Type)
		{
			if (const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(Type))
			{
				TSharedPtr<FMIForgeTextureInfo> ExistingItem =
					TextureCatalog->FindTextureListItem(*TextureInfo);

				if (ExistingItem.IsValid())
				{
					NewSelection.AddUnique(ExistingItem);
				}
			}
		};

	if (ViewModel.IsValid() &&
		ViewModel->GetPreset() == EMIForgeGenerationPreset::Standard)
	{
		SelectTextureType(EMIForgeTextureType::Albedo);
		SelectTextureType(EMIForgeTextureType::Normal);
		SelectTextureType(EMIForgeTextureType::ORM);

		if (ViewModel->GetUseEmissiveTextures())
		{
			SelectTextureType(EMIForgeTextureType::Emissive);
		}

		if (ViewModel->GetUseDetailNormalTextures())
		{
			SelectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else if (ViewModel.IsValid() &&
		ViewModel->GetPreset() == EMIForgeGenerationPreset::RGBMask)
	{
		SelectTextureType(EMIForgeTextureType::Albedo);
		SelectTextureType(EMIForgeTextureType::Normal);
		SelectTextureType(EMIForgeTextureType::RGB);

		if (ViewModel->GetUseBaseORMTexture())
		{
			SelectTextureType(EMIForgeTextureType::ORM);
		}

		if (ViewModel->GetUseDetailNormalTextureRGB())
		{
			SelectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}

	ViewModel->SetSelectedTextures(NewSelection);
}

void SMIForgeAssetBrowserPanel::UnselectTexturesInSet(
	const FMIForgeTextureSet& TextureSet)
{
	TArray<TSharedPtr<FMIForgeTextureInfo>> NewSelection =
		ViewModel->GetSelectedTextures();

	auto UnselectTextureType =
		[this, &TextureSet, &NewSelection](EMIForgeTextureType Type)
		{
			if (const FMIForgeTextureInfo* TextureInfo =
				TextureSet.Textures.Find(Type))
			{
				TSharedPtr<FMIForgeTextureInfo> ExistingItem =
					TextureCatalog->FindTextureListItem(*TextureInfo);

				if (ExistingItem.IsValid())
				{
					NewSelection.Remove(ExistingItem);
				}
			}
		};

	if (ViewModel.IsValid() &&
		ViewModel->GetPreset() == EMIForgeGenerationPreset::Standard)
	{
		UnselectTextureType(EMIForgeTextureType::Albedo);
		UnselectTextureType(EMIForgeTextureType::Normal);
		UnselectTextureType(EMIForgeTextureType::ORM);

		if (ViewModel->GetUseEmissiveTextures())
		{
			UnselectTextureType(EMIForgeTextureType::Emissive);
		}

		if (ViewModel->GetUseDetailNormalTextures())
		{
			UnselectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}
	else if (ViewModel.IsValid() &&
		ViewModel->GetPreset() == EMIForgeGenerationPreset::RGBMask)
	{
		UnselectTextureType(EMIForgeTextureType::Albedo);
		UnselectTextureType(EMIForgeTextureType::Normal);
		UnselectTextureType(EMIForgeTextureType::RGB);

		if (ViewModel->GetUseBaseORMTexture())
		{
			UnselectTextureType(EMIForgeTextureType::ORM);
		}

		if (ViewModel->GetUseDetailNormalTextureRGB())
		{
			UnselectTextureType(EMIForgeTextureType::DetailNormal);
		}
	}

	ViewModel->SetSelectedTextures(NewSelection);
}

void SMIForgeAssetBrowserPanel::HandleTextureSetCheckStateChanged(
	const TSharedPtr<FMIForgeTextureSet>& TextureSet,
	ECheckBoxState CheckState)
{
	if (!TextureSet.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	switch (CheckState)
	{
	case ECheckBoxState::Checked:
		ViewModel->SelectTextureSet(TextureSet);
		SelectTexturesInSet(*TextureSet);
		break;

	case ECheckBoxState::Unchecked:
		ViewModel->UnselectTextureSet(TextureSet);
		UnselectTexturesInSet(*TextureSet);
		break;

	default:
		break;
	}
}

void SMIForgeAssetBrowserPanel::RefreshFilteredTextures()
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
		FilterType =
			Classifier.TextureTypeFromName(*CurrentTextureFilterOption);
	}

	for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo :
		TextureCatalog->GetTextures())
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
				TextureInfo->AssetName.Contains(
					SearchString,
					ESearchCase::IgnoreCase) ||
				TextureInfo->BaseName.Contains(
					SearchString,
					ESearchCase::IgnoreCase) ||
				TextureInfo->MatchedSuffix.Contains(
					SearchString,
					ESearchCase::IgnoreCase);

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

void SMIForgeAssetBrowserPanel::RefreshFilteredTextureSets()
{
	FilteredTextureSetListItems.Empty();

	FMIForgeValidator Validator;
	const FString SearchString =
		CurrentSearchText.ToString().TrimStartAndEnd();
	const bool bUseTypeFilter =
		CurrentTextureSetFilterOption.IsValid() &&
		*CurrentTextureSetFilterOption != TEXT("All");

	for (const TSharedPtr<FMIForgeTextureSet>& TextureSet :
		TextureCatalog->GetTextureSets())
	{
		if (!TextureSet.IsValid())
		{
			continue;
		}

		if (bUseTypeFilter)
		{
			FMIForgeValidator::EMIForgeTextureSetStatus Status;
			if (ViewModel.IsValid() &&
				ViewModel->GetPreset() == EMIForgeGenerationPreset::RGBMask)
			{
				Status = Validator.GetRGBSetStatus(
					*TextureSet,
					ViewModel->GetUseBaseORMTexture(),
					ViewModel->GetEnableEmissiveChannel(),
					ViewModel->GetUseDetailNormalTextureRGB(),
					ViewModel->GetIgnoreUnrecognizedTextures());
			}
			else
			{
				Status = Validator.GetStandardSetStatus(
					*TextureSet,
					ViewModel->GetUseEmissiveTextures(),
					ViewModel->GetUseDetailNormalTextures(),
					ViewModel->GetIgnoreUnrecognizedTextures());
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

		if (!SearchString.IsEmpty() &&
			!TextureSet->SetName.Contains(
				SearchString,
				ESearchCase::IgnoreCase))
		{
			continue;
		}

		FilteredTextureSetListItems.Add(TextureSet);
	}

	if (TexSetListView.IsValid())
	{
		TexSetListView->RequestListRefresh();
	}
}

void SMIForgeAssetBrowserPanel::RefreshListViews()
{
	if (TexListView.IsValid() &&
		TexSetListView.IsValid() &&
		TextureCatalog.IsValid())
	{
		TextureCatalog->Refresh();
	}
}

void SMIForgeAssetBrowserPanel::HandleTextureCatalogRefreshStarted()
{
	if (TexListView.IsValid() && TexSetListView.IsValid())
	{
		ViewModel->ClearAllSelections();
	}
}

void SMIForgeAssetBrowserPanel::HandleTextureCatalogChanged()
{
	if (!TexListView.IsValid() || !TexSetListView.IsValid())
	{
		return;
	}

	RefreshFilteredTextures();
	RefreshFilteredTextureSets();
	TexListView->RebuildList();
	TexSetListView->RebuildList();
}

void SMIForgeAssetBrowserPanel::HandlePresetChanged(
	EMIForgeGenerationPreset NewPreset)
{
	if (NewPreset == EMIForgeGenerationPreset::VertexPainting)
	{
		ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);

		if (ViewModeSwitcher.IsValid())
		{
			ViewModeSwitcher->SetActiveWidgetIndex(1);
		}

		ActiveFilterOptions = TextureSetFilterOptions;
		CurrentFilterOption = CurrentTextureSetFilterOption;

		if (FilterComboBox.IsValid())
		{
			FilterComboBox->RefreshOptions();
		}
	}

	RefreshFilteredTextureSets();
	if (TexSetListView.IsValid())
	{
		TexSetListView->RequestListRefresh();
	}
}

void SMIForgeAssetBrowserPanel::HandleGenerationOptionsChanged()
{
	RefreshFilteredTextureSets();
	if (TexSetListView.IsValid())
	{
		TexSetListView->RequestListRefresh();
	}
}

void SMIForgeAssetBrowserPanel::HandleSelectionChanged()
{
	if (TexListView.IsValid())
	{
		TexListView->RequestListRefresh();
	}

	if (TexSetListView.IsValid())
	{
		TexSetListView->RequestListRefresh();
	}
}

TSharedPtr<SWidget>
SMIForgeAssetBrowserPanel::GenerateRightClickMenuWidget()
{
	FMenuBuilder MenuBuilder(true, TSharedPtr<FUICommandList>());

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Browse to Assets")),
		FText::FromString(
			TEXT("Opens the content browser and selects the assets.")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
			{
				if (ViewModel->GetInputMode() ==
					EMIForgeInputMode::IndividualTextures)
				{
					TArray<FAssetData> AssetsToSelect;
					if (ViewModel->GetSelectedTextures().Num() > 0)
					{
						for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo :
							ViewModel->GetSelectedTextures())
						{
							if (TextureInfo.IsValid())
							{
								AssetsToSelect.Add(TextureInfo->AssetData);
							}
						}
					}
					else
					{
						for (const TSharedPtr<FMIForgeTextureInfo>& TextureInfo :
							TexListView->GetSelectedItems())
						{
							if (TextureInfo.IsValid())
							{
								AssetsToSelect.Add(TextureInfo->AssetData);
							}
						}
					}

					if (!AssetsToSelect.IsEmpty())
					{
						MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(
							AssetsToSelect);
					}
				}
				else
				{
					TArray<FAssetData> AssetsToSelect;
					const TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets =
						ViewModel->GetSelectedTextureSets().Num() > 0
							? ViewModel->GetSelectedTextureSets()
							: TexSetListView->GetSelectedItems();

					for (const TSharedPtr<FMIForgeTextureSet>& TextureSet :
						TextureSets)
					{
						if (!TextureSet.IsValid())
						{
							continue;
						}

						for (const auto& Pair : TextureSet->Textures)
						{
							AssetsToSelect.Add(Pair.Value.AssetData);
						}
					}

					if (!AssetsToSelect.IsEmpty())
					{
						MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(
							AssetsToSelect);
					}
				}
			}))); 

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
			})));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Clear Selection")),
		FText::FromString(TEXT("Clears the selection of assets.")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this]()
			{
				ViewModel->ClearAllSelections();
				TexListView->ClearSelection();
				TexSetListView->ClearSelection();
			})));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Ignore Unrecognized Textures")),
		FText::FromString(
			TEXT("Exclude unrecognized and needless textures in the current preset")),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
				{
					if (ViewModel->GetInputMode() ==
						EMIForgeInputMode::TextureSets)
					{
						ViewModel->SetIgnoreUnrecognizedTextures(
							!ViewModel->GetIgnoreUnrecognizedTextures());
					}
				}),
			FCanExecuteAction::CreateLambda([this]()
				{
					return ViewModel->GetInputMode() ==
						EMIForgeInputMode::TextureSets;
				}),
			FIsActionChecked::CreateLambda([this]()
				{
					return ViewModel->GetIgnoreUnrecognizedTextures();
				})),
		NAME_None,
		EUserInterfaceActionType::ToggleButton);

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SMIForgeAssetBrowserPanel::ListViewSwitcher()
{
	return SAssignNew(ViewModeSwitcher, SWidgetSwitcher)

		+ SWidgetSwitcher::Slot()
		[
			IndividualModeListView()
		]

		+ SWidgetSwitcher::Slot()
		[
			TextureSetModeListView()
		];
}

TSharedRef<SWidget> SMIForgeAssetBrowserPanel::IndividualModeListView()
{
	return SAssignNew(
		TexListView,
		SListView<TSharedPtr<FMIForgeTextureInfo>>)
		.ListItemsSource(&FilteredTextureListItems)
		.OnGenerateRow_Lambda(
			[this](
				TSharedPtr<FMIForgeTextureInfo> Item,
				const TSharedRef<STableViewBase>& OwnerTable)
			{
				return SNew(STextureTableRow, OwnerTable)
					.TextureListItems(Item)
					.ViewModel(ViewModel);
			})
		.HeaderRow(SetupHeaderRow())
		.OnContextMenuOpening(
			this,
			&SMIForgeAssetBrowserPanel::GenerateRightClickMenuWidget);
}

TSharedRef<SWidget> SMIForgeAssetBrowserPanel::TextureSetModeListView()
{
	return SAssignNew(
		TexSetListView,
		SListView<TSharedPtr<FMIForgeTextureSet>>)
		.ListItemsSource(&FilteredTextureSetListItems)
		.OnGenerateRow_Lambda(
			[this](
				TSharedPtr<FMIForgeTextureSet> Item,
				const TSharedRef<STableViewBase>& OwnerTable)
			{
				return SNew(STextureSetTableRow, OwnerTable)
					.TextureSets(Item)
					.ViewModel(ViewModel)
					.OnTextureSetCheckStateChanged(
						FMIForgeOnTextureSetCheckStateChanged::CreateSP(
							SharedThis(this),
							&SMIForgeAssetBrowserPanel::
								HandleTextureSetCheckStateChanged));
			})
		.HeaderRow(SetupTexSetHeaderRow())
		.OnContextMenuOpening(
			this,
			&SMIForgeAssetBrowserPanel::GenerateRightClickMenuWidget);
}

TSharedPtr<SHeaderRow> SMIForgeAssetBrowserPanel::SetupHeaderRow()
{
	return SNew(SHeaderRow)
		+ SHeaderRow::Column(FName("Select"))
		.FixedWidth(28.f)
		.HAlignHeader(HAlign_Center)
		.HAlignCell(HAlign_Center)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState)
				{
					switch (CheckState)
					{
					case ECheckBoxState::Checked:
						ViewModel->SetSelectedTextures(FilteredTextureListItems);
						break;
					case ECheckBoxState::Unchecked:
						ViewModel->ClearAllSelections();
						break;
					default:
						break;
					}
				})
			.IsChecked_Lambda([this]()
				{
					if (ViewModel->GetSelectedTextures().Num() == 0)
					{
						return ECheckBoxState::Unchecked;
					}
					if (ViewModel->GetSelectedTextures().Num() ==
						TextureCatalog->GetTextures().Num())
					{
						return ECheckBoxState::Checked;
					}
					return ECheckBoxState::Undetermined;
				})
		]

		+ SHeaderRow::Column(FName("AssetName"))
		.FillWidth(0.5f)
		.DefaultLabel(FText::FromString(TEXT("[Asset Name]")))

		+ SHeaderRow::Column(FName("TextureSize"))
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString(TEXT("[Texture Size]")))

		+ SHeaderRow::Column(FName("AssetReferencers"))
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString(TEXT("[Asset Referencers]")));
}

TSharedPtr<SHeaderRow> SMIForgeAssetBrowserPanel::SetupTexSetHeaderRow()
{
	return SNew(SHeaderRow)
		+ SHeaderRow::Column(FName("Select"))
		.FixedWidth(28.f)
		.HAlignHeader(HAlign_Center)
		.HAlignCell(HAlign_Center)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState)
				{
					switch (CheckState)
					{
					case ECheckBoxState::Checked:
						ViewModel->SetSelectedTextureSets(
							FilteredTextureSetListItems);
						for (const TSharedPtr<FMIForgeTextureSet>& TextureSet :
							FilteredTextureSetListItems)
						{
							SelectTexturesInSet(*TextureSet);
						}
						break;

					case ECheckBoxState::Unchecked:
						ViewModel->ClearTextureSetSelection();
						for (const TSharedPtr<FMIForgeTextureSet>& TextureSet :
							FilteredTextureSetListItems)
						{
							UnselectTexturesInSet(*TextureSet);
						}
						break;

					default:
						break;
					}
				})
			.IsChecked_Lambda([this]()
				{
					if (ViewModel->GetSelectedTextureSets().Num() == 0)
					{
						return ECheckBoxState::Unchecked;
					}
					if (ViewModel->GetSelectedTextureSets().Num() ==
						TextureCatalog->GetTextureSets().Num())
					{
						return ECheckBoxState::Checked;
					}
					return ECheckBoxState::Undetermined;
				})
		]

		+ SHeaderRow::Column(FName("Status"))
		.FillWidth_Lambda([this]()
			{
				return ViewModel.IsValid() &&
					ViewModel->GetPreset() ==
					EMIForgeGenerationPreset::VertexPainting
					? 0.08f
					: 0.04f;
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

		+ SHeaderRow::Column(FName("AssetName"))
		.FillWidth(0.5f)
		.DefaultLabel(FText::FromString(TEXT("[Asset Name]")))

		+ SHeaderRow::Column(FName("TextureSize"))
		.FillWidth(0.25f)
		.DefaultLabel(FText::FromString(TEXT("[Texture Size]")));
}

#undef LOCTEXT_NAMESPACE
