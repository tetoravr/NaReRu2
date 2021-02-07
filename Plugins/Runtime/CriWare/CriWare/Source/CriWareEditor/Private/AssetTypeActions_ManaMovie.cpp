/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2017 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Asset Type Action of Mana Movie Source
* File     : AssetTypeActions_ManaMovie.cpp
*
****************************************************************************/

/* モジュールヘッダ */
#include "AssetTypeActions_ManaMovie.h"

/* ProjectPluginとして利用する場合MonolithicHeaderBoilerplate.hをインクルード */
#if !defined(UE_IS_ENGINE_MODULE)
#include "Misc/MonolithicHeaderBoilerplate.h"
MONOLITHIC_HEADER_BOILERPLATE()
#endif

/* CRIWAREプラグインヘッダ */
#include "CriWareEditorPrivatePCH.h"
#include "CriWareInitializer.h"
#include "CriWarePreviewer.h"
#include "ManaPlaylist.h"
#include "ManaPlaylistFactory.h"
#include "ManaTexture.h"
#include "ManaTextureFactory.h"
#include "ThumbnailRendering/ManaMovieThumbnailInfo.h"
#include "Toolkits/ManaMovieEditorToolkit.h"

/* Unreal Engine 4関連ヘッダ */
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Interfaces/ITextureEditorToolkit.h"
#include "Interfaces/ITextureEditorModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "EditorFramework/AssetImportData.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

#include "CriWareEditorStyle.h"

FAssetTypeActions_ManaMovie::FAssetTypeActions_ManaMovie(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{
}

void FAssetTypeActions_ManaMovie::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	/* ManaTextureアセットの右クリックメニューに「マテリアルを作成」項目を追加 */
	auto ManaMovie = GetTypedWeakObjectPtrs<UManaMovie>(InObjects);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("ManaMovie_CreateManaTexture", "Create Mana Texture"),
		LOCTEXT("Texture_CreateManaTextureTooltip", "Creates a new texture using this Sofdec2 Movie as source."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.ManaTexture"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_ManaMovie::ExecuteCreateTexture, ManaMovie),
			FCanExecuteAction()
		)
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("ManaMovie_CreateManaMoviePlaylist", "Create Mana Movie Playlist"),
		LOCTEXT("Texture_CreateManaMoviePlaylistTooltip", "Creates a new playlist using this Sofdec2 Movie."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.ManaTexture"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_ManaMovie::ExecuteCreatePlaylist, ManaMovie),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_ManaMovie::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		if (Asset->GetClass() == UFileManaMovie::StaticClass()) {
			auto* ManaMovie = CastChecked<UFileManaMovie>(Asset);
			auto ResolvedSourceFilePath = ManaMovie->GetUrl().RightChop(7);
			OutSourceFilePaths.Add(UAssetImportData::ResolveImportFilename(ResolvedSourceFilePath, ManaMovie->GetOutermost()));
		}
	}
}

void FAssetTypeActions_ManaMovie::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto ManaMovie = Cast<UManaMovie>(*ObjIt);

		if (ManaMovie != nullptr)
		{
			TSharedRef<FManaMovieEditorToolkit> EditorToolkit = MakeShareable(new FManaMovieEditorToolkit());
			EditorToolkit->Initialize(ManaMovie, Mode, EditWithinLevelEditor);
		}
	}
}

void FAssetTypeActions_ManaMovie::ExecuteCreateTexture(TArray<TWeakObjectPtr<UManaMovie>> ManaMovies)
{
	const FString Suffix = TEXT("_Texture");

	IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<UObject*> ObjectsToSync;

	for (auto MovieIt = ManaMovies.CreateConstIterator(); MovieIt; ++MovieIt) {
		auto ManaMovie = (*MovieIt).Get();
		if (ManaMovie) {
			FString Name;
			FString PackagePath;
			CreateUniqueAssetName(ManaMovie->GetOutermost()->GetName(), Suffix, PackagePath, Name);

			UManaTextureFactory* Factory = NewObject<UManaTextureFactory>();
			Factory->InitialMovieSource = ManaMovie;

			FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
			UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UManaTexture::StaticClass(), Factory);

			if (NewAsset) {
				ObjectsToSync.Add(NewAsset);
			}
		}
	}

	if (ObjectsToSync.Num() > 0) {
		ContentBrowserSingleton.SyncBrowserToAssets(ObjectsToSync);
	}
}

void FAssetTypeActions_ManaMovie::ExecuteCreatePlaylist(TArray<TWeakObjectPtr<UManaMovie>> ManaMovies)
{
	const FString Suffix = TEXT("_Playlist");

	if (ManaMovies.Num() > 0) {

		FString Name;
		FString PackagePath;
		CreateUniqueAssetName(ManaMovies[0]->GetOutermost()->GetName(), Suffix, PackagePath, Name);

		UManaPlaylistFactory* Factory = NewObject<UManaPlaylistFactory>();
		
		// initialize play list with selected movies 
		Factory->InitialMovies = ManaMovies;

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackagePath), UManaPlaylist::StaticClass(), Factory);
	}
}

bool FAssetTypeActions_ManaMovie::AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType)
{
	/* プレビュー要求かどうかチェック */
	if (ActivationType == EAssetTypeActivationMethod::Previewed) {
		/* 複数選択時は最初のアセットのみをプレビュー対象とする */
		UManaMovie* TargetMovie = nullptr;
		for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt) {
			TargetMovie = Cast<UManaMovie>(*ObjIt);
			if (TargetMovie) {
				break;
			}
		}

		/* 再生状態の確認 */
		if ((TargetMovie == nullptr) || IsMoviePlaying(TargetMovie)) {
			/* 指定された音声が再生中の場合は停止 */
			StopMovie();
		} else {
			/* 未再生時は新規に再生を開始 */
			PlayMovie(TargetMovie);
		}

		return true;
	}
	
	/* プレビュー要求以外はデフォルト処理 */
	return false;
}

UThumbnailInfo* FAssetTypeActions_ManaMovie::GetThumbnailInfo(UObject* Asset) const
{
	UManaMovie* Movie = CastChecked<UManaMovie>(Asset);
	UThumbnailInfo* ThumbnailInfo = Movie->ThumbnailInfo;
	if (ThumbnailInfo == nullptr)
	{
		ThumbnailInfo = NewObject<UManaMovieThumbnailInfo>(Movie, NAME_None, RF_Transactional);
		Movie->ThumbnailInfo = ThumbnailInfo;
	}

	return ThumbnailInfo;
}

TSharedPtr<SWidget> FAssetTypeActions_ManaMovie::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	auto OnGetDisplayBrushLambda = [this, AssetData]() -> const FSlateBrush*
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset) {
			auto* Movie = CastChecked<UManaMovie>(Asset);
			if (IsMoviePlaying(Movie)) {
				return FEditorStyle::GetBrush("MediaAsset.AssetActions.Stop.Large");
			}
		}

		return FEditorStyle::GetBrush("MediaAsset.AssetActions.Play.Large");
	};

	auto OnClickedLambda = [this, AssetData]() -> FReply
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset) {
			auto* Movie = CastChecked<UManaMovie>(Asset);

			if (IsMoviePlaying(Movie)) {
				StopMovie();
			}
			else {
				PlayMovie(Movie);
			}
		}
		return FReply::Handled();
	};

	auto OnToolTipTextLambda = [this, AssetData]() -> FText
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset) {
			auto* Movie = CastChecked<UManaMovie>(Asset);
			if (IsMoviePlaying(Movie)) {
				return LOCTEXT("Thumbnail_StopMovieToolTip", "Stop selected movie");
			}
		}

		return LOCTEXT("Thumbnail_PlayMovieToolTip", "Play selected movie");
	};

	TSharedPtr<SBox> Box;
	SAssignNew(Box, SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(2));

	auto OnGetVisibilityLambda = [this, Box, AssetData]() -> EVisibility
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset) {
			auto* Movie = CastChecked<UManaMovie>(Asset);
			if (Box.IsValid()) {
				if (Box->IsHovered() || IsMoviePlaying(Movie)) {
					return EVisibility::Visible;
				}
			}
		}

		return EVisibility::Hidden;
	};

	TSharedPtr<SButton> Widget;
	SAssignNew(Widget, SButton)
		.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
		.ToolTipText_Lambda(OnToolTipTextLambda)
		.Cursor(EMouseCursor::Default) // The outer widget can specify a DragHand cursor, so we need to override that here
		.ForegroundColor(FSlateColor::UseForeground())
		.OnClicked_Lambda(OnClickedLambda)
		.Visibility_Lambda(OnGetVisibilityLambda)
		[
			SNew(SImage)
			.Image_Lambda(OnGetDisplayBrushLambda)
		];

	Box->SetContent(Widget.ToSharedRef());
	Box->SetVisibility(EVisibility::Visible);

	return Box;
}

void FAssetTypeActions_ManaMovie::PlayMovie(UManaMovie* Movie) const
{
	if (Movie) {
		GCriWarePreviewer->PlayPreviewMovie(Movie);
	} else {
		StopMovie();
	}
}

void FAssetTypeActions_ManaMovie::StopMovie() const
{
	GCriWarePreviewer->StopPreviewMovie();
}

bool FAssetTypeActions_ManaMovie::IsMoviePlaying(UManaMovie* Movie) const
{
	return GCriWarePreviewer->IsPreviewMoviePlaying(Movie);
}

void FAssetTypeActions_ManaMovie::ExecutePlayMovie(TArray<TWeakObjectPtr<UManaMovie>> Objects) const
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt) {
		UManaMovie* Movie = (*ObjIt).Get();
		if (Movie) {
			// Only play the first valid sound
			PlayMovie(Movie);
			break;
		}
	}
}

void FAssetTypeActions_ManaMovie::ExecuteStopMovie(TArray<TWeakObjectPtr<UManaMovie>> Objects) const
{
	StopMovie();
}

bool FAssetTypeActions_ManaMovie::CanExecutePlayCommand(TArray<TWeakObjectPtr<UManaMovie>> Objects) const
{
	return Objects.Num() == 1;
}

#undef LOCTEXT_NAMESPACE
