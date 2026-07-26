// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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
