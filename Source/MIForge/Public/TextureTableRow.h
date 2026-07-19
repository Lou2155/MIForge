// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
struct FMIForgeTextureInfo;
class FMIForgeMainTabViewModel;
/**
 * 
 */
class STextureTableRow : public SMultiColumnTableRow<TSharedPtr<FMIForgeTextureInfo>>
{
public:
	SLATE_BEGIN_ARGS(STextureTableRow) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeTextureInfo>, TextureListItems)
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FMIForgeTextureInfo> TextureListItems;
	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
	
};
