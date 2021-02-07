/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Apple plugin for Unreal Engine 4
 * Module   : Sofdec2 Mana for Apple
 * File     : CriWareManaApple.h
 *
 ****************************************************************************/

#pragma once

#include "ManaComponent.h"
#include "ManaTexture.h"
#include "CriWarePlatform.h"


class FCriWareManaApple : ICriWarePlatformMana
{
public:
	FCriWareManaApple();
	virtual ~FCriWareManaApple();

	virtual void InitializeManaLibrary();
    virtual void FinalizeManaLibrary();
    
    virtual FTextureResource* CreateManaTextureResource(UManaTexture* ManaTexture);
};


class FAppleManaTextureSampler : public FManaTextureResource
{
public:
    
    FAppleManaTextureSampler(FCriWareManaApple& InManaApple, const UManaTexture* InOwner);
    
    virtual void InitDynamicRHI() override;
    
    virtual void ReleaseDynamicRHI() override;
    
    FRHITexture2D* UpdateComponentTexture(const UManaTexture& InOwner,
        const CriManaTextureBuffer& InManaTextureBuffer,
        EManaComponentTextureType InComponentType,
        const FIntPoint& InDimension,
        FRHITexture2D* InTexture);
    
private:
    
    TArray<FTexture2DRHIRef, TFixedAllocator<3>> VideoTextures;
    
    FCriWareManaApple& ManaApple;
    EManaMovieType CurrentMovieType;
};
