// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeValidationSummaryPanel.h"

#include "MIForgeStyle.h"
#include "PopupWindowCreator.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/STextBlock.h"

void SMIForgeValidationSummaryPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(6.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex_Lambda([this]()
				{
					return ViewModel.IsValid() &&
						ViewModel->GetPreset() == EMIForgeGenerationPreset::VertexPainting
						? 1
						: 0;
				})
				+ SWidgetSwitcher::Slot()
				[
					CreateMaterialSummaryWidget()
				]
				+ SWidgetSwitcher::Slot()
				[
					CreateVertexPaintSummaryWidget()
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
				.OnClicked_Lambda([this]()
				{
					if (!ViewModel.IsValid())
					{
						return FReply::Handled();
					}

					if (ViewModel->GetPreset() == EMIForgeGenerationPreset::Standard)
					{
						PopupWindowCreator::OpenPopupWindow(
							FText::FromString(TEXT("Validation Details")),
							CreateMaterialValidationDetailsWidget(false),
							FVector2D(560.f, 460.f),
							true);
					}
					else if (ViewModel->GetPreset() == EMIForgeGenerationPreset::RGBMask)
					{
						PopupWindowCreator::OpenPopupWindow(
							FText::FromString(TEXT("Validation Details")),
							CreateMaterialValidationDetailsWidget(true),
							FVector2D(560.f, 460.f),
							true);
					}

					// Vertex Paint intentionally preserves the existing no-op behavior.
					return FReply::Handled();
				})
			]
		]
	];
}

TSharedRef<SWidget> SMIForgeValidationSummaryPanel::CreateMaterialSummaryWidget()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Validation Summary:\n")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				const FMIForgeValidationSummary& Summary = ViewModel->GetValidationSummary();
				return FText::Format(
					FText::FromString(TEXT("Ready to create: {0}/{1} MI(s)")),
					Summary.ReadyToCreateCount,
					Summary.TotalSets);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				const FMIForgeValidationSummary& Summary = ViewModel->GetValidationSummary();
				if (Summary.ReadyToCreateCount == 0 ||
					Summary.ReadyToCreateCount < Summary.TotalSets)
				{
					return FSlateColor(FLinearColor::Red);
				}
				return Summary.ReadyToCreateCount == Summary.TotalSets
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
					ViewModel->GetValidationSummary().MissingRequiredTextureCount);
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
					ViewModel->GetValidationSummary().MissingOptionalTextureCount);
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
					ViewModel->GetValidationSummary().UnrecognizedTextureCount);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetValidationSummary().UnrecognizedTextureCount == 0
					? FSlateColor(FLinearColor::Green)
					: FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f));
			})
		];
}

TSharedRef<SWidget> SMIForgeValidationSummaryPanel::CreateVertexPaintSummaryWidget()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Vertex Paint Validation Summary: \n")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(
					FText::FromString(TEXT("Ready to generate: {0} \n")),
					ViewModel->GetVertexPaintValidationSummary().bCanGenerate
						? FText::FromString(TEXT("Yes"))
						: FText::FromString(TEXT("No")));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetVertexPaintValidationSummary().bCanGenerate
					? FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f))
					: FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.f));
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(
					FText::FromString(TEXT("Layers Assigned: {0} / 4")),
					ViewModel->GetVertexPaintValidationSummary().AssignedLayerCount);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetVertexPaintValidationSummary().AssignedLayerCount < 2
					? FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.f))
					: FSlateColor::UseForeground();
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(
					FText::FromString(TEXT("Required Missing: {0}")),
					ViewModel->GetVertexPaintValidationSummary().MissingRequiredTextureCount);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetVertexPaintValidationSummary().MissingRequiredTextureCount > 0
					? FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.f))
					: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f));
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(
					FText::FromString(TEXT("Optional Missing: {0}")),
					ViewModel->GetVertexPaintValidationSummary().MissingOptionalTextureCount);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetVertexPaintValidationSummary().MissingOptionalTextureCount > 0
					? FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f))
					: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f));
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(
					FText::FromString(TEXT("Unrecognized Textures: {0} \n")),
					ViewModel->GetVertexPaintValidationSummary().UnrecognizedTextureCount);
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return ViewModel->GetVertexPaintValidationSummary().UnrecognizedTextureCount > 0
					? FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f))
					: FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f));
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(FText::FromString(TEXT("Base Layer: {0}")),
					VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().BaseStatus));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return VertexPaintStatusToColor(ViewModel->GetVertexPaintValidationSummary().BaseStatus);
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(FText::FromString(TEXT("R Layer: {0}")),
					VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerRStatus));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return VertexPaintStatusToColor(ViewModel->GetVertexPaintValidationSummary().LayerRStatus);
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(FText::FromString(TEXT("G Layer: {0}")),
					VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerGStatus));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return VertexPaintStatusToColor(ViewModel->GetVertexPaintValidationSummary().LayerGStatus);
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return FText::Format(FText::FromString(TEXT("B Layer: {0}")),
					VertexPaintStatusToText(ViewModel->GetVertexPaintValidationSummary().LayerBStatus));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return VertexPaintStatusToColor(ViewModel->GetVertexPaintValidationSummary().LayerBStatus);
			})
		];
}

TSharedRef<SWidget> SMIForgeValidationSummaryPanel::CreateMaterialValidationDetailsWidget(bool bRGBMask) const
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	const FMIForgeValidationSummary& Summary = ViewModel->GetValidationSummary();

	if (Summary.SetResults.Num() == 0)
	{
		ScrollBox->AddSlot().Padding(4.f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("No validation results available.")))
		];
		return ScrollBox;
	}

	auto TextureTypeToText = [bRGBMask](EMIForgeTextureType Type)
	{
		switch (Type)
		{
		case EMIForgeTextureType::Albedo: return FText::FromString(TEXT("Albedo"));
		case EMIForgeTextureType::Normal: return FText::FromString(TEXT("Normal"));
		case EMIForgeTextureType::RGB:
			return bRGBMask ? FText::FromString(TEXT("RGB Mask")) : FText::FromString(TEXT("Unknown"));
		case EMIForgeTextureType::ORM: return FText::FromString(TEXT("ORM"));
		case EMIForgeTextureType::Emissive: return FText::FromString(TEXT("Emissive"));
		case EMIForgeTextureType::DetailNormal: return FText::FromString(TEXT("Detail Normal"));
		default: return FText::FromString(TEXT("Unknown"));
		}
	};

	auto AddTableRow = [](TSharedRef<SGridPanel> Grid, int32& RowIndex,
		const FText& TypeText, const FText& DetailText, const FSlateBrush* Icon)
	{
		Grid->AddSlot(0, RowIndex).Padding(4.f, 3.f)
		[
			SNew(STextBlock).Text(TypeText)
		];
		Grid->AddSlot(1, RowIndex).Padding(4.f, 3.f)
		[
			SNew(STextBlock).Text(DetailText)
		];
		Grid->AddSlot(2, RowIndex).Padding(4.f, 3.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SImage).Image(Icon)
		];
		++RowIndex;
		Grid->AddSlot(0, RowIndex).ColumnSpan(3).Padding(0.f, 1.f)
		[
			SNew(SSeparator)
		];
		++RowIndex;
	};

	for (const FMIForgeTextureSetValidationResult& SetResult : Summary.SetResults)
	{
		TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
		int32 RowIndex = 0;
		Grid->AddSlot(0, RowIndex).Padding(4.f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Type")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
		];
		Grid->AddSlot(1, RowIndex).Padding(4.f)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Texture / Issue")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
		];
		Grid->AddSlot(2, RowIndex).Padding(4.f).HAlign(HAlign_Center)
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Status")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9.f))
		];
		++RowIndex;
		Grid->AddSlot(0, RowIndex).ColumnSpan(3)[SNew(SSeparator)];
		++RowIndex;

		for (const FMIForgeTextureRequirement& Requirement : SetResult.SuccessfulAppliedTextures)
		{
			AddTableRow(Grid, RowIndex, TextureTypeToText(Requirement.TextureType),
				FText::FromString(Requirement.TextureName), FMIForgeStyle::Get().GetBrush("ListView.Row.Accept"));
		}
		for (const FMIForgeTextureRequirement& Requirement : SetResult.MissingRequiredTextures)
		{
			AddTableRow(Grid, RowIndex, TextureTypeToText(Requirement.TextureType),
				FText::FromString(TEXT("Missing required texture")), FMIForgeStyle::Get().GetBrush("ListView.Row.Reject"));
		}
		for (const FMIForgeTextureRequirement& Requirement : SetResult.MissingOptionalTextures)
		{
			AddTableRow(Grid, RowIndex, TextureTypeToText(Requirement.TextureType),
				FText::FromString(TEXT("Missing optional texture")), FMIForgeStyle::Get().GetBrush("ListView.Row.Warning"));
		}
		for (const FMIForgeTextureRequirement& Requirement : SetResult.UnrecognizedTextures)
		{
			AddTableRow(Grid, RowIndex, TextureTypeToText(Requirement.TextureType),
				FText::FromString(Requirement.TextureName), FMIForgeStyle::Get().GetBrush("ListView.Row.Warning"));
		}

		ScrollBox->AddSlot().Padding(4.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(6.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock).Text(FText::FromString(SetResult.SetName))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10.f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					Grid
				]
			]
		];
	}

	return ScrollBox;
}

FText SMIForgeValidationSummaryPanel::VertexPaintStatusToText(EMIForgeVertexPaintLayerStatus Status)
{
	switch (Status)
	{
	case EMIForgeVertexPaintLayerStatus::Valid: return FText::FromString(TEXT("Ready"));
	case EMIForgeVertexPaintLayerStatus::Warning: return FText::FromString(TEXT("Warning"));
	case EMIForgeVertexPaintLayerStatus::Error: return FText::FromString(TEXT("Error"));
	case EMIForgeVertexPaintLayerStatus::Empty:
	default: return FText::FromString(TEXT("Empty"));
	}
}

FSlateColor SMIForgeValidationSummaryPanel::VertexPaintStatusToColor(EMIForgeVertexPaintLayerStatus Status)
{
	switch (Status)
	{
	case EMIForgeVertexPaintLayerStatus::Valid: return FSlateColor(FLinearColor(0.f, 1.f, 0.f, 1.f));
	case EMIForgeVertexPaintLayerStatus::Warning: return FSlateColor(FLinearColor(1.f, 0.75f, 0.f, 1.f));
	case EMIForgeVertexPaintLayerStatus::Error: return FSlateColor(FLinearColor(1.f, 0.f, 0.f, 1.f));
	case EMIForgeVertexPaintLayerStatus::Empty:
	default: return FSlateColor::UseForeground();
	}
}
