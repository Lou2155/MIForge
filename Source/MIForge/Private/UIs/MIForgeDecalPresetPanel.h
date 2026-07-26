// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FMIForgeMainTabViewModel;

class SMIForgeDecalPresetPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeDecalPresetPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetGenerateButtonText() const;
	FReply Generate();

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;

	
};
