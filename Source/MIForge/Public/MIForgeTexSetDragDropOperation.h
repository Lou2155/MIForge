// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "MIForgeTypes.h"

class FMIForgeTexSetDragDropOperation : public FDecoratedDragDropOp
{
public:

	DRAG_DROP_OPERATOR_TYPE(FMIForgeTexSetDragDropOperation, FDecoratedDragDropOp)

	TSharedPtr<FMIForgeTextureSet> TextureSet;
	
	
	static TSharedRef<FMIForgeTexSetDragDropOperation> New(TSharedPtr<FMIForgeTextureSet> InTextureSet)
	{
		TSharedRef<FMIForgeTexSetDragDropOperation> Operation = MakeShareable(new FMIForgeTexSetDragDropOperation());

		Operation->TextureSet = InTextureSet;

		Operation->CurrentHoverText = InTextureSet.IsValid()
			? FText::FromString(
				FString::Printf(
					TEXT("Assign texture set: %s"),
					*InTextureSet->SetName
				)
			)
			: FText::FromString(TEXT("Assign texture set"));

		Operation->CurrentIconBrush = nullptr;

		Operation->SetupDefaults();
		Operation->Construct();

		return Operation;
	}
};
