// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#if WITH_EDITOR

#include "ComponentVisualizer.h"

/** Base class for clickable spline editing proxies */
struct HPathFollowerVisProxy : public HComponentVisProxy
{
  DECLARE_HIT_PROXY();

	HPathFollowerVisProxy(const UActorComponent* InComponent, int32 InKeyIndex)
    : HComponentVisProxy(InComponent, HPP_Wireframe)
		, KeyIndex(InKeyIndex)
  {}

	int32 KeyIndex;
};

/** Proxy for a spline key */
struct HRollAngleKeyProxy : public HPathFollowerVisProxy
{
  DECLARE_HIT_PROXY();

	HRollAngleKeyProxy(const UActorComponent* InComponent, int32 InKeyIndex)
		: HPathFollowerVisProxy(InComponent, InKeyIndex)
	{}
};

/** Proxy for a spline key */
struct HEventPointKeyProxy : public HPathFollowerVisProxy
{
  DECLARE_HIT_PROXY();

  HEventPointKeyProxy(const UActorComponent* InComponent, int32 InKeyIndex)
		: HPathFollowerVisProxy(InComponent, InKeyIndex)
  {}
};

struct HSpeedPointKeyProxy : public HPathFollowerVisProxy
{
	DECLARE_HIT_PROXY();

	HSpeedPointKeyProxy(const UActorComponent* InComponent, int32 InKeyIndex)
		: HPathFollowerVisProxy(InComponent, InKeyIndex)
	{}

	
};

#endif
