/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2019-2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : Sofdec2 Mana for Android
 * File     : CriWareManaAndroid.h
 *
 ****************************************************************************/

#pragma once

#include "ManaComponent.h"
#include "ManaTexture.h"
#include "CriWarePlatform.h"

#define CRIMANA_NUMBER_OF_FRAME_POOLS_MEDIACODEC_ANDROID (8)

class FCriWareManaAndroid : ICriWarePlatformMana {

public:
	FCriWareManaAndroid();
	virtual ~FCriWareManaAndroid();

	virtual void InitializeManaLibrary() override;
	virtual void FinalizeManaLibrary() override;

	virtual FTextureResource* CreateManaTextureResource(UManaTexture* ManaTexture) override;
};

/** Sampler for texture resource to display ManaTexture on android. */
class FAndroidManaTextureSampler : public FManaTextureResource
{
public:

	FAndroidManaTextureSampler(const UManaTexture* InOwner);
	
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;

	FRHITexture2D* UpdateComponentTexture(const UManaTexture& InOwner,
		const CriManaTextureBuffer& InManaTextureBuffer,
		EManaComponentTextureType InComponentType,
		const FIntPoint& InDimension,
		FRHITexture2D* InTexture);

protected:

	FTextureRHIRef VideoTexture;
	FGuid Guid;
	float UScale, VScale, UOffset, VOffset;
};
