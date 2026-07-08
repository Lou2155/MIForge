// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MIForgeGenerationUndoRecord.generated.h"

/**
 * 
 */
UCLASS()
class MIFORGE_API UMIForgeGenerationUndoRecord : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	bool bAssetsShouldExist = false;

	UPROPERTY()
	TArray<FSoftObjectPath> CreatedAssetPaths;

	virtual void PostTransacted(
		const FTransactionObjectEvent& TransactionEvent
	) override;

private:
	bool bDeleteQueued = false;
};
