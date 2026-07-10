// Fill out your copyright notice in the Description page of Project Settings.


#include "MIForgeUtilities.h"
#include "EditorAssetLibrary.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "MIForgeSettings.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "MIForgeAssetScanner.h"
#include "MIForgeTypes.h"
#include "MIForgeTextureClassifier.h"

#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"



TArray<TSharedPtr<FAssetData>> MIForgeUtilities::GetTexturesFromSelectedFolders(const TArray<FString>& FoldersPath)
{
	TArray<TSharedPtr<FAssetData>> AssetDataList;

	for (const FString& fp : FoldersPath)
	{
		TArray<FString> AssetPaths = UEditorAssetLibrary::ListAssets(fp);

		AssetDataList.Reserve(AssetDataList.Num() + AssetPaths.Num());
		for (const FString& ap : AssetPaths)
		{
			FAssetData AssetData = UEditorAssetLibrary::FindAssetData(ap);

			if (!AssetData.IsValid())
			{
				continue;
			}

			// Robust texture detection: allow UTexture and any subclass (UTexture2D, UTextureCube, etc.)
			UClass* AssetClass = AssetData.GetClass();
			if (AssetClass == nullptr || !AssetClass->IsChildOf(UTexture::StaticClass()))
			{
				continue;
			}

			AssetDataList.Add(MakeShared<FAssetData>(MoveTemp(AssetData)));
		}
	}

	return AssetDataList;
}

void MIForgeUtilities::CreatePathSelector(TSharedRef<SWidget> ParentWidget, FOnPathSelected OnPathSelected)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(FName("ContentBrowser"));
	FPathPickerConfig Config;
	Config.bShowFavorites = false; // hide the favorites selector
	Config.bAllowContextMenu = false; // disable right-click context menu in the path picker
	Config.OnPathSelected.BindLambda([OnPathSelected](const FString& Path) {
		OnPathSelected.ExecuteIfBound(Path);
		FSlateApplication::Get().DismissAllMenus(); // close the menu after selecting a path
		});
	//create rollout menu to select path, return the path selected by user
	FSlateApplication::Get().PushMenu(
		ParentWidget,
		FWidgetPath(),  // empty widget path means the menu will be attached to the parent widget
		ContentBrowserModule.Get().CreatePathPicker(Config),  // create the path picker widget
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
	);
}

TArray<TSharedPtr<FString>> MIForgeUtilities::GetFilterOptions()
{
	TArray<TSharedPtr<FString>> TextureFilterOptions; //keys
	TextureFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	for (const TPair<FString, FTextureExtension>& te : UMIForgeSettings::Get()->TextureExtensions)
	{
		TextureFilterOptions.Add(MakeShared<FString>(te.Key));
	}
	return TextureFilterOptions;
}

TArray<TSharedPtr<FAssetData>> MIForgeUtilities::FilterTextures(const TArray<TSharedPtr<FAssetData>>& AllAssetsData, const TSharedPtr<FString>& TextureType)
{
	TArray<TSharedPtr<FString>> ExtensionsToFilter;

	for (const TPair<FString, FTextureExtension>& te : UMIForgeSettings::Get()->TextureExtensions)
	{
		if (*TextureType == te.Key)
		{
			for (const FString& Extension : te.Value.Extension)
			{
				ExtensionsToFilter.Add(MakeShared<FString>(Extension));
			};
		}
	}

	TArray<TSharedPtr<FAssetData>> FilteredAssets;
	if (ExtensionsToFilter.Num() == 0) // if no filter is selected, return all assets
	{
		return AllAssetsData;
	}

	for (const TSharedPtr<FAssetData>& AssetData : AllAssetsData)
	{
		if (!AssetData.IsValid())
		{
			continue;
		}

		FString AssetName = AssetData->AssetName.ToString();
		for (const TSharedPtr<FString>& Extension : ExtensionsToFilter)
		{
			if (AssetName.EndsWith(*Extension))
			{
				FilteredAssets.Add(AssetData);
				break;
			}
		}
	}

	return FilteredAssets;
}

void MIForgeUtilities::PrintDebug(const FString& message, const FColor& color, const float& duration) {

	GEngine->AddOnScreenDebugMessage(-1, duration, color, message);
}

void MIForgeUtilities::PrintLog(const FString& message, const ELogVerbosity::Type& type)
{
	if (type == ELogVerbosity::Log) {
		UE_LOG(LogTemp, Log, TEXT("%s"), *message);  // * swap UE string(FString) to C string. %s expects a TCHAR*, and *message converts an FString into exactly that.
	}
	else if (type == ELogVerbosity::Warning) {
		UE_LOG(LogTemp, Warning, TEXT("%s"), *message);
	}
	else if (type == ELogVerbosity::Error) {
		UE_LOG(LogTemp, Error, TEXT("%s"), *message);

	}
}

EAppReturnType::Type MIForgeUtilities::PrintWindow(const FString& message, const EAppMsgType::Type& msgType)
{
	return FMessageDialog::Open(msgType, FText::FromString(message));

}

void MIForgeUtilities::PrintNotification(const FString& message, const float& duration)
{
	FNotificationInfo NotInfo1(FText::FromString(message));
	NotInfo1.ExpireDuration = duration;
	FSlateNotificationManager::Get().AddNotification(NotInfo1);
}

void MIForgeUtilities::ActivateContentBrowserTabAndSyncToAssets(const TArray<FAssetData>& AssetDatas)
{
	if (AssetDatas.IsEmpty())
	{
		return;
	}

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(
			TEXT("ContentBrowser")
		);

	ContentBrowserModule.Get().SyncBrowserToAssets(
		AssetDatas,
		/* bAllowLockedBrowsers */ false,
		/* bFocusContentBrowser */ true
	);
}

void MIForgeUtilities::FixTextureCompressionInSelectedFolders(TArray<FString> SelectedFolderPaths)
{
	FMIForgeAssetScanner Scanner;
	FMIForgeTextureClassifier Classifier;
	TArray<FAssetData> FoundTextures = Scanner.FindTexturesInFolders(SelectedFolderPaths);
	TArray<FMIForgeTextureInfo> ClassifiedTextureInfos = Classifier.ClassifyTextures(FoundTextures);

	const UMIForgeSettings* Settings = UMIForgeSettings::Get();

	Settings->TextureCompressionSettingsMap;
	
	int32 FixedCount = 0;
	int32 SkippedCount = 0;

	for (const FMIForgeTextureInfo& TextureInfo : ClassifiedTextureInfos)
	{
		if (TextureInfo.TextureType == EMIForgeTextureType::Unknown)
		{
			SkippedCount++;
			continue;
		}

		const FMIForgeTextureCompressionSettings* FoundCompression =
			Settings->TextureCompressionSettingsMap.Find(TextureInfo.TextureType);

		if (!FoundCompression)
		{
			SkippedCount++;
			continue;
		}

		UTexture* Texture = Cast<UTexture>(TextureInfo.AssetData.GetAsset());
		if (!Texture)
		{
			SkippedCount++;
			continue;
		}

		const bool bNeedsFix =
			Texture->CompressionSettings.GetValue() != FoundCompression->CompressionSettings.GetValue() ||
			Texture->SRGB != FoundCompression->bSRGB;

		if (!bNeedsFix)
		{
			SkippedCount++;
			continue;
		}

		Texture->Modify();

		Texture->CompressionSettings = FoundCompression->CompressionSettings;
		Texture->SRGB = FoundCompression->bSRGB;
		Texture->UpdateResource();
		Texture->MarkPackageDirty();

		FixedCount++;
	}

	MIForgeUtilities::PrintNotification(
		FString::Printf(
			TEXT("Texture compression fixed: %d updated, %d skipped."),
			FixedCount,
			SkippedCount
		)
	);
}

TSharedRef<SWidget> MIForgeUtilities::WithRowSeparator(TSharedRef<SWidget> Content)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.VAlign(VAlign_Center)
		[
			Content
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
				.Thickness(1.0f)
				.ColorAndOpacity(FLinearColor(.15f, .15f, .15f, 1.0f))
		];
}

void MIForgeUtilities::SafelyDeleteAssets(UObject* ObjectToDelete, bool bWasCreated, TArray<UObject*> CreatedAssets)
{
	if (bWasCreated && ObjectToDelete && IsValid(ObjectToDelete) && !ObjectToDelete->IsUnreachable())
	{


		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		TSet<FString> UniquePathsToScan;
		TArray<UObject*> ObjectsToDelete;

		ObjectsToDelete.Add(ObjectToDelete);

		const FString PackageName = ObjectToDelete->GetPackage()->GetName();
		UniquePathsToScan.Add(FPackageName::GetLongPackagePath(PackageName));

		for (UObject* Object : ObjectsToDelete)
		{
			if (Object && IsValid(Object) && !Object->IsUnreachable())
			{
				AssetRegistry.AssetDeleted(Object);
			}
		}

		CreatedAssets.Remove(ObjectToDelete);

		ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);

		if (UniquePathsToScan.Num() > 0)
		{
			const TArray<FString> PathsToScan = UniquePathsToScan.Array();

			AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
		}


	}
}

void MIForgeUtilities::CloseOpenAssetEditors(const TArray<UObject*>& ObjectsToDelete)
{
	if (GEditor)
	{
		UAssetEditorSubsystem* AssetEditorSubsystem =
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

		if (AssetEditorSubsystem)
		{
			for (UObject* Object : ObjectsToDelete)
			{
				if (Object && IsValid(Object) && !Object->IsUnreachable())
				{
					AssetEditorSubsystem->CloseAllEditorsForAsset(Object);
				}
			}
		}
	}
}


