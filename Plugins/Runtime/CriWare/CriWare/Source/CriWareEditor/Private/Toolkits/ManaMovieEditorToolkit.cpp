/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEditor
 * File     : ManaMovieEditorToolkit.cpp
 *
 ****************************************************************************/

#include "Toolkits/ManaMovieEditorToolkit.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "ManaMovie.h"

#include "CriWareEditorStyle.h"
#include "Widgets/Docking/SDockTab.h"

#include "Widgets/SManaMovieEditorDetails.h"
//#include "Widgets/SManaMovieEditorMedia.h"

#define LOCTEXT_NAMESPACE "FManaMovieEditorToolkit"

namespace ManaMovieEditorToolkit
{
	static const FName AppIdentifier("ManaMovieEditorApp");
	static const FName DetailsTabId("Details");
	//static const FName MediaTabId("Mana");
}

/* FManaMovieEditorToolkit structors
 *****************************************************************************/

FManaMovieEditorToolkit::FManaMovieEditorToolkit()
	: ManaMovie(nullptr)
{ }

FManaMovieEditorToolkit::~FManaMovieEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}

/* FMediaPlaylistEditorToolkit interface
 *****************************************************************************/

void FManaMovieEditorToolkit::Initialize(UManaMovie* InManaMovie, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	ManaMovie = InManaMovie;

	if (ManaMovie == nullptr)
	{
		return;
	}

	// Support undo/redo
	ManaMovie->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_ManaMovieEditor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// tool bar
				FTabManager::NewStack()
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.1f)

			)
			->Split
			(
				// details
				FTabManager::NewStack()
				->AddTab(ManaMovieEditorToolkit::DetailsTabId, ETabState::OpenedTab)
				->SetSizeCoefficient(0.45f)
			)
			//->Split
			//(
			//	// media library
			//	FTabManager::NewStack()
			//	->AddTab(ManaMovieEditorToolkit::ManaTabId, ETabState::OpenedTab)
			//	->SetHideTabWell(true)
			//	->SetSizeCoefficient(0.45f)
			//)
		);

	FManaMovieEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		ManaMovieEditorToolkit::AppIdentifier,
		Layout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InManaMovie
	);

	RegenerateMenusAndToolbars();
}

/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FManaMovieEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("Engine/Content/Types/MediaAssets/Properties/Interface"));
}

void FManaMovieEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_ManaMovieEditor", "Mana Movie Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ManaMovieEditorToolkit::DetailsTabId, FOnSpawnTab::CreateSP(this, &FManaMovieEditorToolkit::HandleTabManagerSpawnTab, ManaMovieEditorToolkit::DetailsTabId))
		.SetDisplayName(LOCTEXT("DetailsTabName", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	//InTabManager->RegisterTabSpawner(ManaMovieEditorToolkit::MediaTabId, FOnSpawnTab::CreateSP(this, &FManaMovieEditorToolkit::HandleTabManagerSpawnTab, ManaMovieEditorToolkit::MediaTabId))
	//	.SetDisplayName(LOCTEXT("MediaTabName", "Media Library"))
	//	.SetGroup(WorkspaceMenuCategoryRef)
	//	.SetIcon(FSlateIcon(Style->GetStyleSetName(), "MediaPlayerEditor.Tabs.Media"));
}

void FManaMovieEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	//InTabManager->UnregisterTabSpawner(ManaMovieEditorToolkit::MediaTabId);
	InTabManager->UnregisterTabSpawner(ManaMovieEditorToolkit::DetailsTabId);
}

/* IToolkit interface
 *****************************************************************************/

FText FManaMovieEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Mana Movie Editor");
}


FName FManaMovieEditorToolkit::GetToolkitFName() const
{
	return FName("ManaMovieEditor");
}


FLinearColor FManaMovieEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FManaMovieEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "ManaMovie ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FManaMovieEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ManaMovie);
}

/* FEditorUndoClient interface
*****************************************************************************/

void FManaMovieEditorToolkit::PostUndo(bool bSuccess)
{
}


void FManaMovieEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

/* FManaMovieEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FManaMovieEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	if (TabIdentifier == ManaMovieEditorToolkit::DetailsTabId)
	{
		TabWidget = SNew(SManaMovieEditorDetails, *ManaMovie);
	}
	//else if (TabIdentifier == ManaMovieEditorToolkit::MediaTabId)
	//{
	//	TabWidget = SNew(SMediaPlaylistEditorMedia, *MediaPlaylist, Style);
	//}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}


#undef LOCTEXT_NAMESPACE
