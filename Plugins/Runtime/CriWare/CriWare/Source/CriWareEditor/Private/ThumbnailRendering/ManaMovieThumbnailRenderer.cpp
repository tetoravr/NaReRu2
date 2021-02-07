/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : ManaMovieThumbnailRenderer.cpp
 *
 ****************************************************************************/

#include "ThumbnailRendering/ManaMovieThumbnailRenderer.h"
#include "ThumbnailRendering/ManaMovieThumbnailInfo.h"
#include "ManaMovie.h"
#include "ManaTexture.h"
#include "ManaPlayer.h"
#include "CriWarePreviewer.h"

//#include "ThumbnailRendering/ThumbnailManager.h"
#include "ObjectTools.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"


UManaMovieThumbnailRenderer::UManaMovieThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool UManaMovieThumbnailRenderer::CanVisualizeAsset(UObject* Object) 
{ 
	return true;
}

void UManaMovieThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	OutWidth = 0;
	OutHeight = 0;

	UManaMovie* Movie = Cast<UManaMovie>(Object);
	if (Movie != nullptr)
	{
		Super::GetThumbnailSize(Movie->Thumbnail, Zoom, OutWidth, OutHeight);
	}
}

void UManaMovieThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UManaMovie* Movie = Cast<UManaMovie>(Object);
	if (Movie != nullptr && Movie->ThumbnailInfo != nullptr && Movie->Validate())
	{
		auto ThumbnailInfo = Cast<UManaMovieThumbnailInfo>(Movie->ThumbnailInfo);
		UTexture* Texture = nullptr;
		bool bPreviewing = GCriWarePreviewer->IsPreviewMoviePlaying(Movie);

		if (bPreviewing)
		{
			// use previewer texture
			Texture = GCriWarePreviewer->PreviewManaPlayer->GetTexture();

			// when preview finished need to draw the thumbnail
			Movie->bNeedsThumbnailGeneration = true;
		} 
		else if (Movie->Thumbnail)
		{	
			Movie->bNeedsThumbnailGeneration = false;
		
			// use stored thumbnail texture
			Texture = Movie->Thumbnail;
		}

		//UE_LOG(LogTemp, Log, TEXT("DRAW THUMB %s %d %d"), *Object->GetName(), Width, Height);
		Super::Draw(Texture, X, Y, Width, Height, nullptr, Canvas, bAdditionalViewFamily);

		// overlays
		FString TypeChars;
		if (bPreviewing) {
			TypeChars = GCriWarePreviewer->PreviewManaPlayer->GetTime().ToString(TEXT("%h:%m:%s")).RightChop(1);
		} else {
			TypeChars = TEXT("Prime");
			if (Movie->GetVideoTrackType(0) == EManaMovieType::H264) {
				TypeChars = TEXT("H264");
			} else if (Movie->GetVideoTrackType(0) == EManaMovieType::VP9) {
				TypeChars = TEXT("VP9");
			}
		}

		int32 TypeWidth = 0;
		int32 TypeHeight = 0;
		StringSize(GEngine->GetLargeFont(), TypeWidth, TypeHeight, *TypeChars);
		float PaddingX = Width / 128.0f;
		float PaddingY = Height / 128.0f;
		float ScaleX = Width / 64.0f; //Text is 1/64'th of the size of the thumbnails
		float ScaleY = Height / 64.0f;
		// Type/Time overlay
		FCanvasTextItem TextItem(FVector2D(Width - PaddingX - TypeWidth * ScaleX, Height - PaddingY - TypeHeight * ScaleY), FText::FromString(TypeChars), GEngine->GetLargeFont(), FLinearColor::White);
		TextItem.EnableShadow(FLinearColor::Black);
		TextItem.Scale = FVector2D(ScaleX, ScaleY);
		TextItem.Draw(Canvas);

		// Add sofdecmovie in top left
	}
}

bool UManaMovieThumbnailRenderer::AllowsRealtimeThumbnails(UObject* Object) const
{
	UManaMovie* Movie = Cast<UManaMovie>(Object);
	if (Movie)
	{
		return GCriWarePreviewer->IsPreviewMoviePlaying(Movie) || Movie->bNeedsThumbnailGeneration;
	}
	return false;
}
