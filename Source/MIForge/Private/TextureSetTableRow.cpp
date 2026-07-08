// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureSetTableRow.h"
#include "MIForgeTypes.h"
#include "EditorAssetLibrary.h"
#include "MainTabWidget.h"
#include "MIForgeValidator.h"
#include "MIForgeStyle.h"
#include "MIForgeUtilities.h"
#include "MIForgeTexSetDragDropOperation.h"

void STextureSetTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{
	TextureSets = InArgs._TextureSets;
	ParentTable = InArgs._ParentTable;
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), OwnerTableView);
}

TSharedRef<SWidget> STextureSetTableRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "Select")
	{
		return MIForgeUtilities::WithRowSeparator(
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState) {
				switch (CheckState) {
				case ECheckBoxState::Checked:
					if (TextureSets.IsValid())
					{
						ParentTable->SelectTextureSet(TextureSets);
						ParentTable->SelectTexturesInSet(*TextureSets);
					}
					break;
				case ECheckBoxState::Unchecked:
					if (TextureSets.IsValid()) {
						ParentTable->UnselectTextureSet(TextureSets);
						ParentTable->UnselectTexturesInSet(*TextureSets);
					}

					break;
				}

				})
			.IsChecked_Lambda([this]() -> ECheckBoxState {
				return ParentTable->IsTextureSetSelected(TextureSets) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
							if (!TextureSets.IsValid() || !ParentTable.IsValid())
							{
								return nullptr;
							}

							if (!ParentTable->CurrentPresetOption.IsValid())
							{
								return nullptr;
							}

							FMIForgeValidator::EMIForgeTextureSetStatus StatusResult;

							const FString& PresetName = *ParentTable->CurrentPresetOption;

							if (PresetName == TEXT("Standard"))
							{
								StatusResult =
									FMIForgeValidator().GetStandardSetStatus(
										*TextureSets,
										ParentTable->bUseEmissiveTextures,
										ParentTable->bUseDetailNormalTextures,
										ParentTable->bIgnoreUnrecognizedTextures
									);
							}
							else if (PresetName == TEXT("RGB Masking"))
							{
								StatusResult =
									FMIForgeValidator().GetRGBSetStatus(
										*TextureSets,
										ParentTable->bUseBaseORMTexture,
										ParentTable->bEnableEmissiveChannel,
										ParentTable->bUseDetailNormalTextureRGB,
										ParentTable->bIgnoreUnrecognizedTextures
									);
							}
							else
							{
								return nullptr;
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
