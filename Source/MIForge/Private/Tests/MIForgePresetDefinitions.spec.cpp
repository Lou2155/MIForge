#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Presets/MIForgePresetDefinitions.h"

namespace
{
	const FMIForgeTextureBinding* FindTextureBinding(
		const FMIForgeMaterialPresetDefinition& Definition,
		EMIForgeTextureType TextureType)
	{
		return Definition.TextureBindings.FindByPredicate(
			[TextureType](const FMIForgeTextureBinding& Binding)
			{
				return Binding.TextureType == TextureType;
			});
	}

	const FMIForgeStaticSwitchBinding* FindSwitchBinding(
		const FMIForgeMaterialPresetDefinition& Definition,
		EMIForgePresetOptions Option)
	{
		return Definition.StaticSwitchBindings.FindByPredicate(
			[Option](const FMIForgeStaticSwitchBinding& Binding)
			{
				return Binding.PresetOption == Option;
			});
	}

	const FMIForgeVertexPaintLayerDefinition* FindLayer(
		const FMIForgeVertexPaintPresetDefinition& Definition,
		EMIForgeVertexPaintLayer Layer)
	{
		return Definition.Layers.FindByPredicate(
			[Layer](const FMIForgeVertexPaintLayerDefinition& LayerDefinition)
			{
				return LayerDefinition.Layer == Layer;
			});
	}
}

DEFINE_SPEC(
	FMIForgePresetDefinitionsSpec,
	"MIForge.Unit.PresetDefinitions",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter)

	void FMIForgePresetDefinitionsSpec::Define()
{
	Describe("Standard", [this]()
		{
			It("should describe the Standard material contract", [this]()
				{
					const FMIForgeMaterialPresetDefinition& Definition =
						FMIForgePresetDefinitions::GetStandard();

					TestTrue(
						TEXT("Correct preset"),
						Definition.Preset ==
						EMIForgeGenerationPreset::Standard);

					TestEqual(
						TEXT("Correct parent material"),
						Definition.ParentMaterialPath.ToString(),
						FString(TEXT(
							"/MIForge/MasterMaterialPresets/"
							"MM_Standard.MM_Standard")));

					TestEqual(
						TEXT("Texture binding count"),
						Definition.TextureBindings.Num(),
						5);

					TestEqual(
						TEXT("Static switch binding count"),
						Definition.StaticSwitchBindings.Num(),
						3);

					auto CheckTextureBinding =
						[this, &Definition](
							EMIForgeTextureType TextureType,
							const TCHAR* ExpectedParameter,
							EMIForgeRequirement ExpectedRequirement,
							EMIForgePresetOptions ExpectedOption)
						{
							const FMIForgeTextureBinding* Binding =
								FindTextureBinding(
									Definition,
									TextureType);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s texture binding exists"),
									ExpectedParameter);

							TestNotNull(
								*ExistsMessage,
								Binding);

							if (!Binding)
							{
								return;
							}

							const FString ParameterMessage =
								FString::Printf(
									TEXT("%s parameter name"),
									ExpectedParameter);

							TestEqual(
								*ParameterMessage,
								Binding->ParameterName.ToString(),
								FString(ExpectedParameter));

							const FString RequirementMessage =
								FString::Printf(
									TEXT("%s requirement"),
									ExpectedParameter);

							TestTrue(
								*RequirementMessage,
								Binding->Requirement ==
								ExpectedRequirement);

							const FString OptionMessage =
								FString::Printf(
									TEXT("%s activation option"),
									ExpectedParameter);

							TestTrue(
								*OptionMessage,
								Binding->PresetOption ==
								ExpectedOption);
						};

					CheckTextureBinding(
						EMIForgeTextureType::Albedo,
						TEXT("Albedo"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::Normal,
						TEXT("Normal"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::ORM,
						TEXT("ORM"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::Emissive,
						TEXT("Emissive"),
						EMIForgeRequirement::Optional,
						EMIForgePresetOptions::UseEmissiveTexture);

					CheckTextureBinding(
						EMIForgeTextureType::DetailNormal,
						TEXT("Detail Normal"),
						EMIForgeRequirement::Optional,
						EMIForgePresetOptions::UseDetailNormalTexture);

					auto CheckStaticSwitch =
						[this, &Definition](
							EMIForgePresetOptions Option,
							const TCHAR* ExpectedParameter)
						{
							const FMIForgeStaticSwitchBinding* Binding =
								FindSwitchBinding(
									Definition,
									Option);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s static switch exists"),
									ExpectedParameter);

							TestNotNull(
								*ExistsMessage,
								Binding);

							if (!Binding)
							{
								return;
							}

							const FString ParameterMessage =
								FString::Printf(
									TEXT("%s static switch parameter"),
									ExpectedParameter);

							TestEqual(
								*ParameterMessage,
								Binding->ParameterName.ToString(),
								FString(ExpectedParameter));
						};

					CheckStaticSwitch(
						EMIForgePresetOptions::UseTriplanar,
						TEXT("UseTriplanar?"));

					CheckStaticSwitch(
						EMIForgePresetOptions::UseEmissiveTexture,
						TEXT("UseEmissiveTex?"));

					CheckStaticSwitch(
						EMIForgePresetOptions::UseDetailNormalTexture,
						TEXT("UseDetailNormal?"));
				});
		});

	Describe("RGB Mask", [this]()
		{
			It("should describe the RGB material contract", [this]()
				{
					const FMIForgeMaterialPresetDefinition& Definition =
						FMIForgePresetDefinitions::GetRGBMask();

					TestTrue(
						TEXT("Correct preset"),
						Definition.Preset ==
						EMIForgeGenerationPreset::RGBMask);

					TestEqual(
						TEXT("Correct parent material"),
						Definition.ParentMaterialPath.ToString(),
						FString(TEXT(
							"/MIForge/MasterMaterialPresets/"
							"MM_RGBmasking.MM_RGBmasking")));

					TestEqual(
						TEXT("Texture binding count"),
						Definition.TextureBindings.Num(),
						5);

					TestEqual(
						TEXT("Static switch binding count"),
						Definition.StaticSwitchBindings.Num(),
						3);

					auto CheckTextureBinding =
						[this, &Definition](
							EMIForgeTextureType TextureType,
							const TCHAR* ExpectedParameter,
							EMIForgeRequirement ExpectedRequirement,
							EMIForgePresetOptions ExpectedOption)
						{
							const FMIForgeTextureBinding* Binding =
								FindTextureBinding(
									Definition,
									TextureType);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s texture binding exists"),
									ExpectedParameter);

							TestNotNull(
								*ExistsMessage,
								Binding);

							if (!Binding)
							{
								return;
							}

							const FString ParameterMessage =
								FString::Printf(
									TEXT("%s parameter name"),
									ExpectedParameter);

							TestEqual(
								*ParameterMessage,
								Binding->ParameterName.ToString(),
								FString(ExpectedParameter));

							const FString RequirementMessage =
								FString::Printf(
									TEXT("%s requirement"),
									ExpectedParameter);

							TestTrue(
								*RequirementMessage,
								Binding->Requirement ==
								ExpectedRequirement);

							const FString OptionMessage =
								FString::Printf(
									TEXT("%s activation option"),
									ExpectedParameter);

							TestTrue(
								*OptionMessage,
								Binding->PresetOption ==
								ExpectedOption);
						};

					CheckTextureBinding(
						EMIForgeTextureType::Albedo,
						TEXT("Albedo"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::Normal,
						TEXT("Base Normal"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::RGB,
						TEXT("RGB_Mask"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::None);

					CheckTextureBinding(
						EMIForgeTextureType::ORM,
						TEXT("ORM"),
						EMIForgeRequirement::Required,
						EMIForgePresetOptions::UseBaseORM);

					CheckTextureBinding(
						EMIForgeTextureType::DetailNormal,
						TEXT("Detail Normal"),
						EMIForgeRequirement::Optional,
						EMIForgePresetOptions::UseDetailNormalTexture);

					TestTrue(
						TEXT("RGB has no Emissive texture binding"),
						FindTextureBinding(
							Definition,
							EMIForgeTextureType::Emissive) ==
						nullptr);

					auto CheckStaticSwitch =
						[this, &Definition](
							EMIForgePresetOptions Option,
							const TCHAR* ExpectedParameter)
						{
							const FMIForgeStaticSwitchBinding* Binding =
								FindSwitchBinding(
									Definition,
									Option);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s static switch exists"),
									ExpectedParameter);

							TestNotNull(
								*ExistsMessage,
								Binding);

							if (!Binding)
							{
								return;
							}

							const FString ParameterMessage =
								FString::Printf(
									TEXT("%s static switch parameter"),
									ExpectedParameter);

							TestEqual(
								*ParameterMessage,
								Binding->ParameterName.ToString(),
								FString(ExpectedParameter));
						};

					CheckStaticSwitch(
						EMIForgePresetOptions::UseBaseORM,
						TEXT("UseBaseORM?"));

					CheckStaticSwitch(
						EMIForgePresetOptions::EnableEmissiveChannel,
						TEXT("EnableEmissiveChannel?"));

					CheckStaticSwitch(
						EMIForgePresetOptions::UseDetailNormalTexture,
						TEXT("UseDetailNormal?"));
				});
		});

	Describe("Vertex Paint", [this]()
		{
			It("should describe the Vertex Paint material contract", [this]()
				{
					const FMIForgeVertexPaintPresetDefinition& Definition =
						FMIForgePresetDefinitions::GetVertexPaint();

					TestEqual(
						TEXT("Correct parent material"),
						Definition.ParentMaterialPath.ToString(),
						FString(TEXT(
							"/MIForge/MasterMaterialPresets/"
							"MM_VertexPainting.MM_VertexPainting")));

					TestEqual(
						TEXT("Layer count"),
						Definition.Layers.Num(),
						4);

					auto CheckLayer =
						[this, &Definition](
							EMIForgeVertexPaintLayer Layer,
							const TCHAR* ExpectedDisplayName,
							const TCHAR* ExpectedChannelName,
							bool bExpectedRequired,
							int32 ExpectedTextureCount,
							const TCHAR* ExpectedEnabledSwitch)
						-> const FMIForgeVertexPaintLayerDefinition*
						{
							const FMIForgeVertexPaintLayerDefinition*
								LayerDefinition =
								FindLayer(
									Definition,
									Layer);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s layer exists"),
									ExpectedDisplayName);

							TestNotNull(
								*ExistsMessage,
								LayerDefinition);

							if (!LayerDefinition)
							{
								return nullptr;
							}

							const FString DisplayNameMessage =
								FString::Printf(
									TEXT("%s display name"),
									ExpectedDisplayName);

							TestEqual(
								*DisplayNameMessage,
								LayerDefinition->DisplayName,
								FString(ExpectedDisplayName));

							const FString ChannelNameMessage =
								FString::Printf(
									TEXT("%s channel name"),
									ExpectedDisplayName);

							TestEqual(
								*ChannelNameMessage,
								LayerDefinition->ChannelName,
								FString(ExpectedChannelName));

							const FString RequiredMessage =
								FString::Printf(
									TEXT("%s required state"),
									ExpectedDisplayName);

							TestTrue(
								*RequiredMessage,
								LayerDefinition->bRequired ==
								bExpectedRequired);

							const FString TextureCountMessage =
								FString::Printf(
									TEXT("%s texture parameter count"),
									ExpectedDisplayName);

							TestEqual(
								*TextureCountMessage,
								LayerDefinition->TextureParameters.Num(),
								ExpectedTextureCount);

							if (ExpectedEnabledSwitch)
							{
								const FString SwitchMessage =
									FString::Printf(
										TEXT("%s enabled switch"),
										ExpectedDisplayName);

								TestEqual(
									*SwitchMessage,
									LayerDefinition->
									EnabledSwitchParameter.ToString(),
									FString(ExpectedEnabledSwitch));
							}
							else
							{
								const FString SwitchMessage =
									FString::Printf(
										TEXT("%s has no enabled switch"),
										ExpectedDisplayName);

								TestTrue(
									*SwitchMessage,
									LayerDefinition->
									EnabledSwitchParameter.IsNone());
							}

							return LayerDefinition;
						};

					const FMIForgeVertexPaintLayerDefinition* BaseLayer =
						CheckLayer(
							EMIForgeVertexPaintLayer::Base,
							TEXT("Base Layer"),
							TEXT("Base"),
							true,
							4,
							nullptr);

					const FMIForgeVertexPaintLayerDefinition* LayerR =
						CheckLayer(
							EMIForgeVertexPaintLayer::LayerR,
							TEXT("Layer 01 / R"),
							TEXT("R"),
							true,
							4,
							nullptr);

					const FMIForgeVertexPaintLayerDefinition* LayerG =
						CheckLayer(
							EMIForgeVertexPaintLayer::LayerG,
							TEXT("Layer 02 / G"),
							TEXT("G"),
							false,
							4,
							TEXT("Enable G Channel?"));

					const FMIForgeVertexPaintLayerDefinition* LayerB =
						CheckLayer(
							EMIForgeVertexPaintLayer::LayerB,
							TEXT("Layer 03 / B"),
							TEXT("B"),
							false,
							3,
							TEXT("Enable B Channel?"));

					auto CheckLayerTexture =
						[this](
							const FMIForgeVertexPaintLayerDefinition*
							LayerDefinition,
							EMIForgeTextureType TextureType,
							const TCHAR* ExpectedParameter)
						{
							if (!LayerDefinition)
							{
								return;
							}

							const FName* Parameter =
								LayerDefinition->
								TextureParameters.Find(TextureType);

							const FString ExistsMessage =
								FString::Printf(
									TEXT("%s exists"),
									ExpectedParameter);

							TestNotNull(
								*ExistsMessage,
								Parameter);

							if (!Parameter)
							{
								return;
							}

							const FString ParameterMessage =
								FString::Printf(
									TEXT("%s matches"),
									ExpectedParameter);

							TestEqual(
								*ParameterMessage,
								Parameter->ToString(),
								FString(ExpectedParameter));
						};

					CheckLayerTexture(
						BaseLayer,
						EMIForgeTextureType::Albedo,
						TEXT("Layer01_Albedo"));

					CheckLayerTexture(
						BaseLayer,
						EMIForgeTextureType::Normal,
						TEXT("Layer01_Normal"));

					CheckLayerTexture(
						BaseLayer,
						EMIForgeTextureType::ORM,
						TEXT("Layer01_ORM"));

					CheckLayerTexture(
						BaseLayer,
						EMIForgeTextureType::Height,
						TEXT("Layer01_Height"));

					CheckLayerTexture(
						LayerR,
						EMIForgeTextureType::Albedo,
						TEXT("Layer02_Albedo"));

					CheckLayerTexture(
						LayerR,
						EMIForgeTextureType::Normal,
						TEXT("Layer02_Normal"));

					CheckLayerTexture(
						LayerR,
						EMIForgeTextureType::ORM,
						TEXT("Layer02_ORM"));

					CheckLayerTexture(
						LayerR,
						EMIForgeTextureType::Height,
						TEXT("Layer02_Height"));

					CheckLayerTexture(
						LayerG,
						EMIForgeTextureType::Albedo,
						TEXT("Layer03_Albedo"));

					CheckLayerTexture(
						LayerG,
						EMIForgeTextureType::Normal,
						TEXT("Layer03_Normal"));

					CheckLayerTexture(
						LayerG,
						EMIForgeTextureType::ORM,
						TEXT("Layer03_ORM"));

					CheckLayerTexture(
						LayerG,
						EMIForgeTextureType::Height,
						TEXT("Layer03_Height"));

					CheckLayerTexture(
						LayerB,
						EMIForgeTextureType::Albedo,
						TEXT("Layer04_Albedo"));

					CheckLayerTexture(
						LayerB,
						EMIForgeTextureType::Normal,
						TEXT("Layer04_Normal"));

					CheckLayerTexture(
						LayerB,
						EMIForgeTextureType::ORM,
						TEXT("Layer04_ORM"));

					if (LayerB)
					{
						TestFalse(
							TEXT("Layer B has no Height parameter"),
							LayerB->TextureParameters.Contains(
								EMIForgeTextureType::Height));
					}
				});
		});
}

#endif