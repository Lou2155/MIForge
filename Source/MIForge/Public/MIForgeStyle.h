// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FMIForgeStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static FName GetStyleSetName();
	static const ISlateStyle& Get();

private:
	static TSharedRef<FSlateStyleSet> CreateStyleSet();
	static TSharedPtr<FSlateStyleSet> StyleSetInstance;
};
