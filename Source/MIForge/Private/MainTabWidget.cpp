// Fill out your copyright notice in the Description page of Project Settings.

#include "MainTabWidget.h"

#include "Catalog/MIForgeTextureCatalog.h"
#include "UIs/MIForgeAssetBrowserPanel.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "UIs/MIForgeRGBMaskPresetPanel.h"
#include "UIs/MIForgeStandardPresetPanel.h"
#include "UIs/MIForgeVertexPaintLayerStackPanel.h"
#include "UIs/MIForgeVertexPaintPresetPanel.h"
#include "UIs/MIForgeDecalPresetPanel.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

void SMainTabWidget::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FMIForgeMainTabViewModel>();
	PresetOptions =
	{
		MakeShared<EMIForgeGenerationPreset>(EMIForgeGenerationPreset::Standard),
		MakeShared<EMIForgeGenerationPreset>(EMIForgeGenerationPreset::RGBMask),
		MakeShared<EMIForgeGenerationPreset>(EMIForgeGenerationPreset::VertexPainting),
		MakeShared<EMIForgeGenerationPreset>(EMIForgeGenerationPreset::Decal)
	};

	ViewModel->OnPresetChanged.AddSP(
		SharedThis(this),
		&SMainTabWidget::HandlePresetChanged);

	TextureCatalog = MakeShared<FMIForgeTextureCatalog>();
	TextureCatalog->Initialize(InArgs._SelectedFolderPaths);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			BuildMainLayout()
		]
	];
}

SMainTabWidget::~SMainTabWidget()
{
	if (TextureCatalog.IsValid())
	{
		TextureCatalog->Shutdown();
	}
}

TSharedRef<SWidget> SMainTabWidget::BuildMainLayout()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(2.5f)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(5.f)
			[
				BuildPresetSelector()
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(5.f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			[
				BuildPresetPanelSwitcher()
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.025f)
		.Padding(1.f)
		[
			SNew(SSeparator)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(5.5f)
		[
			BuildAssetWorkSpace()
		];
}

TSharedRef<SWidget> SMainTabWidget::BuildPresetSelector()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(3.f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Master Material Preset: ")))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SComboBox<TSharedPtr<EMIForgeGenerationPreset>>)
			.OptionsSource(&PresetOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<EMIForgeGenerationPreset> Item)
			{
				return SNew(STextBlock)
					.Text(Item.IsValid()
						? FMIForgeMainTabViewModel::GetPresetDisplayText(*Item)
						: FText::GetEmpty());
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<EMIForgeGenerationPreset> Item, ESelectInfo::Type)
			{
				if (ViewModel.IsValid() && Item.IsValid())
				{
					ViewModel->SetPreset(*Item);
				}
			})
			.Content()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return ViewModel.IsValid()
						? ViewModel->GetPresetDisplayText()
						: FText::GetEmpty();
				})
			]
		];
}

TSharedRef<SWidget> SMainTabWidget::BuildPresetPanelSwitcher()
{
	return SAssignNew(PresetPanelSwitcher, SWidgetSwitcher)
		+ SWidgetSwitcher::Slot()
		[
			SNew(SMIForgeStandardPresetPanel).ViewModel(ViewModel)
		]
		+ SWidgetSwitcher::Slot()
		[
			SNew(SMIForgeRGBMaskPresetPanel).ViewModel(ViewModel)
		]
		+ SWidgetSwitcher::Slot()
		[
			SNew(SMIForgeVertexPaintPresetPanel).ViewModel(ViewModel)
		]
		+ SWidgetSwitcher::Slot()
		[
			SNew(SMIForgeDecalPresetPanel).ViewModel(ViewModel)
		];

}

TSharedRef<SWidget> SMainTabWidget::BuildAssetWorkSpace()
{
	return SNew(SMIForgeAssetBrowserPanel)
		.ViewModel(ViewModel)
		.TextureCatalog(TextureCatalog)
		[
			SNew(SMIForgeVertexPaintLayerStackPanel)
			.ViewModel(ViewModel)
		];
}

void SMainTabWidget::HandlePresetChanged(EMIForgeGenerationPreset NewPreset)
{
	if (!PresetPanelSwitcher.IsValid())
	{
		return;
	}

	switch (NewPreset)
	{
	case EMIForgeGenerationPreset::Standard:
		PresetPanelSwitcher->SetActiveWidgetIndex(0);
		break;
	case EMIForgeGenerationPreset::RGBMask:
		PresetPanelSwitcher->SetActiveWidgetIndex(1);
		break;
	case EMIForgeGenerationPreset::VertexPainting:
		PresetPanelSwitcher->SetActiveWidgetIndex(2);
		break;
	case EMIForgeGenerationPreset::Decal:
		PresetPanelSwitcher->SetActiveWidgetIndex(3);
		break;
	}
}
