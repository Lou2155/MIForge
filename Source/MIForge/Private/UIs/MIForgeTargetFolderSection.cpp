// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeTargetFolderSection.h"

#include "MIForgeStyle.h"
#include "MIForgeUtilities.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SMIForgeTargetFolderSection::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Target Folder:")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		]
		+ SVerticalBox::Slot()
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
				.OnClicked_Lambda([this]()
				{
					MIForgeUtilities::CreatePathSelector(
						SharedThis(this),
						FOnPathSelected::CreateLambda([this](const FString& SelectedPath)
						{
							ViewModel->SetTargetPath(SelectedPath);
						}));
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FMIForgeStyle::Get().GetBrush("Panel.FolderSelection"))
				]
			]
		]
	];
}
