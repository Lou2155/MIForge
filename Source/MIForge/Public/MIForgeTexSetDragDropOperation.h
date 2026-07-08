// Fill out your copyright notice in the Description page of Project Settings.

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
