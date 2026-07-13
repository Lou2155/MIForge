// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeBatchAdjustMIParams/MIForgeBatchParamModelBuilder.h"

#include "Materials/MaterialInstanceConstant.h"
#include "MaterialEditingLibrary.h"


FMIForgeBatchParameterModel FMIForgeBatchParamModelBuilder::BuildFromParentGroup(const FMIForgeMaterialParentGroup& ParentGroup)
{
	FMIForgeBatchParameterModel Model;
	Model.ParentGroup = ParentGroup;

	UMaterialInterface* ParentMaterial = ParentGroup.RootParentMaterial.Get();
	if (!ParentMaterial)
	{
		return Model;
	}

	const TArray<UMaterialInstanceConstant*> MaterialInstances =
		GetUniqueMaterialInstances(ParentGroup);

	if (MaterialInstances.Num() == 0)
	{
		return Model;
	}

	UMaterialInstanceConstant* SampleMIC = MaterialInstances[0];

	AddScalarParameters(ParentMaterial, SampleMIC, MaterialInstances, Model);
	AddVectorParameters(ParentMaterial, SampleMIC, MaterialInstances, Model);
	AddTextureParameters(ParentMaterial, SampleMIC, MaterialInstances, Model);
	AddStaticSwitchParameters(ParentMaterial, SampleMIC, MaterialInstances, Model);


	return Model;
}

void FMIForgeBatchParamModelBuilder::AddScalarParameters(UMaterialInterface* ParentMaterial, UMaterialInstanceConstant* SampleMI, const TArray<UMaterialInstanceConstant*>& MaterialInstances, FMIForgeBatchParameterModel& OutModel)
{
	TArray<FName> ScalarParameterNames;
	UMaterialEditingLibrary::GetScalarParameterNames(ParentMaterial, ScalarParameterNames);

	for (const FName& pn : ScalarParameterNames)
	{
		FMIForgeBatchParameterRow Row;
		Row.Type = EMIForgeBatchParameterType::Scalar;
		Row.ParameterName = pn;
		
		ApplyParameterMetadata(ParentMaterial, Row);

		Row.ScalarValue =
			UMaterialEditingLibrary::GetMaterialInstanceScalarParameterValue(
				SampleMI,
				pn
			);


		Row.bHasMixedValue =
			IsScalarParameterMixed(MaterialInstances, pn);

		OutModel.Parameters.Add(Row);
		OutModel.ScalarCount++;
		
	}
}

void FMIForgeBatchParamModelBuilder::AddVectorParameters(UMaterialInterface* ParentMaterial, UMaterialInstanceConstant* SampleMI, const TArray<UMaterialInstanceConstant*>& MaterialInstances, FMIForgeBatchParameterModel& OutModel)
{
	TArray<FName> VectorParameterNames;
	UMaterialEditingLibrary::GetVectorParameterNames(ParentMaterial, VectorParameterNames);

	for (const FName& pn : VectorParameterNames)
	{
		FMIForgeBatchParameterRow Row;
		Row.Type = EMIForgeBatchParameterType::Vector;
		Row.ParameterName = pn;
		
		ApplyParameterMetadata(ParentMaterial, Row);

		Row.VectorValue = UMaterialEditingLibrary::GetMaterialInstanceVectorParameterValue(SampleMI, pn);
		
		Row.bHasMixedValue =
			IsVectorParameterMixed(MaterialInstances, pn);

		OutModel.Parameters.Add(Row);
		OutModel.VectorCount++;
		
	}
}

void FMIForgeBatchParamModelBuilder::AddTextureParameters(UMaterialInterface* ParentMaterial, UMaterialInstanceConstant* SampleMI, const TArray<UMaterialInstanceConstant*>& MaterialInstances, FMIForgeBatchParameterModel& OutModel)
{
	TArray<FName> TextureParameterNames;
	UMaterialEditingLibrary::GetTextureParameterNames(ParentMaterial, TextureParameterNames);
	for (const FName& pn : TextureParameterNames)
	{
		FMIForgeBatchParameterRow Row;
		Row.Type = EMIForgeBatchParameterType::Texture;
		Row.ParameterName = pn;

		ApplyParameterMetadata(ParentMaterial, Row);
		
		Row.TextureValue = UMaterialEditingLibrary::GetMaterialInstanceTextureParameterValue(SampleMI, pn);
		
		Row.bHasMixedValue =
			IsTextureParameterMixed(MaterialInstances, pn);

		OutModel.Parameters.Add(Row);
		OutModel.TextureCount++;
		
	}
}

void FMIForgeBatchParamModelBuilder::AddStaticSwitchParameters(UMaterialInterface* ParentMaterial, UMaterialInstanceConstant* SampleMI, const TArray<UMaterialInstanceConstant*>& MaterialInstances, FMIForgeBatchParameterModel& OutModel)
{
	TArray<FName> StaticSwitchParameterNames;
	UMaterialEditingLibrary::GetStaticSwitchParameterNames(ParentMaterial, StaticSwitchParameterNames);
	for (const FName& pn : StaticSwitchParameterNames)
	{
		FMIForgeBatchParameterRow Row;
		Row.Type = EMIForgeBatchParameterType::StaticSwitch;
		Row.ParameterName = pn;

		ApplyParameterMetadata(ParentMaterial, Row);
		
		Row.BoolValue = UMaterialEditingLibrary::GetMaterialInstanceStaticSwitchParameterValue(SampleMI, pn);
		
		Row.bHasMixedValue = IsStaticSwitchParameterMixed(MaterialInstances, pn);

		OutModel.Parameters.Add(Row);
		OutModel.StaticSwitchCount++;
		
	}
}

TArray<UMaterialInstanceConstant*> FMIForgeBatchParamModelBuilder::GetUniqueMaterialInstances(const FMIForgeMaterialParentGroup& ParentGroup)
{
	TArray<UMaterialInstanceConstant*> Result;
	TSet<UMaterialInstanceConstant*> SeenMIs;

	for (const FMIForgeMaterialSlotTarget& Target : ParentGroup.Targets)
	{
		UMaterialInstanceConstant* MIC = Target.MaterialInstance.Get();

		if (!MIC || SeenMIs.Contains(MIC))
		{
			continue;
		}

		SeenMIs.Add(MIC);
		Result.Add(MIC);
	}

	return Result;
}

bool FMIForgeBatchParamModelBuilder::IsScalarParameterMixed(const TArray<UMaterialInstanceConstant*>& MaterialInstances, FName ParameterName)
{
	if (MaterialInstances.Num() <= 1)
	{
		return false;
	}

	const float FirstValue =
		UMaterialEditingLibrary::GetMaterialInstanceScalarParameterValue(
			MaterialInstances[0],
			ParameterName
		);
	for (size_t i = 1; i < MaterialInstances.Num(); ++i)
	{
		const float OtherValue =
			UMaterialEditingLibrary::GetMaterialInstanceScalarParameterValue(
				MaterialInstances[i],
				ParameterName
			);
		if(!FMath::IsNearlyEqual(FirstValue, OtherValue))
		{
			return true;
		}
	}
	return false;
}

bool FMIForgeBatchParamModelBuilder::IsVectorParameterMixed(const TArray<UMaterialInstanceConstant*>& MaterialInstances, FName ParameterName)
{
	if (MaterialInstances.Num() <= 1)
	{
		return false;
	}

	const FLinearColor FirstValue =
		UMaterialEditingLibrary::GetMaterialInstanceVectorParameterValue(
			MaterialInstances[0],
			ParameterName
		);

	for (int32 i = 1; i < MaterialInstances.Num(); ++i)
	{
		const FLinearColor OtherValue =
			UMaterialEditingLibrary::GetMaterialInstanceVectorParameterValue(
				MaterialInstances[i],
				ParameterName
			);

		if (!FirstValue.Equals(OtherValue))
		{
			return true;
		}
	}

	return false;
}

bool FMIForgeBatchParamModelBuilder::IsTextureParameterMixed(const TArray<UMaterialInstanceConstant*>& MaterialInstances, FName ParameterName)
{
	if (MaterialInstances.Num() <= 1)
	{
		return false;
	}

	UTexture* FirstValue =
		UMaterialEditingLibrary::GetMaterialInstanceTextureParameterValue(
			MaterialInstances[0],
			ParameterName
		);

	for (int32 i = 1; i < MaterialInstances.Num(); ++i)
	{
		UTexture* OtherValue =
			UMaterialEditingLibrary::GetMaterialInstanceTextureParameterValue(
				MaterialInstances[i],
				ParameterName
			);

		if (FirstValue != OtherValue)
		{
			return true;
		}
	}

	return false;
}

bool FMIForgeBatchParamModelBuilder::IsStaticSwitchParameterMixed(const TArray<UMaterialInstanceConstant*>& MaterialInstances, FName ParameterName)
{
	if (MaterialInstances.Num() <= 1)
	{
		return false;
	}

	const bool bFirstValue =
		UMaterialEditingLibrary::GetMaterialInstanceStaticSwitchParameterValue(
			MaterialInstances[0],
			ParameterName
		);

	for (int32 i = 1; i < MaterialInstances.Num(); ++i)
	{
		const bool bOtherValue =
			UMaterialEditingLibrary::GetMaterialInstanceStaticSwitchParameterValue(
				MaterialInstances[i],
				ParameterName
			);

		if (bFirstValue != bOtherValue)
		{
			return true;
		}
	}

	return false;
}

void FMIForgeBatchParamModelBuilder::ApplyParameterMetadata(UMaterialInterface* ParentMaterial, FMIForgeBatchParameterRow& Row)
{
	if (!ParentMaterial)
	{
		return;
	}

	const FHashedMaterialParameterInfo ParameterInfo(Row.ParameterName, EMaterialParameterAssociation::GlobalParameter);

	FName GroupName;
	if(ParentMaterial->GetGroupName(ParameterInfo, GroupName) && !GroupName.IsNone())
	{
		Row.GroupName = GroupName;
	}

	int32 SortPriority = 0;
	if(ParentMaterial->GetParameterSortPriority(ParameterInfo, SortPriority))
	{
		Row.SortPriority = SortPriority;
	}
}
