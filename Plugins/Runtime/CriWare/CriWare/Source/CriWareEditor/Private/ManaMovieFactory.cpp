/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2017 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Asset Factory of ManaMovie
* File     : ManaMovieFactory.cpp
*
****************************************************************************/

#include "ManaMovieFactory.h"

#include "EditorFramework/AssetImportData.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#include "CriWareEditorPrivatePCH.h"
#include "CriWareInitializer.h"
#include "ManaMovie.h"
#include "ManaTexture.h"
#include "ManaTextureFactory.h"
#include "ThumbnailRendering/ManaMovieThumbnailInfo.h"


#define LOCTEXT_NAMESPACE "UManaMovieFactory"


UManaMovieFactory::UManaMovieFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("usm;Sofdec2 Movie File"));

	bCreateNew = true;
	bEditorImport = true;

	SupportedClass = UFileManaMovie::StaticClass();
}

UObject* UManaMovieFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto ManaMovie = NewObject<UFileManaMovie>(InParent, InClass, InName, Flags);

	if (ManaMovie != nullptr)
	{
		// Get file path if imported
		FString MovieFilePath = GetCurrentFilename();
		if (!MovieFilePath.IsEmpty()) {
			/* DefaultCriWare.iniファイルで設定したContentDirへの相対パスを保管する */
			FPaths::MakePathRelativeTo(MovieFilePath, *UCriWareInitializer::GetContentDir());
		}

		ManaMovie->SetFilePath(MovieFilePath);

		UManaMovieThumbnailInfo* ManaMovieThumbnailInfo = NewObject<UManaMovieThumbnailInfo>(ManaMovie, NAME_None, RF_Transactional);
		ManaMovie->ThumbnailInfo = ManaMovieThumbnailInfo;

		// if movie is nor validated, nor validating, it means path is not loadable
		if (ManaMovie->IsValidating() || ManaMovie->Validate()) {

			// FIXME: force validation for asset importation.

			if (ManaMovie->AssetImportData == nullptr) {
				ManaMovie->AssetImportData = NewObject<UAssetImportData>(ManaMovie, TEXT("AssetImportData"));
			}

			if (ManaMovie->AssetImportData != nullptr) {
				ManaMovie->AssetImportData->Update(CurrentFilename);
			}

			ManaMovie->CaptureThumbnail(ManaMovieThumbnailInfo->FrameNumber, FIntPoint(ManaMovieThumbnailInfo->ThumbnailResolution, ManaMovieThumbnailInfo->ThumbnailResolution));
		}
	}

	return ManaMovie;
}

//~ Begin FReimportHandler interface

bool UManaMovieFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UManaMovie* ManaMovie = Cast<UManaMovie>(Obj);

	if (ManaMovie && ManaMovie->AssetImportData) {
		ManaMovie->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}

	return false;
}

void UManaMovieFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UManaMovie* ManaMovie = Cast<UManaMovie>(Obj);
	if (ManaMovie && ensure(NewReimportPaths.Num() == 1)) {
		ManaMovie->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UManaMovieFactory::Reimport(UObject* Obj)
{
	/* インポート情報の取得 */
	UManaMovie* ManaMovie = Cast<UManaMovie>(Obj);
	if (!ManaMovie || !ManaMovie->AssetImportData) {
		return EReimportResult::Failed;
	}

	// TODO: check playing

	/* ファイルパスの作成 */
	FString ResolvedSourceFilePath = ManaMovie->AssetImportData->GetFirstFilename();
	if (!ResolvedSourceFilePath.Len()) {
		return EReimportResult::Failed;
	}

	UE_LOG(LogCriWareEditor, Log, TEXT("Performing atomic reimport of [%s]."), *ResolvedSourceFilePath);

	/* ファイルの有無をチェック */
	if (IFileManager::Get().FileSize(*ResolvedSourceFilePath) == INDEX_NONE) {
		UE_LOG(LogCriWareEditor, Error, TEXT("Cannot reimport: source file cannot be found."));
		return EReimportResult::Failed;
	}

	bool Result = EReimportResult::Failed;

	/* アセットの差し替え */
	if (ManaMovie->GetClass() == UFileManaMovie::StaticClass()) {
		auto* FileManaMovie = CastChecked<UFileManaMovie>(ManaMovie);

		ManaMovie->AssetImportData->Update(ResolvedSourceFilePath);

		// Get file path if imported
		auto PrevFile = FileManaMovie->GetFilePath();
		/* DefaultCriWare.iniファイルで設定したContentDirへの相対パスを保管する */
		FPaths::MakePathRelativeTo(ResolvedSourceFilePath, *UCriWareInitializer::GetContentDir());
		FileManaMovie->SetFilePath(ResolvedSourceFilePath);

		// if movie is nor validated, nor validating, it means path is not loadable
		if (ManaMovie->IsValidating() || ManaMovie->Validate()) {
			Result = EReimportResult::Succeeded;

			UE_LOG(LogCriWareEditor, Log, TEXT("Imported successfully"));

			// FIXME block with a timeout until validation is posible for asset importation.
		}
		else {
			// restore 
			FileManaMovie->SetFilePath(PrevFile);
		}
	}

	if (Result == EReimportResult::Succeeded) {
		// Try to find the outer package so we can dirty it up
		if (ManaMovie->GetOuter()) {
			ManaMovie->GetOuter()->MarkPackageDirty();
		}
		else {
			ManaMovie->MarkPackageDirty();
		}
		return EReimportResult::Succeeded;
	}

	UE_LOG(LogCriWareEditor, Warning, TEXT("-- import failed"));

	return EReimportResult::Failed;
}

#undef LOCTEXT_NAMESPACE
