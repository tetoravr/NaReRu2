// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFVisualizerProxies.h"

#if WITH_EDITOR

IMPLEMENT_HIT_PROXY(HPathFollowerVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HRollAngleKeyProxy, HPathFollowerVisProxy);
IMPLEMENT_HIT_PROXY(HEventPointKeyProxy, HPathFollowerVisProxy);
IMPLEMENT_HIT_PROXY(HSpeedPointKeyProxy, HPathFollowerVisProxy);

#endif
