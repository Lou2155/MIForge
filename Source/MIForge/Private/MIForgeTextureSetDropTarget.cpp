// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

// MIForgeTextureSetDropTarget.cpp

#include "MIForgeTextureSetDropTarget.h"
#include "MIForgeTexSetDragDropOperation.h"

void SMIForgeTextureSetDropTarget::Construct(const FArguments& InArgs)
{
	Layer = InArgs._Layer;
	OnTextureSetDropped = InArgs._OnTextureSetDropped;

	SBorder::Construct(
		SBorder::FArguments()
		.BorderImage(InArgs._BorderImage)
		.BorderBackgroundColor_Lambda([this]()
			{
				return bIsDragHovered
					? FSlateColor(FLinearColor(0.08f, 0.32f, 0.85f, 0.55f))
					: FSlateColor(FLinearColor::Gray);
			})
		.Padding(InArgs._Padding)
		[
			InArgs._Content.Widget
		]
	);
}

FReply SMIForgeTextureSetDropTarget::OnDrop(  //this function is called when a drag and drop operation is dropped onto the layer drop target. It checks if the operation is valid and executes the OnTextureSetDropped delegate if it is.
	const FGeometry& MyGeometry,
	const FDragDropEvent& DragDropEvent
)
{	
	bIsDragHovered = false;

	TSharedPtr<FMIForgeTexSetDragDropOperation> DragOp =
		DragDropEvent.GetOperationAs<FMIForgeTexSetDragDropOperation>();

	if (!DragOp.IsValid() || !DragOp->TextureSet.IsValid())
	{
		return FReply::Unhandled();
	}

	if (!OnTextureSetDropped.IsBound())
	{
		return FReply::Unhandled();
	}

	return OnTextureSetDropped.Execute(DragOp->TextureSet, Layer);
}

void SMIForgeTextureSetDropTarget::OnDragEnter(
	const FGeometry& MyGeometry,
	const FDragDropEvent& DragDropEvent
)
{
	TSharedPtr<FMIForgeTexSetDragDropOperation> DragOp =
		DragDropEvent.GetOperationAs<FMIForgeTexSetDragDropOperation>();

	bIsDragHovered = DragOp.IsValid() && DragOp->TextureSet.IsValid();
}

void SMIForgeTextureSetDropTarget::OnDragLeave(
	const FDragDropEvent& DragDropEvent
)
{
	bIsDragHovered = false;
}

FReply SMIForgeTextureSetDropTarget::OnDragOver(
	const FGeometry& MyGeometry,
	const FDragDropEvent& DragDropEvent
)
{
	TSharedPtr<FMIForgeTexSetDragDropOperation> DragOp =
		DragDropEvent.GetOperationAs<FMIForgeTexSetDragDropOperation>();

	if (DragOp.IsValid() && DragOp->TextureSet.IsValid())
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

