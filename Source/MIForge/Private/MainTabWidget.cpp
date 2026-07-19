// Fill out your copyright notice in the Description page of Project Settings.


#include "MainTabWidget.h"
#include "Catalog/MIForgeTextureCatalog.h"
#include "MIForgeTypes.h"
#include "MIForgeUtilities.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "MIForgeValidator.h"
#include "MIForgeStyle.h"
#include "PopupWindowCreator.h"
#include "MIForgeTextureSetDropTarget.h"
#include "Presets/MIForgePresetDefinitions.h"
#include "Generation/MIForgeGenerationCoordinator.h"

#include "UIs/MIForgeMainTabViewModel.h"
#include "UIs/MIForgeAssetBrowserPanel.h"


#define LOCTEXT_NAMESPACE "MainTabWidget"

namespace
{
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

	ViewModel = MakeShared<FMIForgeMainTabViewModel>();

	PresetOptions =
	{
		MakeShared<EMIForgeGenerationPreset>(
			EMIForgeGenerationPreset::Standard),

		MakeShared<EMIForgeGenerationPreset>(
			EMIForgeGenerationPreset::RGBMask),

		MakeShared<EMIForgeGenerationPreset>(
			EMIForgeGenerationPreset::VertexPainting)
	};

	ViewModel->OnPresetChanged.AddSP(
		SharedThis(this),
		&SMainTabWidget::HandlePresetChanged);

	ViewModel->OnVertexPaintChanged.AddSP(
		SharedThis(this),
		&SMainTabWidget::HandleVertexPaintChanged);

	TextureCatalog = MakeShared<FMIForgeTextureCatalog>();
	TextureCatalog->Initialize(InArgs._SelectedFolderPaths);

	CurrentMaxLayersOption = MakeShared<FString>(TEXT("2 (R)"));
	
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

}

SMainTabWidget::~SMainTabWidget()
{
	if (TextureCatalog.IsValid())
	{
		TextureCatalog->Shutdown();
	}
}

TSharedRef<SWidget> SMainTabWidget::Page1()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(2.5f)
		.Padding(5.0f)
		[
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

		+ SHorizontalBox::Slot()
		.FillWidth(0.025f)
		.Padding(1.0f)
		[
			SNew(SSeparator)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(5.5f)
		[
			RightContentWidget()
		];
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
			SNew(SComboBox<TSharedPtr<EMIForgeGenerationPreset>>)
				.OptionsSource(&PresetOptions)
				.OnGenerateWidget_Lambda([](TSharedPtr<EMIForgeGenerationPreset> InItem) {
				const FText Label =
					InItem.IsValid()
					? FMIForgeMainTabViewModel::
					GetPresetDisplayText(*InItem)
					: FText::GetEmpty();

				return SNew(STextBlock)
					.Text(Label);
					})
				.OnSelectionChanged_Lambda([this](TSharedPtr<EMIForgeGenerationPreset> Item, ESelectInfo::Type SelectInfo) {

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

TSharedRef<SWidget> SMainTabWidget::RightContentWidget()
{
	return SNew(SMIForgeAssetBrowserPanel)
			.ViewModel(ViewModel)
			.TextureCatalog(TextureCatalog)
		[
			VertexPaintingLayerStackPanel()
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
							ViewModel->GetValidationSummary().ReadyToCreateCount,
							ViewModel->GetValidationSummary().TotalSets
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						if (ViewModel->GetValidationSummary().ReadyToCreateCount == 0)
							return FSlateColor(FLinearColor::Red);
						else if (ViewModel->GetValidationSummary().ReadyToCreateCount < ViewModel->GetValidationSummary().TotalSets)
							return FSlateColor(FLinearColor::Red);
						else
							return ViewModel->GetValidationSummary().ReadyToCreateCount == ViewModel->GetValidationSummary().TotalSets
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
							ViewModel->GetValidationSummary().MissingRequiredTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return ViewModel->GetValidationSummary().MissingRequiredTextureCount == 0
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
							ViewModel->GetValidationSummary().MissingOptionalTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return ViewModel->GetValidationSummary().MissingOptionalTextureCount == 0
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
							ViewModel->GetValidationSummary().UnrecognizedTextureCount
						);
					})
				.ColorAndOpacity_Lambda([this]()
					{
						return ViewModel->GetValidationSummary().UnrecognizedTextureCount == 0
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
							ViewModel->GetVertexPaintValidationSummary().bCanGenerate
							? FText::FromString(TEXT("Yes"))
							: FText::FromString(TEXT("No"))
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return ViewModel->GetVertexPaintValidationSummary().bCanGenerate
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
							ViewModel->GetVertexPaintValidationSummary().AssignedLayerCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return ViewModel->GetVertexPaintValidationSummary().AssignedLayerCount < 2
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
							ViewModel->GetVertexPaintValidationSummary().MissingRequiredTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return ViewModel->GetVertexPaintValidationSummary().MissingRequiredTextureCount > 0
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
							ViewModel->GetVertexPaintValidationSummary().MissingOptionalTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return ViewModel->GetVertexPaintValidationSummary().MissingOptionalTextureCount > 0
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
							ViewModel->GetVertexPaintValidationSummary().UnrecognizedTextureCount
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return ViewModel->GetVertexPaintValidationSummary().UnrecognizedTextureCount > 0
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
							VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().BaseStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							ViewModel->GetVertexPaintValidationSummary().BaseStatus
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
							VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerRStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							ViewModel->GetVertexPaintValidationSummary().LayerRStatus
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
							VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerGStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							ViewModel->GetVertexPaintValidationSummary().LayerGStatus
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
							VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerBStatus)
						);
					}),
				TAttribute<FSlateColor>::CreateLambda([this]()
					{
						return VertexPaintStatusToColor(
							ViewModel->GetVertexPaintValidationSummary().LayerBStatus
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
						return (ViewModel.IsValid() && ViewModel->GetPreset() == EMIForgeGenerationPreset::VertexPainting) ? 1 : 0;
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
						if(ViewModel.IsValid() && ViewModel->GetPreset() == EMIForgeGenerationPreset::Standard)
						{
							PopupWindowCreator::OpenPopupWindow(FText::FromString(TEXT("Validation Details")),
								CreateStandardValidationDetailsWidget(),
								FVector2D(560.f, 460.f),
								true
							);
						}
						else if (ViewModel.IsValid() && ViewModel->GetPreset() == EMIForgeGenerationPreset::RGBMask)
						{
							PopupWindowCreator::OpenPopupWindow(FText::FromString(TEXT("Validation Details")),
								CreateRGBmaskingValidationDetailsWidget(),
								FVector2D(560.f, 460.f),
								true
							);
						}
						else if(ViewModel.IsValid() && ViewModel->GetPreset() == EMIForgeGenerationPreset::VertexPainting)
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
				if (ViewModel->GetTargetPath().IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}
				if (ViewModel->GetValidationSummary().SetsWithErrors > 0)
				{
					MIForgeUtilities::PrintWindow((TEXT("Please fix all the errors before proceeding. (See 'Validation Summary' or click on 'View Details' for more information)")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				FMIForgeMaterialGenerationRequest Request;

				Request.TextureSets = ViewModel->BuildGenerationTextureSets();
				Request.Options.Preset = EMIForgeGenerationPreset::Standard;
				Request.Options.TargetPath = ViewModel->GetTargetPath();
				Request.Options.bUseEmissive = ViewModel->GetUseEmissiveTextures();
				Request.Options.bUseDetailNormal = ViewModel->GetUseDetailNormalTextures();
				Request.Options.bUseTriplanar = ViewModel->GetUseTriplanarProjection();
				Request.Options.IfMIExists = ViewModel->GetIfMIExists();

				
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
	const FMIForgeVertexPaintLayerSlot* Slot = ViewModel->FindVertexPaintLayerSlot(Layer);

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
				if (!ViewModel->AssignTextureSetToVertexLayer(DroppedLayer, DroppedTextureSet))
				{
					return FReply::Unhandled();
				}

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
							if (const FMIForgeVertexPaintLayerSlot* CurrentSlot = ViewModel->FindVertexPaintLayerSlot(Layer))
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
								ViewModel->ClearVertexPaintLayer(Layer);
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
									if (const FMIForgeVertexPaintLayerSlot* CurrentSlot = ViewModel->FindVertexPaintLayerSlot(Layer))
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
												const FMIForgeVertexPaintLayerSlot* CurrentSlot = ViewModel->FindVertexPaintLayerSlot(Layer);
												if (CurrentSlot && CurrentSlot->IsAssigned())
												{
													return FText::FromString(CurrentSlot->GetAddedTextureTypeText());
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
												{	const FMIForgeVertexPaintLayerSlot* CurrentSlot = ViewModel->FindVertexPaintLayerSlot(Layer);
													if (CurrentSlot && CurrentSlot->IsAssigned())
													{
														return FText::FromString(CurrentSlot->GetTextureSizeText());
													}
													return FText::FromString(TEXT(""));
												})
											
									]
						
						]
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
								return FText::FromString(ViewModel->GetTargetPath());
							})
						.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
							{
								ViewModel->SetTargetPath(NewText.ToString());
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
							ViewModel->SetTargetPath(SelectedPath);
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
						ViewModel->SetUseEmissiveTextures(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetUseEmissiveTextures() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
						ViewModel->SetUseDetailNormalTextures(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetUseDetailNormalTextures() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
						ViewModel->SetUseTriplanarProjection(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetUseTriplanarProjection() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
						ViewModel->SetUseBaseORMTexture(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetUseBaseORMTexture() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
						ViewModel->SetEnableEmissiveChannel(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetEnableEmissiveChannel() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
						ViewModel->SetUseDetailNormalTextureRGB(NewState == ECheckBoxState::Checked);
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return ViewModel->GetUseDetailNormalTextureRGB() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
				if (ViewModel->GetTargetPath().IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}
				if (ViewModel->GetValidationSummary().SetsWithErrors > 0)
				{
					MIForgeUtilities::PrintWindow((TEXT("Please fix all the errors before proceeding. (See 'Validation Summary' or click on 'View Details' for more information)")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				FMIForgeMaterialGenerationRequest Request;

				Request.TextureSets = ViewModel->BuildGenerationTextureSets();
				Request.Options.Preset = EMIForgeGenerationPreset::RGBMask;
				Request.Options.TargetPath = ViewModel->GetTargetPath();
				Request.Options.bUseBaseORMTexture = ViewModel->GetUseBaseORMTexture();
				Request.Options.bEnableEmissiveChannel = ViewModel->GetEnableEmissiveChannel();
				Request.Options.bUseDetailNormalTextureRGB = ViewModel->GetUseDetailNormalTextureRGB();
				Request.Options.IfMIExists = ViewModel->GetIfMIExists();

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
				if (ViewModel->GetTargetPath().IsEmpty())
				{
					MIForgeUtilities::PrintWindow((TEXT("Please specify a target path for the generated material instances.")), EAppMsgType::Ok);
					return FReply::Handled();
				}

				ViewModel->RefreshValidation();
				if (!ViewModel->GetVertexPaintValidationSummary().bCanGenerate)
				{
					MIForgeUtilities::PrintWindow(
						TEXT("Please fix all Vertex Paint layer errors before proceeding."),
						EAppMsgType::Ok
					);
					return FReply::Handled();
				}

				FMIForgeVertexPaintGenerationRequest Request;
				Request.LayerStack = ViewModel->GetVertexPaintLayerStack();
				Request.Options.TargetPath = ViewModel->GetTargetPath();
				Request.Options.IfMIExists = ViewModel->GetIfMIExists();
				

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
									ViewModel->GetVertexPaintLayerStack()
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

							FMIForgeVertexPaintLayerStack LoadedStack = ViewModel->GetVertexPaintLayerStack();
							if (VertexPaintRecipeManager.LoadRecipe(*Option, LoadedStack))
							{
								ViewModel->SetVertexPaintLayerStack(LoadedStack);
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
								FText Error;
								if (!ViewModel->AssignSelectedTextureSetsToVertexLayers(Error))
								{
									MIForgeUtilities::PrintNotification(Error.ToString());
									return FReply::Unhandled();
								}

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
								ViewModel->ClearAllVertexPaintLayers();

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
							ViewModel->SetIfMIExists(EIfMIExistsOption::Skip);
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (ViewModel->GetIfMIExists() == EIfMIExistsOption::Skip) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
							ViewModel->SetIfMIExists(EIfMIExistsOption::Overwrite);
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (ViewModel->GetIfMIExists() == EIfMIExistsOption::Overwrite) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
							ViewModel->SetIfMIExists(EIfMIExistsOption::CreateUnique);
						}
					})
				.IsChecked_Lambda([this]() -> ECheckBoxState
					{
						return (ViewModel->GetIfMIExists() == EIfMIExistsOption::CreateUnique) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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

	if (ViewModel->GetUseEmissiveTextures())
	{
		AddTextureIfPresent(EMIForgeTextureType::Emissive);
	}

	if (ViewModel->GetUseDetailNormalTextures())
	{
		AddTextureIfPresent(EMIForgeTextureType::DetailNormal);
	}

	return Result;
}

FText SMainTabWidget::GetValidationSummaryText() const
{
	return FText::Format(
		FText::FromString(TEXT("Validation Summary: \n\nReady to create: {0}/{1} MI(s)\nMissing required textures: {2}\nMissing optional textures: {3}\nUnrecognized textures: {4}")),
		ViewModel->GetValidationSummary().ReadyToCreateCount,
		ViewModel->GetValidationSummary().TotalSets,
		ViewModel->GetValidationSummary().MissingRequiredTextureCount,
		ViewModel->GetValidationSummary().MissingOptionalTextureCount,
		ViewModel->GetValidationSummary().UnrecognizedTextureCount
	);
}

FText SMainTabWidget::GetMIReadyCount() const
{
	return FText::Format(
		FText::FromString(TEXT("Generate Instances ({0})")),
		ViewModel->GetValidationSummary().ReadyToCreateCount
	);
}

TSharedRef<SWidget> SMainTabWidget::CreateStandardValidationDetailsWidget()
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	 if(ViewModel->GetValidationSummary().SetResults.Num() == 0)
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
		 ViewModel->GetValidationSummary().SetResults)
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

	if (ViewModel->GetValidationSummary().SetResults.Num() == 0)
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
		ViewModel->GetValidationSummary().SetResults)
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

FText SMainTabWidget::GetVertexPaintLayerStatusText(EMIForgeVertexPaintLayer Layer) const
{
	for (const FMIForgeVertexPaintLayerValidationResult& Result :
		ViewModel->GetVertexPaintValidationResult().LayerResults)
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
		ViewModel->GetVertexPaintValidationSummary().bCanGenerate ? TEXT("Yes") : TEXT("No")
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
		ViewModel->GetVertexPaintValidationSummary().LayerResults)
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
		ViewModel->GetVertexPaintValidationResult().LayerResults)
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
	const FMIForgeVertexPaintLayerSlot* Slot =
		ViewModel->FindVertexPaintLayerSlot(Layer);

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

void SMainTabWidget::HandlePresetChanged(EMIForgeGenerationPreset NewPreset)
{
	switch (NewPreset)
	{
	case EMIForgeGenerationPreset::Standard:
		PresetPannelSwitcher0->SetActiveWidgetIndex(0);
		break;

	case EMIForgeGenerationPreset::RGBMask:
		PresetPannelSwitcher0->SetActiveWidgetIndex(1);
		break;

	case EMIForgeGenerationPreset::VertexPainting:
		PresetPannelSwitcher0->SetActiveWidgetIndex(2);
		break;
	}
}

void SMainTabWidget::HandleVertexPaintChanged()
{
	RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::Base);
	RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerR);
	RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerG);
	RefreshVertexPaintLayerThumbnail(EMIForgeVertexPaintLayer::LayerB);
}


#undef LOCTEXT_NAMESPACE
