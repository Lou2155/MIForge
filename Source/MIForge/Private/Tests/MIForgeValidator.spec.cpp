#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "MIForgeTypes.h"
#include "MIForgeValidator.h"

namespace MIForgeValidatorSpec
{
	FMIForgeTextureInfo MakeTexture(
		const FString& SetName,
		const EMIForgeTextureType TextureType)
	{
		FMIForgeTextureInfo Texture;
		Texture.AssetName = FString::Printf(
			TEXT("T_%s_%d"),
			*SetName,
			static_cast<int32>(TextureType));
		Texture.BaseName = SetName;
		Texture.TextureType = TextureType;
		return Texture;
	}

	FMIForgeTextureSet MakeSet(
		const TCHAR* SetName,
		const TArray<EMIForgeTextureType>& TextureTypes)
	{
		FMIForgeTextureSet TextureSet;
		TextureSet.SetName = SetName;

		for (const EMIForgeTextureType TextureType : TextureTypes)
		{
			TextureSet.Textures.Add(
				TextureType,
				MakeTexture(TextureSet.SetName, TextureType));
		}

		return TextureSet;
	}

	bool ContainsRequirement(
		const TArray<FMIForgeTextureRequirement>& Requirements,
		const EMIForgeTextureType TextureType)
	{
		return Requirements.ContainsByPredicate(
			[TextureType](const FMIForgeTextureRequirement& Requirement)
			{
				return Requirement.TextureType == TextureType;
			});
	}

	FMIForgeVertexPaintLayerStack MakeLayerStack()
	{
		FMIForgeVertexPaintLayerStack Stack;

		Stack.BaseLayer.Layer = EMIForgeVertexPaintLayer::Base;
		Stack.BaseLayer.DisplayName = TEXT("Base");
		Stack.BaseLayer.bRequired = true;

		Stack.LayerR.Layer = EMIForgeVertexPaintLayer::LayerR;
		Stack.LayerR.DisplayName = TEXT("Layer R");
		Stack.LayerR.bRequired = true;

		Stack.LayerG.Layer = EMIForgeVertexPaintLayer::LayerG;
		Stack.LayerG.DisplayName = TEXT("Layer G");
		Stack.LayerG.bRequired = false;

		Stack.LayerB.Layer = EMIForgeVertexPaintLayer::LayerB;
		Stack.LayerB.DisplayName = TEXT("Layer B");
		Stack.LayerB.bRequired = false;

		return Stack;
	}

	const FMIForgeVertexPaintLayerValidationResult* FindLayerResult(
		const FMIForgeVertexPaintLayerStackValidationResult& Result,
		const EMIForgeVertexPaintLayer Layer)
	{
		return Result.LayerResults.FindByPredicate(
			[Layer](const FMIForgeVertexPaintLayerValidationResult& LayerResult)
			{
				return LayerResult.Layer == Layer;
			});
	}
}

DEFINE_SPEC(
	FMIForgeValidatorSpec,
	"MIForge.Unit.Validator",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeValidatorSpec::Define()
{
	using namespace MIForgeValidatorSpec;

	Describe("ValidateStandardSet", [this]()
	{
		It("should accept a set containing all required textures", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateStandardSet(Set, false, false, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Missing required count"), Result.MissingRequiredTextures.Num(), 0);
			TestEqual(TEXT("Applied texture count"), Result.SuccessfulAppliedTextures.Num(), 3);
		});

		struct FMissingRequiredCase
		{
			EMIForgeTextureType TextureType;
			const TCHAR* DisplayName;
		};

		const TArray<FMissingRequiredCase> MissingCases = {
			{ EMIForgeTextureType::Albedo, TEXT("Albedo") },
			{ EMIForgeTextureType::Normal, TEXT("Normal") },
			{ EMIForgeTextureType::ORM, TEXT("ORM") }
		};

		for (const FMissingRequiredCase& MissingCase : MissingCases)
		{
			It(
				FString::Printf(
					TEXT("should reject a set missing required %s"),
					MissingCase.DisplayName),
				[this, MissingCase]()
				{
					FMIForgeTextureSet Set = MakeSet(
						TEXT("Rock"),
						{
							EMIForgeTextureType::Albedo,
							EMIForgeTextureType::Normal,
							EMIForgeTextureType::ORM
						});
					Set.Textures.Remove(MissingCase.TextureType);

					const FMIForgeTextureSetValidationResult Result =
						FMIForgeValidator().ValidateStandardSet(Set, false, false, false);

					TestFalse(TEXT("Cannot generate"), Result.bCanGenerate);
					TestEqual(TEXT("Missing required count"), Result.MissingRequiredTextures.Num(), 1);
					TestTrue(
						TEXT("Contains expected missing type"),
						ContainsRequirement(Result.MissingRequiredTextures, MissingCase.TextureType));
				});
		}

		It("should not report disabled optional textures", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateStandardSet(Set, false, false, false);

			TestEqual(TEXT("Missing optional count"), Result.MissingOptionalTextures.Num(), 0);
		});

		It("should warn about enabled optional textures without blocking generation", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateStandardSet(Set, true, true, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Missing optional count"), Result.MissingOptionalTextures.Num(), 2);
			TestTrue(
				TEXT("Missing Emissive"),
				ContainsRequirement(Result.MissingOptionalTextures, EMIForgeTextureType::Emissive));
			TestTrue(
				TEXT("Missing Detail Normal"),
				ContainsRequirement(Result.MissingOptionalTextures, EMIForgeTextureType::DetailNormal));
		});

		It("should report or ignore unrecognized textures based on the option", [this]()
		{
			FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});
			FMIForgeTextureInfo UnknownTexture;
			UnknownTexture.AssetName = TEXT("T_Rock_Custom");
			UnknownTexture.TextureType = EMIForgeTextureType::Unknown;
			Set.UnrecognizedTextures.Add(UnknownTexture);

			const FMIForgeTextureSetValidationResult Reported =
				FMIForgeValidator().ValidateStandardSet(Set, false, false, false);
			const FMIForgeTextureSetValidationResult Ignored =
				FMIForgeValidator().ValidateStandardSet(Set, false, false, true);

			TestEqual(TEXT("Reported unknown count"), Reported.UnrecognizedTextures.Num(), 1);
			TestEqual(TEXT("Ignored unknown count"), Ignored.UnrecognizedTextures.Num(), 0);
		});
	});

	Describe("ValidateRGBSet", [this]()
	{
		It("should accept the minimum RGB set when base ORM is disabled", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::RGB
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateRGBSet(Set, false, false, false, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Missing required count"), Result.MissingRequiredTextures.Num(), 0);
		});

		It("should require ORM when base ORM is enabled", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::RGB
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateRGBSet(Set, true, false, false, false);

			TestFalse(TEXT("Cannot generate"), Result.bCanGenerate);
			TestTrue(
				TEXT("Missing ORM"),
				ContainsRequirement(Result.MissingRequiredTextures, EMIForgeTextureType::ORM));
		});

		It("should treat enabled emissive and detail normal as optional", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::RGB
				});

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateRGBSet(Set, false, true, true, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Missing optional count"), Result.MissingOptionalTextures.Num(), 2);
			TestTrue(
				TEXT("Missing Emissive"),
				ContainsRequirement(Result.MissingOptionalTextures, EMIForgeTextureType::Emissive));
			TestTrue(
				TEXT("Missing Detail Normal"),
				ContainsRequirement(Result.MissingOptionalTextures, EMIForgeTextureType::DetailNormal));
		});
	});

	Describe("ValidateVertexPaintSet", [this]()
	{
		It("should require only Albedo and report the other supported types as optional", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{ EMIForgeTextureType::Albedo });

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateVertexPaintSet(Set, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Missing required count"), Result.MissingRequiredTextures.Num(), 0);
			TestEqual(TEXT("Missing optional count"), Result.MissingOptionalTextures.Num(), 3);
		});

		It("should reject a set without Albedo", [this]()
		{
			const FMIForgeTextureSet Set = MakeSet(
				TEXT("Rock"),
				{ EMIForgeTextureType::Normal });

			const FMIForgeTextureSetValidationResult Result =
				FMIForgeValidator().ValidateVertexPaintSet(Set, false);

			TestFalse(TEXT("Cannot generate"), Result.bCanGenerate);
			TestTrue(
				TEXT("Missing Albedo"),
				ContainsRequirement(Result.MissingRequiredTextures, EMIForgeTextureType::Albedo));
		});
	});

	Describe("ValidateVertexPaintLayerStack", [this]()
	{
		It("should allow required Base and R layers with optional layers empty", [this]()
		{
			FMIForgeVertexPaintLayerStack Stack = MakeLayerStack();
			Stack.BaseLayer.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("Base"), { EMIForgeTextureType::Albedo }));
			Stack.LayerR.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("R"), { EMIForgeTextureType::Albedo }));

			const FMIForgeVertexPaintLayerStackValidationResult Result =
				FMIForgeValidator().ValidateVertexPaintLayerStack(Stack, false);

			TestTrue(TEXT("Can generate"), Result.bCanGenerate);
			TestEqual(TEXT("Assigned layer count"), Result.AssignedLayerCount, 2);
		});

		It("should reject an empty required R layer", [this]()
		{
			FMIForgeVertexPaintLayerStack Stack = MakeLayerStack();
			Stack.BaseLayer.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("Base"), { EMIForgeTextureType::Albedo }));

			const FMIForgeVertexPaintLayerStackValidationResult Result =
				FMIForgeValidator().ValidateVertexPaintLayerStack(Stack, false);

			TestFalse(TEXT("Cannot generate"), Result.bCanGenerate);
			const FMIForgeVertexPaintLayerValidationResult* LayerRResult =
				FindLayerResult(Result, EMIForgeVertexPaintLayer::LayerR);
			TestTrue(TEXT("Contains Layer R result"), LayerRResult != nullptr);
			if (LayerRResult)
			{
				TestTrue(
					TEXT("Layer R has error status"),
					LayerRResult->Status == EMIForgeVertexPaintLayerStatus::Error);
			}
		});

		It("should not report missing Height for Layer B", [this]()
		{
			FMIForgeVertexPaintLayerStack Stack = MakeLayerStack();
			Stack.BaseLayer.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("Base"), { EMIForgeTextureType::Albedo }));
			Stack.LayerR.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("R"), { EMIForgeTextureType::Albedo }));
			Stack.LayerB.AssignedTextureSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(TEXT("B"), { EMIForgeTextureType::Albedo }));

			const FMIForgeVertexPaintLayerStackValidationResult Result =
				FMIForgeValidator().ValidateVertexPaintLayerStack(Stack, false);

			const FMIForgeVertexPaintLayerValidationResult* LayerBResult =
				FindLayerResult(Result, EMIForgeVertexPaintLayer::LayerB);
			TestTrue(TEXT("Contains Layer B result"), LayerBResult != nullptr);
			if (LayerBResult)
			{
				TestFalse(
					TEXT("Height warning is removed"),
					ContainsRequirement(
						LayerBResult->MissingOptionalTextures,
						EMIForgeTextureType::Height));
				TestEqual(
					TEXT("Remaining optional warnings"),
					LayerBResult->MissingOptionalTextures.Num(),
					2);
			}
		});
	});

	Describe("BuildStandardSummaryFromTextureSets", [this]()
	{
		It("should aggregate ready, error, and optional warning counts", [this]()
		{
			const TSharedPtr<FMIForgeTextureSet> ReadySet = MakeShared<FMIForgeTextureSet>(
				MakeSet(
					TEXT("Ready"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					}));
			const TSharedPtr<FMIForgeTextureSet> ErrorSet = MakeShared<FMIForgeTextureSet>(
				MakeSet(
					TEXT("Error"),
					{ EMIForgeTextureType::Albedo }));

			const TArray<TSharedPtr<FMIForgeTextureSet>> Sets = { ReadySet, ErrorSet };
			const FMIForgeValidationSummary Summary =
				FMIForgeValidator().BuildStandardSummaryFromTextureSets(
					Sets,
					true,
					false,
					false);

			TestEqual(TEXT("Total sets"), Summary.TotalSets, 2);
			TestEqual(TEXT("Ready sets"), Summary.ReadyToCreateCount, 1);
			TestEqual(TEXT("Sets with errors"), Summary.SetsWithErrors, 1);
			TestEqual(TEXT("Sets with optional warnings"), Summary.SetsWithWarnings, 2);
			TestEqual(TEXT("Missing required textures"), Summary.MissingRequiredTextureCount, 2);
			TestEqual(TEXT("Missing optional textures"), Summary.MissingOptionalTextureCount, 2);
		});
	});
}

#endif
