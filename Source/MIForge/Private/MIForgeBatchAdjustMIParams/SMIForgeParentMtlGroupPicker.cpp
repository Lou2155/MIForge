// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeBatchAdjustMIParams/SMIForgeParentMtlGroupPicker.h"

void SMIForgeParentMtlGroupPicker::Construct(
	const FArguments& InArgs
)
{
	OnGroupChosen = InArgs._OnGroupChosen;
	OwnerWindow = InArgs._OwnerWindow;

	for (const FMIForgeMaterialParentGroup& Group : InArgs._ParentGroups)
	{
		GroupItems.Add(MakeShared<FMIForgeMaterialParentGroup>(Group));
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Select Parent Material Group")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12.f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(
				TEXT("Selected actors contain Material Instances with multiple root parent materials.")
				))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SListView<TSharedPtr<FMIForgeMaterialParentGroup>>)
				.ListItemsSource(&GroupItems)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SMIForgeParentMtlGroupPicker::GenerateGroupRow)
				.OnSelectionChanged_Lambda(
					[this](TSharedPtr<FMIForgeMaterialParentGroup> InSelectedItem, ESelectInfo::Type)
					{
						SelectedGroup = InSelectedItem;
					}
				)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f, 0.f)
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Cancel")))
						.OnClicked(this, &SMIForgeParentMtlGroupPicker::OnCancelClicked)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f, 0.f)
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Continue")))
						.IsEnabled(this, &SMIForgeParentMtlGroupPicker::CanContinue)
						.OnClicked(this, &SMIForgeParentMtlGroupPicker::OnContinueClicked)
				]
			]

		]

	];

}

TSharedRef<ITableRow> SMIForgeParentMtlGroupPicker::GenerateGroupRow(TSharedPtr<FMIForgeMaterialParentGroup> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FString ParentName =
		Item.IsValid() && Item->RootParentMaterial.IsValid()
		? Item->RootParentMaterial->GetName()
		: TEXT("Invalid Parent");

	const int32 ActorCount = Item.IsValid() ? Item->ActorCount : 0;
	const int32 SlotCount = Item.IsValid() ? Item->SlotCount : 0;
	const int32 UniqueMICCount = Item.IsValid() ? Item->UniqueMICCount : 0;

	return SNew(STableRow<TSharedPtr<FMIForgeMaterialParentGroup>>, OwnerTable)
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(4.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(ParentName))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(12.f, 4.f)
				[
					SNew(STextBlock)
						.Text(FText::Format(
							FText::FromString(TEXT("Actors: {0}")),
							ActorCount
						))
				]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(12.f, 4.f)
				[
					SNew(STextBlock)
						.Text(FText::Format(
							FText::FromString(TEXT("Slots: {0}")),
							SlotCount
						))
				]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(12.f, 4.f)
				[
					SNew(STextBlock)
						.Text(FText::Format(
							FText::FromString(TEXT("Unique MIs: {0}")),
							UniqueMICCount
						))
				]
		];
}

bool SMIForgeParentMtlGroupPicker::CanContinue() const
{
	return SelectedGroup.IsValid();
}

FReply SMIForgeParentMtlGroupPicker::OnContinueClicked()
{
	if (SelectedGroup.IsValid() && OnGroupChosen.IsBound())
	{
		OnGroupChosen.Execute(*SelectedGroup);
	}

	if (TSharedPtr<SWindow> Window = OwnerWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SMIForgeParentMtlGroupPicker::OnCancelClicked()
{
	if (TSharedPtr<SWindow> Window = OwnerWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}

	return FReply::Handled();
}
