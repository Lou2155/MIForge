// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MIForgeTypes.h"


class FMIForgeVertexPaintRecipeManager
{
public:
	void SaveRecipe(
		const FString& RecipeName,
		const FMIForgeVertexPaintLayerStack& LayerStack
	);

	bool LoadRecipe(
		const FString& RecipeName,
		FMIForgeVertexPaintLayerStack& OutLayerStack
	) const;

	bool DeleteRecipe(const FString& RecipeName);

	const TArray<FMIForgeVertexPaintRecipe>& GetRecipes() const;

	bool LoadRecipesFromDisk();
	bool SaveRecipesToDisk() const;

private:
	TArray<FMIForgeVertexPaintRecipe> Recipes;

	FMIForgeVertexPaintRecipeLayer MakeRecipeLayer(
		const FMIForgeVertexPaintLayerSlot& Slot
	) const;

	TSharedPtr<FMIForgeTextureSet> BuildTextureSetFromRecipeLayer(
		const FMIForgeVertexPaintRecipeLayer& RecipeLayer
	) const;

	//todo: implement this function to resolve the texture reference to an actual texture object
	/*UObject* ResolveRecipeTexture(
		const FMIForgeVertexPaintRecipeTextureRef& TextureRef
	) const;*/
	

#pragma region Serialization helpers
	FString GetRecipeSaveFilePath() const;

	FString TextureTypeToString(EMIForgeTextureType TextureType) const;
	EMIForgeTextureType TextureTypeFromString(const FString& TextureTypeString) const;

	TSharedPtr<FJsonObject> RecipeToJsonObject(
		const FMIForgeVertexPaintRecipe& Recipe
	) const;

	TSharedPtr<FJsonObject> RecipeLayerToJsonObject(
		const FMIForgeVertexPaintRecipeLayer& Layer
	) const;

	bool JsonObjectToRecipe(
		const TSharedPtr<FJsonObject>& JsonObject,
		FMIForgeVertexPaintRecipe& OutRecipe
	) const;

	bool JsonObjectToRecipeLayer(
		const TSharedPtr<FJsonObject>& JsonObject,
		FMIForgeVertexPaintRecipeLayer& OutLayer
	) const;
#pragma endregion
};
