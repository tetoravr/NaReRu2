/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Mana movie player
 * File     : ManaPlayer.cpp
 *
 ****************************************************************************/

#include "ManaPlayer.h"

#include "CriWareRuntimePrivatePCH.h"

#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/LocalPlayer.h"

//#include "IManaEventSink.h"
#include "ManaComponent.h"
#include "ManaTexture.h"
#include "CriWareLocalization.h"


#define LOCTEXT_NAMESPACE "ManaPlayer"


UManaPlayer::UManaPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bPlayOnOpen(false)
	, bIsSeeking(false)
	, bIsInitialized(false)
#if WITH_EDITORONLY_DATA
	, AffectedByPIEHandling(true)
	, WasPlayingInPIE(false)
#endif
{
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		ManaComponent = NewObject<UManaComponent>(this, MakeUniqueObjectName(this, UManaComponent::StaticClass()));
		ManaTexture = NewObject<UManaTexture>(this, MakeUniqueObjectName(this, UManaTexture::StaticClass()));

		UWorld* World = GetWorld();
		if (World) {
			ManaComponent->RegisterComponentWithWorld(World); // this call InitializeComponent()
			ManaComponent->SetTexture(ManaTexture);
		}
		else {
			ManaComponent->bRegistered = true; // forced registration
		}

		ManaComponent->bTickInEditor = false;

		ManaComponent->OnSeekCompleted.AddDynamic(this, &UManaPlayer::HandleManaComponentSeekCompleted);
		ManaComponent->OnStatusChanged.AddDynamic(this, &UManaPlayer::HandleManaComponentStatusChanged);
		ManaComponent->OnMovieChanged.AddDynamic(this, &UManaPlayer::HandleManaComponentMovieChanged);
		ManaComponent->OnPlayerDestroyed.AddDynamic(this, &UManaPlayer::HandleManaComponentPlayerDestroyed);
	}

#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCCLASS(GetClass());
#endif
}

UManaPlayer::~UManaPlayer()
{ 

}

void UManaPlayer::Tick(float DeltaTime)
{
	// process deferred events
	EManaEvent Event;
	while (QueuedEvents.Dequeue(Event)) {
		ProcessManaEvent(Event);
	}

	if (ManaComponent && ManaComponent->IsActive())
	{
		ManaComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}
}

bool UManaPlayer::IsTickable() const
{
	return ManaComponent && (ManaComponent->IsActive() || !QueuedEvents.IsEmpty());
}

#if WITH_EDITOR

void UManaPlayer::PausePIE()
{
	UE_LOG(LogCriWareRuntime, Verbose, TEXT("%s.PausePIE"), *GetFName().ToString());

	WasPlayingInPIE = !IsPaused();

	if (WasPlayingInPIE)
	{
		Pause();
	}
}


void UManaPlayer::ResumePIE()
{
	UE_LOG(LogCriWareRuntime, Verbose, TEXT("%s.ResumePIE"), *GetFName().ToString());

	if (WasPlayingInPIE)
	{
		Play();
	}
}

#endif

void UManaPlayer::OpenMovieSource(UManaMovie* InManaMovie)
{
	Close();

	check(ManaComponent);
	check(ManaTexture);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.OpenMovieSource"), *GetFName().ToString());

	if (!bIsInitialized) {
		bIsInitialized = true;
		if (!ManaComponent->HasBeenInitialized())
		{
			ManaComponent->InitializeComponent();
			ManaComponent->SetTexture(ManaTexture);
		}		
		// default player options
		ManaComponent->SetMaxDropFrames(EManaMaxFrameDrop::Disabled);
		ManaComponent->SetPlaybackTimer(EManaPlaybackTimer::RealTime);
		ManaComponent->SetEndFrameAction(EManaFrameAction::Clear);
	}

	ManaMovie = InManaMovie;
	ManaTexture->SetMovieSource(ManaMovie);

	if (bPlayOnOpen) {
		Play();
	} else {
		PrepareInternal();
	}
}

bool UManaPlayer::OpenPlaylistIndex(UManaPlaylist* InPlaylist, int32 Index)
{
	Close();

	check(ManaComponent);
	check(ManaTexture);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.OpenPlaylistIndex"), *GetFName().ToString());

	if (!bIsInitialized) {
		bIsInitialized = true;
		if (!ManaComponent->HasBeenInitialized())
		{
			ManaComponent->InitializeComponent();
			ManaComponent->SetTexture(ManaTexture);
		}
		// default player options
		ManaComponent->SetMaxDropFrames(EManaMaxFrameDrop::Disabled);
		ManaComponent->SetPlaybackTimer(EManaPlaybackTimer::RealTime);
		ManaComponent->SetEndFrameAction(EManaFrameAction::Hold);
	}

	ManaMovie = nullptr;
	Playlist = nullptr;
	//PlaylistIndex = 0;

	ManaTexture->SetMovieSource(InPlaylist);
	bool Result = ManaComponent->ToIndex(Index);

	Playlist = InPlaylist;
	//PlaylistIndex = Index;

	if (bPlayOnOpen) {
		Play();
	} else {
		PrepareInternal();
	}

	return Result;
}

void UManaPlayer::SetTexture(UManaTexture* InManaTexture)
{
	check(ManaComponent);

	ManaTexture = InManaTexture;
	if (ManaTexture == nullptr) {
		ManaTexture = NewObject<UManaTexture>(this, MakeUniqueObjectName(this, UManaTexture::StaticClass()));
	}
	if (ManaMovie) {
		ManaTexture->SetMovieSource(ManaMovie);
	}
	ManaComponent->SetTexture(ManaTexture);
}

void UManaPlayer::Close()
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Close"), *GetFName().ToString());

	if (bIsInitialized) {
		ManaComponent->UninitializeComponent();
		bIsInitialized = false;
	}

	Playlist = nullptr;
	ManaMovie = nullptr;
}

void UManaPlayer::Play()
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Play"), *GetFName().ToString());
	ManaComponent->Play();
}

void UManaPlayer::Pause(bool bPause /* = true */)
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Pause"), *GetFName().ToString());
	ManaComponent->Pause(bPause);
	if (bPause) {
		QueuedEvents.Enqueue(EManaEvent::PlaybackSuspended);
	} else {
		QueuedEvents.Enqueue(EManaEvent::PlaybackResumed);
	}
}

void UManaPlayer::Seek(FTimespan Time)
{
	check(ManaComponent);

	bIsSeeking = true;
	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Seek %s"), *GetFName().ToString(), *Time.ToString(TEXT("%h:%m:%s.%t")));
	ManaComponent->Seek(Time);
}

bool UManaPlayer::SelectTrack(EManaPlayerTrack TrackType, int32 TrackIndex)
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, Verbose, TEXT("%s.SelectTrack %s %i"), *GetFName().ToString(), *UEnum::GetValueAsString(TEXT("CriWareRuntime.EManaPlayerTrack"), TrackType), TrackIndex);
	
	switch (TrackType) {
	case EManaPlayerTrack::Audio:
		ManaComponent->SetAudioTrack(TrackIndex);
		return true;
	case EManaPlayerTrack::SubAudio:
		ManaComponent->SetSubAudioTrack(TrackIndex);
		return true;
	case EManaPlayerTrack::ExtraAudio:
		ManaComponent->SetExtraAudioTrack(TrackIndex);
		return true;
	case EManaPlayerTrack::Subtitle:
		if (TrackIndex > UManaPlayer::DisabledTrack)
		{
			ManaComponent->ChangeSubtitlesChannel(TrackIndex);
		} else {
			ManaComponent->EnableSubtitles(false);
		}
		return true;
	case EManaPlayerTrack::Alpha:
		ManaComponent->SetAlphaTrack(TrackIndex);
		return true;
	}
	
	return false;
}

bool UManaPlayer::DisableTrack(EManaPlayerTrack TrackType)
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, Verbose, TEXT("%s.DisableTrack %s"), *GetFName().ToString(), *UEnum::GetValueAsString(TEXT("CriWareRuntime.EManaPlayerTrack"), TrackType));

	switch (TrackType) {
	case EManaPlayerTrack::Audio:
		ManaComponent->SetAudioTrack(UManaPlayer::DisabledTrack);
		return true;
	case EManaPlayerTrack::SubAudio:
		ManaComponent->SetSubAudioTrack(UManaPlayer::DisabledTrack);
		return true;
	case EManaPlayerTrack::ExtraAudio:
		ManaComponent->SetExtraAudioTrack(UManaPlayer::DisabledTrack);
		return true;
	case EManaPlayerTrack::Subtitle:
		ManaComponent->EnableSubtitles(false);
		return true;
	}

	return false;
}

void UManaPlayer::BindOnFrameUpdated(const FOnFrameUpdatedDelegate& Event)
{
	if (ManaComponent)
	{
		OnFrameUpdated = Event;
		ManaComponent->OnFrameOnTime.AddDynamic(this, &UManaPlayer::HandleManaComponentFrameOnTime);
	}
}

void UManaPlayer::ProcessManaEvent(EManaEvent Event)
{
	switch (Event)
	{
	case EManaEvent::Buffering:
		//OnBuffering.Broadcast();
		break;
	case EManaEvent::MovieConnecting:
		//OnMovieConnecting.Broadcast();
		break;
	case EManaEvent::MovieOpened:
		OnMovieOpened.Broadcast(ManaComponent->GetSource()->GetUrl());
		break;
	case EManaEvent::MovieOpenFailed:
		OnMovieOpenFailed.Broadcast(ManaComponent->GetSource()->GetUrl());
		break;
	case EManaEvent::MovieClosed:
		OnMovieClosed.Broadcast();
		break;
	case EManaEvent::PlaybackEndReached:
		OnEndReached.Broadcast();
		break;
	case EManaEvent::PlaybackResumed:
		OnPlaybackResumed.Broadcast();
		break;
	case EManaEvent::PlaybackSuspended:
		OnPlaybackSuspended.Broadcast();
		break;
	case EManaEvent::SeekCompleted:
		OnSeekCompleted.Broadcast();
		break;
	case EManaEvent::TracksChanged:
		OnTracksChanged.Broadcast();
		break;
	default:
		break;
	}

	ManaEvent.Broadcast(Event);
}

//~ Mana event handeling > TODO: move event to Component and add Multicast delegate for each event in player

void UManaPlayer::HandleManaComponentSeekCompleted(int32 InFrameNumber, UManaComponent* InManaComponent)
{
	bIsSeeking = false;

	QueuedEvents.Enqueue(EManaEvent::SeekCompleted);
}


void UManaPlayer::HandleManaComponentStatusChanged(EManaComponentStatus Status, UManaComponent* InManaComponent)
{
	switch (Status)
	{
	case EManaComponentStatus::DecHdr:
		QueuedEvents.Enqueue(EManaEvent::Buffering);
		break;
	case EManaComponentStatus::Playing:
		if (InManaComponent->IsPaused()) {
			QueuedEvents.Enqueue(EManaEvent::PlaybackSuspended);
		} else {
			QueuedEvents.Enqueue(EManaEvent::PlaybackResumed);
		}
		break;
	case EManaComponentStatus::Stop:
		break;
	case EManaComponentStatus::StopProcessing:
		//QueuedEvents.Enqueue(EManaEvent::PlaybackSuspended);
		break;
	case EManaComponentStatus::PlayEnd:
		QueuedEvents.Enqueue(EManaEvent::PlaybackEndReached);
		break;
	case EManaComponentStatus::Ready:
		QueuedEvents.Enqueue(EManaEvent::MovieOpened);
		break;
	case EManaComponentStatus::Error:
		QueuedEvents.Enqueue(EManaEvent::MovieOpenFailed);
		break;
	default:
		break;
	}
}

void UManaPlayer::HandleManaComponentPlayerDestroyed(UManaComponent* InManaComponent)
{
	QueuedEvents.Enqueue(EManaEvent::MovieClosed);
}

void UManaPlayer::HandleManaComponentMovieChanged(UManaMovie* InManaMovie, UManaComponent* InManaComponent)
{
	QueuedEvents.Enqueue(EManaEvent::TracksChanged);
}

void UManaPlayer::HandleManaComponentFrameOnTime(int32 InFrameNumber, UManaComponent* InManaComponent)
{
	OnFrameUpdated.ExecuteIfBound(InFrameNumber);
	OnFrameUpdated.Unbind();

	FrameUpdatedDelegate.ExecuteIfBound(InFrameNumber);
	FrameUpdatedDelegate.Unbind();
}

void UManaPlayer::PrepareInternal()
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Prepare"), *GetFName().ToString());

	if (ManaComponent->GetStatus() == EManaComponentStatus::Stop ||
		ManaComponent->GetStatus() == EManaComponentStatus::PlayEnd)
	{
		ManaComponent->Prepare();
	}
}

void UManaPlayer::StopInternal()
{
	check(ManaComponent);

	UE_LOG(LogCriWareRuntime, VeryVerbose, TEXT("%s.Stop"), *GetFName().ToString());

	ManaComponent->Stop();
}

#undef LOCTEXT_NAMESPACE
