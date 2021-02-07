// Copyright Epic Games, Inc. All Rights Reserved.

#include "Toolkits/ManaPlayerEditorToolkit.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Editor.h"
#include "SlateOptMacros.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "ManaPlaylist.h"
#include "Widgets/Docking/SDockTab.h"
//#include "Models/ManaPlayerEditorCommands.h"
#include "Widgets/SManaPlayerEditorDetails.h"
//#include "Widgets/SManaPlayerEditorInfo.h"
//#include "Widgets/SManaPlayerEditorMedia.h"
//#include "Widgets/SManaPlayerEditorPlaylist.h"
//#include "Widgets/SManaPlayerEditorStats.h"
#include "CriWareEditorStyle.h"
#include "ManaPlayer.h"
//#include "Widgets/SManaPlayerEditorViewer.h"


#define LOCTEXT_NAMESPACE "FManaPlayerEditorToolkit"


namespace ManaPlayerEditorToolkit
{
	static const FName AppIdentifier("ManaPlayerEditorApp");
	static const FName DetailsTabId("Details");
	//static const FName InfoTabId("Info");
	//static const FName MediaTabId("Media");
	//static const FName PlaylistTabId("Playlist");
	//static const FName StatsTabId("Stats");
	//static const FName ViewerTabId("Viewer");
}


/* FManaPlayerEditorToolkit structors
 *****************************************************************************/

FManaPlayerEditorToolkit::FManaPlayerEditorToolkit()
	: ManaPlayer(nullptr)
{ }


FManaPlayerEditorToolkit::~FManaPlayerEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}


/* FManaPlayerEditorToolkit interface
 *****************************************************************************/

void FManaPlayerEditorToolkit::Initialize(UManaPlayer* InManaPlayer, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	ManaPlayer = InManaPlayer;

	if (ManaPlayer == nullptr)
	{
		return;
	}

	// support undo/redo
	ManaPlayer->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	BindCommands();

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_ManaPlayerEditor_v10")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			/*->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.66f)
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
					// viewer
					FTabManager::NewStack()
					->AddTab(ManaPlayerEditorToolkit::ViewerTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.6f)
				)
				->Split
				(
					// media library
					FTabManager::NewStack()
					->AddTab(ManaPlayerEditorToolkit::MediaTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.3f)
				)
			)*/
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.33f)
				/*->Split
				(
					// playlist
					FTabManager::NewStack()
					->AddTab(ManaPlayerEditorToolkit::PlaylistTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.5f)
				)*/
				->Split
				(
					// details, info, stats
					FTabManager::NewStack()
					->AddTab(ManaPlayerEditorToolkit::DetailsTabId, ETabState::OpenedTab)
					//->AddTab(ManaPlayerEditorToolkit::InfoTabId, ETabState::OpenedTab)
					//->AddTab(ManaPlayerEditorToolkit::StatsTabId, ETabState::ClosedTab)
					->SetForegroundTab(ManaPlayerEditorToolkit::DetailsTabId)
					->SetSizeCoefficient(0.5f)
				)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		ManaPlayerEditorToolkit::AppIdentifier,
		Layout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InManaPlayer
	);

	// UE4 comented
	//	IManaPlayerEditorModule* ManaPlayerEditorModule = &FModuleManager::LoadModuleChecked<IManaPlayerEditorModule>("ManaPlayerEditor");
	//	AddMenuExtender(ManaPlayerEditorModule->GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));

	ExtendToolBar();
	RegenerateMenusAndToolbars();
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FManaPlayerEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("Engine/Content/Types/MediaAssets/Properties/Interface"));
}


bool FManaPlayerEditorToolkit::OnRequestClose()
{
	if (ManaPlayer != nullptr)
	{
		ManaPlayer->Close();
	}

	return FAssetEditorToolkit::OnRequestClose();
}


void FManaPlayerEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_ManaPlayerEditor", "Media Player Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::DetailsTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::DetailsTabId))
		.SetDisplayName(LOCTEXT("DetailsTabName", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	/*InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::InfoTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::InfoTabId))
		.SetDisplayName(LOCTEXT("InfoTabName", "Info"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(Style->GetStyleSetName(), "ManaPlayerEditor.Tabs.Info"));
		*/

	/*InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::MediaTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::MediaTabId))
		.SetDisplayName(LOCTEXT("MediaTabName", "Media Library"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(Style->GetStyleSetName(), "ManaPlayerEditor.Tabs.Media"));

	InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::ViewerTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::ViewerTabId))
		.SetDisplayName(LOCTEXT("PlayerTabName", "Player"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(Style->GetStyleSetName(), "ManaPlayerEditor.Tabs.Player"));

	InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::PlaylistTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::PlaylistTabId))
		.SetDisplayName(LOCTEXT("PlaylistTabName", "Playlist"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(Style->GetStyleSetName(), "ManaPlayerEditor.Tabs.Playlist"));

	InTabManager->RegisterTabSpawner(ManaPlayerEditorToolkit::StatsTabId, FOnSpawnTab::CreateSP(this, &FManaPlayerEditorToolkit::HandleTabManagerSpawnTab, ManaPlayerEditorToolkit::StatsTabId))
		.SetDisplayName(LOCTEXT("StatsTabName", "Stats"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(Style->GetStyleSetName(), "ManaPlayerEditor.Tabs.Stats"));*/
}


void FManaPlayerEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	//InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::ViewerTabId);
	//InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::StatsTabId);
	//InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::PlaylistTabId);
	//InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::MediaTabId);
	//InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::InfoTabId);
	InTabManager->UnregisterTabSpawner(ManaPlayerEditorToolkit::DetailsTabId);
}


/* IToolkit interface
 *****************************************************************************/

FText FManaPlayerEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Media Player Editor");
}


FName FManaPlayerEditorToolkit::GetToolkitFName() const
{
	return FName("ManaPlayerEditor");
}


FLinearColor FManaPlayerEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FManaPlayerEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "ManaPlayer ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FManaPlayerEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ManaPlayer);
}


/* FEditorUndoClient interface
*****************************************************************************/

void FManaPlayerEditorToolkit::PostUndo(bool bSuccess)
{
	// do nothing
}


void FManaPlayerEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}


/* FManaPlayerEditorToolkit implementation
 *****************************************************************************/

void FManaPlayerEditorToolkit::BindCommands()
{
	/*const FManaPlayerEditorCommands& Commands = FManaPlayerEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.CloseMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Close(); }),
		FCanExecuteAction::CreateLambda([this] { return !ManaPlayer->GetUrl().IsEmpty(); })
	);

	ToolkitCommands->MapAction(
		Commands.ForwardMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->SetRate(GetForwardRate()); }),
		FCanExecuteAction::CreateLambda([this] { return ManaPlayer->IsReady() && ManaPlayer->SupportsRate(GetForwardRate(), false); })
	);

	ToolkitCommands->MapAction(
		Commands.NextMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Next(); }),
		FCanExecuteAction::CreateLambda([this] { return (ManaPlayer->GetPlaylistRef().Num() > 1); })
	);

	ToolkitCommands->MapAction(
		Commands.PauseMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Pause(); }),
		FCanExecuteAction::CreateLambda([this] { return ManaPlayer->CanPause() && !ManaPlayer->IsPaused(); })
	);

	ToolkitCommands->MapAction(
		Commands.PlayMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Play(); }),
		FCanExecuteAction::CreateLambda([this] { return ManaPlayer->IsReady() && (!ManaPlayer->IsPlaying() || (ManaPlayer->GetRate() != 1.0f)); })
	);

	ToolkitCommands->MapAction(
		Commands.PreviousMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Previous(); }),
		FCanExecuteAction::CreateLambda([this] { return (ManaPlayer->GetPlaylistRef().Num() > 1); })
	);

	ToolkitCommands->MapAction(
		Commands.ReverseMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->SetRate(GetReverseRate()); }),
		FCanExecuteAction::CreateLambda([this] { return ManaPlayer->IsReady() && ManaPlayer->SupportsRate(GetReverseRate(), false); })
	);

	ToolkitCommands->MapAction(
		Commands.RewindMedia,
		FExecuteAction::CreateLambda([this] { ManaPlayer->Rewind(); }),
		FCanExecuteAction::CreateLambda([this] { return ManaPlayer->IsReady() && ManaPlayer->SupportsSeeking() && ManaPlayer->GetTime() > FTimespan::Zero(); })
	);*/
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FManaPlayerEditorToolkit::ExtendToolBar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder, const TSharedRef<FUICommandList> ToolkitCommands)
		{
			/*ToolbarBuilder.BeginSection("PlaybackControls");
			{
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().PreviousMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().RewindMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().ReverseMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().PlayMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().PauseMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().ForwardMedia);
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().NextMedia);
			}
			ToolbarBuilder.EndSection();

			ToolbarBuilder.BeginSection("MediaControls");
			{
				ToolbarBuilder.AddToolBarButton(FManaPlayerEditorCommands::Get().CloseMedia);
			}
			ToolbarBuilder.EndSection();*/
		}
	};


	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar, GetToolkitCommands())
	);

	AddToolbarExtender(ToolbarExtender);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


float FManaPlayerEditorToolkit::GetForwardRate() const
{
	float Rate = ManaPlayer->GetRate();

	if (Rate < 1.0f)
	{
		Rate = 1.0f;
	}

	return 2.0f * Rate;
}


float FManaPlayerEditorToolkit::GetReverseRate() const
{
	float Rate = ManaPlayer->GetRate();

	if (Rate > -1.0f)
	{
		return -1.0f;
	}

	return 2.0f * Rate;
}


/* FManaPlayerEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FManaPlayerEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	if (TabIdentifier == ManaPlayerEditorToolkit::DetailsTabId)
	{
		TabWidget = SNew(SManaPlayerEditorDetails, *ManaPlayer);
	}
	/*else if (TabIdentifier == ManaPlayerEditorToolkit::InfoTabId)
	{
		TabWidget = SNew(SManaPlayerEditorInfo, *ManaPlayer, Style);
	}
	else if (TabIdentifier == ManaPlayerEditorToolkit::MediaTabId)
	{
		TabWidget = SNew(SManaPlayerEditorMedia, *ManaPlayer, Style);
	}
	else if (TabIdentifier == ManaPlayerEditorToolkit::PlaylistTabId)
	{
		TabWidget = SNew(SManaPlayerEditorPlaylist, *ManaPlayer, Style);
	}
	else if (TabIdentifier == ManaPlayerEditorToolkit::StatsTabId)
	{
		TabWidget = SNew(SManaPlayerEditorStats, *ManaPlayer, Style);
	}
	else if (TabIdentifier == ManaPlayerEditorToolkit::ViewerTabId)
	{
		TabWidget = SNew(SManaPlayerEditorViewer, *ManaPlayer, Style);
	}*/

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}


#undef LOCTEXT_NAMESPACE
