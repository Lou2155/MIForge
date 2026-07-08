// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeVertexPaintRecipeManager.h"

void FMIForgeVertexPaintRecipeManager::SaveRecipe(const FString& RecipeName, const FMIForgeVertexPaintLayerStack& LayerStack)
{
	if (RecipeName.IsEmpty())
	{
		return;
	}

	FMIForgeVertexPaintRecipe NewRecipe;
	NewRecipe.RecipeName = RecipeName;
	NewRecipe.LayerR = MakeRecipeLayer(LayerStack.LayerR);
	NewRecipe.LayerG = MakeRecipeLayer(LayerStack.LayerG);
	NewRecipe.LayerB = MakeRecipeLayer(LayerStack.LayerB);

	for (FMIForgeVertexPaintRecipe& ExistingRecipe : Recipes)
	{
		if (ExistingRecipe.RecipeName == RecipeName)
		{
			ExistingRecipe = MoveTemp(NewRecipe);
			SaveRecipesToDisk();
			return;
		}
	}

	Recipes.Add(MoveTemp(NewRecipe));
	SaveRecipesToDisk();
}

bool FMIForgeVertexPaintRecipeManager::LoadRecipe(const FString& RecipeName, FMIForgeVertexPaintLayerStack& LayerStack) const
{	
	for (const FMIForgeVertexPaintRecipe& Recipe : Recipes)
	{
		if (Recipe.RecipeName == RecipeName)
		{
			LayerStack.LayerR.AssignedTextureSet =
				BuildTextureSetFromRecipeLayer(Recipe.LayerR);

			LayerStack.LayerG.AssignedTextureSet =
				BuildTextureSetFromRecipeLayer(Recipe.LayerG);

			LayerStack.LayerB.AssignedTextureSet =
				BuildTextureSetFromRecipeLayer(Recipe.LayerB);

			return true;
		}
	}

	return false;

}

bool FMIForgeVertexPaintRecipeManager::DeleteRecipe(const FString& RecipeName)
{
	const int32 RemovedCount = Recipes.RemoveAll(
		[&RecipeName](const FMIForgeVertexPaintRecipe& Recipe)
		{
			return Recipe.RecipeName == RecipeName;
		}
	);

	if (RemovedCount > 0)
	{
		SaveRecipesToDisk();
	}

	return RemovedCount > 0;
	
}

const TArray<FMIForgeVertexPaintRecipe>& FMIForgeVertexPaintRecipeManager::GetRecipes() const
{
	return Recipes;
}

bool FMIForgeVertexPaintRecipeManager::LoadRecipesFromDisk()
{
	const FString SavePath = GetRecipeSaveFilePath();

	if (!FPaths::FileExists(SavePath))
	{
		Recipes.Empty();
		return true;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *SavePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* RecipeValues = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("Recipes"), RecipeValues))
	{
		Recipes.Empty();
		return true;
	}

	Recipes.Empty();

	for (const TSharedPtr<FJsonValue>& RecipeValue : *RecipeValues)
	{
		const TSharedPtr<FJsonObject> RecipeObject = RecipeValue->AsObject();
		if (!RecipeObject.IsValid())
		{
			continue;
		}

		FMIForgeVertexPaintRecipe Recipe;
		if (JsonObjectToRecipe(RecipeObject, Recipe))
		{
			Recipes.Add(MoveTemp(Recipe));
		}
	}

	return true;
}

bool FMIForgeVertexPaintRecipeManager::SaveRecipesToDisk() const
{
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();

	TArray<TSharedPtr<FJsonValue>> RecipeValues;

	for (const FMIForgeVertexPaintRecipe& Recipe : Recipes)
	{
		RecipeValues.Add(
			MakeShared<FJsonValueObject>(RecipeToJsonObject(Recipe))
		);
	}

	RootObject->SetArrayField(TEXT("Recipes"), RecipeValues);

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

	if (!FJsonSerializer::Serialize(RootObject, Writer))
	{
		return false;

	}

	const FString SavePath = GetRecipeSaveFilePath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(SavePath), true);

	return FFileHelper::SaveStringToFile(OutputString, *SavePath);
}

FMIForgeVertexPaintRecipeLayer FMIForgeVertexPaintRecipeManager::MakeRecipeLayer(const FMIForgeVertexPaintLayerSlot& Slot) const
{
	FMIForgeVertexPaintRecipeLayer RecipeLayer;

	if (!Slot.AssignedTextureSet.IsValid())
	{
		return RecipeLayer;
	}

	const FMIForgeTextureSet& TextureSet = *Slot.AssignedTextureSet;

	RecipeLayer.SetName = TextureSet.SetName;

	for (const TPair<EMIForgeTextureType, FMIForgeTextureInfo>& Pair : TextureSet.Textures)
	{
		const FMIForgeTextureInfo& TextureInfo = Pair.Value;

		if(TextureInfo.AssetData.IsValid())
		{
			RecipeLayer.TexturePaths.Add(
				Pair.Key,
				FSoftObjectPath(TextureInfo.AssetData.ToSoftObjectPath())
			);
		}
	}
	return RecipeLayer;
}

TSharedPtr<FMIForgeTextureSet> FMIForgeVertexPaintRecipeManager::BuildTextureSetFromRecipeLayer(const FMIForgeVertexPaintRecipeLayer& RecipeLayer) const
{
	if (RecipeLayer.SetName.IsEmpty())
	{
		return nullptr;
	}

	TSharedPtr<FMIForgeTextureSet> TextureSet =
		MakeShared<FMIForgeTextureSet>();

	TextureSet->SetName = RecipeLayer.SetName;

	for (const TPair<EMIForgeTextureType, FSoftObjectPath>& Pair :
		RecipeLayer.TexturePaths)
	{
		const FSoftObjectPath& TexturePath = Pair.Value;
		
		UObject* Object = TexturePath.TryLoad();

		UTexture* Texture = Cast<UTexture>(Object);
		if (!Texture)
		{
			continue;
		}

		FAssetData AssetData(Texture);

		FMIForgeTextureInfo TextureInfo;
		TextureInfo.AssetData = AssetData;
		TextureInfo.AssetName = AssetData.AssetName.ToString();
		TextureInfo.PackagePath = AssetData.PackagePath.ToString();
		TextureInfo.ObjectPath = TexturePath.ToString();
		TextureInfo.TextureType = Pair.Key;
		TextureInfo.BaseName = RecipeLayer.SetName;

		TextureSet->Textures.Add(Pair.Key, TextureInfo);
	}

	if (TextureSet->Textures.Num() == 0)
	{
		return nullptr;
	}

	return TextureSet;
}

FString FMIForgeVertexPaintRecipeManager::GetRecipeSaveFilePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("MIForge/VertexPaintRecipes.json");
}

FString FMIForgeVertexPaintRecipeManager::TextureTypeToString(EMIForgeTextureType TextureType) const
{
	switch (TextureType)
	{
	case EMIForgeTextureType::Albedo:
		return TEXT("Albedo");
	case EMIForgeTextureType::Normal:
		return TEXT("Normal");
	case EMIForgeTextureType::ORM:
		return TEXT("ORM");
	case EMIForgeTextureType::Emissive:
		return TEXT("Emissive");
	case EMIForgeTextureType::DetailNormal:
		return TEXT("DetailNormal");
	case EMIForgeTextureType::RGB:
		return TEXT("RGB");
	case EMIForgeTextureType::Height:
		return TEXT("Height");
	default:
		return TEXT("Unknown");
	}
}

EMIForgeTextureType FMIForgeVertexPaintRecipeManager::TextureTypeFromString(const FString& TextureTypeString) const
{
	if (TextureTypeString == TEXT("Albedo")) return EMIForgeTextureType::Albedo;
	if (TextureTypeString == TEXT("Normal")) return EMIForgeTextureType::Normal;
	if (TextureTypeString == TEXT("ORM")) return EMIForgeTextureType::ORM;
	if (TextureTypeString == TEXT("Emissive")) return EMIForgeTextureType::Emissive;
	if (TextureTypeString == TEXT("DetailNormal")) return EMIForgeTextureType::DetailNormal;
	if (TextureTypeString == TEXT("RGB")) return EMIForgeTextureType::RGB;
	if (TextureTypeString == TEXT("Height")) return EMIForgeTextureType::Height;

	return EMIForgeTextureType::Unknown;
}

TSharedPtr<FJsonObject> FMIForgeVertexPaintRecipeManager::RecipeToJsonObject(const FMIForgeVertexPaintRecipe& Recipe) const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("RecipeName"), Recipe.RecipeName);
	JsonObject->SetObjectField(TEXT("LayerR"), RecipeLayerToJsonObject(Recipe.LayerR));
	JsonObject->SetObjectField(TEXT("LayerG"), RecipeLayerToJsonObject(Recipe.LayerG));
	JsonObject->SetObjectField(TEXT("LayerB"), RecipeLayerToJsonObject(Recipe.LayerB));

	return JsonObject;
}

TSharedPtr<FJsonObject> FMIForgeVertexPaintRecipeManager::RecipeLayerToJsonObject(const FMIForgeVertexPaintRecipeLayer& Layer) const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	JsonObject->SetStringField(TEXT("SetName"), Layer.SetName);

	TArray<TSharedPtr<FJsonValue>> TextureValues;

	for (const TPair<EMIForgeTextureType, FSoftObjectPath>& Pair : Layer.TexturePaths)
	{
		TSharedPtr<FJsonObject> TextureObject = MakeShared<FJsonObject>();

		TextureObject->SetStringField(TEXT("TextureType"), TextureTypeToString(Pair.Key));
		TextureObject->SetStringField(TEXT("Path"), Pair.Value.ToString());

		TextureValues.Add(MakeShared<FJsonValueObject>(TextureObject));
	}

	JsonObject->SetArrayField(TEXT("Textures"), TextureValues);

	return JsonObject;
}

bool FMIForgeVertexPaintRecipeManager::JsonObjectToRecipe(const TSharedPtr<FJsonObject>& JsonObject, FMIForgeVertexPaintRecipe& OutRecipe) const
{	
	OutRecipe = FMIForgeVertexPaintRecipe();

	if (!JsonObject.IsValid())
	{
		return false;
	}

	if (!JsonObject->TryGetStringField(TEXT("RecipeName"), OutRecipe.RecipeName))
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* LayerRObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("LayerR"), LayerRObject))
	{
		JsonObjectToRecipeLayer(*LayerRObject, OutRecipe.LayerR);
	}
	
	const TSharedPtr<FJsonObject>* LayerGObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("LayerG"), LayerGObject))
	{
		JsonObjectToRecipeLayer(*LayerGObject, OutRecipe.LayerG);
	}

	const TSharedPtr<FJsonObject>* LayerBObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("LayerB"), LayerBObject))
	{
		JsonObjectToRecipeLayer(*LayerBObject, OutRecipe.LayerB);
	}
	return true;
}

bool FMIForgeVertexPaintRecipeManager::JsonObjectToRecipeLayer(const TSharedPtr<FJsonObject>& JsonObject, FMIForgeVertexPaintRecipeLayer& OutLayer) const
{	
	OutLayer = FMIForgeVertexPaintRecipeLayer();

	if (!JsonObject.IsValid())
	{
		return false;
	}

	JsonObject->TryGetStringField(TEXT("SetName"), OutLayer.SetName);

	const TArray<TSharedPtr<FJsonValue>>* TextureValues = nullptr;
	if (!JsonObject->TryGetArrayField(TEXT("Textures"), TextureValues))
	{
		return true;
	}

	for (const TSharedPtr<FJsonValue>& TextureValue : *TextureValues)
	{
		const TSharedPtr<FJsonObject> TextureObject = TextureValue->AsObject();
		if (!TextureObject.IsValid())
		{
			continue;
		}

		FString TextureTypeString;
		FString PathString;

		if (!TextureObject->TryGetStringField(TEXT("TextureType"), TextureTypeString) ||
			!TextureObject->TryGetStringField(TEXT("Path"), PathString))
		{
			continue;
		}

		const EMIForgeTextureType TextureType = TextureTypeFromString(TextureTypeString);
		if (TextureType == EMIForgeTextureType::Unknown)
		{
			continue;
		}

		OutLayer.TexturePaths.Add(TextureType, FSoftObjectPath(PathString));
	}

	return true;
}
