/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Mana movie asset
 * File     : ManaMovie.cpp
 *
 ****************************************************************************/

#include "ManaMovie.h"

#include "CriWareRuntimePrivatePCH.h"

#include "UObject/Package.h"
#include "Serialization/CustomVersion.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"

#if WITH_EDITORONLY_DATA
#include "EditorFramework/AssetImportData.h"
#endif

#if WITH_EDITOR
#include "ManaPlayer.h"
//#include "Engine/TextureRenderTarget2D.h"
#endif


#define LOCTEXT_NAMESPACE "ManaMovie"

 //----------------------------------------
 // FManaMovieCustomVersion struct
 //----------------------------------------

 /* ManaTextureパッケージのバージョン情報 */
struct FManaMovieCustomVersion
{
	enum Type
	{
		// ManaTextureにバージョン番号が付与される前
		BeforeCustomVersionWasAdded = 0,

		// multiple alpha video tracks support added
		SupportAlphaTracks,

		/* Store a Thumbnail texture */
		StoreThumbnail,

		// 最新バージョンの番号を取得するための番兵
		VersionPlusOne,

		// 最新バージョンの番号を指す
		LatestVersion = VersionPlusOne - 1,
	};

	static const FGuid GUID;
};

const FGuid FManaMovieCustomVersion::GUID(0x122C2078, 0x957272B6, 0xB166BB9A, 0xF49AD29E);

/* シリアライズ時に最新バージョンを埋め込み */
FCustomVersionRegistration GRegisterManaMovieCustomVersion(FManaMovieCustomVersion::GUID, FManaMovieCustomVersion::LatestVersion, TEXT("ManaMovieVer"));

//----------------------------------------
// FManaVideoTrackInfo struct
//----------------------------------------
FManaVideoTrackInfo::FManaVideoTrackInfo()
	:TextureDimensions(0, 0)
	, DisplayDimensions(0, 0)
	, FrameRate(0)
	, TotalFrames(0)
	, bIsAlpha(false)
	, Type(EManaMovieType::Unknown)
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCSTRUCT(StaticStruct());
#endif
}

//----------------------------------------
// FManaAudioTrackInfo struct
//----------------------------------------
FManaAudioTrackInfo::FManaAudioTrackInfo()
	:NumChannels(1)
	, SamplingRate(44100)
	, TotalSamples(0)
	, bIsAmbisonics(false)
	, Type(EManaSoundType::Unknown)
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCSTRUCT(StaticStruct());
#endif
}

//----------------------------------------
// UManaMovie class
//----------------------------------------

#if WITH_EDITOR
uint32 UManaMovie::NumThumbnailGenerators = 0;
#endif
TAtomic<uint32> UManaMovie::NumMovieInfoLoaders(0);

UManaMovie::UManaMovie(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsAlpha(false)
#if WITH_EDITOR
	, ThumbnailFrameNumber(10)
	, ThumbnailDimensions(512, 512)
	, bIsRequestingThumbnail(false)
#endif
	, bIsRequestingMovieInfo(false)
	, ManaPlayer(nullptr)
{
#if WITH_EDITOR
	// Localization of unreal properties metadata with LOCTEXT markups and reflection
	CRI_LOCCLASS(GetClass());
#endif
}

/* UObject  overrides
*****************************************************************************/

#if WITH_EDITOR
void UManaMovie::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UManaMovie::PostInitProperties()
{
#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad)) {
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif
	Super::PostInitProperties();
}

void UManaMovie::PostLoad()
{
	// try reload movie only if needed
	if (Validate() == false) {
		bool bMovieInfoReloadSucceed = ReloadMovieInfo();
		if (!bMovieInfoReloadSucceed) {
			UE_LOG(LogCriWareRuntime, Warning, TEXT("UManaMovie::PostLoad: Failed to parse movie infomation. Please check the movie path."));
		}
	}

#if WITH_EDITORONLY_DATA
	if (AssetImportData == nullptr) {
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	bool bUpdated = false;
	/* 最後にSerializeしたときのバージョン番号を取得 */
	const auto ManaTextureVer = GetLinkerCustomVersion(FManaMovieCustomVersion::GUID);

	/* - UObjectBaseUtility::MarkPackageDirty関数を参考。エディタ上でも保存できるようにする。 */
	if (bUpdated) {
		auto Package = GetOutermost();
		if (Package) {
			Package->SetDirtyFlag(true);
			Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
		}
	}

	bNeedsThumbnailGeneration = true;
#endif

	Super::PostLoad();
}

void UManaMovie::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	/* 書き込み時に最新のバージョン番号を埋め込み */
	Ar.UsingCustomVersion(FManaMovieCustomVersion::GUID);

	int32 archiveVer = Ar.CustomVer(FManaMovieCustomVersion::GUID);

	if (Ar.IsLoading()) {

#if WITH_EDITORONLY_DATA
		bool bUpdated = false;

		if (archiveVer < FManaMovieCustomVersion::StoreThumbnail) {
			bIsRequestingThumbnail = true;
			bUpdated = true;
		}

		if (bUpdated) {
			auto Package = GetOutermost();
			if (Package) {
				Package->SetDirtyFlag(true);
				Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
			}
		}
#endif
	}

	bool bIsEditorDataIncluded = false;
	if (archiveVer >= FManaMovieCustomVersion::StoreThumbnail) {
		FStripDataFlags StripFlags(Ar);
		bIsEditorDataIncluded = !StripFlags.IsEditorDataStripped();
	}

#if WITH_EDITORONLY_DATA
	// captured thumbnail info
	if (bIsEditorDataIncluded) {
		Ar << ThumbnailFrameNumber;
		Ar << ThumbnailDimensions;
	}
#endif
}

// FTickableGameObject interface

void UManaMovie::Tick(float /*DeltaTime*/)
{
	if (bIsRequestingMovieInfo)
	{
		if (ManaPlayer == nullptr)
		{
			if (NumMovieInfoLoaders < MaxSimultaneousMovieInfoLoaders)
			{
				// if not yet loading due to maximum loaders limit, 
				// retry to create a player.
				ManaPlayer = AsyncLoadMovieInfo();
			}
		}
		else
		{
			// check if header has been decoded
			if (IsManaPlayerDecodingHeader(ManaPlayer))
			{
				// if not, execute mana main
				criMana_ExecuteMain();
			}
			else
			{
				// load is over
				bIsRequestingMovieInfo = false; // stop tick
				
				// get info from player, destroy the player and copy info to this ManaMovie
				CriManaMovieInfo MovieInfo = ManaPlayerGetMovieInfo(ManaPlayer, EventPoints);
				DestroyManaPlayer(ManaPlayer);
				ManaPlayer = nullptr;
				LoadMovieInfo(MovieInfo);
			}
		}
	}

#if WITH_EDITOR
	if (bIsRequestingThumbnail)
	{
		CaptureThumbnail(ThumbnailFrameNumber, ThumbnailDimensions);
	}
#endif
}

bool UManaMovie::IsTickable() const
{
#if WITH_EDITOR
	return bIsRequestingMovieInfo || bIsRequestingThumbnail;
#else
	return bIsRequestingMovieInfo;
#endif
}

//~ FTickableGameObject interface

#if WITH_EDITOR
void UManaMovie::CaptureThumbnail(FFrameNumber FrameNumber, FIntPoint Dimensions)
{
	ThumbnailFrameNumber = FrameNumber;
	ThumbnailDimensions = Dimensions;
	bIsRequestingThumbnail = false;
	
	// clear current thumbnail
	/*if (Thumbnail)
	{
		Thumbnail->ReleaseResource();
		Thumbnail = nullptr;
		bNeedsThumbnailGeneration = true;
	}*/

	// check if we can generate it now
	if (NumThumbnailGenerators >= MaxSimultaneousThumbnailGenerators || IsValidating())
	{
		bIsRequestingThumbnail = true;
		return;
	}

	// signal new data
	bNeedsThumbnailGeneration = true;

	if (Validate())
	{
		// Get frame info
		const FFrameTime FrameTime(FrameNumber);
		const int64 FrameRateTicks = GetVideoTrackFrameRate(0) * ETimespan::TicksPerSecond;
		const int64 FrameTicks = FMath::DivideAndRoundNearest(int64(FrameTime.GetFrame().Value * ETimespan::TicksPerSecond * ETimespan::TicksPerSecond), FrameRateTicks);
		const int64 FrameSubTicks = FMath::DivideAndRoundNearest(int64(FrameTime.GetSubFrame() * ETimespan::TicksPerSecond * ETimespan::TicksPerSecond), FrameRateTicks);

		// Create texture
		auto ManaTexture = NewObject<UManaTexture>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UManaMovie::StaticClass()));
		ManaTexture->bRenderToTexture = true; // enable YUV -> RGB convertion
		ManaTexture->TargetGamma = 0.99f; // Compense for UI, force gamma corrected
		ManaTexture->bOverrideDimensions = true; // set output resolution
		ManaTexture->TargetDimensions = Dimensions;

		// create player
		auto Player = NewObject<UManaPlayer>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UManaPlayer::StaticClass()));
		Player->OpenMovieSource(this);
		Player->SetTexture(ManaTexture);
		Player->SelectTrack(EManaPlayerTrack::Audio, UManaPlayer::DisabledTrack); // no sound

		// seek media player to thumbnail position
		const FTimespan EvalTime = FrameTicks + FrameSubTicks;
		const FTimespan MediaTime = EvalTime % Player->GetDuration();

		Player->Seek(MediaTime);

		if (Player->HasError())
		{
			Player->Close();
			// ERROR
			// todo: show error on thumbnail
			bNeedsThumbnailGeneration = false;
			return;
		}

		Player->OnFrameUpdatedEvent().BindLambda([Player](int32 FrameNumber) {
			auto Movie = Player->GetMovieSource();
			auto Texture = Player->GetTexture();

			if (IsValid(Movie))
			{
				// wait rendering
				FlushRenderingCommands();

				int32 SizeX = Texture->GetSurfaceWidth();
				int32 SizeY = Texture->GetSurfaceHeight();
			
				FRenderTarget* RenderTarget = static_cast<FTextureRenderTargetResource*>(Texture->Resource);

				if (!Movie->Thumbnail)
				{
					static const FName ManaMovieThumbnailName(TEXT("_Thumbnail"));
					Movie->Thumbnail = NewObject<UTexture2D>(Movie, MakeUniqueObjectName(Movie, UManaMovie::StaticClass(), ManaMovieThumbnailName));
				}

				auto Texture2D = Movie->Thumbnail;

				Texture2D->Source.Init(SizeX, SizeY, 1, 1, TSF_BGRA8);

				uint32* TextureData = (uint32*)Texture2D->Source.LockMip(0);
				const int32 TextureDataSize = Texture2D->Source.CalcMipSize(0);

				TArray<FColor> SurfData;
				RenderTarget->ReadPixels(SurfData);

				// copy the 2d surface data to the first mip of the static 2d texture
				check(TextureDataSize == SurfData.Num() * sizeof(FColor));
				FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);

				Texture2D->Source.UnlockMip(0);
				Texture2D->CompressionSettings = TC_Default;
				Texture2D->UpdateResource();
			}

			// release mana player
			Player->Close();
			Player->RemoveFromRoot();
			NumThumbnailGenerators--;
		});

		// start generation
		Player->Play();

		if (Player->HasError())
		{
			Player->Close();
			// ERROR
			// todo: show error on thumbnail
			bNeedsThumbnailGeneration = false;
			return;
		}

		// keep mana player
		Player->AddToRoot();
		NumThumbnailGenerators++;
	}
}
#endif

bool UManaMovie::LoadMovieInfo(const CriManaMovieInfo& MovieInfo)
{
	if (MovieInfo.is_playable == 0) {
		return false;
	}

	/* トラック情報を格納する配列を確保 */
	VideoTracks.SetNumUninitialized(MovieInfo.num_video_streams);

	// TODO: FIND A WAY TO DISAMBIGUE USER WITH APLHA STREAM ("2nd DECODER) and ALPHA CHANNEL (1 channel alpha decoded on 2nd decoder)

	/* 動画トラック情報を取得 */
	const CriManaVideoInfo* ManaVideoInfo = MovieInfo.video_prm;
	for (auto& VideoInfo : VideoTracks) {
		VideoInfo.TextureDimensions = FIntPoint(ManaVideoInfo->width, ManaVideoInfo->height);
		VideoInfo.DisplayDimensions = FIntPoint(ManaVideoInfo->disp_width, ManaVideoInfo->disp_height);
		VideoInfo.FrameRate = ManaVideoInfo->framerate / 1000.0f;
		VideoInfo.TotalFrames = ManaVideoInfo->total_frames;
		VideoInfo.bIsAlpha = false; // WARNING: NOW THIS MEANS THAT IS A COLOR VIDEO STREAM TRAGET "SOFDEC 1ST VIDEO DECODER (CALLED VIDEO DECODER)"
		switch (ManaVideoInfo->codec_type) {
		case CRIMANA_VIDEO_CODEC_SOFDEC_PRIME: VideoInfo.Type = EManaMovieType::SofdecPrime; break;
		case CRIMANA_VIDEO_CODEC_H264: VideoInfo.Type = EManaMovieType::H264; break;
		case CRIMANA_VIDEO_CODEC_VP9: VideoInfo.Type = EManaMovieType::VP9; break;
		case CRIMANA_VIDEO_CODEC_UNKNOWN:
		default: VideoInfo.Type = EManaMovieType::Unknown;
		}

		/* 次の動画トラック情報を参照 */
		++ManaVideoInfo;
	}

	/* トラック情報を格納する配列を確保 */
	AlphaTracks.SetNumUninitialized(MovieInfo.num_alpha_streams);

	/* 動画トラック情報を取得 */
	const CriManaAlphaInfo* ManaAlphaInfo = MovieInfo.alpha_prm;
	for (auto& AlphaInfo : AlphaTracks) {
		AlphaInfo.TextureDimensions = FIntPoint(ManaAlphaInfo->width, ManaAlphaInfo->height);
		AlphaInfo.DisplayDimensions = FIntPoint(ManaAlphaInfo->disp_width, ManaAlphaInfo->disp_height);
		AlphaInfo.FrameRate = ManaAlphaInfo->framerate / 1000.0f;
		AlphaInfo.TotalFrames = ManaAlphaInfo->total_frames;
		AlphaInfo.bIsAlpha = true; // ManaAlphaInfo->alpha_type == CRIMANA_COMPO_ALPHFULL; // WARNING: NOW THIS MEANS THAT IS ALPHA STREAM TARGET FOR "SOFDEC 2ND VIDEO DECODER (CALLED ALPHA DECODER)" IT MAY BE COLOR OR ALPHA CHANNELS
		switch (ManaAlphaInfo->codec_type) {
		case CRIMANA_VIDEO_CODEC_SOFDEC_PRIME: AlphaInfo.Type = EManaMovieType::SofdecPrime; break;
		case CRIMANA_VIDEO_CODEC_H264: AlphaInfo.Type = EManaMovieType::H264; break;
		case CRIMANA_VIDEO_CODEC_VP9: AlphaInfo.Type = EManaMovieType::VP9; break;
		case CRIMANA_VIDEO_CODEC_UNKNOWN:
		default: AlphaInfo.Type = EManaMovieType::Unknown;
		}

		/* 次の動画トラック情報を参照 */
		++ManaAlphaInfo;
	}

	bIsAlpha = AlphaTracks.Num() == 1; // WARNING: CURRENT SOFDEC LIMITATION => IF MORE THAN ONE 1 ALPHA STREAM, ALL ALPHA STREAMS ARE CONSIDERED AS COLOR STREAMS SO THE MOVIE IS NOT ALPHA MOVIE 

	// audio
	AudioTracks.SetNumUninitialized(MovieInfo.num_audio_streams);

	const CriManaAudioInfo* ManaAudioInfo = MovieInfo.audio_prm;
	for (auto& AudioInfo : AudioTracks) {
		AudioInfo.NumChannels = ManaAudioInfo->num_channels;
		AudioInfo.SamplingRate = ManaAudioInfo->sampling_rate;
		AudioInfo.TotalSamples = ManaAudioInfo->total_samples;
		AudioInfo.bIsAmbisonics = (ManaAudioInfo->ambisonics > 0);
		switch (ManaAudioInfo->codec_type) {
		case CRIMANA_AUDIO_CODEC_ADX: AudioInfo.Type = EManaSoundType::Adx; break;
		case CRIMANA_AUDIO_CODEC_HCA: AudioInfo.Type = EManaSoundType::HCA; break;
		case CRIMANA_AUDIO_CODEC_UNKNOWN:
		default: AudioInfo.Type = EManaSoundType::Unknown;
		}

		++ManaAudioInfo;
	}

	// subtitles
	NumSubtitleChannels = (int32)MovieInfo.num_subtitle_channels;
	MaxSubtitleSize = MovieInfo.max_subtitle_size;

	return true;
}

bool UManaMovie::MakeSureMovieIsLoaded()
{
	auto SyncManaPlayer = AsyncLoadMovieInfo();
	if (SyncManaPlayer)
	{
		const double Timeout = FPlatformTime::Seconds() + 1.0f; // 1 seconds
		while (IsManaPlayerDecodingHeader(SyncManaPlayer))
		{
			// exceeded timeout ?
			if (FPlatformTime::Seconds() > Timeout)
			{
				UE_LOG(LogCriWareRuntime, Error, TEXT("Cannot load movie information for %s. Exceeded the timeout."), *GetFName().ToString());
				DestroyManaPlayer(SyncManaPlayer);
				return false;
			}
			// excute mana main
			criMana_ExecuteMain();
			// sleep
			FPlatformProcess::Sleep(0.001f);
		}

		// get info from player, destroy the player and copy info to this ManaMovie
		CriManaMovieInfo MovieInfo = ManaPlayerGetMovieInfo(SyncManaPlayer, EventPoints);
		DestroyManaPlayer(SyncManaPlayer);
		LoadMovieInfo(MovieInfo);

		return Validate();
	}

	return false;
}

void UManaMovie::Clear()
{
	// clear all info
	VideoTracks.Empty();
	AlphaTracks.Empty();
	AudioTracks.Empty();
	EventPoints.Empty();
	bIsAlpha = false;
	NumSubtitleChannels = 0;
	MaxSubtitleSize = 0;
}

/* ムービーの情報を取得するため、ムービーのヘッダーを解析する */
CriManaPlayerHn UManaMovie::CreateManaPlayer()
{
	/* Manaを利用するかどうかチェック */
	if (criMana_IsInitialized() == CRI_FALSE) {
		return nullptr;
	}

	/* ヘッダーをデコード */
	CriManaPlayerHn ManaPlayer = criManaPlayer_Create(NULL, 0);
	if (!ManaPlayer) {
		return nullptr;
	}

	criManaPlayer_SetMetaDataWorkAllocator(ManaPlayer,
		[](void *, CriUint32 size) { return FMemory::Malloc(size); },
		[](void *, void* ptr) { return FMemory::Free(ptr); },
		nullptr,
		CRIMANA_META_FLAG_ALL
	);

	NumMovieInfoLoaders++;

	return ManaPlayer;
}

/* ムービーヘッダーのデコード中かどうかを返す */
bool UManaMovie::IsManaPlayerDecodingHeader(CriManaPlayerHn ManaPlayer)
{
	return criManaPlayer_GetStatus(ManaPlayer) < CRIMANAPLAYER_STATUS_WAIT_PREP;
}


void UManaMovie::ManaPlayerDecodeHeader(CriManaPlayerHn ManaPlayer)
{
	if (ManaPlayer == nullptr) { return; }

	criManaPlayer_DecodeHeader(ManaPlayer);
}

CriManaMovieInfo UManaMovie::ManaPlayerGetMovieInfo(CriManaPlayerHn ManaPlayer, TArray<FManaEventPointInfo>& EventPoints)
{
	/* 失敗時には、再生不可能なムービー情報として返す。 */
	CriManaMovieInfo MovieInfo;
	MovieInfo.is_playable = 0;

	if (ManaPlayer == nullptr) { return MovieInfo; }

	/* ムービー情報を取得 */
	criManaPlayer_GetMovieInfo(ManaPlayer, &MovieInfo);

	/* キューポイント情報を取得 */
	const CriManaCuePointInfo* CuePointInfo = criManaPlayer_GetCuePointInfo(ManaPlayer);
	if ((CuePointInfo != nullptr) && (CuePointInfo->num_eventpoint > 0)) {
		/* イベントポイント情報を格納する配列を確保 */
		EventPoints.SetNumZeroed(CuePointInfo->num_eventpoint);

		/* イベントポイント情報の保存 */
		const CriManaEventPoint* EventPoint = CuePointInfo->eventtable;
		for (auto& EventPointInfo : EventPoints) {
			EventPointInfo.Name = ((EventPoint->cue_name != nullptr) ? (UTF8_TO_TCHAR(EventPoint->cue_name)) : TEXT(""));
			EventPointInfo.Time = ((EventPoint->tunit > 0) ? ((float)EventPoint->time / (float)EventPoint->tunit) : 0.0f);
			EventPointInfo.Type = EventPoint->type;
			EventPointInfo.Parameter = ((EventPoint->param_string != nullptr) ? (UTF8_TO_TCHAR(EventPoint->param_string)) : TEXT(""));
			++EventPoint;
		}
	}

	return MovieInfo;
}

void UManaMovie::ManaPlayerSetFile(CriManaPlayerHn ManaPlayer, const FString& FilePath)
{
	if (ManaPlayer == nullptr) { return; }
	criManaPlayer_SetFile(ManaPlayer, NULL, TCHAR_TO_UTF8(*FilePath));
}

void UManaMovie::ManaPlayerSetData(CriManaPlayerHn ManaPlayer, const TArray<uint8>& DataArray)
{
	if (ManaPlayer == nullptr) { return; }
	criManaPlayer_SetData(ManaPlayer, DataArray.GetData(), DataArray.Num());
}

void UManaMovie::DestroyManaPlayer(CriManaPlayerHn ManaPlayer)
{
	if (ManaPlayer == nullptr) { return; }

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ManaPlayer]()
	{
		if (criMana_IsInitialized() != CRI_FALSE)
		{
			// wait mana to stop all operations on player
			criManaPlayer_StopAndWaitCompletion(ManaPlayer);

			// destroy player
			criManaPlayer_Destroy(ManaPlayer);

			NumMovieInfoLoaders--;
		}
	});
}

//----------------------------------------
// UFileManaMovie class
//----------------------------------------

UFileManaMovie::UFileManaMovie(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// Localization of unreal properties metadata with LOCTEXT markups and reflection
	CRI_LOCCLASS(GetClass());
#endif
}

/* UObject  overrides
*****************************************************************************/

#if WITH_EDITOR
void UFileManaMovie::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bool bIsSourceUpdated = false;

	if (PropertyChangedEvent.Property != nullptr)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();

#if WITH_EDITORONLY_DATA
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UManaMovie, AssetImportData->SourceData))
		{
			SetFilePath(AssetImportData->GetFirstFilename());

			bIsSourceUpdated = true;
		}
#endif
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UFileManaMovie, FilePath))
		{	
			Clear();
			
			bool bMovieInfoReloadSucceed = !FilePath.IsEmpty() && MakeSureMovieIsLoaded();
			if (!bMovieInfoReloadSucceed) {
				AssetImportData->Update(TEXT(""));
				UE_LOG(LogCriWareRuntime, Warning, TEXT("UManaMovie::PostEditChangeProperty: Failed to parse movie infomation. Please check the movie file path."));
			} else {
				AssetImportData->Update(GetFullPath());
			}

			bIsSourceUpdated = true;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIsSourceUpdated)
	{
		CaptureThumbnail(ThumbnailFrameNumber, ThumbnailDimensions);
	}
}
#endif

/* UManaMovie overrides
*****************************************************************************/

FString UFileManaMovie::GetUrl() const
{
	FString FullPath = GetFullPath();
	if (!FullPath.IsEmpty()) {
		return FString(TEXT("file://")) + FullPath;
	}
	return FullPath;
}

bool UFileManaMovie::Validate() const
{
	if (!FPaths::FileExists(GetFullPath())) {
		return false;
	}

	if (VideoTracks.Num() <= 0) {
		return false;
	}

	return true;
}

bool UFileManaMovie::ReloadMovieInfo()
{
	if (IsValidating()) { return true; }

	// clear data
	Clear();
	// check if path is present
	if (FilePath.IsEmpty()) {
		return false;
	}

	if (!GIsCookerLoadingPackage && 
		!GIsEditorLoadingPackage)
	{
		// request movie info to do not block the game
		ManaPlayer = AsyncLoadMovieInfo();
	}
	else
	{
		// synchroneous movie info loading while editor or cooker is loading
		return MakeSureMovieIsLoaded();
	}

	return true;
}

CriManaPlayerHn UFileManaMovie::AsyncLoadMovieInfo()
{
	bIsRequestingMovieInfo = true;

	// check if we can create a Mana player, if not ManaPlayer stay to nullptr.
	if (NumMovieInfoLoaders >= MaxSimultaneousMovieInfoLoaders)
	{
		return nullptr;
	}

	// create the player
	CriManaPlayerHn AsyncManaPlayer = CreateManaPlayer();
	if (AsyncManaPlayer)
	{
		// setup path
		FString Path = GetFullPath();
		ManaPlayerSetFile(AsyncManaPlayer, Path);
		// request decode header
		ManaPlayerDecodeHeader(AsyncManaPlayer);
	}

	return AsyncManaPlayer;
}


/* UFileManaMovie implementation
*****************************************************************************/

void UFileManaMovie::SetFilePath(const FString& Path)
{
	FilePath = Path;
	ReloadMovieInfo();
}

FString UFileManaMovie::GetFullPath() const
{
	if (!FPaths::IsRelative(FilePath)) {
		return FilePath;
	}

	if (FilePath.StartsWith(TEXT("./"))) {
		return FPaths::ConvertRelativePathToFull(UCriWareInitializer::GetContentDir(), FilePath.RightChop(2));
	}

	return FPaths::ConvertRelativePathToFull(UCriWareInitializer::GetContentDir(), FilePath);
}

//----------------------------------------
// UDataManaMovie class
//----------------------------------------

UDataManaMovie::UDataManaMovie(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// Localization of unreal properties metadata with LOCTEXT markups and reflection
	CRI_LOCCLASS(GetClass());
#endif
}

bool UDataManaMovie::ReloadMovieInfo()
{
	if (IsValidating()) { return true; }

	// clear movie info data
	Clear();

	// Check if data are present
	if (DataArray == nullptr || DataArray->Num() == 0) {
		return false;
	}

	if (!GIsCookerLoadingPackage &&
		!GIsEditorLoadingPackage)
	{
		// request movie info to do not block the game
		ManaPlayer = AsyncLoadMovieInfo();
	}
	else
	{
		// synchroneous movie info loading while editor or cooker is loading
		return MakeSureMovieIsLoaded();
	}

	return true;
}

CriManaPlayerHn UDataManaMovie::AsyncLoadMovieInfo()
{
	bIsRequestingMovieInfo = true;

	// check if we can create a Mana player, if not ManaPlayer stay to nullptr.
	if (NumMovieInfoLoaders >= MaxSimultaneousMovieInfoLoaders)
	{
		return nullptr;
	}

	// create the player
	CriManaPlayerHn AsyncManaPlayer = CreateManaPlayer();
	if (AsyncManaPlayer)
	{
		// setup data
		ManaPlayerSetData(AsyncManaPlayer, *DataArray);
		// request decode header
		ManaPlayerDecodeHeader(AsyncManaPlayer);
	}

	return AsyncManaPlayer;
}

void UDataManaMovie::SetDataArray(TArray<uint8>& InDataArray)
{
	DataArray = &InDataArray;
	ReloadMovieInfo();
}

bool UDataManaMovie::Validate() const
{
	if (DataArray == nullptr || DataArray->Num() == 0) {
		return false;
	}

	if (VideoTracks.Num() <= 0) {
		return false;
	}

	return true;
}

//----------------------------------------
// UStreamManaMovie class
//----------------------------------------

UStreamManaMovie::UStreamManaMovie(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// Localization of unreal properties metadata with LOCTEXT markups and reflection
	CRI_LOCCLASS(GetClass());
#endif
}

/* UManaMovie overrides
 *****************************************************************************/

FString UStreamManaMovie::GetUrl() const
{
	return StreamUrl;
}


bool UStreamManaMovie::Validate() const
{
	return StreamUrl.Contains(TEXT("://"));
}

bool UStreamManaMovie::ReloadMovieInfo()
{
	return true;
}

CriManaPlayerHn UStreamManaMovie::AsyncLoadMovieInfo()
{
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
