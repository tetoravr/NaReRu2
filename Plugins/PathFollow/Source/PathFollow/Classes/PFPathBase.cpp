// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFPathBase.h"
#include "Components/PFPathComponent.h"


APFPathBase::APFPathBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PathToFollow = CreateDefaultSubobject<UPFPathComponent>(TEXT("PathToFollow"));
	RootComponent = PathToFollow;
	SetHidden(true);
}
