// MIForge
// Copyright (c) 2026 Tianshuo Liu
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.

#pragma once

#include "CoreMinimal.h"


class FMIForgeAssetScanner
{
public:
	TArray<FAssetData> FindTexturesInFolders(const TArray<FString>& FolderPaths) const;
	
};
