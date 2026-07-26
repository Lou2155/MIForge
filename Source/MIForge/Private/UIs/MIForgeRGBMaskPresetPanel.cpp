// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeRGBMaskPresetPanel.h"

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

void SMIForgeRGBMaskPresetPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Generate RGB Mask material instances. (ORM Workflow)")))
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
				ViewModel->SetUseBaseORMTexture(State == ECheckBoxState::Checked);
			})
			.IsChecked_Lambda([this]()
			{
				return ViewModel->GetUseBaseORMTexture() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			[SNew(STextBlock).Text(FText::FromString(TEXT("Use Base ORM Texture")))]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				ViewModel->SetEnableEmissiveChannel(State == ECheckBoxState::Checked);
			})
			.IsChecked_Lambda([this]()
			{
				return ViewModel->GetEnableEmissiveChannel() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			[SNew(STextBlock).Text(FText::FromString(TEXT("Enable Emissive Channel")))]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(6.f)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				ViewModel->SetUseDetailNormalTextureRGB(State == ECheckBoxState::Checked);
			})
			.IsChecked_Lambda([this]()
			{
				return ViewModel->GetUseDetailNormalTextureRGB() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			[SNew(STextBlock).Text(FText::FromString(TEXT("Use Detail Normal Texture")))]
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
			.OnClicked(this, &SMIForgeRGBMaskPresetPanel::Generate)
			[
				SNew(STextBlock)
				.Text(this, &SMIForgeRGBMaskPresetPanel::GetGenerateButtonText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.f))
			]
		]
	];
}

FText SMIForgeRGBMaskPresetPanel::GetGenerateButtonText() const
{
	return FText::Format(
		FText::FromString(TEXT("Generate Instances ({0})")),
		ViewModel->GetValidationSummary().ReadyToCreateCount);
}

FReply SMIForgeRGBMaskPresetPanel::Generate()
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
	Request.Options.Preset = EMIForgeGenerationPreset::RGBMask;
	Request.Options.TargetPath = ViewModel->GetTargetPath();
	Request.Options.bUseBaseORMTexture = ViewModel->GetUseBaseORMTexture();
	Request.Options.bEnableEmissiveChannel = ViewModel->GetEnableEmissiveChannel();
	Request.Options.bUseDetailNormalTextureRGB = ViewModel->GetUseDetailNormalTextureRGB();
	Request.Options.IfMIExists = ViewModel->GetIfMIExists();

	const FMIForgeGenerationOutcome Outcome =
		FMIForgeGenerationCoordinator().ExecuteMaterialGeneration(Request);
	MIForgeUtilities::PrintNotification(Outcome.SummaryText.ToString(), 5.f);
	return FReply::Handled();
}
