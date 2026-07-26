// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"

enum class EMIForgeInputMode : uint8
{
	IndividualTextures,
	TextureSets
};

DECLARE_MULTICAST_DELEGATE_OneParam(
	FMIForgeOnPresetChanged,
	EMIForgeGenerationPreset)

DECLARE_MULTICAST_DELEGATE(
	FMIForgeOnGenerationOptionsChanged)

DECLARE_MULTICAST_DELEGATE_OneParam(
	FMIForgeOnInputModeChanged,
	EMIForgeInputMode)

DECLARE_MULTICAST_DELEGATE(
	FMIForgeOnSelectionChanged)

DECLARE_MULTICAST_DELEGATE(
	FMIForgeOnValidationChanged)

DECLARE_MULTICAST_DELEGATE(
	FMIForgeOnVertexPaintChanged)

class FMIForgeMainTabViewModel : public TSharedFromThis<FMIForgeMainTabViewModel>
{
public:
	FMIForgeMainTabViewModel();

	EMIForgeGenerationPreset GetPreset() const;
	void SetPreset(EMIForgeGenerationPreset NewPreset);

	FText GetPresetDisplayText() const;
	static FText GetPresetDisplayText(EMIForgeGenerationPreset Preset);

	const FString& GetTargetPath() const;
	void SetTargetPath(const FString& NewPath);

	EIfMIExistsOption GetIfMIExists() const;
	void SetIfMIExists(EIfMIExistsOption NewOption);

	//standard
	bool GetUseEmissiveTextures() const;
	void SetUseEmissiveTextures(bool bEnabled);

	bool GetUseDetailNormalTextures() const;
	void SetUseDetailNormalTextures(bool bEnabled);

	bool GetUseTriplanarProjection() const;
	void SetUseTriplanarProjection(bool bEnabled);

	//RGB Mask
	bool GetUseBaseORMTexture() const;
	void SetUseBaseORMTexture(bool bEnabled);

	bool GetEnableEmissiveChannel() const;
	void SetEnableEmissiveChannel(bool bEnabled);

	bool GetUseDetailNormalTextureRGB() const;
	void SetUseDetailNormalTextureRGB(bool bEnabled);

	//Decal
	bool GetUseDecalNormal() const;
	void SetUseDecalNormal(bool bEnabled);

	bool GetUseDecalORM() const;
	void SetUseDecalORM(bool bEnabled);

	bool GetUseOrientationMask() const;
	void SetUseOrientationMask(bool bEnabled);

	bool GetIgnoreUnrecognizedTextures() const;
	void SetIgnoreUnrecognizedTextures(bool bEnabled);

	EMIForgeInputMode GetInputMode() const;
	void SetInputMode(EMIForgeInputMode NewMode);

	const TArray<TSharedPtr<FMIForgeTextureInfo>>& GetSelectedTextures() const;
	const TArray<TSharedPtr<FMIForgeTextureSet>>& GetSelectedTextureSets() const;

	void SelectTexture(const TSharedPtr<FMIForgeTextureInfo>& Item);
	void UnselectTexture(const TSharedPtr<FMIForgeTextureInfo>& Item);
	bool IsTextureSelected(const TSharedPtr<FMIForgeTextureInfo>& Item) const;

	void SelectTextureSet(const TSharedPtr<FMIForgeTextureSet>& Item);
	void UnselectTextureSet(const TSharedPtr<FMIForgeTextureSet>& Item);
	bool IsTextureSetSelected(const TSharedPtr<FMIForgeTextureSet>& Item) const;

	void SetSelectedTextures(const TArray<TSharedPtr<FMIForgeTextureInfo>>& Items);
	void SetSelectedTextureSets(const TArray<TSharedPtr<FMIForgeTextureSet>>& Items);
	void ClearTextureSelection();
	void ClearTextureSetSelection();
	void ClearAllSelections();

	TArray<TSharedPtr<FMIForgeTextureSet>> BuildGenerationTextureSets() const;

	const FMIForgeValidationSummary& GetValidationSummary() const;
	const FMIForgeVertexPaintLayerStackValidationResult& GetVertexPaintValidationResult() const;
	const FMIForgeVertexPaintValidationSummary& GetVertexPaintValidationSummary() const;
	void RefreshValidation();

	const FMIForgeVertexPaintLayerStack& GetVertexPaintLayerStack() const;
	const FMIForgeVertexPaintLayerSlot* FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer) const;
	bool AssignTextureSetToVertexLayer(
		EMIForgeVertexPaintLayer Layer,
		const TSharedPtr<FMIForgeTextureSet>& TextureSet);
	bool AssignSelectedTextureSetsToVertexLayers(FText& OutError);
	void ClearVertexPaintLayer(EMIForgeVertexPaintLayer Layer);
	void ClearAllVertexPaintLayers();
	void SetVertexPaintLayerStack(const FMIForgeVertexPaintLayerStack& NewStack);

	FMIForgeOnPresetChanged OnPresetChanged;
	FMIForgeOnGenerationOptionsChanged OnGenerationOptionsChanged;
	FMIForgeOnInputModeChanged OnInputModeChanged;
	FMIForgeOnSelectionChanged OnSelectionChanged;
	FMIForgeOnValidationChanged OnValidationChanged;
	FMIForgeOnVertexPaintChanged OnVertexPaintChanged;

private:
	void NotifyGenerationOptionsChanged();
	void NotifySelectionChanged();
	void NotifyVertexPaintChanged();
	void InitializeVertexPaintLayerStack();
	FMIForgeVertexPaintLayerSlot* FindMutableVertexPaintLayerSlot(EMIForgeVertexPaintLayer Layer);

	EMIForgeGenerationPreset CurrentPreset = EMIForgeGenerationPreset::Standard;
	FString TargetPath;
	EIfMIExistsOption IfMIExists = EIfMIExistsOption::Skip;

	//Standard Generation Options
	bool bUseEmissiveTextures = false;
	bool bUseDetailNormalTextures = false;
	bool bUseTriplanarProjection = false;
	//RGB Mask Generation Options
	bool bUseBaseORMTexture = true;
	bool bEnableEmissiveChannel = false;
	bool bUseDetailNormalTextureRGB = false;

	//Decal Generation Options
	bool bUseDecalNormal = false;
	bool bUseDecalORM = false;
	bool bUseOrientationMask = false;

	bool bIgnoreUnrecognizedTextures = false;

	EMIForgeInputMode CurrentInputMode = EMIForgeInputMode::IndividualTextures;
	TArray<TSharedPtr<FMIForgeTextureInfo>> SelectedTextureItems;
	TArray<TSharedPtr<FMIForgeTextureSet>> SelectedTextureSetItems;

	FMIForgeValidationSummary CurrentValidationSummary;
	FMIForgeVertexPaintLayerStackValidationResult CurrentVertexPaintValidationResult;
	FMIForgeVertexPaintValidationSummary CurrentVertexPaintValidationSummary;
	FMIForgeVertexPaintLayerStack VertexPaintLayerStack;
};
