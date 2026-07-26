// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SBorder.h"
#include "MIForgeTypes.h"

DECLARE_DELEGATE_RetVal_TwoParams(FReply, FMIForgeOnTextureSetDropped, const TSharedPtr<FMIForgeTextureSet>&, const EMIForgeVertexPaintLayer&)

class MIFORGE_API SMIForgeTextureSetDropTarget : public SBorder
{
	SLATE_BEGIN_ARGS(SMIForgeTextureSetDropTarget) {}
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_ARGUMENT(EMIForgeVertexPaintLayer, Layer)
		SLATE_ARGUMENT(const FSlateBrush*, BorderImage)
		SLATE_ARGUMENT(FMargin, Padding)
		SLATE_EVENT(FMIForgeOnTextureSetDropped, OnTextureSetDropped)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual FReply OnDrop(
		const FGeometry& MyGeometry,
		const FDragDropEvent& DragDropEvent
	) override;

	virtual void OnDragEnter(
		const FGeometry& MyGeometry,
		const FDragDropEvent& DragDropEvent
	) override;

	virtual void OnDragLeave(
		const FDragDropEvent& DragDropEvent
	) override;

	virtual FReply OnDragOver(
		const FGeometry& MyGeometry,
		const FDragDropEvent& DragDropEvent
	) override;

private:
	EMIForgeVertexPaintLayer Layer;
	FMIForgeOnTextureSetDropped OnTextureSetDropped;

	bool bIsDragHovered = false;
};
