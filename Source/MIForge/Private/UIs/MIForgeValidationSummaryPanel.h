// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"
#include "Widgets/SCompoundWidget.h"

class FMIForgeMainTabViewModel;


class SMIForgeValidationSummaryPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeValidationSummaryPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateMaterialSummaryWidget();
	TSharedRef<SWidget> CreateVertexPaintSummaryWidget();
	TSharedRef<SWidget> CreateMaterialValidationDetailsWidget(EMIForgeGenerationPreset Preset) const;

	static FText VertexPaintStatusToText(EMIForgeVertexPaintLayerStatus Status);
	static FSlateColor VertexPaintStatusToColor(EMIForgeVertexPaintLayerStatus Status);

	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
};
