// Fill out your copyright notice in the Description page of Project Settings.


#include "UIs/MIForgeDecalPresetPanel.h"

#include "Generation/MIForgeGenerationCoordinator.h"
#include "MIForgeUtilities.h"
#include "Styling/CoreStyle.h"
#include "UIs/MIForgeExistingAssetOptions.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "UIs/MIForgeTargetFolderSection.h"
#include "UIs/MIForgeValidationSummaryPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SMIForgeDecalPresetPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Generate decal material instances. ")))
						.ColorAndOpacity(FLinearColor(.15f, .15f, .15f, 1.f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SMIForgeTargetFolderSection).ViewModel(ViewModel)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Instance Options")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
				[
					SNew(SCheckBox)
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								ViewModel->SetUseDecalNormal(State == ECheckBoxState::Checked);
							})
						.IsChecked_Lambda([this]()
							{
								return ViewModel->GetUseDecalNormal() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
							})
						[SNew(STextBlock).Text(FText::FromString(TEXT("Use Normal Texture ")))]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
				[
					SNew(SCheckBox)
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								ViewModel->SetUseDecalORM(State == ECheckBoxState::Checked);
							})
						.IsChecked_Lambda([this]()
							{
								return ViewModel->GetUseDecalORM() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
							})
						[SNew(STextBlock).Text(FText::FromString(TEXT("Use ORM Texture ")))]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
				[
					SNew(SCheckBox)
						.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								ViewModel->SetUseOrientationMask(State == ECheckBoxState::Checked);
							})
						.IsChecked_Lambda([this]()
							{
								return ViewModel->GetUseOrientationMask() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
							})
						[SNew(STextBlock).Text(FText::FromString(TEXT("Use Orientation Mask ")))]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("If MI exists: ")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
				[
					SNew(SMIForgeExistingAssetOptions).ViewModel(ViewModel)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
				[
					SNew(SMIForgeValidationSummaryPanel).ViewModel(ViewModel)
				]
				+ SVerticalBox::Slot().FillHeight(12.f).Padding(10.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SButton)
						.OnClicked(this, &SMIForgeDecalPresetPanel::Generate)
						[
							SNew(STextBlock)
								.Text(this, &SMIForgeDecalPresetPanel::GetGenerateButtonText)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.f))
						]
				]
		];
}

FText SMIForgeDecalPresetPanel::GetGenerateButtonText() const
{
	return FText::Format(
		FText::FromString(TEXT("Generate Instances ({0})")),
		ViewModel->GetValidationSummary().ReadyToCreateCount);
}

FReply SMIForgeDecalPresetPanel::Generate()
{
	if (ViewModel->GetTargetPath().IsEmpty())
	{
		MIForgeUtilities::PrintWindow(
			TEXT("Please specify a target path for the generated material instances."),
			EAppMsgType::Ok);
		return FReply::Handled();
	}

	if (ViewModel->GetValidationSummary().SetsWithErrors > 0)
	{
		MIForgeUtilities::PrintWindow(
			TEXT("Please fix all the errors before proceeding. (See 'Validation Summary' or click on 'View Details' for more information)"),
			EAppMsgType::Ok);
		return FReply::Handled();
	}

	FMIForgeMaterialGenerationRequest Request;
	Request.TextureSets = ViewModel->BuildGenerationTextureSets();
	Request.Options.Preset = EMIForgeGenerationPreset::Decal;
	Request.Options.TargetPath = ViewModel->GetTargetPath();
	Request.Options.bUseDecalNormal = ViewModel->GetUseDecalNormal();
	Request.Options.bUseDecalORM = ViewModel->GetUseDecalORM();
	Request.Options.bUseOrientationMask = ViewModel->GetUseOrientationMask();
	Request.Options.IfMIExists = ViewModel->GetIfMIExists();

	const FMIForgeGenerationOutcome Outcome =
		FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Request);
	MIForgeUtilities::PrintNotification(Outcome.SummaryText.ToString(), 5.f);
	return FReply::Handled();
}
