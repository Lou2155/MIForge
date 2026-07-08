// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureTableRow.h"
#include "MIForgeTypes.h"
#include "EditorAssetLibrary.h"
#include "MainTabWidget.h"
#include "MIForgeUtilities.h"

void STextureTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{	
	TextureListItems = InArgs._TextureListItems;
	ParentTable = InArgs._ParentTable;
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), OwnerTableView);
}

TSharedRef<SWidget> STextureTableRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "Select")
	{
		return MIForgeUtilities::WithRowSeparator(
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckState) {
				switch (CheckState) {
				case ECheckBoxState::Checked:
					ParentTable->SelectTexture(TextureListItems);
					break;
				case ECheckBoxState::Unchecked:
					ParentTable->UnselectTexture(TextureListItems);
					break;
				}

				})
			.IsChecked_Lambda([this]() -> ECheckBoxState {
				return ParentTable->IsTextureSelected(TextureListItems) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
		);
	}

	if (ColumnName == "AssetName") {

		return MIForgeUtilities::WithRowSeparator(
			SNew(STextBlock).Text(FText::FromString(TextureListItems->AssetName))
		);
	}
	
	if (ColumnName == "TextureSize") {

		return MIForgeUtilities::WithRowSeparator(
			SNew(STextBlock).Text(FText::FromString(TextureListItems->TextureSizeText))
		);
	}

	if (ColumnName == "AssetReferencers") {

		//get the list of referencers for this asset	
		TArray<FString> ReferencersList =
			UEditorAssetLibrary::FindPackageReferencersForAsset(TextureListItems->ObjectPath);
		//return the number of referencers as a text block
		return MIForgeUtilities::WithRowSeparator(
			SNew(STextBlock).Text(FText::AsNumber(ReferencersList.Num()))
		);
	}

	return SNullWidget::NullWidget; // you have to return something
	
}
