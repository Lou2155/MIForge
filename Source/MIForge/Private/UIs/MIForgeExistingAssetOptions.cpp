// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeExistingAssetOptions.h"

#include "Styling/AppStyle.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SMIForgeExistingAssetOptions::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	auto MakeOption = [this](EIfMIExistsOption Option, const TCHAR* Label)
	{
		return SNew(SCheckBox)
			.Style(FAppStyle::Get(), "RadioButton")
			.OnCheckStateChanged_Lambda([this, Option](ECheckBoxState NewState)
			{
				if (NewState == ECheckBoxState::Checked)
				{
					ViewModel->SetIfMIExists(Option);
				}
			})
			.IsChecked_Lambda([this, Option]()
			{
				return ViewModel->GetIfMIExists() == Option
					? ECheckBoxState::Checked
					: ECheckBoxState::Unchecked;
			})
			[
				SNew(STextBlock).Text(FText::FromString(Label))
			];
	};

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			MakeOption(EIfMIExistsOption::Skip, TEXT("Skip"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			MakeOption(EIfMIExistsOption::Overwrite, TEXT("Overwrite"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(1.f)
		[
			MakeOption(EIfMIExistsOption::CreateUnique, TEXT("Create Unique Name"))
		]
	];
}
