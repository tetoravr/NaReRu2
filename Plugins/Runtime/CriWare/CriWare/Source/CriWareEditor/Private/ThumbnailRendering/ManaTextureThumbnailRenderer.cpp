/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : ManaTextureThumbnailRenderer.cpp
 *
 ****************************************************************************/

#include "ThumbnailRendering/ManaTextureThumbnailRenderer.h"

#include "Engine/Engine.h"
#include "CanvasItem.h"
#include "EngineGlobals.h"
#include "CanvasTypes.h"

#include "ManaMovie.h"
#include "ManaTexture.h"
#include "CriWarePreviewer.h"

UManaTextureThumbnailRenderer::UManaTextureThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UManaTextureThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	UTexture* Texture = Cast<UTexture>(Object);
	if (Texture != nullptr)
	{
		OutWidth = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceWidth());
		OutHeight = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceHeight());
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

void UManaTextureThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UTexture* Texture = Cast<UTexture>(Object);
	if (Texture != nullptr && Texture->Resource != nullptr)
	{
		bool bIsAlpha = false;
		UManaTexture* ManaTexture = Cast<UManaTexture>(Object);
		if (ManaTexture) {
			bIsAlpha = ManaTexture->IsAlphaMovie();
		} else {
			UTexture2D* Texture2D = Cast<UTexture2D>(Object);
			if (Texture2D) {
				bIsAlpha = Texture2D->HasAlphaChannel();
			}
		}

		// Take the alpha channel into account for textures that have one.
		// This provides a much better preview than just showing RGB,
		// Because the RGB content in areas with an alpha of 0 is often garbage that will not be seen in normal conditions.
		const bool bUseTranslucentBlend = Texture && bIsAlpha; 

		if (bUseTranslucentBlend)
		{
			// If using alpha, draw a checkerboard underneath first.
			const int32 CheckerDensity = 8;
			auto* Checker = UThumbnailManager::Get().CheckerboardTexture;
			Canvas->DrawTile(
				0.0f, 0.0f, Width, Height,							// Dimensions
				0.0f, 0.0f, CheckerDensity, CheckerDensity,			// UVs
				FLinearColor::White, Checker->Resource);			// Tint & Texture
		}

		// Use A canvas tile item to draw
		TRefCountPtr<FBatchedElementParameters> BatchedElementParameters;
		FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->Resource, FVector2D(Width, Height), FLinearColor::White);
		CanvasTile.BlendMode = bUseTranslucentBlend ? SE_BLEND_Translucent : SE_BLEND_Opaque;
		CanvasTile.BatchedElementParameters = BatchedElementParameters;
		CanvasTile.Draw(Canvas);
	}
}