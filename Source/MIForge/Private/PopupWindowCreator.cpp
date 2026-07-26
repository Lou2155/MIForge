// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#include "PopupWindowCreator.h"

void PopupWindowCreator::OpenPopupWindow(const FText& Title, const TSharedRef<SWidget>& Content, const FVector2D& Size, bool bModal)
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
		.Title(Title)
		.ClientSize(Size)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::UserSized)
		[
			SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(8.f)
				[
					Content
				]
		];

	if (bModal)
	{
		FSlateApplication::Get().AddModalWindow(Window, nullptr);
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window);
	}

}

void PopupWindowCreator::OpenTextInputPopup(const FText& Title, const FText& Label, const FString& InitialValue, TFunction<void(const FString&)> OnConfirmed, const FVector2D& Size)
{
	TSharedPtr<SWindow> Window;
	TSharedPtr<SEditableTextBox> InputBox;

	SAssignNew(Window, SWindow)
		.Title(Title)
		.ClientSize(Size)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::FixedSize);

	Window->SetContent(
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.f)
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
						.Text(Label)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 10.f)
				[
					SAssignNew(InputBox, SEditableTextBox)
						.Text(FText::FromString(InitialValue))
						.SelectAllTextWhenFocused(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(4.f, 0.f)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Cancel")))
								.OnClicked_Lambda([Window]()
									{
										Window->RequestDestroyWindow();
										return FReply::Handled();
									})
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(4.f, 0.f)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Save")))
								.OnClicked_Lambda([Window, InputBox, OnConfirmed]()
									{
										const FString Value = InputBox.IsValid()
											? InputBox->GetText().ToString().TrimStartAndEnd()
											: FString();

										OnConfirmed(Value);

										Window->RequestDestroyWindow();
										return FReply::Handled();
									})
						]
				]
		]
	);

	FSlateApplication::Get().AddModalWindow(Window.ToSharedRef(), nullptr);
}
