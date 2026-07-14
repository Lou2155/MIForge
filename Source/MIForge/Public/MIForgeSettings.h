// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "Engine/DeveloperSettings.h"
#include "MIForgeTypes.h"
#include "MIForgeSettings.generated.h"


USTRUCT()
struct FTextureExtension
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Texture Extension")
	TArray<FString> Extension;
};

USTRUCT()
struct FMIForgeTextureCompressionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Texture Compression")
	TEnumAsByte<TextureCompressionSettings> CompressionSettings;
	UPROPERTY(EditAnywhere, Category = "Texture Compression")
	bool bSRGB = true;
};

UCLASS(Config = MIForge, DefaultConfig)
class MIFORGE_API UMIForgeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual FName GetCategoryName() const override { return FName("MIForge Settings"); }

public:
	UPROPERTY(Config, EditAnywhere, Category = "Texture Extensions")
	TMap<FString, FTextureExtension> TextureExtensions;

	UPROPERTY(Config, EditAnywhere, Category = "Texture Compression")
	TMap<EMIForgeTextureType, FMIForgeTextureCompressionSettings> TextureCompressionSettingsMap;

	static UMIForgeSettings* Get() {
		return GetMutableDefault<UMIForgeSettings>(); //get the default object of this class, which is where the settings are stored.  This allows us to access the settings from anywhere in our code.
	}
	
};
