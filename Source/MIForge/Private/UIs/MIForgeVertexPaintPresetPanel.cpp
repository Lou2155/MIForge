// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeVertexPaintPresetPanel.h"

#include "Generation/MIForgeGenerationCoordinator.h"
#include "MIForgeUtilities.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "UIs/MIForgeExistingAssetOptions.h"
#include "UIs/MIForgeMainTabViewModel.h"
#include "UIs/MIForgeTargetFolderSection.h"
#include "UIs/MIForgeValidationSummaryPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MIForgeVertexPaintPresetPanel"

void SMIForgeVertexPaintPresetPanel::Construct(const FArguments& InArgs)
{
	ViewModel = InArgs._ViewModel;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Generate Vertex Paint material instances with layer stack.")))
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
			.Text(FText::FromString(TEXT("Material Instance Name:")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12.5f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
		[
			SNew(SEditableTextBox)
			.HintText(FText::FromString(TEXT("Leave blank to use default name.")))
			.Text_Lambda([this]() { return FText::FromString(MaterialInstanceName); })
			.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
			{
				MaterialInstanceName = NewText.ToString().TrimStartAndEnd();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(2.f)
		[
			SNew(SSeparator)
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
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f, 10.f, 4.f, 4.f)
		[
			SNew(SMIForgeValidationSummaryPanel).ViewModel(ViewModel)
		]
		+ SVerticalBox::Slot().FillHeight(12.f).Padding(10.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SecondaryButton")
			.OnClicked(this, &SMIForgeVertexPaintPresetPanel::Generate)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GenerateVPInstance", "Generate VP Instance"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16.f))
			]
		]
	];
}

FReply SMIForgeVertexPaintPresetPanel::Generate()
{
	if (ViewModel->GetTargetPath().IsEmpty())
	{
		MIForgeUtilities::PrintWindow(
			TEXT("Please specify a target path for the generated material instances."),
			EAppMsgType::Ok);
		return FReply::Handled();
	}

	ViewModel->RefreshValidation();
	if (!ViewModel->GetVertexPaintValidationSummary().bCanGenerate)
	{
		MIForgeUtilities::PrintWindow(
			TEXT("Please fix all Vertex Paint layer errors before proceeding."),
			EAppMsgType::Ok);
		return FReply::Handled();
	}

	FMIForgeVertexPaintGenerationRequest Request;
	Request.LayerStack = ViewModel->GetVertexPaintLayerStack();
	Request.Options.TargetPath = ViewModel->GetTargetPath();
	Request.Options.IfMIExists = ViewModel->GetIfMIExists();

	const FString RequestedName = MaterialInstanceName.TrimStartAndEnd();
	if (!RequestedName.IsEmpty())
	{
		FText NameError;
		if (!FName::IsValidXName(RequestedName, INVALID_OBJECTNAME_CHARACTERS, &NameError))
		{
			MIForgeUtilities::PrintWindow(
				FString::Printf(TEXT("Invalid material instance name: %s"), *NameError.ToString()),
				EAppMsgType::Ok);
			return FReply::Handled();
		}
		Request.Options.MaterialInstanceName = RequestedName;
	}

	const FMIForgeGenerationOutcome Outcome =
		FMIForgeGenerationCoordinator().ExecuteVertexPaintGeneration(Request);
	MIForgeUtilities::PrintNotification(Outcome.SummaryText.ToString(), 5.f);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
