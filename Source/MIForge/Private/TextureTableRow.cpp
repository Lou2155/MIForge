// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureTableRow.h"
#include "MIForgeTypes.h"
#include "EditorAssetLibrary.h"
#include "MIForgeUtilities.h"
#include "UIs/MIForgeMainTabViewModel.h"

void STextureTableRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{	
	TextureListItems = InArgs._TextureListItems;
	ViewModel = InArgs._ViewModel;
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
					if (ViewModel.IsValid()) ViewModel->SelectTexture(TextureListItems);
					break;
				case ECheckBoxState::Unchecked:
					if (ViewModel.IsValid()) ViewModel->UnselectTexture(TextureListItems);
					break;
				}

				})
			.IsChecked_Lambda([this]() -> ECheckBoxState {
				return ViewModel.IsValid() && ViewModel->IsTextureSelected(TextureListItems)
					? ECheckBoxState::Checked
					: ECheckBoxState::Unchecked;
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
