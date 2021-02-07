/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Mana movie player
 * File     : ManaPlayer.h
 *
 ****************************************************************************/

#pragma once

#include "ManaComponent.h"

#include "ManaPlayer.generated.h"

/**
 * Enumerates Mana related events.
 */
enum class EManaEvent
{
	/** The Mana source started buffering data. */
	Buffering,

	/** The current Mana source has been closed. */
	MovieClosed,

	/** The player started connecting to the Mana source. */
	MovieConnecting,

	/** A new Mana source has been opened. */
	MovieOpened,

	/** A Mana source failed to open. */
	MovieOpenFailed,

	/** The end of the Mana movie (or beginning if playing in reverse) has been reached. */
	PlaybackEndReached,

	/** Playback has been resumed. */
	PlaybackResumed,

	/** Playback has been suspended. */
	PlaybackSuspended,

	/** Seek operation has completed successfully. */
	SeekCompleted,

	/** Mana tracks have changed. */
	TracksChanged
};

/**
 * Mana track types.
 *
 */
UENUM(BlueprintType)
enum class EManaPlayerTrack : uint8
{
	/** Audio track. */
	Audio,

	/** SubAudio track. */ 
	SubAudio, // -> Caption?

	/** ExtraAudio track. */
	ExtraAudio,

	/** Caption track. */
	//Caption,

	/** Metadata track. */
	//Metadata,

	/** Script track. */ 
	//Script, // -> Event Point?

	/** Subtitle track. */
	Subtitle,

	/** Text track. */
	//Text,

	/** Video track. */
	Video,

	/** Alpha track. */
	Alpha
};

class UManaTexture;
class UManaComponent;
enum class EManaComponentStatus : uint8;

/** Multicast delegate that is invoked when a Mana event occurred in the player. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManaPlayerManaEvent);

/** Multicast delegate that is invoked when a Mana player's movie has been opened. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaPlayerMovieOpened, FString, OpenedUrl);

/** Multicast delegate that is invoked when a Mana player's movie has failed to open. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaPlayerMovieOpenFailed, FString, FailedUrl);

/**
 * Implements a Mana player asset that can play Mana movies and other Mana sources.
 */
UCLASS(BlueprintType, hidecategories = (Object))
class CRIWARERUNTIME_API UManaPlayer 
	: public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static const int32 DisabledTrack = -1;

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFrameUpdatedDelegate, int32, FrameNumber);

	UManaPlayer(const FObjectInitializer& ObjectInitializer);
	virtual ~UManaPlayer();

	//~ FTickableGameObject Interface
	
	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override { return true; }
	bool IsTickableWhenPaused() const override { return false; }
	TStatId GetStatId() const override { return TStatId(); }

public:

#if WITH_EDITOR
	/**
	 * Called when PIE has been paused.
	 *
	 * @see ResumePIE
	 */
	void PausePIE();

	/**
	 * Called when PIE has been resumed.
	 *
	 * @see PausePIE
	 */
	void ResumePIE();
#endif

public:

	//~ UObject Interface

	UWorld* GetWorld() const override { return GetOuter()->GetWorld(); }
	FString GetDesc() { return TEXT("UManaPlayer"); }

	////////

	/**
	 * Open the first media source in the specified play list.
	 *
	 * @param InPlaylist The play list to open.
	 * @return true if the source will be opened, false otherwise.
	 * @see Close, OpenFile, OpenPlaylistIndex, OpenSource, OpenUrl, Reopen
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool OpenPlaylist(UManaPlaylist* InPlaylist)
	{
		return OpenPlaylistIndex(InPlaylist, 0);
	}

	/**
	 * Open a particular media source in the specified play list.
	 *
	 * @param InPlaylist The play list to open.
	 * @param Index The index of the source to open.
	 * @return true if the source will be opened, false otherwise.
	 * @see Close, OpenFile, OpenPlaylist, OpenSource, OpenUrl, Reopen
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool OpenPlaylistIndex(UManaPlaylist* InPlaylist, int32 Index);

	/**
	 * Open the next item in the current play list.
	 *
	 * The player will start playing the new media source if it was playing
	 * something previously, otherwise it will only open the media source.
	 *
	 * @return true on success, false otherwise.
	 * @see Close, OpenUrl, OpenSource, Play, Previous, SetPlaylist
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool Next() { check(ManaComponent); return ManaComponent->Next(); }

	/**
	 * Open the previous item in the current play list.
	 *
	 * The player will start playing the new media source if it was playing
	 * something previously, otherwise it will only open the media source.
	 *
	 * @return true on success, false otherwise.
	 * @see Close, Next, OpenUrl, OpenSource, Play, SetPlaylist
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool Previous() { check(ManaComponent); return ManaComponent->Previous(); }

	/**
	 * Get the current play list.
	 *
	 * @return The play list.
	 * @see GetPlaylistIndex, GetPlaylist
	 */
	UManaPlaylist& GetPlaylistRef() const
	{
		check(Playlist != nullptr);
		return *Playlist;
	}

	/**
	 * Get the current play list index.
	 *
	 * @return Play list index.
	 * @see GetPlaylist
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	int32 GetPlaylistIndex() const
	{
		check(ManaComponent); 
		return ManaComponent->GetPlaylistIndex();
	}

	////////

	//~ Settings

	//UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	//void OpenFile(FString InFile);

	//UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	//void OpenPlaylist(UManaPlaylist* InManaPlaylist)
	//{
	//	return OpenPlaylistIndex(InManaPlaylist, 0);
	//}

	//UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	//void OpenPlaylistWithIndex(UManaPlaylist* InManaPlaylist, int32 InIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void OpenMovieSource(UManaMovie* InManaMovie);

	//UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	//void OpenMovieSourceLatent(UManaMovie* InManaMovie);

	//UFUNCTION(BlueprintCallable, Category = "Media|MediaPlayer")
	//bool OpenSourceWithOptions(UManaSource* ManaSource, const FManaPlayerOptions& PlayerOptions);

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void Close();

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void SetTexture(UManaTexture* InManaTexture);

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	UManaTexture* GetTexture() const { return ManaTexture; }

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	UManaMovie* GetMovieSource() const { return (Playlist && ManaComponent) ? ManaComponent->Source.Get() : ManaMovie; }

	/**
	 * Automatically start playback after media opened successfully.
	 *
	 * If disabled, listen to the OnMediaOpened Blueprint event to detect when
	 * the media finished opening, and then start playback using the Play function.
	 *
	 * @see OpenFile, OpenPlaylist, OpenPlaylistIndex, OpenSource, OpenUrl, Play
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mana", meta = (ToolTip = "Automatically start playback after media opened successfully."))
	bool bPlayOnOpen;

	//~ Playback

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void Pause(bool bPause = true);

	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void Seek(FTimespan Time);

	/**
	 * Select the active track of the given type.
	 *
	 * The selected track will use its currently active format. Active formats will
	 * be remembered on a per track basis. The first available format is active by
	 * default. To switch the track format, use SetTrackFormat instead.
	 *
	 * @param TrackType The type of track to select.
	 * @param TrackIndex The index of the track to select, or INDEX_NONE to deselect.
	 * @return true if the track was selected, false otherwise.
	 * @see GetNumTracks, GetSelectedTrack, SetTrackFormat, DisableTrack
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool SelectTrack(EManaPlayerTrack TrackType, int32 TrackIndex);

	/**
	 * Disable the active track of the given type.
	 *
	 * Some tracks cannot be disbaled.
	 *
	 * @param TrackType The type of track to select.
	 * @return true if the track was disbaled, false otherwise.
	 * @see GetNumTracks, GetSelectedTrack, SetTrackFormat
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool DisableTrack(EManaPlayerTrack TrackType);

	/**
	 * Check whether the player is in an error state.
	 *
	 * When the player is in an error state, no further operations are possible.
	 * The current media must be closed, and a new media source must be opened
	 * before the player can be used again. Errors are usually caused by faulty
	 * movie files or interrupted network connections.
	 *
	 * @see IsReady
	 */
	UFUNCTION(BlueprintCallable, Category= "Mana|ManaPlayer")
	bool HasError() const { return ManaComponent ? ManaComponent->GetStatus() == EManaComponentStatus::Error : true; }

	/**
	 * Check whether playback is buffering data.
	 *
	 * @return true if looping, false otherwise.
	 * @see IsConnecting, IsLooping, IsPaused, IsPlaying, IsPreparing, IsReady
	 */
	//UFUNCTION(BlueprintCallable, Category= "Mana|ManaPlayer")
	//bool IsBuffering() const;

	/**
	 * Check whether the player is currently connecting to a movie source.
	 *
	 * @return true if connecting, false otherwise.
	 * @see IsBuffering, IsLooping, IsPaused, IsPlaying, IsPreparing, IsReady
	 */
	//UFUNCTION(BlueprintCallable, Category= "Mana|ManaPlayer")
	//bool IsConnecting() const;

	/**
	 * Check whether playback is looping.
	 *
	 * @return true if looping, false otherwise.
	 * @see IsBuffering, IsConnecting, IsPaused, IsPlaying, IsPreparing, IsReady, SetLooping
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsLooping() const { check(ManaComponent); return ManaComponent->IsLooping(); }

	/**
	 * Check whether playback is currently paused.
	 *
	 * @return true if playback is paused, false otherwise.
	 * @see CanPause, IsBuffering, IsConnecting, IsLooping, IsPaused, IsPlaying, IsPreparing, IsReady, Pause
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsPaused() const { check(ManaComponent); return bIsInitialized && ManaComponent->IsPaused(); }

	/**
	 * Check whether playback has started.
	 *
	 * @return true if playback has started, false otherwise.
	 * @see CanPlay, IsBuffering, IsConnecting, IsLooping, IsPaused, IsPlaying, IsPreparing, IsReady, Play
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsPlaying() const { check(ManaComponent); return bIsInitialized && ManaComponent->IsPlaying(); }

	/**
	 * Check whether the movie is currently opening or buffering.
	 *
	 * @return true if playback is being prepared, false otherwise.
	 * @see CanPlay, IsBuffering, IsConnecting, IsLooping, IsPaused, IsPlaying, IsReady, Play
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsPreparing() const { check(ManaComponent); return bIsInitialized && ManaComponent->IsPreparing(); }

	/**
	 * Whether movie is currently closed.
	 *
	 * @return true if movie is closed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsClosed() const { return !bIsInitialized; }

	/**
	 * Check whether media is ready for playback.
	 *
	 * A player is ready for playback if it has a media source opened that
	 * finished preparing and is not in an error state.
	 *
	 * @return true if media is ready, false otherwise.
	 * @see HasError, IsBuffering, IsConnecting, IsLooping, IsPaused, IsPlaying, IsPreparing
	 */
	UFUNCTION(BlueprintCallable, Category = "Media|MediaPlayer")
	bool IsReady() const { check(ManaComponent); return bIsInitialized && ManaComponent->IsReady(); }


	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool IsSeeking() const { return bIsSeeking; }


	/**
	 * Get the mana movie's duration.
	 *
	 * @return A time span representing the duration.
	 * @see GetTime, Seek
     */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	FTimespan GetDuration() const { return ManaMovie ? FTimespan::FromSeconds(ManaMovie->GetVideoTrackTotalFrames(0) / ManaMovie->GetVideoTrackFrameRate(0)) : 0; }

	/**
     * Enables or disables playback looping.
     *
     * @param Looping Whether playback should be looped.
     * @return true on success, false otherwise.
     * @see IsLooping
     */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool SetLooping(bool Looping) { check(ManaComponent); ManaComponent->Loop(Looping); return true; }

	//UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	//void SetBlockOnTime(FTimespan Time) { check(ManaComponent); ManaComponent->SetBlockOnTime(Time); }
	
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	EManaComponentStatus GetStatus() const { return ManaComponent ? ManaComponent->GetStatus() : EManaComponentStatus::Error; }

	/**
	 * Get the movie's current playback rate.
	 *
	 * @return The playback rate.
	 * @see SetRate, SupportsRate
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	float GetRate() const { check(ManaComponent); return (ManaComponent->IsPlaying() && !ManaComponent->IsPaused()) ? 1.0f: 0.0f; }

	/**
	 * Changes the movie's playback rate.
	 *
	 * @param Rate The playback rate to set.
	 * @return true on success, false otherwise.
	 * @see GetRate, SupportsRate
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	bool SetRate(float Rate) { return false; }

	/**
	 * Get the Mana current total playback time.
	 *
	 * @return Playback time.
	 * @see GetMovieTime, GetDuration, Seek
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	FTimespan GetTime() const { check(ManaComponent); return ManaComponent->GetTime(); }

	/**
	 * Get the current movie playback time.
	 *
	 * @return Playback time.
	 * @see GetTime, GetDuration, Seek
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	FTimespan GetMovieTime() const { check(ManaComponent); return ManaComponent->GetMovieTime(); }

	/**
	 * Get the Mana current total decoded frames.
	 *
	 * @return Playback time.
	 * @see GetMovieFrames
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	int32 GetFrames() const { check(ManaComponent); return ManaComponent->GetFrameNumber(); }

	/**
	 * Get the current movie decoded frames.
	 *
	 * @return Playback time.
	 * @see GetFrames
	 */
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")	
	int32 GetMovieFrames() const { check(ManaComponent); return ManaComponent->GetSource() && (ManaComponent->GetSource()->GetVideoTrackTotalFrames(0) > 0) ? ManaComponent->GetFrameNumber() % ManaComponent->GetSource()->GetVideoTrackTotalFrames(0) : 0; }


	// test
	UFUNCTION(BlueprintCallable, Category = "Mana|ManaPlayer")
	void BindOnFrameUpdated(const FOnFrameUpdatedDelegate& Event);

public:

	/** A delegate that is invoked when playback has reached the end of the media. */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnEndReached;

	/** A delegate that is invoked when a media source has been closed. */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnMovieClosed;

	/**
	 * A delegate that is invoked when a Mana movie source has been opened.
	 *
	 * Depending on whether the underlying player implementation opens the media
	 * synchronously or asynchronously, this event may be executed before or
	 * after the call to OpenSource / OpenUrl returns.
	 *
	 * @see OnMediaOpenFailed, OnTracksChanged
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerMovieOpened OnMovieOpened;

	/**
	 * A delegate that is invoked when a Mana movie source has failed to open.
	 *
	 * This delegate is only executed if OpenSource / OpenUrl returned true and
	 * the media failed to open asynchronously later. It is not executed if
	 * OpenSource / OpenUrl returned false, indicating an immediate failure.
	 *
	 * @see OnMediaOpened
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerMovieOpenFailed OnMovieOpenFailed;

	/**
	 * A delegate that is invoked when movie playback has been resumed.
	 *
	 * @see OnPlaybackSuspended
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnPlaybackResumed;

	/**
	 * A delegate that is invoked when movie playback has been suspended.
	 *
	 * @see OnPlaybackResumed
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnPlaybackSuspended;

	/**
	 * A delegate that is invoked when a seek operation completed successfully.
	 *
	 * Depending on whether the underlying player implementation performs seeks
	 * synchronously or asynchronously, this event may be executed before or
	 * after the call to Seek returns.
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnSeekCompleted;

	/**
	 * A delegate that is invoked when the movie track collection changed.
	 *
	 * @see OnMediaOpened
	 */
	UPROPERTY(BlueprintAssignable, Category="Mana|ManaPlayer", meta = (HideInDetailPanel))
	FOnManaPlayerManaEvent OnTracksChanged;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mana", meta = (ToolTip = "ManaTexture to render."))
	UManaTexture* ManaTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mana", meta = (ToolTip = "ManaMovie to play."))
	UManaMovie* ManaMovie;

	/**
	 * Get an event delegate that is invoked when a Mana event occurred.
	 *
	 * @return The delegate.
	 */
	DECLARE_EVENT_OneParam(UManaPlayer, FOnManaEvent, EManaEvent /*Event*/)
	FOnManaEvent& OnManaEvent()
	{
		return ManaEvent;
	}

	/**
	 * Get an event delegate that is invoked when a next movie frame was sent to RenderThread.
	 *
	 * This delegate if fired only once then removed automatically.
	 * @return The delegate.
	 */
	DECLARE_DELEGATE_OneParam(FOnFrameUpdatedEvent, int32 /*FrameNumber*/)
	FOnFrameUpdatedEvent& OnFrameUpdatedEvent()
	{
		check(ManaComponent);

		ManaComponent->OnFrameOnTime.AddDynamic(this, &UManaPlayer::HandleManaComponentFrameOnTime);
		return FrameUpdatedDelegate;
	}

protected:

	/**
	 * Whether the player should loop when media playback reaches the end.
	 *
	 * Use the SetLooping function to change this value at runtime.
	 *
	 * @see IsLooping, SetLooping
	 */
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Playback)
	//uint32 Loop : 1;

	/**
	 * The play list to use, if any.
	 *
	 * Use the OpenPlaylist or OpenPlaylistIndex function to change this value at runtime.
	 *
	 * @see OpenPlaylist, OpenPlaylistIndex
	 */
	UPROPERTY(BlueprintReadOnly, transient, Category = Playback)
	UManaPlaylist* Playlist;

	/**
	 * The current index of the source in the play list being played.
	 *
	 * Use the Previous and Next methods to change this value at runtime.
	 *
	 * @see Next, Previous
	 */
	//UPROPERTY(BlueprintReadOnly, Category = Playback)
	//int32 PlaylistIndex;

protected:
	/**
	 * Process the given Mana event.
	 *
	 * @param Event The event to process.
	**/
	void ProcessManaEvent(EManaEvent Event);

	UPROPERTY(Transient)
	UManaComponent* ManaComponent;

	bool bIsSeeking;
	bool bIsInitialized;

private:
	/** An event delegate that is invoked when a Mana event occurred. */
	FOnManaEvent ManaEvent;
	/** An event delegate that is invoked when the movie frame was updated. */
	FOnFrameUpdatedEvent FrameUpdatedDelegate;
	/** Dynamic version for BP */
	FOnFrameUpdatedDelegate OnFrameUpdated;
	/** Mana player event queue. */
	TQueue<EManaEvent, EQueueMode::Mpsc> QueuedEvents;

	UFUNCTION()
	void HandleManaComponentSeekCompleted(int32 InFrameNumber, UManaComponent* InManaComponent);
	
	UFUNCTION()
	void HandleManaComponentStatusChanged(EManaComponentStatus Status, UManaComponent* InManaComponent);

	UFUNCTION()
	void HandleManaComponentMovieChanged(UManaMovie* InManaMovie, UManaComponent* InManaComponent);

	UFUNCTION()
	void HandleManaComponentFrameOnTime(int32 InFrameNumber, UManaComponent* InManaComponent);

	UFUNCTION()
	void HandleManaComponentPlayerDestroyed(UManaComponent* InManaComponent);

	void PrepareInternal();

	void StopInternal();

#if WITH_EDITORONLY_DATA
public:
	/** Whether this player should stop when entering or exiting PIE. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = Editor)
	bool AffectedByPIEHandling;

private:
	/** Whether the player was playing in PIE/SIE. */
	bool WasPlayingInPIE;
#endif
};
