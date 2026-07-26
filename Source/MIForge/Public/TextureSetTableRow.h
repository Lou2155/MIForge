// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"


struct FMIForgeTextureSet;
class FMIForgeMainTabViewModel;

DECLARE_DELEGATE_TwoParams(
	FMIForgeOnTextureSetCheckStateChanged,
	const TSharedPtr<FMIForgeTextureSet>&,
	ECheckBoxState)


class STextureSetTableRow : public SMultiColumnTableRow<TSharedPtr<FMIForgeTextureSet>>
{
public:
    SLATE_BEGIN_ARGS(STextureSetTableRow) {}
        SLATE_ARGUMENT(TSharedPtr<FMIForgeTextureSet>, TextureSets)
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
		SLATE_EVENT(
			FMIForgeOnTextureSetCheckStateChanged,
			OnTextureSetCheckStateChanged)
    SLATE_END_ARGS()

    void Construct(
        const FArguments& InArgs,
        const TSharedRef<STableViewBase>& OwnerTableView
    );

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

    virtual FReply OnMouseButtonDown(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

    virtual FReply OnDragDetected(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

private:
    TSharedPtr<FMIForgeTextureSet> TextureSets;
    TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	FMIForgeOnTextureSetCheckStateChanged OnTextureSetCheckStateChanged;
};
