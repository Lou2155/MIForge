// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FMIForgeMainTabViewModel;

class SMIForgeTargetFolderSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMIForgeTargetFolderSection) {}
		SLATE_ARGUMENT(TSharedPtr<FMIForgeMainTabViewModel>, ViewModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FMIForgeMainTabViewModel> ViewModel;
};
