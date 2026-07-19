// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FMIForgeMainTabViewModel;

class SMIForgeStandardPresetPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeStandardPresetPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetGenerateButtonText() const;
	FReply Generate();

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
};
