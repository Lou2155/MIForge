// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetData.h"
#include "EditorAssetLibrary.h"
#include "Engine/Texture2D.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"

#include "MIForgeMaterialInstanceGenerator.h"
#include "MIForgeTypes.h"

namespace MIForgeMaterialInstanceGeneratorSpec
{
	class FGeneratorTestContext
	{
	public:
		FGeneratorTestContext()
			: TargetPath(FString::Printf(
				TEXT("/Game/MIForgeAutomation/%s"),
				*FGuid::NewGuid().ToString(EGuidFormats::Digits)))
		{
			UEditorAssetLibrary::MakeDirectory(TargetPath);
		}

		~FGeneratorTestContext()
		{
			for (const FString& AssetPath : GeneratedAssetPaths)
			{
				if (UEditorAssetLibrary::DoesAssetExist(AssetPath))
				{
					UEditorAssetLibrary::DeleteAsset(AssetPath);
				}
			}

			UEditorAssetLibrary::DeleteDirectory(TargetPath);

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
				TEXT("/Engine/Transient/MIForgeGeneratorTests/%s"),
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

		void Track(const FMIForgeGenerationResult& Result)
		{
			for (const UObject* Asset : Result.AffectedAssets)
			{
				if (Asset)
				{
					GeneratedAssetPaths.Add(Asset->GetPathName());
				}
			}
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
		TSet<FString> GeneratedAssetPaths;
	};

	FMIForgeGenerationOptions MakeStandardOptions(const FString& TargetPath)
	{
		FMIForgeGenerationOptions Options;
		Options.Preset = EMIForgeGenerationPreset::Standard;
		Options.TargetPath = TargetPath;
		Options.IfMIExists = EIfMIExistsOption::Skip;
		return Options;
	}

	FMIForgeGenerationOptions MakeDecalOptions(const FString& TargetPath)
	{
		FMIForgeGenerationOptions Options;
		Options.Preset = EMIForgeGenerationPreset::Decal;
		Options.TargetPath = TargetPath;
		Options.IfMIExists = EIfMIExistsOption::Skip;
		return Options;
	}

	FMIForgeVertexPaintLayerStack MakeVertexPaintLayerStack(
		FGeneratorTestContext& Context)
	{
		FMIForgeVertexPaintLayerStack Stack;

		Stack.BaseLayer.Layer = EMIForgeVertexPaintLayer::Base;
		Stack.BaseLayer.DisplayName = TEXT("Base");
		Stack.BaseLayer.ChannelName = TEXT("Base");
		Stack.BaseLayer.bRequired = true;
		Stack.BaseLayer.AssignedTextureSet = Context.MakeTextureSet(
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
		Stack.LayerR.AssignedTextureSet = Context.MakeTextureSet(
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
}

DEFINE_SPEC(
	FMIForgeMaterialInstanceGeneratorSpec,
	"MIForge.Integration.MaterialInstanceGenerator",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeMaterialInstanceGeneratorSpec::Define()
{
	using namespace MIForgeMaterialInstanceGeneratorSpec;

	Describe("GenerateMaterialInstances", [this]()
	{
		It("should reject an invalid texture set without creating an asset", [this]()
		{
			FGeneratorTestContext Context;
			const FMIForgeGenerationOptions Options =
				MakeStandardOptions(Context.TargetPath);
			const TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets = { nullptr };

			const FMIForgeGenerationResult Result =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);

			TestEqual(TEXT("Failed count"), Result.FailedCount, 1);
			TestEqual(TEXT("Created count"), Result.CreatedCount, 0);
			TestEqual(TEXT("Created asset count"), Result.CreatedAssets.Num(), 0);
		});

		It("should create and then skip an existing Standard material instance", [this]()
		{

			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("StandardSkip"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets = { TextureSet };
			const FMIForgeGenerationOptions Options =
				MakeStandardOptions(Context.TargetPath);

			const FMIForgeGenerationResult FirstResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);
			Context.Track(FirstResult);

			const FMIForgeGenerationResult SecondResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);

			TestEqual(TEXT("First created count"), FirstResult.CreatedCount, 1);
			TestEqual(TEXT("First failed count"), FirstResult.FailedCount, 0);
			TestTrue(
				TEXT("Standard asset exists"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_StandardSkip"))));
			TestEqual(TEXT("Second skipped count"), SecondResult.SkippedCount, 1);
			TestEqual(TEXT("Second created count"), SecondResult.CreatedCount, 0);
			TestEqual(TEXT("Second updated count"), SecondResult.UpdatedCount, 0);
		});

		It("should overwrite an existing Standard material instance", [this]()
		{

			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("StandardOverwrite"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets = { TextureSet };
			FMIForgeGenerationOptions Options =
				MakeStandardOptions(Context.TargetPath);

			const FMIForgeGenerationResult FirstResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);
			Context.Track(FirstResult);

			Options.IfMIExists = EIfMIExistsOption::Overwrite;
			const FMIForgeGenerationResult SecondResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);
			Context.Track(SecondResult);

			TestEqual(TEXT("First created count"), FirstResult.CreatedCount, 1);
			TestEqual(TEXT("Overwrite updated count"), SecondResult.UpdatedCount, 1);
			TestEqual(TEXT("Overwrite created count"), SecondResult.CreatedCount, 0);
			TestEqual(TEXT("Overwrite failed count"), SecondResult.FailedCount, 0);
			TestEqual(
				TEXT("Overwrite affected asset count"),
				SecondResult.AffectedAssets.Num(),
				1);
		});

		It("should create a uniquely named Standard material instance", [this]()
		{

			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("StandardUnique"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			const TArray<TSharedPtr<FMIForgeTextureSet>> TextureSets = { TextureSet };
			FMIForgeGenerationOptions Options =
				MakeStandardOptions(Context.TargetPath);

			const FMIForgeGenerationResult FirstResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);
			Context.Track(FirstResult);

			Options.IfMIExists = EIfMIExistsOption::CreateUnique;
			const FMIForgeGenerationResult SecondResult =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					TextureSets,
					Options);
			Context.Track(SecondResult);

			TestEqual(TEXT("First created count"), FirstResult.CreatedCount, 1);
			TestEqual(TEXT("Unique created count"), SecondResult.CreatedCount, 1);
			TestEqual(TEXT("Unique failed count"), SecondResult.FailedCount, 0);
			TestEqual(
				TEXT("Unique created asset count"),
				SecondResult.CreatedAssets.Num(),
				1);

			if (SecondResult.CreatedAssets.Num() == 1 && SecondResult.CreatedAssets[0])
			{
				TestTrue(
					TEXT("Unique asset keeps the base name prefix"),
					SecondResult.CreatedAssets[0]->GetName().StartsWith(
						TEXT("MI_StandardUnique")));
				TestTrue(
					TEXT("Unique asset name differs from the original"),
					SecondResult.CreatedAssets[0]->GetName() !=
						TEXT("MI_StandardUnique"));
			}
		});

		It("should generate RGB Mask with Emissive Channel enabled and no Emissive texture", [this]()
		{
			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("RGBEmissiveChannel"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::RGB
					});

			FMIForgeGenerationOptions Options;
			Options.Preset = EMIForgeGenerationPreset::RGBMask;
			Options.TargetPath = Context.TargetPath;
			Options.IfMIExists = EIfMIExistsOption::Skip;
			Options.bUseBaseORMTexture = false;
			Options.bEnableEmissiveChannel = true;
			Options.bUseDetailNormalTextureRGB = false;

			const FMIForgeGenerationResult Result =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					{ TextureSet },
					Options);
			Context.Track(Result);

			TestEqual(TEXT("Created count"), Result.CreatedCount, 1);
			TestEqual(TEXT("Failed count"), Result.FailedCount, 0);
			TestTrue(
				TEXT("RGB material instance exists"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_RGB_RGBEmissiveChannel"))));
		});

		It("should generate an Albedo-only Decal material instance", [this]()
		{
			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("Graffiti"),
					{ EMIForgeTextureType::Albedo });
			const FMIForgeGenerationOptions Options =
				MakeDecalOptions(Context.TargetPath);

			const FMIForgeGenerationResult Result =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					{ TextureSet },
					Options);
			Context.Track(Result);

			TestEqual(TEXT("Created count"), Result.CreatedCount, 1);
			TestEqual(TEXT("Failed count"), Result.FailedCount, 0);
			TestEqual(
				TEXT("Created asset count"),
				Result.CreatedAssets.Num(),
				1);
			TestTrue(
				TEXT("Decal material instance exists"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_Decal_Graffiti"))));

			UMaterialInstanceConstant* MaterialInstance =
				Result.CreatedAssets.Num() == 1
				? Cast<UMaterialInstanceConstant>(Result.CreatedAssets[0])
				: nullptr;
			TestNotNull(TEXT("Created asset is a material instance"), MaterialInstance);

			if (MaterialInstance)
			{
				const FMIForgeTextureInfo* Albedo =
					TextureSet->Textures.Find(EMIForgeTextureType::Albedo);
				UTexture* ExpectedAlbedo = Albedo
					? Cast<UTexture>(Albedo->AssetData.GetAsset())
					: nullptr;

				TestTrue(
					TEXT("Albedo texture is assigned"),
					UMaterialEditingLibrary::
						GetMaterialInstanceTextureParameterValue(
							MaterialInstance,
							FName(TEXT("Albedo"))) ==
					ExpectedAlbedo);
				TestFalse(
					TEXT("Normal switch is disabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseNormal?"))));
				TestFalse(
					TEXT("ORM switch is disabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseORM?"))));
				TestFalse(
					TEXT("Orientation Mask switch is disabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseOrientationMask?"))));
			}
		});

		It("should assign requested Decal textures and enable their switches", [this]()
		{
			FGeneratorTestContext Context;
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				Context.MakeTextureSet(
					TEXT("WetMark"),
					{
						EMIForgeTextureType::Albedo,
						EMIForgeTextureType::Normal,
						EMIForgeTextureType::ORM
					});
			FMIForgeGenerationOptions Options =
				MakeDecalOptions(Context.TargetPath);
			Options.bUseDecalNormal = true;
			Options.bUseDecalORM = true;
			Options.bUseOrientationMask = true;

			const FMIForgeGenerationResult Result =
				FMIForgeMaterialInstanceGenerator().GenerateMaterialInstances(
					{ TextureSet },
					Options);
			Context.Track(Result);

			TestEqual(TEXT("Created count"), Result.CreatedCount, 1);
			TestEqual(TEXT("Failed count"), Result.FailedCount, 0);

			UMaterialInstanceConstant* MaterialInstance =
				Result.CreatedAssets.Num() == 1
				? Cast<UMaterialInstanceConstant>(Result.CreatedAssets[0])
				: nullptr;
			TestNotNull(TEXT("Created asset is a material instance"), MaterialInstance);

			if (MaterialInstance)
			{
				auto GetExpectedTexture =
					[&TextureSet](EMIForgeTextureType TextureType)
					{
						const FMIForgeTextureInfo* TextureInfo =
							TextureSet->Textures.Find(TextureType);
						return TextureInfo
							? Cast<UTexture>(
								TextureInfo->AssetData.GetAsset())
							: nullptr;
					};

				TestTrue(
					TEXT("Normal texture is assigned"),
					UMaterialEditingLibrary::
						GetMaterialInstanceTextureParameterValue(
							MaterialInstance,
							FName(TEXT("Normal"))) ==
					GetExpectedTexture(EMIForgeTextureType::Normal));
				TestTrue(
					TEXT("ORM texture is assigned"),
					UMaterialEditingLibrary::
						GetMaterialInstanceTextureParameterValue(
							MaterialInstance,
							FName(TEXT("ORM"))) ==
					GetExpectedTexture(EMIForgeTextureType::ORM));
				TestTrue(
					TEXT("Normal switch is enabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseNormal?"))));
				TestTrue(
					TEXT("ORM switch is enabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseORM?"))));
				TestTrue(
					TEXT("Orientation Mask switch is enabled"),
					UMaterialEditingLibrary::
						GetMaterialInstanceStaticSwitchParameterValue(
							MaterialInstance,
							FName(TEXT("UseOrientationMask?"))));
			}
		});
	});

	Describe("GenerateVertexPaintMaterialInstance", [this]()
	{
		It("should use the default and custom Vertex Paint material instance names", [this]()
		{
			FGeneratorTestContext Context;
			const FMIForgeVertexPaintLayerStack Stack =
				MakeVertexPaintLayerStack(Context);

			FMIForgeVertexPaintGenerationOptions Options;
			Options.TargetPath = Context.TargetPath;
			Options.IfMIExists = EIfMIExistsOption::Skip;

			const FMIForgeGenerationResult DefaultNameResult =
				FMIForgeMaterialInstanceGenerator()
					.GenerateVertexPaintMaterialInstance(Stack, Options);
			Context.Track(DefaultNameResult);

			Options.MaterialInstanceName = TEXT("CustomVertex");
			const FMIForgeGenerationResult CustomNameResult =
				FMIForgeMaterialInstanceGenerator()
					.GenerateVertexPaintMaterialInstance(Stack, Options);
			Context.Track(CustomNameResult);

			TestEqual(
				TEXT("Default name created count"),
				DefaultNameResult.CreatedCount,
				1);
			TestTrue(
				TEXT("Default Vertex Paint asset exists"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_VP_VertexBase"))));
			TestEqual(
				TEXT("Custom name created count"),
				CustomNameResult.CreatedCount,
				1);
			TestTrue(
				TEXT("Custom Vertex Paint asset exists"),
				UEditorAssetLibrary::DoesAssetExist(
					Context.ObjectPath(TEXT("MI_VP_CustomVertex"))));
		});
	});
}

#endif
