// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetData.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture2D.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"

#include "Generation/MIForgeGenerationPlanner.h"
#include "Generation/MIForgeGenerationPlan.h"
#include "MIForgeTypes.h"

namespace MIForgeGenerationPlannerSpec
{
	class FPlannerTestContext
	{
	public:
		FPlannerTestContext()
			: TargetPath(FString::Printf(
				TEXT("/Game/MIForgePlannerAutomation/%s"),
				*FGuid::NewGuid().ToString(EGuidFormats::Digits)))
		{
		}

		~FPlannerTestContext()
		{
			for (UTexture2D* Texture : TransientTextures)
			{
				if (Texture && Texture->IsRooted())
				{
					Texture->RemoveFromRoot();
				}
			}
		}

		FMIForgeTextureInfo MakeTextureInfo(
			const FString& SetName,
			EMIForgeTextureType TextureType)
		{
			const FString TextureName = FString::Printf(
				TEXT("T_%s_%d"),
				*SetName,
				static_cast<int32>(TextureType));
			const FString PackageName = FString::Printf(
				TEXT("/Engine/Transient/MIForgePlannerTests/%s"),
				*FGuid::NewGuid().ToString(EGuidFormats::Digits));

			UPackage* Package = CreatePackage(*PackageName);
			Package->SetFlags(RF_Transient);

			UTexture2D* Texture = NewObject<UTexture2D>(
				Package,
				FName(TextureName),
				RF_Transient);
			Texture->AddToRoot();
			TransientTextures.Add(Texture);

			FMIForgeTextureInfo TextureInfo;
			TextureInfo.AssetData = FAssetData(Texture);
			TextureInfo.AssetName = TextureName;
			TextureInfo.TextureType = TextureType;
			TextureInfo.BaseName = SetName;
			return TextureInfo;
		}

		TSharedPtr<FMIForgeTextureSet> MakeTextureSet(
			const FString& SetName,
			const TArray<EMIForgeTextureType>& TextureTypes)
		{
			TSharedPtr<FMIForgeTextureSet> TextureSet =
				MakeShared<FMIForgeTextureSet>();
			TextureSet->SetName = SetName;

			for (const EMIForgeTextureType TextureType : TextureTypes)
			{
				TextureSet->Textures.Add(
					TextureType,
					MakeTextureInfo(SetName, TextureType));
			}

			return TextureSet;
		}

		FMIForgeVertexPaintLayerStack MakeVertexPaintLayerStack()
		{
			FMIForgeVertexPaintLayerStack Stack;

			Stack.BaseLayer.Layer = EMIForgeVertexPaintLayer::Base;
			Stack.BaseLayer.DisplayName = TEXT("Base");
			Stack.BaseLayer.ChannelName = TEXT("Base");
			Stack.BaseLayer.bRequired = true;
			Stack.BaseLayer.AssignedTextureSet = MakeTextureSet(
				TEXT("VertexBase"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM,
					EMIForgeTextureType::Height
				});

			Stack.LayerR.Layer = EMIForgeVertexPaintLayer::LayerR;
			Stack.LayerR.DisplayName = TEXT("Layer R");
			Stack.LayerR.ChannelName = TEXT("R");
			Stack.LayerR.bRequired = true;
			Stack.LayerR.AssignedTextureSet = MakeTextureSet(
				TEXT("VertexR"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});

			Stack.LayerG.Layer = EMIForgeVertexPaintLayer::LayerG;
			Stack.LayerG.DisplayName = TEXT("Layer G");
			Stack.LayerG.ChannelName = TEXT("G");

			Stack.LayerB.Layer = EMIForgeVertexPaintLayer::LayerB;
			Stack.LayerB.DisplayName = TEXT("Layer B");
			Stack.LayerB.ChannelName = TEXT("B");

			return Stack;
		}

		FString ObjectPath(const FString& AssetName) const
		{
			return FString::Printf(
				TEXT("%s/%s.%s"),
				*TargetPath,
				*AssetName,
				*AssetName);
		}

		const FString TargetPath;

	private:
		TArray<UTexture2D*> TransientTextures;
	};

	FMIForgeGenerationOptions MakeMaterialOptions(
		EMIForgeGenerationPreset Preset,
		const FString& TargetPath)
	{
		FMIForgeGenerationOptions Options;
		Options.Preset = Preset;
		Options.TargetPath = TargetPath;
		Options.IfMIExists = EIfMIExistsOption::Skip;
		return Options;
	}
}

DEFINE_SPEC(
	FMIForgeGenerationPlannerSpec,
	"MIForge.Integration.GenerationPlanner",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeGenerationPlannerSpec::Define()
{
	using namespace MIForgeGenerationPlannerSpec;

	Describe("PlanMaterialGeneration", [this]()
	{
		It("should plan a valid Standard texture set without creating an asset", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("Stone"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Standard,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 1);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 0);

			if (Plan.Items.Num() == 1)
			{
				TestEqual(
					TEXT("Desired asset name"),
					Plan.Items[0].DesiredAssetName,
					FString(TEXT("MI_Stone")));
				TestNotNull(
					TEXT("Parent material"),
					Plan.Items[0].ParentMaterial);
				TestTrue(
					TEXT("Texture set is preserved"),
					Plan.Items[0].TextureSet.Get() == TextureSet.Get());
			}

			TestFalse(
				TEXT("Planning does not create the material instance"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_Stone"))));
		});

		It("should keep valid items when another batch item is invalid", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> ValidSet =
				Context.MakeTextureSet(
					TEXT("Valid"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Standard,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ nullptr, ValidSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 1);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 1);
			TestTrue(TEXT("Failure message recorded"), Plan.Messages.Num() > 0);
		});

		It("should fail every item when the target path is outside Game", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> FirstSet =
				Context.MakeTextureSet(
					TEXT("First"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const TSharedPtr<FMIForgeTextureSet> SecondSet =
				Context.MakeTextureSet(
					TEXT("Second"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Standard,
				TEXT("/Engine/MIForgePlannerTests"));

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ FirstSet, SecondSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 0);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 2);
		});

		It("should fail every item for an unsupported material preset", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("Unsupported"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::VertexPainting,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 0);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 1);
		});

		It("should accept RGB Mask without ORM when Base ORM is disabled", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("RGBNoORM"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::RGB
					});
			FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::RGBMask,
				Context.TargetPath);
			Options.bUseBaseORMTexture = false;

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 1);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 0);
		});

		It("should require ORM when Base ORM is enabled", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("RGBRequiresORM"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::RGB
					});
			FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::RGBMask,
				Context.TargetPath);
			Options.bUseBaseORMTexture = true;

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 0);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 1);
		});

		It("should plan an Albedo-only Decal with its Decal asset name", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("Graffiti"),
					{ EMIForgeTextureType::Albedo });
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Decal,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 1);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 0);

			if (Plan.Items.Num() == 1)
			{
				TestEqual(
					TEXT("Desired Decal asset name"),
					Plan.Items[0].DesiredAssetName,
					FString(TEXT("MI_Decal_Graffiti")));
				TestNotNull(
					TEXT("Decal parent material"),
					Plan.Items[0].ParentMaterial);
				TestTrue(
					TEXT("Decal preset is preserved"),
					Plan.Items[0].Options.Preset ==
					EMIForgeGenerationPreset::Decal);
			}

			TestFalse(
				TEXT("Planning does not create the Decal material instance"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_Decal_Graffiti"))));
		});

		It("should plan a Decal when requested optional textures are absent", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("OptionalMissing"),
					{ EMIForgeTextureType::Albedo });
			FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Decal,
				Context.TargetPath);
			Options.bUseDecalNormal = true;
			Options.bUseDecalORM = true;
			Options.bUseOrientationMask = true;

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 1);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 0);
		});

		It("should reject a Decal without its required Albedo texture", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("MissingAlbedo"),
					{ EMIForgeTextureType::Normal });
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Decal,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 0);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 1);
			TestTrue(TEXT("Failure message recorded"), Plan.Messages.Num() > 0);
		});

		It("should reject an invalid generated package name", [this]()
		{
			FPlannerTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("ValidSource"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			TextureSet->SetName = TEXT("Invalid Name");
			const FMIForgeGenerationOptions Options = MakeMaterialOptions(
				EMIForgeGenerationPreset::Standard,
				Context.TargetPath);

			const FMIForgeMaterialGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanMaterialGeneration(
					{ TextureSet },
					Options);

			TestEqual(TEXT("Planned item count"), Plan.Items.Num(), 0);
			TestEqual(TEXT("Failed planning count"), Plan.FailedPlanningCount, 1);
		});
	});

	Describe("PlanVertexPaintGeneration", [this]()
	{
		It("should accept empty optional layers and use the Base set for the default name", [this]()
		{
			FPlannerTestContext Context;
			const FMIForgeVertexPaintLayerStack Stack =
				Context.MakeVertexPaintLayerStack();
			FMIForgeVertexPaintGenerationOptions Options;
			Options.TargetPath = Context.TargetPath;

			const FMIForgeVertexPaintGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanVertexPaintGeneration(
					Stack,
					Options);

			TestTrue(TEXT("Plan is valid"), Plan.bValid);
			TestEqual(
				TEXT("Default asset name"),
				Plan.DesiredAssetName,
				FString(TEXT("MI_VP_VertexBase")));
			TestNotNull(TEXT("Parent material"), Plan.ParentMaterial);
			TestFalse(
				TEXT("Planning does not create the material instance"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_VP_VertexBase"))));
		});

		It("should reject a missing required R layer", [this]()
		{
			FPlannerTestContext Context;
			FMIForgeVertexPaintLayerStack Stack =
				Context.MakeVertexPaintLayerStack();
			Stack.LayerR.AssignedTextureSet.Reset();
			FMIForgeVertexPaintGenerationOptions Options;
			Options.TargetPath = Context.TargetPath;

			const FMIForgeVertexPaintGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanVertexPaintGeneration(
					Stack,
					Options);

			TestFalse(TEXT("Plan is invalid"), Plan.bValid);
			TestTrue(TEXT("Failure message recorded"), Plan.Messages.Num() > 0);
		});

		It("should reject an invalid required Base Albedo asset", [this]()
		{
			FPlannerTestContext Context;
			FMIForgeVertexPaintLayerStack Stack =
				Context.MakeVertexPaintLayerStack();
			FMIForgeTextureInfo* BaseAlbedo =
				Stack.BaseLayer.AssignedTextureSet->Textures.Find(
					EMIForgeTextureType::Albedo);
			TestNotNull(TEXT("Base Albedo test fixture"), BaseAlbedo);
			if (!BaseAlbedo)
			{
				return;
			}
			BaseAlbedo->AssetData = FAssetData();

			FMIForgeVertexPaintGenerationOptions Options;
			Options.TargetPath = Context.TargetPath;

			const FMIForgeVertexPaintGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanVertexPaintGeneration(
					Stack,
					Options);

			TestFalse(TEXT("Plan is invalid"), Plan.bValid);
			TestTrue(TEXT("Failure message recorded"), Plan.Messages.Num() > 0);
		});

		It("should add the Vertex Paint prefix to a custom name", [this]()
		{
			FPlannerTestContext Context;
			const FMIForgeVertexPaintLayerStack Stack =
				Context.MakeVertexPaintLayerStack();
			FMIForgeVertexPaintGenerationOptions Options;
			Options.TargetPath = Context.TargetPath;
			Options.MaterialInstanceName = TEXT("CustomVertex");

			const FMIForgeVertexPaintGenerationPlan Plan =
				FMIForgeGenerationPlanner().PlanVertexPaintGeneration(
					Stack,
					Options);

			TestTrue(TEXT("Plan is valid"), Plan.bValid);
			TestEqual(
				TEXT("Custom asset name"),
				Plan.DesiredAssetName,
				FString(TEXT("MI_VP_CustomVertex")));
		});
	});
}

#endif
