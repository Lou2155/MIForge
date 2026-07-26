// Fill out your copyright notice in the Description page of Project Settings.

#include "UIs/MIForgeMainTabViewModel.h"

#include "MIForgeTextureSetBuilder.h"
#include "MIForgeValidator.h"
#include "Presets/MIForgePresetDefinitions.h"

FMIForgeMainTabViewModel::FMIForgeMainTabViewModel()
{
	InitializeVertexPaintLayerStack();
	RefreshValidation();
}

EMIForgeGenerationPreset FMIForgeMainTabViewModel::GetPreset() const
{
	return CurrentPreset;
}

void FMIForgeMainTabViewModel::SetPreset(EMIForgeGenerationPreset NewPreset)
{
	if (CurrentPreset == NewPreset)
	{
		return;
	}

	CurrentPreset = NewPreset;
	RefreshValidation();
	OnPresetChanged.Broadcast(CurrentPreset);
}

FText FMIForgeMainTabViewModel::GetPresetDisplayText() const
{
	return GetPresetDisplayText(CurrentPreset);
}

FText FMIForgeMainTabViewModel::GetPresetDisplayText(EMIForgeGenerationPreset Preset)
{
	switch (Preset)
	{
	case EMIForgeGenerationPreset::Standard:
		return FText::FromString(TEXT("Standard"));
	case EMIForgeGenerationPreset::RGBMask:
		return FText::FromString(TEXT("RGB Masking"));
	case EMIForgeGenerationPreset::VertexPainting:
		return FText::FromString(TEXT("Vertex Painting"));
	case EMIForgeGenerationPreset::Decal:
		return FText::FromString(TEXT("Decal"));
	}

	return FText::FromString(TEXT("Unknown"));
}

const FString& FMIForgeMainTabViewModel::GetTargetPath() const
{
	return TargetPath;
}

void FMIForgeMainTabViewModel::SetTargetPath(const FString& NewPath)
{
	TargetPath = NewPath;
}

EIfMIExistsOption FMIForgeMainTabViewModel::GetIfMIExists() const
{
	return IfMIExists;
}

void FMIForgeMainTabViewModel::SetIfMIExists(EIfMIExistsOption NewOption)
{
	IfMIExists = NewOption;
}

bool FMIForgeMainTabViewModel::GetUseEmissiveTextures() const { return bUseEmissiveTextures; }
bool FMIForgeMainTabViewModel::GetUseDetailNormalTextures() const { return bUseDetailNormalTextures; }
bool FMIForgeMainTabViewModel::GetUseTriplanarProjection() const { return bUseTriplanarProjection; }
bool FMIForgeMainTabViewModel::GetUseBaseORMTexture() const { return bUseBaseORMTexture; }
bool FMIForgeMainTabViewModel::GetEnableEmissiveChannel() const { return bEnableEmissiveChannel; }
bool FMIForgeMainTabViewModel::GetUseDetailNormalTextureRGB() const { return bUseDetailNormalTextureRGB; }
bool FMIForgeMainTabViewModel::GetIgnoreUnrecognizedTextures() const { return bIgnoreUnrecognizedTextures; }
bool FMIForgeMainTabViewModel::GetUseDecalNormal() const { return bUseDecalNormal; }
bool FMIForgeMainTabViewModel::GetUseDecalORM() const { return bUseDecalORM; }
bool FMIForgeMainTabViewModel::GetUseOrientationMask() const { return bUseOrientationMask; }

void FMIForgeMainTabViewModel::SetUseEmissiveTextures(bool bEnabled)
{
	if (bUseEmissiveTextures == bEnabled) return;
	bUseEmissiveTextures = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseDetailNormalTextures(bool bEnabled)
{
	if (bUseDetailNormalTextures == bEnabled) return;
	bUseDetailNormalTextures = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseTriplanarProjection(bool bEnabled)
{
	if (bUseTriplanarProjection == bEnabled) return;
	bUseTriplanarProjection = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseBaseORMTexture(bool bEnabled)
{
	if (bUseBaseORMTexture == bEnabled) return;
	bUseBaseORMTexture = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetEnableEmissiveChannel(bool bEnabled)
{
	if (bEnableEmissiveChannel == bEnabled) return;
	bEnableEmissiveChannel = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseDetailNormalTextureRGB(bool bEnabled)
{
	if (bUseDetailNormalTextureRGB == bEnabled) return;
	bUseDetailNormalTextureRGB = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseDecalNormal(bool bEnabled)
{
	if (bUseDecalNormal == bEnabled) return;
	bUseDecalNormal = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseDecalORM(bool bEnabled)
{
	if (bUseDecalORM == bEnabled) return;
	bUseDecalORM = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetUseOrientationMask(bool bEnabled)
{
	if (bUseOrientationMask == bEnabled) return;
	bUseOrientationMask = bEnabled;
	NotifyGenerationOptionsChanged();
}

void FMIForgeMainTabViewModel::SetIgnoreUnrecognizedTextures(bool bEnabled)
{
	if (bIgnoreUnrecognizedTextures == bEnabled) return;
	bIgnoreUnrecognizedTextures = bEnabled;
	NotifyGenerationOptionsChanged();
}

EMIForgeInputMode FMIForgeMainTabViewModel::GetInputMode() const
{
	return CurrentInputMode;
}

void FMIForgeMainTabViewModel::SetInputMode(EMIForgeInputMode NewMode)
{
	if (CurrentInputMode == NewMode)
	{
		return;
	}

	CurrentInputMode = NewMode;
	if (CurrentInputMode == EMIForgeInputMode::IndividualTextures)
	{
		SelectedTextureSetItems.Empty();
	}
	else
	{
		SelectedTextureItems.Empty();
	}

	NotifySelectionChanged();
	OnInputModeChanged.Broadcast(CurrentInputMode);
}

const TArray<TSharedPtr<FMIForgeTextureInfo>>& FMIForgeMainTabViewModel::GetSelectedTextures() const
{
	return SelectedTextureItems;
}

const TArray<TSharedPtr<FMIForgeTextureSet>>& FMIForgeMainTabViewModel::GetSelectedTextureSets() const
{
	return SelectedTextureSetItems;
}

void FMIForgeMainTabViewModel::SelectTexture(const TSharedPtr<FMIForgeTextureInfo>& Item)
{
	if (!Item.IsValid() || SelectedTextureItems.Contains(Item)) return;
	SelectedTextureItems.Add(Item);
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::UnselectTexture(const TSharedPtr<FMIForgeTextureInfo>& Item)
{
	if (SelectedTextureItems.Remove(Item) == 0) return;
	NotifySelectionChanged();
}

bool FMIForgeMainTabViewModel::IsTextureSelected(const TSharedPtr<FMIForgeTextureInfo>& Item) const
{
	return SelectedTextureItems.Contains(Item);
}

void FMIForgeMainTabViewModel::SelectTextureSet(const TSharedPtr<FMIForgeTextureSet>& Item)
{
	if (!Item.IsValid() || SelectedTextureSetItems.Contains(Item)) return;
	SelectedTextureSetItems.Add(Item);
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::UnselectTextureSet(const TSharedPtr<FMIForgeTextureSet>& Item)
{
	if (SelectedTextureSetItems.Remove(Item) == 0) return;
	NotifySelectionChanged();
}

bool FMIForgeMainTabViewModel::IsTextureSetSelected(const TSharedPtr<FMIForgeTextureSet>& Item) const
{
	return SelectedTextureSetItems.Contains(Item);
}

void FMIForgeMainTabViewModel::SetSelectedTextures(const TArray<TSharedPtr<FMIForgeTextureInfo>>& Items)
{
	TArray<TSharedPtr<FMIForgeTextureInfo>> NewItems;
	for (const TSharedPtr<FMIForgeTextureInfo>& Item : Items)
	{
		if (Item.IsValid()) NewItems.AddUnique(Item);
	}
	if (SelectedTextureItems == NewItems) return;
	SelectedTextureItems = MoveTemp(NewItems);
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::SetSelectedTextureSets(const TArray<TSharedPtr<FMIForgeTextureSet>>& Items)
{
	TArray<TSharedPtr<FMIForgeTextureSet>> NewItems;
	for (const TSharedPtr<FMIForgeTextureSet>& Item : Items)
	{
		if (Item.IsValid()) NewItems.AddUnique(Item);
	}
	if (SelectedTextureSetItems == NewItems) return;
	SelectedTextureSetItems = MoveTemp(NewItems);
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::ClearTextureSelection()
{
	if (SelectedTextureItems.IsEmpty()) return;
	SelectedTextureItems.Empty();
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::ClearTextureSetSelection()
{
	if (SelectedTextureSetItems.IsEmpty()) return;
	SelectedTextureSetItems.Empty();
	NotifySelectionChanged();
}

void FMIForgeMainTabViewModel::ClearAllSelections()
{
	if (SelectedTextureItems.IsEmpty() && SelectedTextureSetItems.IsEmpty()) return;
	SelectedTextureItems.Empty();
	SelectedTextureSetItems.Empty();
	NotifySelectionChanged();
}

TArray<TSharedPtr<FMIForgeTextureSet>> FMIForgeMainTabViewModel::BuildGenerationTextureSets() const
{
	if (CurrentInputMode == EMIForgeInputMode::TextureSets)
	{
		return SelectedTextureSetItems;
	}

	TArray<FMIForgeTextureInfo> RawTextures;
	for (const TSharedPtr<FMIForgeTextureInfo>& Texture : SelectedTextureItems)
	{
		if (Texture.IsValid()) RawTextures.Add(*Texture);
	}

	TArray<FMIForgeTextureSet> BuiltSets = FMIForgeTextureSetBuilder().BuildTextureSets(RawTextures);
	TArray<TSharedPtr<FMIForgeTextureSet>> Result;
	Result.Reserve(BuiltSets.Num());
	for (FMIForgeTextureSet& Set : BuiltSets)
	{
		Result.Add(MakeShared<FMIForgeTextureSet>(MoveTemp(Set)));
	}
	return Result;
}

const FMIForgeValidationSummary& FMIForgeMainTabViewModel::GetValidationSummary() const
{
	return CurrentValidationSummary;
}

const FMIForgeVertexPaintLayerStackValidationResult& FMIForgeMainTabViewModel::GetVertexPaintValidationResult() const
{
	return CurrentVertexPaintValidationResult;
}

const FMIForgeVertexPaintValidationSummary& FMIForgeMainTabViewModel::GetVertexPaintValidationSummary() const
{
	return CurrentVertexPaintValidationSummary;
}

void FMIForgeMainTabViewModel::RefreshValidation()
{
	FMIForgeValidator Validator;
	CurrentValidationSummary = FMIForgeValidationSummary();
	CurrentVertexPaintValidationResult = FMIForgeVertexPaintLayerStackValidationResult();
	CurrentVertexPaintValidationSummary = FMIForgeVertexPaintValidationSummary();

	if (CurrentPreset == EMIForgeGenerationPreset::Standard)
	{
		CurrentValidationSummary = CurrentInputMode == EMIForgeInputMode::TextureSets
			? Validator.BuildStandardSummaryFromTextureSets(BuildGenerationTextureSets(), bUseEmissiveTextures, bUseDetailNormalTextures, bIgnoreUnrecognizedTextures)
			: Validator.BuildStandardSummaryFromTextures(SelectedTextureItems, bUseEmissiveTextures, bUseDetailNormalTextures, bIgnoreUnrecognizedTextures);
	}
	else if (CurrentPreset == EMIForgeGenerationPreset::RGBMask)
	{
		CurrentValidationSummary = CurrentInputMode == EMIForgeInputMode::TextureSets
			? Validator.BuildRGBSummaryFromTextureSets(BuildGenerationTextureSets(), bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures)
			: Validator.BuildRGBSummaryFromTextures(SelectedTextureItems, bUseBaseORMTexture, bEnableEmissiveChannel, bUseDetailNormalTextureRGB, bIgnoreUnrecognizedTextures);
	}
	else if (CurrentPreset == EMIForgeGenerationPreset::Decal)
	{
		CurrentValidationSummary = CurrentInputMode == EMIForgeInputMode::TextureSets
			? Validator.BuildDecalSummaryFromTextureSets(BuildGenerationTextureSets(), bUseDecalNormal, bUseDecalORM, bIgnoreUnrecognizedTextures)
			: Validator.BuildDecalSummaryFromTextures(SelectedTextureItems, bUseDecalNormal, bUseDecalORM, bUseOrientationMask, bIgnoreUnrecognizedTextures);
	}
	else
	{
		CurrentVertexPaintValidationResult = Validator.ValidateVertexPaintLayerStack(VertexPaintLayerStack, bIgnoreUnrecognizedTextures);
		CurrentVertexPaintValidationSummary = Validator.BuildVertexPaintLayerStackSummary(CurrentVertexPaintValidationResult);
	}

	OnValidationChanged.Broadcast();
}

const FMIForgeVertexPaintLayerStack& FMIForgeMainTabViewModel::GetVertexPaintLayerStack() const
{
	return VertexPaintLayerStack;
}

const FMIForgeVertexPaintLayerSlot* FMIForgeMainTabViewModel::FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer) const
{
	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base: return &VertexPaintLayerStack.BaseLayer;
	case EMIForgeVertexPaintLayer::LayerR: return &VertexPaintLayerStack.LayerR;
	case EMIForgeVertexPaintLayer::LayerG: return &VertexPaintLayerStack.LayerG;
	case EMIForgeVertexPaintLayer::LayerB: return &VertexPaintLayerStack.LayerB;
	}
	return nullptr;
}

bool FMIForgeMainTabViewModel::AssignTextureSetToVertexLayer(
	EMIForgeVertexPaintLayer Layer,
	const TSharedPtr<FMIForgeTextureSet>& TextureSet)
{
	FMIForgeVertexPaintLayerSlot* Slot = FindMutableVertexPaintLayerSlot(Layer);
	if (!Slot || !TextureSet.IsValid()) return false;
	if (Slot->AssignedTextureSet == TextureSet) return true;
	Slot->AssignedTextureSet = TextureSet;
	NotifyVertexPaintChanged();
	return true;
}

bool FMIForgeMainTabViewModel::AssignSelectedTextureSetsToVertexLayers(FText& OutError)
{
	OutError = FText::GetEmpty();
	if (SelectedTextureSetItems.IsEmpty())
	{
		OutError = FText::FromString(TEXT("Please select at least one texture set to assign."));
		return false;
	}
	if (SelectedTextureSetItems.Num() > 4)
	{
		OutError = FText::FromString(TEXT("Please select no more than 4 texture sets to assign."));
		return false;
	}

	VertexPaintLayerStack.BaseLayer.AssignedTextureSet.Reset();
	VertexPaintLayerStack.LayerR.AssignedTextureSet.Reset();
	VertexPaintLayerStack.LayerG.AssignedTextureSet.Reset();
	VertexPaintLayerStack.LayerB.AssignedTextureSet.Reset();
	VertexPaintLayerStack.BaseLayer.AssignedTextureSet = SelectedTextureSetItems[0];
	if (SelectedTextureSetItems.Num() > 1) VertexPaintLayerStack.LayerR.AssignedTextureSet = SelectedTextureSetItems[1];
	if (SelectedTextureSetItems.Num() > 2) VertexPaintLayerStack.LayerG.AssignedTextureSet = SelectedTextureSetItems[2];
	if (SelectedTextureSetItems.Num() > 3) VertexPaintLayerStack.LayerB.AssignedTextureSet = SelectedTextureSetItems[3];
	NotifyVertexPaintChanged();
	return true;
}

void FMIForgeMainTabViewModel::ClearVertexPaintLayer(EMIForgeVertexPaintLayer Layer)
{
	FMIForgeVertexPaintLayerSlot* Slot = FindMutableVertexPaintLayerSlot(Layer);
	if (!Slot || !Slot->AssignedTextureSet.IsValid()) return;
	Slot->AssignedTextureSet.Reset();
	NotifyVertexPaintChanged();
}

void FMIForgeMainTabViewModel::ClearAllVertexPaintLayers()
{
	bool bChanged = false;
	for (FMIForgeVertexPaintLayerSlot* Slot : VertexPaintLayerStack.GetSlots())
	{
		if (Slot && Slot->AssignedTextureSet.IsValid())
		{
			Slot->AssignedTextureSet.Reset();
			bChanged = true;
		}
	}
	if (bChanged) NotifyVertexPaintChanged();
}

void FMIForgeMainTabViewModel::SetVertexPaintLayerStack(const FMIForgeVertexPaintLayerStack& NewStack)
{
	VertexPaintLayerStack = NewStack;
	NotifyVertexPaintChanged();
}

void FMIForgeMainTabViewModel::NotifyGenerationOptionsChanged()
{
	RefreshValidation();
	OnGenerationOptionsChanged.Broadcast();
}

void FMIForgeMainTabViewModel::NotifySelectionChanged()
{
	RefreshValidation();
	OnSelectionChanged.Broadcast();
}

void FMIForgeMainTabViewModel::NotifyVertexPaintChanged()
{
	RefreshValidation();
	OnVertexPaintChanged.Broadcast();
}

void FMIForgeMainTabViewModel::InitializeVertexPaintLayerStack()
{
	const FMIForgeVertexPaintPresetDefinition& Definition = FMIForgePresetDefinitions::GetVertexPaint();
	for (const FMIForgeVertexPaintLayerDefinition& LayerDefinition : Definition.Layers)
	{
		FMIForgeVertexPaintLayerSlot* Slot = FindMutableVertexPaintLayerSlot(LayerDefinition.Layer);
		if (!Slot) continue;
		Slot->Layer = LayerDefinition.Layer;
		Slot->DisplayName = LayerDefinition.DisplayName;
		Slot->ChannelName = LayerDefinition.ChannelName;
		Slot->bRequired = LayerDefinition.bRequired;
	}
}

FMIForgeVertexPaintLayerSlot* FMIForgeMainTabViewModel::FindMutableVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer)
{
	switch (Layer)
	{
	case EMIForgeVertexPaintLayer::Base: return &VertexPaintLayerStack.BaseLayer;
	case EMIForgeVertexPaintLayer::LayerR: return &VertexPaintLayerStack.LayerR;
	case EMIForgeVertexPaintLayer::LayerG: return &VertexPaintLayerStack.LayerG;
	case EMIForgeVertexPaintLayer::LayerB: return &VertexPaintLayerStack.LayerB;
	}
	return nullptr;
}


