// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeStyle.h"
#include "Styling/SlateStyleRegistry.h" 
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir StyleSet->RootToContentDir

TSharedPtr<FSlateStyleSet> FMIForgeStyle::StyleSetInstance = nullptr;

void FMIForgeStyle::Initialize()
{
	if (!StyleSetInstance.IsValid())
	{
		StyleSetInstance = CreateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSetInstance);
	}
}

void FMIForgeStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSetInstance);
	ensure(StyleSetInstance.IsUnique());	// Verify no one else is holding references (debug check)
	StyleSetInstance.Reset();	// Release our reference (destroys if unique)
}

FName FMIForgeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MIForgeStyle"));
	return StyleSetName;
}

const ISlateStyle& FMIForgeStyle::Get()
{
	return *StyleSetInstance;
}

TSharedRef<FSlateStyleSet> FMIForgeStyle::CreateStyleSet()
{
	TSharedRef<FSlateStyleSet> StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	const FString ContentDir = FPaths::ProjectPluginsDir() / TEXT("MIForge/Resources");
	StyleSet->SetContentRoot(ContentDir / TEXT("Icons"));
	
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon36x36(36.0f, 36.0f);

	//PNG Icons
	StyleSet->Set("ListView.Row.Accept", new IMAGE_BRUSH(TEXT("Accept"), Icon16x16));
	StyleSet->Set("ListView.Row.Warning", new IMAGE_BRUSH(TEXT("Warning"), Icon16x16));
	StyleSet->Set("ListView.Row.Reject", new IMAGE_BRUSH(TEXT("Reject"), Icon16x16));
	StyleSet->Set("ListView.Row.Info", new IMAGE_BRUSH(TEXT("Info"), Icon16x16));
	StyleSet->Set("MIForgeLogo.Compact", new IMAGE_BRUSH(TEXT("MIForgeLogo_Compact"), Icon36x36));
	//SVG Icons
	StyleSet->Set("Panel.FolderSelection", new IMAGE_BRUSH_SVG(TEXT("SelectFolder"), Icon16x16));
	StyleSet->Set("Panel.RefreshButton", new IMAGE_BRUSH_SVG(TEXT("Refresh"), Icon16x16));


	return StyleSet;
}
