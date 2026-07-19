#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "UIs/MIForgeMainTabViewModel.h"

namespace MIForgeMainTabViewModelSpec
{
	TSharedPtr<FMIForgeTextureInfo> MakeTexture(
		const FString& SetName,
		const EMIForgeTextureType TextureType)
	{
		TSharedPtr<FMIForgeTextureInfo> Texture = MakeShared<FMIForgeTextureInfo>();
		Texture->AssetName = FString::Printf(
			TEXT("T_%s_%d"),
			*SetName,
			static_cast<int32>(TextureType));
		Texture->BaseName = SetName;
		Texture->TextureType = TextureType;
		return Texture;
	}

	TSharedPtr<FMIForgeTextureSet> MakeTextureSet(
		const FString& SetName,
		const TArray<EMIForgeTextureType>& TextureTypes)
	{
		TSharedPtr<FMIForgeTextureSet> TextureSet = MakeShared<FMIForgeTextureSet>();
		TextureSet->SetName = SetName;

		for (const EMIForgeTextureType TextureType : TextureTypes)
		{
			TextureSet->Textures.Add(TextureType, *MakeTexture(SetName, TextureType));
		}

		return TextureSet;
	}
}

DEFINE_SPEC(
	FMIForgeMainTabViewModelSpec,
	"MIForge.Unit.MainTabViewModel",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

void FMIForgeMainTabViewModelSpec::Define()
{
	using namespace MIForgeMainTabViewModelSpec;

	Describe("Initialization", [this]()
	{
		It("should initialize the default state and Vertex Paint layer definitions", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();

			TestTrue(
				TEXT("Default preset is Standard"),
				ViewModel->GetPreset() == EMIForgeGenerationPreset::Standard);
			TestTrue(
				TEXT("Default input mode is Individual Textures"),
				ViewModel->GetInputMode() == EMIForgeInputMode::IndividualTextures);
			TestEqual(TEXT("Selected texture count"), ViewModel->GetSelectedTextures().Num(), 0);
			TestEqual(TEXT("Selected set count"), ViewModel->GetSelectedTextureSets().Num(), 0);

			const FMIForgeVertexPaintLayerSlot* Base =
				ViewModel->FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer::Base);
			const FMIForgeVertexPaintLayerSlot* LayerR =
				ViewModel->FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer::LayerR);
			const FMIForgeVertexPaintLayerSlot* LayerG =
				ViewModel->FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer::LayerG);
			const FMIForgeVertexPaintLayerSlot* LayerB =
				ViewModel->FindVertexPaintLayerSlot(EMIForgeVertexPaintLayer::LayerB);

			TestTrue(TEXT("Base definition exists"), Base != nullptr);
			TestTrue(TEXT("Layer R definition exists"), LayerR != nullptr);
			TestTrue(TEXT("Layer G definition exists"), LayerG != nullptr);
			TestTrue(TEXT("Layer B definition exists"), LayerB != nullptr);
			if (Base && LayerR && LayerG && LayerB)
			{
				TestTrue(TEXT("Base is required"), Base->bRequired);
				TestTrue(TEXT("Layer R is required"), LayerR->bRequired);
				TestFalse(TEXT("Layer G is optional"), LayerG->bRequired);
				TestFalse(TEXT("Layer B is optional"), LayerB->bRequired);
			}
		});
	});

	Describe("Selection", [this]()
	{
		It("should de-duplicate selections and broadcast only for actual changes", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureInfo> Texture =
				MakeTexture(TEXT("Rock"), EMIForgeTextureType::Albedo);
			int32 SelectionChangedCount = 0;
			ViewModel->OnSelectionChanged.AddLambda(
				[&SelectionChangedCount]()
				{
					++SelectionChangedCount;
				});

			ViewModel->SelectTexture(Texture);
			ViewModel->SelectTexture(Texture);

			TestEqual(TEXT("One selected texture"), ViewModel->GetSelectedTextures().Num(), 1);
			TestTrue(TEXT("Texture is selected"), ViewModel->IsTextureSelected(Texture));
			TestEqual(TEXT("One selection notification"), SelectionChangedCount, 1);

			ViewModel->UnselectTexture(Texture);
			ViewModel->UnselectTexture(Texture);

			TestEqual(TEXT("Selection is empty"), ViewModel->GetSelectedTextures().Num(), 0);
			TestEqual(TEXT("Two selection notifications"), SelectionChangedCount, 2);
		});

		It("should clear the inactive selection when the input mode changes", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureInfo> Texture =
				MakeTexture(TEXT("Rock"), EMIForgeTextureType::Albedo);
			const TSharedPtr<FMIForgeTextureSet> TextureSet =
				MakeTextureSet(TEXT("Rock"), { EMIForgeTextureType::Albedo });

			ViewModel->SelectTexture(Texture);
			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			TestEqual(
				TEXT("Individual texture selection is cleared"),
				ViewModel->GetSelectedTextures().Num(),
				0);

			ViewModel->SelectTextureSet(TextureSet);
			ViewModel->SetInputMode(EMIForgeInputMode::IndividualTextures);
			TestEqual(
				TEXT("Texture set selection is cleared"),
				ViewModel->GetSelectedTextureSets().Num(),
				0);
		});
	});

	Describe("BuildGenerationTextureSets", [this]()
	{
		It("should build grouped sets from selected individual textures", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			ViewModel->SetSelectedTextures({
				MakeTexture(TEXT("Rock"), EMIForgeTextureType::Albedo),
				MakeTexture(TEXT("Rock"), EMIForgeTextureType::Normal),
				MakeTexture(TEXT("Rock"), EMIForgeTextureType::ORM)
			});

			const TArray<TSharedPtr<FMIForgeTextureSet>> Result =
				ViewModel->BuildGenerationTextureSets();

			TestEqual(TEXT("Built set count"), Result.Num(), 1);
			if (Result.Num() == 1 && Result[0].IsValid())
			{
				TestEqual(TEXT("Built set name"), Result[0]->SetName, FString(TEXT("Rock")));
				TestEqual(TEXT("Built texture count"), Result[0]->Textures.Num(), 3);
			}
		});

		It("should return the selected sets in Texture Sets mode", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureSet> Rock =
				MakeTextureSet(TEXT("Rock"), { EMIForgeTextureType::Albedo });
			const TSharedPtr<FMIForgeTextureSet> Mud =
				MakeTextureSet(TEXT("Mud"), { EMIForgeTextureType::Albedo });

			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			ViewModel->SetSelectedTextureSets({ Rock, Mud });
			const TArray<TSharedPtr<FMIForgeTextureSet>> Result =
				ViewModel->BuildGenerationTextureSets();

			TestEqual(TEXT("Selected set count"), Result.Num(), 2);
			TestTrue(TEXT("Returns Rock by identity"), Result.Contains(Rock));
			TestTrue(TEXT("Returns Mud by identity"), Result.Contains(Mud));
		});
	});

	Describe("Validation", [this]()
	{
		It("should refresh Standard validation after selection and option changes", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureSet> TextureSet = MakeTextureSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::ORM
				});

			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			ViewModel->SelectTextureSet(TextureSet);

			TestEqual(TEXT("One Standard set is ready"), ViewModel->GetValidationSummary().ReadyToCreateCount, 1);
			TestEqual(TEXT("No Standard warning"), ViewModel->GetValidationSummary().SetsWithWarnings, 0);

			ViewModel->SetUseEmissiveTextures(true);

			TestEqual(TEXT("Set remains ready"), ViewModel->GetValidationSummary().ReadyToCreateCount, 1);
			TestEqual(TEXT("Missing Emissive creates a warning"), ViewModel->GetValidationSummary().SetsWithWarnings, 1);
			TestEqual(TEXT("One optional texture is missing"), ViewModel->GetValidationSummary().MissingOptionalTextureCount, 1);
		});

		It("should not require an Emissive texture for the RGB Emissive Channel switch", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureSet> TextureSet = MakeTextureSet(
				TEXT("Rock"),
				{
					EMIForgeTextureType::Albedo,
					EMIForgeTextureType::Normal,
					EMIForgeTextureType::RGB
				});

			ViewModel->SetPreset(EMIForgeGenerationPreset::RGBMask);
			ViewModel->SetUseBaseORMTexture(false);
			ViewModel->SetEnableEmissiveChannel(true);
			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			ViewModel->SelectTextureSet(TextureSet);

			TestEqual(TEXT("One RGB set is ready"), ViewModel->GetValidationSummary().ReadyToCreateCount, 1);
			TestEqual(TEXT("No required texture is missing"), ViewModel->GetValidationSummary().MissingRequiredTextureCount, 0);
			TestEqual(TEXT("No optional Emissive texture is reported"), ViewModel->GetValidationSummary().MissingOptionalTextureCount, 0);
		});
	});

	Describe("Vertex Paint", [this]()
	{
		It("should assign selected sets in layer order and refresh validation when layers are cleared", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			const TSharedPtr<FMIForgeTextureSet> Base =
				MakeTextureSet(TEXT("Base"), { EMIForgeTextureType::Albedo });
			const TSharedPtr<FMIForgeTextureSet> LayerR =
				MakeTextureSet(TEXT("R"), { EMIForgeTextureType::Albedo });
			int32 VertexPaintChangedCount = 0;
			ViewModel->OnVertexPaintChanged.AddLambda(
				[&VertexPaintChangedCount]()
				{
					++VertexPaintChangedCount;
				});

			ViewModel->SetPreset(EMIForgeGenerationPreset::VertexPainting);
			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			ViewModel->SetSelectedTextureSets({ Base, LayerR });
			FText Error;
			const bool bAssigned = ViewModel->AssignSelectedTextureSetsToVertexLayers(Error);

			TestTrue(TEXT("Assignment succeeds"), bAssigned);
			TestTrue(TEXT("Assignment error is empty"), Error.IsEmpty());
			TestTrue(
				TEXT("Base receives the first selected set"),
				ViewModel->GetVertexPaintLayerStack().BaseLayer.AssignedTextureSet == Base);
			TestTrue(
				TEXT("Layer R receives the second selected set"),
				ViewModel->GetVertexPaintLayerStack().LayerR.AssignedTextureSet == LayerR);
			TestTrue(TEXT("Required layers can generate"), ViewModel->GetVertexPaintValidationResult().bCanGenerate);
			TestEqual(TEXT("One Vertex Paint notification"), VertexPaintChangedCount, 1);

			ViewModel->ClearVertexPaintLayer(EMIForgeVertexPaintLayer::LayerR);

			TestFalse(TEXT("Missing Layer R blocks generation"), ViewModel->GetVertexPaintValidationResult().bCanGenerate);
			TestEqual(TEXT("Two Vertex Paint notifications"), VertexPaintChangedCount, 2);
		});

		It("should reject assigning more than four selected sets without changing layers", [this]()
		{
			const TSharedRef<FMIForgeMainTabViewModel> ViewModel =
				MakeShared<FMIForgeMainTabViewModel>();
			ViewModel->SetInputMode(EMIForgeInputMode::TextureSets);
			ViewModel->SetSelectedTextureSets({
				MakeTextureSet(TEXT("One"), { EMIForgeTextureType::Albedo }),
				MakeTextureSet(TEXT("Two"), { EMIForgeTextureType::Albedo }),
				MakeTextureSet(TEXT("Three"), { EMIForgeTextureType::Albedo }),
				MakeTextureSet(TEXT("Four"), { EMIForgeTextureType::Albedo }),
				MakeTextureSet(TEXT("Five"), { EMIForgeTextureType::Albedo })
			});
			int32 VertexPaintChangedCount = 0;
			ViewModel->OnVertexPaintChanged.AddLambda(
				[&VertexPaintChangedCount]()
				{
					++VertexPaintChangedCount;
				});

			FText Error;
			const bool bAssigned = ViewModel->AssignSelectedTextureSetsToVertexLayers(Error);

			TestFalse(TEXT("Assignment is rejected"), bAssigned);
			TestFalse(TEXT("Assignment explains the error"), Error.IsEmpty());
			TestFalse(TEXT("Base remains unassigned"), ViewModel->GetVertexPaintLayerStack().BaseLayer.IsAssigned());
			TestEqual(TEXT("No Vertex Paint notification"), VertexPaintChangedCount, 0);
		});
	});
}

#endif
