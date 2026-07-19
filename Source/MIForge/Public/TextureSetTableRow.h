// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


struct FMIForgeTextureSet;
class SMainTabWidget;
class FMIForgeMainTabViewModel;


class STextureSetTableRow : public SMultiColumnTableRow<TSharedPtr<FMIForgeTextureSet>>
{
public:
    SLATE_BEGIN_ARGS(STextureSetTableRow) {}
        SLATE_ARGUMENT(TSharedPtr<FMIForgeTextureSet>, TextureSets)
        SLATE_ARGUMENT(TSharedPtr<SMainTabWidget>, ParentTable)
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
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
    TSharedPtr<SMainTabWidget> ParentTable;
    TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
};
