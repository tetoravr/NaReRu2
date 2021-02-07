/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2015-2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Mana Movie Texture Asset
 * File     : ManaTexture.h
 *
 ****************************************************************************/

#pragma once

#include "Tickable.h"
#include "CriWareApi.h"
#include "ManaComponentTexture.h"
#include "ManaMovie.h"
#include "ManaTexture.generated.h"


class UManaSource;
class UManaComponent;

/** Movie Frame consumed by UManaTexture */
struct CRIWARERUNTIME_API FManaFrame
{
	CriManaFrameInfo Info;
	CriManaTextureBuffer Buffers[6];

	FManaFrame();
	FManaFrame(const CriManaFrameInfo& Info);

	void Invalidate(); // set in an invalid status
	bool IsValid();
};

/* Movie Formats */
enum class EManaMovieFormat : uint8
{
	/** YUV 4:2:0 bi-planar */
	NV12,
	/** YUV 4:2:0 tri-planar */
	IYUV,
	/** RGB 4:4:4 packed */
	RGB,
	/** OpenGL External Texture */
	ExternalTexture,
	/** No output */
	None,
	Num,

	PRIME = IYUV,
	DEFAULT = IYUV,
};

enum class EManaMovieVideoType : uint8
{
	/** */
	Video,
	/** */
	Alpha,
	Num
};

UCLASS(hidecategories = (Object, Texture, Adjustments, Compression, TextureRenderTarget2D, LevelOfDetail, Compositing), meta = (ToolTip = "ManaTexture class."))
class CRIWARERUNTIME_API UManaTexture : public UTexture, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTextureUpdated, UManaTexture*, ManaTexture);

	// Friends.
	friend class FManaPlayerMovieStreamer;
	friend class UManaComponent;

public:
	//~ Begin UObject interface
#if WITH_EDITOR
	/** USMファイルパスがエディタ上で変更されたときの処理 */
	 virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual bool CanBeInCluster() const override { return false; }
	virtual void BeginDestroy() override;
	//~ End UObject interface

	//~ Begin FTickableGameObject interface
	void Tick(float DeltaTime) override;
	bool IsTickable() const override { return bIsValidatingCurrentMovie; }
	bool IsTickableInEditor() const override { return true; }
	bool IsTickableWhenPaused() const override { return true; }
	TStatId GetStatId() const override { return TStatId(); }
	//~ End FTickableGameObject interface

	//~ Begin UTexture interface
	/** 描画用のリソースを作成して返す。 */
	virtual FTextureResource* CreateResource() override;
	virtual void UpdateResource() override;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 17
	virtual EMaterialValueType GetMaterialType() const override;
#else
	virtual EMaterialValueType GetMaterialType() override;
#endif
	virtual float GetSurfaceWidth() const override;
	virtual float GetSurfaceHeight() const override;
	//~ End UTexture interface

	/**
	* Gets the width and height of the texture (in pixels).
	*
	* @return Texture dimensions.
	*/
	FORCEINLINE FIntPoint GetDimensions() const
	{
		return bOverrideDimensions ? TargetDimensions : CachedDimensions;
	}

	/**
	* Gets the texture's pixel format.
	*
	* @return Pixel format (always PF_B8G8R8A8 for all movie textures).
	*/
	TEnumAsByte<enum EPixelFormat> GetFormat() const
	{
		if (OverrideFormat == PF_Unknown) {
			return bHDR ? PF_FloatRGBA : PF_B8G8R8A8;
		}
		return OverrideFormat;
	}

	/**
	* Gets the output movie's pixel format from mana decoder.
	* Some hardware decoders outputs only to specific pixel format.
	*
	* @return Output Movie Pixel format (may be different by platform except for Prime movies that always output to IYUV).
	*/
	FORCEINLINE EManaMovieFormat GetOutputMovieFormat() const
	{
		return OutputMovieFormat;
	}

	/* UManaTexture固有API */

	/** アルファムービかどうかを返す */
	FORCEINLINE bool IsAlphaMovie() const { return bIsAlpha; }

	/** 描画準備が完了しているかどうか */
	bool IsRenderResourcePrepared() const;

	/** */
	UFUNCTION(BlueprintCallable, Category = Mana, meta = (ToolTip = "Sets the ManaSource for this texture."))
	void SetMovieSource(UManaSource* InSource);

	/** */
	FORCEINLINE UManaSource* GetMovieSource() const { return MovieSource; }

	/** Set the player object used with this VIPTexture - (internal) */
	//FORCEINLINE void SetManaPlayer(UManaComponent* InVIPPlayer) { ManaPlayer = InVIPPlayer; }

	/** Get the player object used with this VIPTexture */
	FORCEINLINE UManaComponent* GetManaPlayer() const { return ManaPlayer; }

	/** */
	FORCEINLINE int32 GetNumComponentTextures() const { return ComponentTextures.Num(); }

	/** */
	FORCEINLINE UManaComponentTexture* GetComponentTexture(int32 Index) const
	{ 
		return ComponentTextures.IsValidIndex(Index) ? ComponentTextures[Index] : nullptr;
	}

	/** */
	int32 GetNumAlphaComponentTextures() const { return AlphaComponentTextures.Num(); }

	/** */
	FORCEINLINE UManaComponentTexture* GetAlphaComponentTexture(int32 Index) const
	{
		return AlphaComponentTextures.IsValidIndex(Index) ? AlphaComponentTextures[Index] : nullptr;
	}

protected:
	/** Mana Source */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mana, meta = (ToolTip = "Movie source."))
	UManaSource* MovieSource;

private:
	//~ Deprecated
	/** USMファイルへの相対パス DEPRECATED */
	UPROPERTY()
	FString MovieFilePath_DEPRECATED;
	//~ End Deprecated

public:
	/** Render movie to single RGBA texture with render target. May slow with 2k/4k movies. You shouln't use this for VR movies.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana)
	uint32 bRenderToTexture : 1;

	//~ Begin Render to Texture sub menu
	/** Gamma adjustment (raises linear-space RGB color to the specified power.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana|Render to Texture", meta = (DisplayName = "Gamma Adjustment"))
	float TargetGamma;

	/** Whether to support storing HDR values, which requires more memory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mana|Render to Texture", meta = (DisplayName = "HDR"), AssetRegistrySearchable)
	uint32 bHDR : 1;
	//~ End Render to Texture sub menu

	/** The addressing mode to use for the X axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana, meta = (DisplayName = "Address X"), AssetRegistrySearchable, AdvancedDisplay)
	TEnumAsByte<enum TextureAddress> AddressX;

	/** The addressing mode to use for the Y axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana, meta = (DisplayName = "Address Y"), AssetRegistrySearchable, AdvancedDisplay)
	TEnumAsByte<enum TextureAddress> AddressY;

	/** The color used to clear the texture if no video data is drawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana, meta = (DisplayName = "Clear Color"), AssetRegistrySearchable, AdvancedDisplay)
	FLinearColor ClearColor;

	/** Enable this to render to a diffent size than the movie source. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana, meta = (DisplayName = "Override Dimensions"), AssetRegistrySearchable, AdvancedDisplay)
	uint32 bOverrideDimensions : 1;

	/** The dimensions to use to render the movie. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mana, meta = (DisplayName = "Texture Dimensions", EditCondition = "bOverrideDimensions"), AssetRegistrySearchable, AdvancedDisplay)
	FIntPoint TargetDimensions;

	/** The format of the texture data.
	 *
	 * Normally the format is derived from bHDR, this allows code to set the format explicitly. 
	 */
	UPROPERTY()
	TEnumAsByte<enum EPixelFormat> OverrideFormat;

	/** Event for updates. */
	UPROPERTY(BlueprintAssignable)
	FOnTextureUpdated OnTextureUpdated;

protected:
	/** Component texures of the main video track. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mana, AdvancedDisplay)
	TArray<UManaComponentTexture*> ComponentTextures;

	/** Component texures of the alpha video track. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mana, AdvancedDisplay)
	TArray<UManaComponentTexture*> AlphaComponentTextures;

	/** The mana player used by this texture */
	UPROPERTY()
	UManaComponent* ManaPlayer;

public:
	/** init */
	virtual void Init(const FString& InMovieFilePath = "");

	/** set current index movie from play list and update texture. */
	bool SetPlaylistMovie(int32 Index);

	/** currently used ManaMovie by ManaTexture */
	FORCEINLINE UManaMovie* GetMovie() const { return CurrentMovie.Get(); }

	/** ManaTextureResourceから、書き込み先フレームを取得 */
	FManaFrame GetWritableFrame();

	/** ManaTextureResourceへ、書き込み済みフレームを登録 */
	void SubmitFrame(const FManaFrame& InManaFrame);

	/* Iterate over free frame queue. */
	uint32 IterateWritableFrames(TFunctionRef<void(FManaFrame& ManaFrame)> Iter);

	/** Clear and reset queued frames. */
	void ResetFrameQueues();

	/** Check if the texture is streamed from movie source. */
	FORCEINLINE bool IsStreamed() const { return bIsStreamed; }

	/** Flag the texture if streamed from movie source. */
	void SetStreamed(bool IsStreamed) { bIsStreamed = IsStreamed; }

protected:
	static const FIntPoint DefaultDimensions;
	static const FIntPoint DefaultTargetDimensions;

	/** The texture's cached width and height (in pixels). */
	FIntPoint CachedDimensions;

	/** If the movie use alpha channel. */
	uint32 bIsAlpha : 1;

	/** If the texture use texture streaming from decoder. */
	uint32 bIsStreamed : 1;

	/** If awaiting current movie information */
	uint32 bIsValidatingCurrentMovie : 1;

	/** Format used by decoder to output the movie. */
	EManaMovieFormat OutputMovieFormat;

	/** Format used by decoder to output the alpha track. */
	EManaMovieFormat OutputAlphaMovieFormat;

	/** update needed format to display the movie */
	virtual void UpdateOutputMovieFormat(UManaMovie* Movie);

	/** Create or update component textures to the desired movie format */
	void UpdateComponentTextures(FIntPoint InSize);
	void UpdateComponentTextures(EManaMovieFormat InMovieFormat, FIntPoint InSize, EManaMovieVideoType VideoType);

	/** Current playback Movie from ManaSource */
	TWeakObjectPtr<UManaMovie> CurrentMovie;

	/** Current Index in play list used */
	int32 CurrentMovieIndex;
    
	/** Internal array of with fixed slots for component texures for main video track.
	 *
	 * This ensures that recycled textures keep type when movie source
	 * or platform changes and permits consistant reference for materials.
	 * [0] -> Y / RGB / External, [1] -> U / UV, [2] -> V, [3] -> Alpha
	 */
	TArray<UManaComponentTexture*, TFixedAllocator<4>> ComponentTextureSlots;

	/** Internal array of with fixed slots for component texures for alpha video track.
	 *
	 * This ensures that recycled textures keep type when movie source
	 * or platform changes and permits consistant reference for materials.
	 * [0] -> Y2 / RGB2 / External2, [1] -> U2 / UV2, [2] -> V2 / V2, [4] -> None
	 */
	TArray<UManaComponentTexture*, TFixedAllocator<4>> AlphaComponentTextureSlots;
};
