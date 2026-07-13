// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"

DECLARE_DELEGATE_OneParam(FMIForgeOnParentMaterialGroupChosen, const FMIForgeMaterialParentGroup&)  // Delegate for when a parent material group is chosen

class SMIForgeParentMtlGroupPicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeParentMtlGroupPicker) {}
		SLATE_ARGUMENT(TArray<FMIForgeMaterialParentGroup>, ParentGroups)
		SLATE_EVENT(FMIForgeOnParentMaterialGroupChosen, OnGroupChosen)
		SLATE_ARGUMENT(TWeakPtr<SWindow>, OwnerWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FMIForgeMaterialParentGroup>> GroupItems;
	TSharedPtr<FMIForgeMaterialParentGroup> SelectedGroup;

	FMIForgeOnParentMaterialGroupChosen OnGroupChosen;
	TWeakPtr<SWindow> OwnerWindow;

	TSharedRef<ITableRow> GenerateGroupRow(
		TSharedPtr<FMIForgeMaterialParentGroup> Item,
		const TSharedRef<STableViewBase>& OwnerTable
	);

	FReply OnContinueClicked();
	FReply OnCancelClicked();

	bool CanContinue() const;
	
};
