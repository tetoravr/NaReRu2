// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "PFRollInterpType.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class  EPFRollInterpType : uint8
{
	Constant UMETA(DisplayName = "Constant"),
	Linear UMETA(DisplayName = "Linear"),
	LinearStable UMETA(DisplayName = "Linear Stable"),
	Cubic UMETA(DisplayName = "Cubic"),
	CubicClamped UMETA(DisplayName = "Cubic Clamped")
};
