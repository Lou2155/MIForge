// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "TextureSetTableRow.h"
#include "MIForgeTypes.h"
#include "EditorAssetLibrary.h"
#include "MIForgeValidator.h"
#include "MIForgeStyle.h"
#include "MIForgeUtilities.h"
#include "MIForgeTexSetDragDropOperation.h"

#include "UIs/MIForgeMainTabViewModel.h"

void STextureSetTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{
	TextureSets = InArgs._TextureSets;
	ViewModel = InArgs._ViewModel;
	OnTextureSetCheckStateChanged =
		InArgs._OnTextureSetCheckStateChanged;
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), OwnerTableView);
}

TSharedRef<SWidget> STextureSetTableRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "Select")
	{
		return MIForgeUtilities::WithRowSeparator(
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState) {
				if (TextureSets.IsValid() && ViewModel.IsValid())
				{
					OnTextureSetCheckStateChanged.ExecuteIfBound(
						TextureSets,
						CheckState);
				}
				})
			.IsChecked_Lambda([this]() -> ECheckBoxState {
				return ViewModel.IsValid() && ViewModel->IsTextureSetSelected(TextureSets)
					? ECheckBoxState::Checked
					: ECheckBoxState::Unchecked;
				})

				);
	}
	if(ColumnName == "Status")
	{	// 3-preset validation need to be implemented
		return MIForgeUtilities::WithRowSeparator(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
					.Image_Lambda([this]() -> const FSlateBrush*
						{
							if (!TextureSets.IsValid())
							{
								return nullptr;
							}

							if (!ViewModel.IsValid())
							{
								return nullptr;
							}

							FMIForgeValidator::EMIForgeTextureSetStatus StatusResult;

							const EMIForgeGenerationPreset Preset = ViewModel->GetPreset();

							if (Preset == EMIForgeGenerationPreset::Standard)
							{
								StatusResult =
									FMIForgeValidator().GetStandardSetStatus(
										*TextureSets,
										ViewModel->GetUseEmissiveTextures(),
										ViewModel->GetUseDetailNormalTextures(),
										ViewModel->GetIgnoreUnrecognizedTextures()
									);
							}
							else if (Preset == EMIForgeGenerationPreset::RGBMask)
							{
								StatusResult =
									FMIForgeValidator().GetRGBSetStatus(
										*TextureSets,
										ViewModel->GetUseBaseORMTexture(),
										ViewModel->GetEnableEmissiveChannel(),
										ViewModel->GetUseDetailNormalTextureRGB(),
										ViewModel->GetIgnoreUnrecognizedTextures()
									);
							}
							else if (Preset == EMIForgeGenerationPreset::Decal)
							{
								StatusResult =
									FMIForgeValidator().GetDecalSetStatus(
										*TextureSets,
										ViewModel->GetUseDecalNormal(),
										ViewModel->GetUseDecalORM(),
										ViewModel->GetIgnoreUnrecognizedTextures()
									);
							}
							else
							{
								StatusResult =
									FMIForgeValidator().GetVertexPaintSetStatus(
										*TextureSets,
										ViewModel->GetIgnoreUnrecognizedTextures()
									);
							}

							if (StatusResult == FMIForgeValidator::EMIForgeTextureSetStatus::Ready)
							{
								return FMIForgeStyle::Get().GetBrush("ListView.Row.Accept");
							}

							if (StatusResult == FMIForgeValidator::EMIForgeTextureSetStatus::Warning)
							{
								return FMIForgeStyle::Get().GetBrush("ListView.Row.Warning");
							}

							return FMIForgeStyle::Get().GetBrush("ListView.Row.Reject");
						})
			]

			);
	}

	if (ColumnName == "AssetName") {

		return MIForgeUtilities::WithRowSeparator(
			SNew(STextBlock).Text(FText::FromString(TextureSets->SetName))
		);
	}

	if (ColumnName == "TextureSize") {
		

		FString SizeText;


		if (TextureSets.IsValid())
		{
			SizeText = TextureSets->GetTextureSizeText();
		}

		return MIForgeUtilities::WithRowSeparator(
			SNew(STextBlock).Text(FText::FromString(SizeText))
		);
	}

	return SNullWidget::NullWidget; // you have to return something
}

FReply STextureSetTableRow::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && TextureSets.IsValid())
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return SMultiColumnTableRow<TSharedPtr<FMIForgeTextureSet>>::OnMouseButtonDown(
		MyGeometry,
		MouseEvent
	);
}

FReply STextureSetTableRow::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!TextureSets.IsValid())
	{
		return FReply::Unhandled();
	}

	return FReply::Handled().BeginDragDrop(
		FMIForgeTexSetDragDropOperation::New(TextureSets)
	);
}
