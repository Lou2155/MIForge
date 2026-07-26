// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"

class PopupWindowCreator
{
public:

	static void OpenPopupWindow(
		const FText& Title,
		const TSharedRef<SWidget>& Content,
		const FVector2D& Size = FVector2D(520.f, 420.f),
		bool bModal = true
	);

	static void OpenTextInputPopup(
		const FText& Title,
		const FText& Label,
		const FString& InitialValue,
		TFunction<void(const FString&)> OnConfirmed,
		const FVector2D& Size = FVector2D(420.f, 160.f)
	);
	
};
