/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : UManaMovieThumbnailRenderer.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Runtime/Engine/Classes/EditorFramework/ThumbnailInfo.h"
#include "ThumbnailRendering/ManaTextureThumbnailRenderer.h"
#include "../Launch/Resources/Version.h"

#include "ManaMovieThumbnailRenderer.generated.h"

class UManaMovie;
class UManaPlayer;

UCLASS()
class UManaMovieThumbnailRenderer : public UManaTextureThumbnailRenderer
{
	GENERATED_UCLASS_BODY()

	//~ Begin UThumbnailRenderer Interface
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
#if	ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 25
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;
#else
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas) override { Draw(Object, X, Y, Width, Height, Viewport, Canvas, false); };
	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily);
#endif
	virtual bool AllowsRealtimeThumbnails(UObject* Object) const override;
	//~ End UThumbnailRenderer Interface
};
