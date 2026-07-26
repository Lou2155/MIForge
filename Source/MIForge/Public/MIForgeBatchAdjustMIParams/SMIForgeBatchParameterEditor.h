// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"

class SMIForgeBatchParameterEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeBatchParameterEditor) {}
		SLATE_ARGUMENT(FMIForgeBatchParameterModel, ParameterModel)
		SLATE_ARGUMENT(TWeakPtr<SDockTab>, OwnerTab)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FMIForgeBatchParameterModel ParameterModel;
	TArray<TSharedPtr<FMIForgeBatchParameterRow>> ParameterRows;

	TWeakPtr<SDockTab> OwnerTab;

	TSharedPtr<SScrollBox> ParameterGroupScrollBox;
	FText CurrentSearchText;

	/*TSharedRef<ITableRow> GenerateParameterRow(
		TSharedPtr<FMIForgeBatchParameterRow> Item,
		const TSharedRef<STableViewBase>& OwnerTable
	);*/

	TSharedRef<SWidget> GenerateValueWidget(
		TSharedPtr<FMIForgeBatchParameterRow> Item
	);

	FText GetParameterTypeText(EMIForgeBatchParameterType Type) const;
	FText GetValueText(TSharedPtr<FMIForgeBatchParameterRow> Item) const;

	FReply OpenVectorColorPicker(
		TSharedPtr<FMIForgeBatchParameterRow> Item
	);

	FReply OnApplyClicked();
	FReply OnCancelClicked();

	void RefreshParameterGroups();

	bool DoesParameterMatchSearch(
		const TSharedPtr<FMIForgeBatchParameterRow>& Item
	) const;

	TSharedRef<SWidget> GenerateParameterRowWidget(
		TSharedPtr<FMIForgeBatchParameterRow> Item
	);

	TArray<UMaterialInstanceConstant*> GetUniqueMaterialInstancesToEdit() const;

int32 GetCheckedParameterCount() const;

void ApplyCheckedRowsToMaterialInstance(
	UMaterialInstanceConstant* MaterialInstance
) const;

void ApplyParameterRowToMaterialInstance(
	UMaterialInstanceConstant* MaterialInstance,
	const FMIForgeBatchParameterRow& Row
) const;
};
