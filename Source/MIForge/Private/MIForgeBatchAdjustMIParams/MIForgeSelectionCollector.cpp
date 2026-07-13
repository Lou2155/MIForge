// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeBatchAdjustMIParams/MIForgeSelectionCollector.h"
#include "Selection.h"
#include "Materials/MaterialInstanceConstant.h"

FMIForgeMaterialSlotCollectionResult MIForgeSelectionCollector::CollectFromCurrentEditorSelection()
{
	FMIForgeMaterialSlotCollectionResult Result;

	if (!GEditor)
	{
		return Result;
	}

	TSet<UMaterialInstanceConstant*> UniqueMICs;

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = Cast<AActor>(*It);
		if (!Actor)
		{
			continue;
		}

		Result.SelectedActorCount++;

		TArray<UMeshComponent*> MeshComponents;
		Actor->GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (!MeshComponent)
			{
				continue;
			}

			Result.MeshComponentCount++;

			const int32 NumMaterials = MeshComponent->GetNumMaterials();

			for (int32 SlotIndex = 0; SlotIndex < NumMaterials; ++SlotIndex)
			{
				Result.MaterialSlotCount++;

				UMaterialInterface* Material = MeshComponent->GetMaterial(SlotIndex);

				if (!Material)
				{
					Result.SkippedSlotCount++;
					continue;
				}

				UMaterialInstanceConstant* MIC =
					Cast<UMaterialInstanceConstant>(Material);

				if (!MIC)
				{
					Result.SkippedSlotCount++;
					continue;
				}

				UMaterialInterface* RootParentMaterial =
					ResolveRootParentMaterial(MIC);

				if (!RootParentMaterial)
				{
					Result.SkippedSlotCount++;
					continue;
				}

				FMIForgeMaterialSlotTarget Target;
				Target.Actor = Actor;
				Target.MeshComponent = MeshComponent;
				Target.SlotIndex = SlotIndex;
				Target.CurrentMaterial = Material;
				Target.MaterialInstance = MIC;
				Target.RootParentMaterial = RootParentMaterial;

				Result.Targets.Add(Target);
				Result.ValidMICSlotCount++;

				UniqueMICs.Add(MIC);
			}
		}
	}

	Result.UniqueMICCount = UniqueMICs.Num();

	TMap<UMaterialInterface*, int32> GroupIndexByRootParent;

	for (const FMIForgeMaterialSlotTarget& Target : Result.Targets)
	{
		UMaterialInterface* RootParent = Target.RootParentMaterial.Get();
		if (!RootParent)
		{
			continue;
		}
		
		int32* ExistingGroupIndex = GroupIndexByRootParent.Find(RootParent);

		if (!ExistingGroupIndex) //if we don't have a group for this root parent yet, make one. if we do, add the target to the existing group
		{
			FMIForgeMaterialParentGroup NewGroup;
			NewGroup.RootParentMaterial = RootParent;

			const int32 NewGroupIndex = Result.ParentGroups.Add(NewGroup);  // Add returns the index of the newly added element
			GroupIndexByRootParent.Add(RootParent, NewGroupIndex);

			ExistingGroupIndex = GroupIndexByRootParent.Find(RootParent);
		}

		FMIForgeMaterialParentGroup& Group = Result.ParentGroups[*ExistingGroupIndex];
		Group.Targets.Add(Target);
	}

	for (FMIForgeMaterialParentGroup& Group : Result.ParentGroups)
	{
		TSet<AActor*> UniqueActors;
		TSet<UMaterialInstanceConstant*> UniqueMIs;

		for (const FMIForgeMaterialSlotTarget& Target : Group.Targets)
		{
			if (AActor * Actor = Target.Actor.Get())
			{
				UniqueActors.Add(Actor);
			}
			if (UMaterialInstanceConstant* MIC = Target.MaterialInstance.Get())
			{
				UniqueMIs.Add(MIC);
			}
		}

		Group.ActorCount = UniqueActors.Num();
		Group.SlotCount = Group.Targets.Num();
		Group.UniqueMICCount = UniqueMIs.Num();

	}

	return Result;
}

UMaterialInterface* MIForgeSelectionCollector::ResolveRootParentMaterial(UMaterialInstanceConstant* MaterialInstance)
{
	if (!MaterialInstance)
	{
		return nullptr;
	}

	UMaterialInterface* CurrentParent = MaterialInstance->Parent;

	while (CurrentParent)
	{
		UMaterialInstance* ParentInstance = Cast<UMaterialInstance>(CurrentParent);

		// If cast fails, we've reached UMaterial (root)
		if (!ParentInstance)
		{
			break;
		}

		CurrentParent = ParentInstance->Parent;
	}

	return CurrentParent;
}
