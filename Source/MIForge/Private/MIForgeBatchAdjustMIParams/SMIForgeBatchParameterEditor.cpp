// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "MIForgeBatchAdjustMIParams/SMIForgeBatchParameterEditor.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SSearchBox.h"

#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ScopedTransaction.h"
#include "MIForgeUtilities.h"

void SMIForgeBatchParameterEditor::Construct(const FArguments& InArgs)
{
	ParameterModel = InArgs._ParameterModel;
	OwnerTab = InArgs._OwnerTab;

	for (FMIForgeBatchParameterRow& Row : ParameterModel.Parameters)
	{
		ParameterRows.Add(MakeShared<FMIForgeBatchParameterRow>(Row));
	}

	UMaterialInterface* ParentMaterial =
		ParameterModel.ParentGroup.RootParentMaterial.Get();

	const FString ParentName = ParentMaterial
		? ParentMaterial->GetName()
		: TEXT("Invalid Parent");

	ChildSlot
		[
			SNew(SBorder)
				.Padding(12.0f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString(
									FString::Printf(TEXT("Parent Material: %s"), *ParentName)
								))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13.0f))
						]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 6.0f)
						[
							SNew(STextBlock)
								.Text(FText::FromString(
									FString::Printf(
										TEXT("Actors: %d | Slots: %d | Unique MIs: %d"),
										ParameterModel.ParentGroup.ActorCount,
										ParameterModel.ParentGroup.SlotCount,
										ParameterModel.ParentGroup.UniqueMICCount
									)
								))
						]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f)
						[
							SNew(SSeparator)
						]
					+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SSearchBox)
										.HintText(FText::FromString(TEXT("Search parameters...")))
										.OnTextChanged_Lambda([this](const FText& NewText)
											{
												CurrentSearchText = NewText;
												RefreshParameterGroups();
											})
								]

							+ SVerticalBox::Slot()
								.FillHeight(1.0f)
								[
									SAssignNew(ParameterGroupScrollBox, SScrollBox)
								]
						]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Right)
						.Padding(0.0f, 10.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0.0f, 0.0f, 8.0f, 0.0f)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Apply")))
										.OnClicked(this, &SMIForgeBatchParameterEditor::OnApplyClicked)
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Close")))
										.OnClicked(this, &SMIForgeBatchParameterEditor::OnCancelClicked)
								]
						]
				]
		];

	RefreshParameterGroups();
}

//TSharedRef<ITableRow> SMIForgeBatchParameterEditor::GenerateParameterRow(
//	TSharedPtr<FMIForgeBatchParameterRow> Item,
//	const TSharedRef<STableViewBase>& OwnerTable
//)
//{
//	return SNew(STableRow<TSharedPtr<FMIForgeBatchParameterRow>>, OwnerTable)
//	[
//		SNew(SBorder)
//			.Padding(6.0f)
//			.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
//			[
//				SNew(SHorizontalBox)
//
//				+ SHorizontalBox::Slot()
//					.AutoWidth()
//					.VAlign(VAlign_Center)
//					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
//					[
//						SNew(SCheckBox)
//							.IsChecked_Lambda([Item]()
//								{
//									return Item->bApply
//										? ECheckBoxState::Checked
//										: ECheckBoxState::Unchecked;
//								})
//							.OnCheckStateChanged_Lambda([Item](ECheckBoxState NewState)
//								{
//									Item->bApply = NewState == ECheckBoxState::Checked;
//								})
//					]
//				+ SHorizontalBox::Slot()
//					.FillWidth(0.25f)
//					.VAlign(VAlign_Center)
//					[
//						SNew(STextBlock)
//							.Text(FText::FromName(Item->ParameterName))
//					]
//				+ SHorizontalBox::Slot()
//					.FillWidth(0.18f)
//					.VAlign(VAlign_Center)
//					[
//						SNew(STextBlock)
//							.Text(GetParameterTypeText(Item->Type))
//							.ColorAndOpacity(FLinearColor(0.65f, 0.65f, 0.65f, 1.0f))
//					]
//				+ SHorizontalBox::Slot()
//					.FillWidth(0.45f)
//					.VAlign(VAlign_Center)
//					[
//						GenerateValueWidget(Item)
//					]
//			]
//	];
//}

FReply SMIForgeBatchParameterEditor::OpenVectorColorPicker(
	TSharedPtr<FMIForgeBatchParameterRow> Item
)
{
	if (!Item.IsValid())
	{
		return FReply::Handled();
	}

	FColorPickerArgs PickerArgs;
	PickerArgs.InitialColor = Item->VectorValue;
	PickerArgs.ParentWidget = AsShared();
	PickerArgs.bUseAlpha = true;
	PickerArgs.bOnlyRefreshOnOk = false;
	PickerArgs.bOnlyRefreshOnMouseUp = true;
	PickerArgs.bExpandAdvancedSection = true;

	PickerArgs.OnColorCommitted =
		FOnLinearColorValueChanged::CreateLambda(
			[Item](FLinearColor NewColor)
			{
				Item->VectorValue = NewColor;
			}
		);

	OpenColorPicker(PickerArgs);

	return FReply::Handled();
}

TSharedRef<SWidget> SMIForgeBatchParameterEditor::GenerateValueWidget(
	TSharedPtr<FMIForgeBatchParameterRow> Item
)
{
	if (!Item.IsValid())
	{
		return SNew(STextBlock).Text(FText::FromString(TEXT("Invalid")));
	}

	if (Item->Type == EMIForgeBatchParameterType::Scalar)
	{
		return SNew(SSpinBox<float>)
			.Value_Lambda([Item]()
				{
					return Item->ScalarValue;
				})
			.OnValueChanged_Lambda([Item](float NewValue)
				{
					Item->ScalarValue = NewValue;
					Item->bHasMixedValue = false;
				})
			.OnGetDisplayValue_Lambda([Item](float CurrentValue) -> TOptional<FText>
				{
					if (Item->bHasMixedValue)
					{
						return FText::FromString(TEXT("Multiple Values"));
					}

					return TOptional<FText>();
				});
	}
	if (Item->Type == EMIForgeBatchParameterType::StaticSwitch)
	{
		return SNew(SCheckBox)
			.IsChecked_Lambda([Item]()
				{
					return Item->BoolValue
						? ECheckBoxState::Checked
						: ECheckBoxState::Unchecked;
				})
			.OnCheckStateChanged_Lambda([Item](ECheckBoxState NewState)
				{
					Item->BoolValue = NewState == ECheckBoxState::Checked;
				})
			[
				SNew(STextBlock)
					.Text_Lambda([Item]()
						{
							if (Item->bHasMixedValue)
							{
								return FText::FromString(TEXT("Multiple Values"));
							}

							return Item->BoolValue
								? FText::FromString(TEXT("True"))
								: FText::FromString(TEXT("False"));
						})
			];
	}
	if (Item->Type == EMIForgeBatchParameterType::Vector)
	{
		return SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SColorBlock)
					.Color_Lambda([Item]()
						{
							return Item->VectorValue;
						})
					.Size(FVector2D(36.0f, 18.0f))
					.ShowBackgroundForAlpha(true)
					.OnMouseButtonDown_Lambda([this, Item](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
						{
							return OpenVectorColorPicker(Item);
						})
			]

		+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text_Lambda([Item]()
						{
							if (Item->bHasMixedValue)
							{
								return FText::FromString(TEXT("Multiple Values"));
							}

							return FText::FromString(Item->VectorValue.ToString());
						})
					.ColorAndOpacity_Lambda([Item]()
						{
							return Item->bHasMixedValue
								? FLinearColor(1.0f, 0.75f, 0.25f, 1.0f)
								: FLinearColor::White;
						})
			];
	}
	if (Item->Type == EMIForgeBatchParameterType::Texture)
	{
		return SNew(SObjectPropertyEntryBox)
			.AllowedClass(UTexture::StaticClass())
			.ObjectPath_Lambda([Item]() -> FString
				{
					if (UTexture* Texture = Item->TextureValue.Get())
					{
						return Texture->GetPathName();
					}

					return FString();
				})
			.OnObjectChanged_Lambda([Item](const FAssetData& AssetData)
				{
					Item->TextureValue = Cast<UTexture>(AssetData.GetAsset());
				});
	}

	return SNew(STextBlock)
		.Text(GetValueText(Item))
		.ColorAndOpacity_Lambda([Item]()
			{
				return Item->bHasMixedValue
					? FLinearColor(1.0f, 0.75f, 0.25f, 1.0f)
					: FLinearColor::White;
			});
}

FText SMIForgeBatchParameterEditor::GetParameterTypeText(EMIForgeBatchParameterType Type) const
{
	switch (Type)
	{
	case EMIForgeBatchParameterType::Scalar:
		return FText::FromString(TEXT("Scalar"));

	case EMIForgeBatchParameterType::Vector:
		return FText::FromString(TEXT("Vector"));

	case EMIForgeBatchParameterType::Texture:
		return FText::FromString(TEXT("Texture"));

	case EMIForgeBatchParameterType::StaticSwitch:
		return FText::FromString(TEXT("Switch"));

	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FText SMIForgeBatchParameterEditor::GetValueText(TSharedPtr<FMIForgeBatchParameterRow> Item) const
{
	if (!Item.IsValid())
	{
		return FText::GetEmpty();
	}

	if (Item->bHasMixedValue)
	{
		return FText::FromString(TEXT("Multiple Values"));
	}

	switch (Item->Type)
	{
	case EMIForgeBatchParameterType::Scalar:
		return FText::AsNumber(Item->ScalarValue);

	case EMIForgeBatchParameterType::Vector:
		return FText::FromString(Item->VectorValue.ToString());

	case EMIForgeBatchParameterType::Texture:
		return Item->TextureValue.IsValid()
			? FText::FromString(Item->TextureValue->GetName())
			: FText::FromString(TEXT("None"));

	case EMIForgeBatchParameterType::StaticSwitch:
		return Item->BoolValue
			? FText::FromString(TEXT("True"))
			: FText::FromString(TEXT("False"));

	default:
		return FText::GetEmpty();
	}
}

FReply SMIForgeBatchParameterEditor::OnApplyClicked()
{
	const int32 CheckedParameterCount = GetCheckedParameterCount();

	if(CheckedParameterCount == 0)
	{	
		MIForgeUtilities::PrintWindow(
			TEXT("No parameters are checked for batch editing."),
			EAppMsgType::Ok
		);

		return FReply::Handled();
	}

	const TArray<UMaterialInstanceConstant*> MaterialInstances = GetUniqueMaterialInstancesToEdit();

	if(MaterialInstances.Num() == 0)
	{
		MIForgeUtilities::PrintWindow(
			TEXT("No valid material instances found to edit."),
			EAppMsgType::Ok
		);
		return FReply::Handled();
	}

	FScopedTransaction Transaction(
		NSLOCTEXT(
			"MIForge",
			"BatchAdjustMIParameters",
			"MIForge Batch Adjust MI Parameters"
		)
	);

	for(UMaterialInstanceConstant* MIC : MaterialInstances)
	{
		if (!MIC)
		{
			continue;
		}
		ApplyCheckedRowsToMaterialInstance(MIC);
	}

	MIForgeUtilities::PrintNotification(
		FString::Printf(
			TEXT("Applied %d parameter(s) to %d material instance(s)."),
			CheckedParameterCount,
			MaterialInstances.Num()
		),
		1.0f
	);

	return FReply::Handled();
}

FReply SMIForgeBatchParameterEditor::OnCancelClicked()
{
	if (TSharedPtr<SDockTab> Tab = OwnerTab.Pin())
	{
		Tab->RequestCloseTab();
	}

	return FReply::Handled();
}

void SMIForgeBatchParameterEditor::RefreshParameterGroups()
{
	if (!ParameterGroupScrollBox.IsValid())
	{
		return;
	}

	ParameterGroupScrollBox->ClearChildren();

	TMap<FName, TArray<TSharedPtr<FMIForgeBatchParameterRow>>> RowsByGroup;

	for (const TSharedPtr<FMIForgeBatchParameterRow>& Row : ParameterRows)
	{
		if (!Row.IsValid())
		{
			continue;
		}

		if (!DoesParameterMatchSearch(Row))
		{
			continue;
		}

		RowsByGroup.FindOrAdd(Row->GroupName).Add(Row);
	}

	TArray<FName> SortedGroupNames;
	RowsByGroup.GetKeys(SortedGroupNames);
	SortedGroupNames.Sort([](const FName& A, const FName& B)
		{
			return A.LexicalLess(B);
		});

	for (const FName& GroupName : SortedGroupNames)
	{
		TArray<TSharedPtr<FMIForgeBatchParameterRow>>& Rows = RowsByGroup[GroupName];

		Rows.Sort([](
			const TSharedPtr<FMIForgeBatchParameterRow>& A,
			const TSharedPtr<FMIForgeBatchParameterRow>& B
			)
			{
				if (A->SortPriority != B->SortPriority)
				{
					return A->SortPriority < B->SortPriority;
				}
				return A->ParameterName.LexicalLess(B->ParameterName);
			}
		);

		TSharedRef<SVerticalBox> GroupContent = SNew(SVerticalBox);

		for (const TSharedPtr<FMIForgeBatchParameterRow>& Row : Rows)
		{
			GroupContent->AddSlot()
				.AutoHeight()
				.Padding(0.0f, 2.0f)
				[
					GenerateParameterRowWidget(Row)
				];
		}

		ParameterGroupScrollBox->AddSlot()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SExpandableArea)
				.InitiallyCollapsed(false)
				.HeaderContent()
				[
					SNew(STextBlock)
						.Text(FText::FromString(
							FString::Printf(
								TEXT("%s (%d)"),
								*GroupName.ToString(),
								Rows.Num()
							)
						))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11.0f))
					]
				.BodyContent()
				[
					GroupContent
				]
		];

	}


}

bool SMIForgeBatchParameterEditor::DoesParameterMatchSearch(const TSharedPtr<FMIForgeBatchParameterRow>& Item) const
{
	if (!Item.IsValid())
	{
		return false;
	}

	const FString SearchString =
		CurrentSearchText.ToString().TrimStartAndEnd();

	if (SearchString.IsEmpty())
	{
		return true;
	}

	return Item->ParameterName.ToString().Contains(
		SearchString,
		ESearchCase::IgnoreCase
	) ||
		Item->GroupName.ToString().Contains(
			SearchString,
			ESearchCase::IgnoreCase
		) ||
		GetParameterTypeText(Item->Type).ToString().Contains(
			SearchString,
			ESearchCase::IgnoreCase
		);
}

TSharedRef<SWidget> SMIForgeBatchParameterEditor::GenerateParameterRowWidget(TSharedPtr<FMIForgeBatchParameterRow> Item)
{
	return SNew(SBorder)
		.Padding(6.0f)
		.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([Item]()
							{
								return Item->bApply
									? ECheckBoxState::Checked
									: ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([Item](ECheckBoxState NewState)
							{
								Item->bApply = NewState == ECheckBoxState::Checked;
							})
				]

			+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromName(Item->ParameterName))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.18f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(GetParameterTypeText(Item->Type))
						.ColorAndOpacity(FLinearColor(0.65f, 0.65f, 0.65f, 1.0f))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.45f)
				.VAlign(VAlign_Center)
				[
					GenerateValueWidget(Item)
				]
		];
}

TArray<UMaterialInstanceConstant*> SMIForgeBatchParameterEditor::GetUniqueMaterialInstancesToEdit() const
{
	TArray<UMaterialInstanceConstant*> Result;
	TSet<UMaterialInstanceConstant*> SeenMIs;

	for (const FMIForgeMaterialSlotTarget& Target :
		ParameterModel.ParentGroup.Targets)
	{
		UMaterialInstanceConstant* MIC = Target.MaterialInstance.Get();

		if (!MIC || SeenMIs.Contains(MIC))
		{
			continue;
		}

		SeenMIs.Add(MIC);
		Result.Add(MIC);
	}

	return Result;
}

int32 SMIForgeBatchParameterEditor::GetCheckedParameterCount() const
{
	int32 Count = 0;

	for (const TSharedPtr<FMIForgeBatchParameterRow>& Row : ParameterRows)
	{
		if (Row.IsValid() && Row->bApply)
		{
			Count++;
		}
	}

	return Count;
}

void SMIForgeBatchParameterEditor::ApplyCheckedRowsToMaterialInstance(UMaterialInstanceConstant* MaterialInstance) const
{
	if (!MaterialInstance)
	{
		return;
	}

	MaterialInstance->SetFlags(RF_Transactional);
	MaterialInstance->Modify();

	for (const TSharedPtr<FMIForgeBatchParameterRow>& Row : ParameterRows)
	{
		if (!Row.IsValid() || !Row->bApply)
		{
			continue;
		}

		ApplyParameterRowToMaterialInstance(MaterialInstance, *Row);
	}

	MaterialInstance->MarkPackageDirty();
}

void SMIForgeBatchParameterEditor::ApplyParameterRowToMaterialInstance(UMaterialInstanceConstant* MaterialInstance, const FMIForgeBatchParameterRow& Row) const
{
	if (!MaterialInstance)
	{
		return;
	}

	switch (Row.Type)
	{
		case EMIForgeBatchParameterType::Scalar:
			UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MaterialInstance, Row.ParameterName, Row.ScalarValue);
			break;

		case EMIForgeBatchParameterType::Vector:
			UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MaterialInstance, Row.ParameterName, Row.VectorValue);
			break;

		case EMIForgeBatchParameterType::Texture:
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(MaterialInstance, Row.ParameterName, Row.TextureValue.Get());
			break;

		case EMIForgeBatchParameterType::StaticSwitch:
			UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(MaterialInstance, Row.ParameterName, Row.BoolValue);
			break;

		default:
			break;
	}
}
