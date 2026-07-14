#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "MIForgeTextureSetBuilder.h"
#include "MIForgeTypes.h"

namespace MIForgeTextureSetBuilderSpec
{
	FMIForgeTextureInfo MakeTexture(
		const TCHAR* AssetName,
		const TCHAR* BaseName,
		const EMIForgeTextureType TextureType)
	{
		FMIForgeTextureInfo Texture;
		Texture.AssetName = AssetName;
		Texture.BaseName = BaseName;
		Texture.TextureType = TextureType;
		return Texture;
	}

	const FMIForgeTextureSet* FindSetByName(
		const TArray<FMIForgeTextureSet>& TextureSets,
		const TCHAR* SetName)
	{
		return TextureSets.FindByPredicate(
			[SetName](const FMIForgeTextureSet& TextureSet)
			{
				return TextureSet.SetName == SetName;
			});
	}
}

DEFINE_SPEC(
	FMIForgeTextureSetBuilderSpec,
	"MIForge.Unit.TextureSetBuilder",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeTextureSetBuilderSpec::Define()
{
	using namespace MIForgeTextureSetBuilderSpec;

	Describe("BuildTextureSets", [this]()
	{
		It("should group different texture types with the same base name", [this]()
		{
			const TArray<FMIForgeTextureInfo> Input = {
				MakeTexture(TEXT("T_Rock_B"), TEXT("Rock"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("T_Rock_N"), TEXT("Rock"), EMIForgeTextureType::Normal),
				MakeTexture(TEXT("T_Rock_ORM"), TEXT("Rock"), EMIForgeTextureType::ORM)
			};

			const TArray<FMIForgeTextureSet> Result =
				FMIForgeTextureSetBuilder().BuildTextureSets(Input);

			TestEqual(TEXT("Texture set count"), Result.Num(), 1);
			if (Result.Num() != 1)
			{
				return;
			}

			TestEqual(TEXT("Set name"), Result[0].SetName, FString(TEXT("Rock")));
			TestEqual(TEXT("Texture count"), Result[0].Textures.Num(), 3);
			TestTrue(
				TEXT("Contains Albedo"),
				Result[0].Textures.Contains(EMIForgeTextureType::Albedo));
			TestTrue(
				TEXT("Contains Normal"),
				Result[0].Textures.Contains(EMIForgeTextureType::Normal));
			TestTrue(
				TEXT("Contains ORM"),
				Result[0].Textures.Contains(EMIForgeTextureType::ORM));
		});

		It("should create separate sets for different base names", [this]()
		{
			const TArray<FMIForgeTextureInfo> Input = {
				MakeTexture(TEXT("T_Rock_B"), TEXT("Rock"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("T_Mud_B"), TEXT("Mud"), EMIForgeTextureType::Albedo)
			};

			const TArray<FMIForgeTextureSet> Result =
				FMIForgeTextureSetBuilder().BuildTextureSets(Input);

			TestEqual(TEXT("Texture set count"), Result.Num(), 2);
			TestTrue(TEXT("Contains Rock set"), FindSetByName(Result, TEXT("Rock")) != nullptr);
			TestTrue(TEXT("Contains Mud set"), FindSetByName(Result, TEXT("Mud")) != nullptr);
		});

		It("should ignore unknown textures", [this]()
		{
			const TArray<FMIForgeTextureInfo> Input = {
				MakeTexture(TEXT("T_Rock_B"), TEXT("Rock"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("T_Rock_Custom"), TEXT("Rock"), EMIForgeTextureType::Unknown)
			};

			const TArray<FMIForgeTextureSet> Result =
				FMIForgeTextureSetBuilder().BuildTextureSets(Input);

			TestEqual(TEXT("Texture set count"), Result.Num(), 1);
			if (Result.Num() == 1)
			{
				TestEqual(TEXT("Recognized texture count"), Result[0].Textures.Num(), 1);
				TestFalse(
					TEXT("Does not contain Unknown"),
					Result[0].Textures.Contains(EMIForgeTextureType::Unknown));
			}
		});

		It("should ignore textures with an empty base name", [this]()
		{
			const TArray<FMIForgeTextureInfo> Input = {
				MakeTexture(TEXT("T_Valid_B"), TEXT("Valid"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("T_Invalid_N"), TEXT(""), EMIForgeTextureType::Normal)
			};

			const TArray<FMIForgeTextureSet> Result =
				FMIForgeTextureSetBuilder().BuildTextureSets(Input);

			TestEqual(TEXT("Texture set count"), Result.Num(), 1);
			TestTrue(TEXT("Contains valid set"), FindSetByName(Result, TEXT("Valid")) != nullptr);
		});

		It("should keep the last texture when a type is duplicated", [this]()
		{
			const TArray<FMIForgeTextureInfo> Input = {
				MakeTexture(TEXT("T_Rock_B_First"), TEXT("Rock"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("T_Rock_B_Second"), TEXT("Rock"), EMIForgeTextureType::Albedo)
			};

			const TArray<FMIForgeTextureSet> Result =
				FMIForgeTextureSetBuilder().BuildTextureSets(Input);

			TestEqual(TEXT("Texture set count"), Result.Num(), 1);
			if (Result.Num() != 1)
			{
				return;
			}

			const FMIForgeTextureInfo* Albedo =
				Result[0].Textures.Find(EMIForgeTextureType::Albedo);
			TestTrue(TEXT("Contains Albedo"), Albedo != nullptr);
			if (Albedo)
			{
				TestEqual(
					TEXT("Last duplicate wins"),
					Albedo->AssetName,
					FString(TEXT("T_Rock_B_Second")));
			}
		});
	});
}

#endif
