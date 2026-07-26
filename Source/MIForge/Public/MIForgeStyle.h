// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FMIForgeStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static FName GetStyleSetName();
	static const ISlateStyle& Get();

private:
	static TSharedRef<FSlateStyleSet> CreateStyleSet();
	static TSharedPtr<FSlateStyleSet> StyleSetInstance;
};
