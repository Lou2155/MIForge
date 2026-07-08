// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class FMIForgeAssetScanner
{
public:
	TArray<FAssetData> FindTexturesInFolders(const TArray<FString>& FolderPaths) const;
	
};
