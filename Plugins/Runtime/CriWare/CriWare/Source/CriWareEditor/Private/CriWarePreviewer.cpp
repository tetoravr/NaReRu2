/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Previewer
 * File     : CriWarePreviewer.cpp
 *
 ****************************************************************************/

/* モジュールヘッダ */
#include "CriWarePreviewer.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareEditorPrivatePCH.h"
#include "SoundAtomCue.h"
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#include "ManaMovie.h"
#include "ManaTexture.h"
#include "ManaPlayer.h"
#endif	/* </cri_delete_if_LE> */


UCriWarePreviewer* GCriWarePreviewer = nullptr;


void UCriWarePreviewer::Init()
{
	// Add to root.
	AddToRoot();

	AtomPreviewer = MakeUnique<FAtomPreviewer>();

	PreviewWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false, TEXT("CriWareEditorPreviewWorld"));
	PreviewActor = PreviewWorld->SpawnActor(AActor::StaticClass());

	GCriWarePreviewer = this;
}

void UCriWarePreviewer::Shutdown()
{
	// Remove from root.
	RemoveFromRoot();

	ConditionalBeginDestroy();
}

void UCriWarePreviewer::BeginDestroy()
{
	Super::BeginDestroy();

	if (AtomPreviewer.IsValid()) {
		AtomPreviewer->ClearPreviewComponents();
	}

	AtomPreviewer = nullptr;
}

void UCriWarePreviewer::FinishDestroy()
{
	if (GCriWarePreviewer == this) {
		GCriWarePreviewer = nullptr;
	}

	Super::FinishDestroy();
}

void UCriWarePreviewer::PlayPreviewSound(USoundAtomCue* Sound)
{
	if (AtomPreviewer.IsValid()) {
		AtomPreviewer->PlayPreviewSound(Sound);
	}
}

bool UCriWarePreviewer::IsPreviewSoundPlaying(USoundAtomCue* Sound)
{
	if (!AtomPreviewer.IsValid()) {
		return false;
	}

	UAtomComponent* AtomComponent = AtomPreviewer->GetPreviewAtomComponent();
	if (AtomComponent && AtomComponent->Sound == Sound && AtomComponent->IsPlaying()) {
		return true;
	}

	return false;
}

void UCriWarePreviewer::StopPreviewSound()
{
	if (AtomPreviewer.IsValid()) {
		AtomPreviewer->ResetPreviewAtomComponent();
	}
}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */

void UCriWarePreviewer::PlayPreviewMovie(UManaMovie* Movie, UManaTexture* Texture /* = nullptr */)
{
	check(Movie);

	if (!PreviewManaPlayer) {
		PreviewManaPlayer = NewObject<UManaPlayer>(PreviewActor, MakeUniqueObjectName(this, UManaPlayer::StaticClass()));
		PreviewManaPlayer->bPlayOnOpen = true;
	}

	if (Texture) {
		PreviewManaPlayer->SetTexture(Texture);
	}
	
	PreviewManaPlayer->GetTexture()->bRenderToTexture = true; // Render in RGB
	PreviewManaPlayer->OpenMovieSource(Movie);
}

bool UCriWarePreviewer::IsPreviewMoviePlaying(UManaMovie* Movie)
{
	check(Movie);

	if (PreviewManaPlayer && PreviewManaPlayer->GetMovieSource() == Movie && PreviewManaPlayer->IsPlaying()) {
		return true;
	}

	return false;
}

void UCriWarePreviewer::StopPreviewMovie()
{
	if (PreviewManaPlayer) {
		PreviewManaPlayer->Close();
	}
}

#endif	/* </cri_delete_if_LE> */

/* --- end of file --- */
