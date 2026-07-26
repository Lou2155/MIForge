// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UTexture;

struct FMIForgeMaterialSlotTarget
{
	TWeakObjectPtr<AActor> Actor;
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	int32 SlotIndex = INDEX_NONE;

	TWeakObjectPtr<UMaterialInterface> CurrentMaterial;
	TWeakObjectPtr<UMaterialInstanceConstant> MaterialInstance;
	TWeakObjectPtr<UMaterialInterface> RootParentMaterial;
};


struct FMIForgeMaterialParentGroup
{
	TWeakObjectPtr<UMaterialInterface> RootParentMaterial;

	TArray<FMIForgeMaterialSlotTarget> Targets;

	int32 ActorCount = 0;
	int32 SlotCount = 0;
	int32 UniqueMICCount = 0;

	UMaterialInstanceConstant* GetFirstValidMaterialInstance() const
	{
		for (const FMIForgeMaterialSlotTarget& Target : Targets)
		{
			if (UMaterialInstanceConstant* MIC = Target.MaterialInstance.Get())
			{
				return MIC;
			}
		}
		return nullptr;
	}
};

struct FMIForgeMaterialSlotCollectionResult
{
	TArray<FMIForgeMaterialSlotTarget> Targets;

	TArray<FMIForgeMaterialParentGroup> ParentGroups;

	int32 SelectedActorCount = 0;
	int32 MeshComponentCount = 0;
	int32 MaterialSlotCount = 0;
	int32 ValidMICSlotCount = 0;
	int32 SkippedSlotCount = 0;
	int32 UniqueMICCount = 0;
};

enum class EMIForgeBatchParameterType : uint8
{
	Scalar,
	Vector,
	Texture,
	StaticSwitch
};

struct FMIForgeBatchParameterRow
{
	EMIForgeBatchParameterType Type;
	FName ParameterName;

	FName GroupName = TEXT("Ungrouped");
	int32 SortPriority = 0;

	bool bApply = false;

	float ScalarValue = 0.0f;
	FLinearColor VectorValue = FLinearColor::White;
	TWeakObjectPtr<UTexture> TextureValue;
	bool BoolValue = false;

	bool bHasMixedValue = false;
};

struct FMIForgeBatchParameterModel
{
	FMIForgeMaterialParentGroup ParentGroup;

	TArray<FMIForgeBatchParameterRow> Parameters;

	int32 ScalarCount = 0;
	int32 VectorCount = 0;
	int32 TextureCount = 0;
	int32 StaticSwitchCount = 0;
};