// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FMIForgeMainTabViewModel;

class SMIForgeExistingAssetOptions : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeExistingAssetOptions) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
};
