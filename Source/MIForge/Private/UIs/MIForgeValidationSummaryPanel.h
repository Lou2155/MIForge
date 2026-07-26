// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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
