// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

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
