// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/Texture2D.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"

#include "MIForgeTextureClassifier.h"
#include "MIForgeTypes.h"

namespace MIForgeTextureClassifierSpec
{
	FAssetData MakeTransientTextureAssetData(const TCHAR* AssetName)
	{
		const FString PackageName = FString::Printf(
			TEXT("/Engine/Transient/MIForgeTests/%s"),
			*FGuid::NewGuid().ToString(EGuidFormats::Digits));
		UPackage* Package = CreatePackage(*PackageName);
		Package->SetFlags(RF_Transient);

		UTexture2D* Texture = NewObject<UTexture2D>(
			Package,
			FName(AssetName),
			RF_Transient);
		return FAssetData(Texture);
	}
}

DEFINE_SPEC(
	FMIForgeTextureClassifierSpec,
	"MIForge.Unit.TextureClassifier",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeTextureClassifierSpec::Define()
{
	using namespace MIForgeTextureClassifierSpec;

	Describe("TextureTypeFromName", [this]()
	{
		struct FTypeCase
		{
			const TCHAR* TypeName;
			EMIForgeTextureType ExpectedType;
		};

		const TArray<FTypeCase> TypeCases = {
			{ TEXT("Albedo"), EMIForgeTextureType::Albedo },
			{ TEXT("Normal"), EMIForgeTextureType::Normal },
			{ TEXT("ORM"), EMIForgeTextureType::ORM },
			{ TEXT("Emissive"), EMIForgeTextureType::Emissive },
			{ TEXT("Detail Normal"), EMIForgeTextureType::DetailNormal },
			{ TEXT("RGB Mask"), EMIForgeTextureType::RGB },
			{ TEXT("Height"), EMIForgeTextureType::Height }
		};

		for (const FTypeCase& TypeCase : TypeCases)
		{
			It(
				FString::Printf(TEXT("should map %s"), TypeCase.TypeName),
				[this, TypeCase]()
				{
					const EMIForgeTextureType ActualType =
						FMIForgeTextureClassifier().TextureTypeFromName(TypeCase.TypeName);
					TestTrue(
						TEXT("Texture type matches"),
						ActualType == TypeCase.ExpectedType);
				});
		}

		It("should return Unknown for an unsupported type name", [this]()
		{
			const EMIForgeTextureType ActualType =
				FMIForgeTextureClassifier().TextureTypeFromName(TEXT("Unsupported"));
			TestTrue(
				TEXT("Returns Unknown"),
				ActualType == EMIForgeTextureType::Unknown);
		});
	});

	Describe("ClassifyTexture", [this]()
	{
		struct FSuffixCase
		{
			const TCHAR* AssetName;
			const TCHAR* ExpectedBaseName;
			const TCHAR* ExpectedSuffix;
			EMIForgeTextureType ExpectedType;
		};

		const TArray<FSuffixCase> SuffixCases = {
			{ TEXT("T_Stone_B"), TEXT("Stone"), TEXT("_B"), EMIForgeTextureType::Albedo },
			{ TEXT("T_Stone_Normal"), TEXT("Stone"), TEXT("_Normal"), EMIForgeTextureType::Normal },
			{ TEXT("T_Stone_ORM"), TEXT("Stone"), TEXT("_ORM"), EMIForgeTextureType::ORM },
			{ TEXT("T_Stone_Emissive"), TEXT("Stone"), TEXT("_Emissive"), EMIForgeTextureType::Emissive },
			{ TEXT("T_Stone_DN"), TEXT("Stone"), TEXT("_DN"), EMIForgeTextureType::DetailNormal },
			{ TEXT("T_Stone_RGBmask"), TEXT("Stone"), TEXT("_RGBmask"), EMIForgeTextureType::RGB },
			{ TEXT("T_Stone_Height"), TEXT("Stone"), TEXT("_Height"), EMIForgeTextureType::Height }
		};

		for (const FSuffixCase& SuffixCase : SuffixCases)
		{
			It(
				FString::Printf(TEXT("should classify %s"), SuffixCase.AssetName),
				[this, SuffixCase]()
				{
					const FMIForgeTextureInfo Result =
						FMIForgeTextureClassifier().ClassifyTexture(
							MakeTransientTextureAssetData(SuffixCase.AssetName));

					TestTrue(
						TEXT("Texture type matches"),
						Result.TextureType == SuffixCase.ExpectedType);
					TestEqual(
						TEXT("Base name"),
						Result.BaseName,
						FString(SuffixCase.ExpectedBaseName));
					TestEqual(
						TEXT("Matched suffix"),
						Result.MatchedSuffix,
						FString(SuffixCase.ExpectedSuffix));
				});
		}

		It("should remove only the optional T_ prefix from the base name", [this]()
		{
			const FMIForgeTextureInfo Result =
				FMIForgeTextureClassifier().ClassifyTexture(
					MakeTransientTextureAssetData(TEXT("Rock_N")));

			TestTrue(
				TEXT("Classified as Normal"),
				Result.TextureType == EMIForgeTextureType::Normal);
			TestEqual(TEXT("Base name"), Result.BaseName, FString(TEXT("Rock")));
		});

		It("should preserve the full asset name for an unknown texture", [this]()
		{
			const FMIForgeTextureInfo Result =
				FMIForgeTextureClassifier().ClassifyTexture(
					MakeTransientTextureAssetData(TEXT("T_Stone_Custom")));

			TestTrue(
				TEXT("Classified as Unknown"),
				Result.TextureType == EMIForgeTextureType::Unknown);
			TestEqual(
				TEXT("Base name"),
				Result.BaseName,
				FString(TEXT("T_Stone_Custom")));
			TestTrue(TEXT("Matched suffix is empty"), Result.MatchedSuffix.IsEmpty());
		});
	});
}

#endif
