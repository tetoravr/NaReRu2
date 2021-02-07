/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : ManaMovieThumbnailInfo.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

#include "ManaMovieThumbnailInfo.generated.h"

UCLASS(MinimalApi)
class UManaMovieThumbnailInfo : public UThumbnailInfo
{
	GENERATED_UCLASS_BODY()

	/** The frame number to use for the thumbnail */
	UPROPERTY(EditAnywhere, Category = Thumbnail)
	int32 FrameNumber;

	static const uint32 ThumbnailResolution = 512;
};
