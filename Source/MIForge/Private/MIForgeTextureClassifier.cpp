// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeTextureClassifier.h"
#include "MIForgeTypes.h"
#include "MIForgeSettings.h"

FMIForgeTextureInfo FMIForgeTextureClassifier::ClassifyTexture(const FAssetData& AssetData) const
{
	FMIForgeTextureInfo Info;
	Info.AssetData = AssetData;
	Info.AssetName = AssetData.AssetName.ToString();
	Info.PackagePath = AssetData.PackagePath.ToString();
	Info.ObjectPath = AssetData.GetSoftObjectPath().ToString();

	FillTextureSize(AssetData, Info);

	const UMIForgeSettings* Settings = UMIForgeSettings::Get();

	for (const TPair<FString, FTextureExtension>& Pair : Settings->TextureExtensions) {
		const FString& TypeName = Pair.Key;
		const FTextureExtension& TextureExtension = Pair.Value;

		for (const FString& Suffix : TextureExtension.Extension) {
			if (Info.AssetName.EndsWith(Suffix))
			{
				Info.TextureType = TextureTypeFromName(TypeName);
				Info.MatchedSuffix = Suffix;
				Info.BaseName = Info.AssetName.LeftChop(Suffix.Len());
				if (Info.AssetName.StartsWith(TEXT("T_"))) {
					Info.BaseName = Info.BaseName.RightChop(2);
				}
				return Info;
			}
		}
	}
	Info.TextureType = EMIForgeTextureType::Unknown;
	Info.BaseName = Info.AssetName;

	

	return Info;
}

TArray<FMIForgeTextureInfo> FMIForgeTextureClassifier::ClassifyTextures(const TArray<FAssetData>& Assets) const
{
	TArray<FMIForgeTextureInfo> Results;
	Results.Reserve(Assets.Num());

	for(const FAssetData& ad : Assets)
	{
		Results.Add(ClassifyTexture(ad));
	}

	return Results;
}

EMIForgeTextureType FMIForgeTextureClassifier::TextureTypeFromName(const FString& TypeName) const
{
	if (TypeName == TEXT("Albedo"))
	{
		return EMIForgeTextureType::Albedo;
	}

	if (TypeName == TEXT("Normal"))
	{
		return EMIForgeTextureType::Normal;
	}

	if (TypeName == TEXT("ORM"))
	{
		return EMIForgeTextureType::ORM;
	}

	if (TypeName == TEXT("Emissive"))
	{
		return EMIForgeTextureType::Emissive;
	}

	if (TypeName == TEXT("Detail Normal"))
	{
		return EMIForgeTextureType::DetailNormal;
	}

	if (TypeName == TEXT("RGB Mask"))
	{
		return EMIForgeTextureType::RGB;
	}
	if (TypeName == TEXT("Height"))
	{
		return EMIForgeTextureType::Height;
	}

	return EMIForgeTextureType::Unknown;
}

void FMIForgeTextureClassifier::FillTextureSize(const FAssetData& AssetData, FMIForgeTextureInfo& Info) const
{
	Info.TextureSize = FIntPoint::ZeroValue;
	Info.TextureSizeText = TEXT("-");

	UTexture2D* Tex2D = Cast<UTexture2D>(AssetData.GetAsset());
	if (!Tex2D)
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	if (Tex2D->Source.IsValid())
	{
		Info.TextureSize = FIntPoint(
			Tex2D->Source.GetSizeX(),
			Tex2D->Source.GetSizeY()
		);
	}
	else
#endif
	{
		Info.TextureSize = FIntPoint(
			Tex2D->GetSizeX(),
			Tex2D->GetSizeY()
		);
	}

	Info.TextureSizeText = FString::Printf(
		TEXT("%dx%d"),
		Info.TextureSize.X,
		Info.TextureSize.Y
	);
}
