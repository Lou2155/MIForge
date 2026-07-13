// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeBatchAdjustMIParams/MIForgeBatchParameterTypes.h"

class FMIForgeBatchParamModelBuilder
{
public:
	static FMIForgeBatchParameterModel BuildFromParentGroup(const FMIForgeMaterialParentGroup& ParentGroup);

private:
	static void AddScalarParameters(
		UMaterialInterface* ParentMaterial,
		UMaterialInstanceConstant* SampleMI,
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FMIForgeBatchParameterModel& OutModel
	);
	static void AddVectorParameters(
		UMaterialInterface* ParentMaterial,
		UMaterialInstanceConstant* SampleMI,
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FMIForgeBatchParameterModel& OutModel
	);

	static void AddTextureParameters(
		UMaterialInterface* ParentMaterial,
		UMaterialInstanceConstant* SampleMI,
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FMIForgeBatchParameterModel& OutModel
	);

	static void AddStaticSwitchParameters(
		UMaterialInterface* ParentMaterial,
		UMaterialInstanceConstant* SampleMI,
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FMIForgeBatchParameterModel& OutModel
	);
	static TArray<UMaterialInstanceConstant*> GetUniqueMaterialInstances(
		const FMIForgeMaterialParentGroup& ParentGroup
	);

	static bool IsScalarParameterMixed(
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FName ParameterName
	);

	static bool IsVectorParameterMixed(
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FName ParameterName
	);

	static bool IsTextureParameterMixed(
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FName ParameterName
	);

	static bool IsStaticSwitchParameterMixed(
		const TArray<UMaterialInstanceConstant*>& MaterialInstances,
		FName ParameterName
	);

	static void ApplyParameterMetadata(
		UMaterialInterface* ParentMaterial,
		FMIForgeBatchParameterRow& Row
	);
};
