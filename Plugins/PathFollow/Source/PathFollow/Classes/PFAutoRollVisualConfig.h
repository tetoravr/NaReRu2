// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "PFAutoRollVisualConfig.generated.h"

/*
* Debug draw config for auto roll feature.
*/
USTRUCT()
struct PATHFOLLOW_API FPFAutoRollVisualConfig
{
	GENERATED_BODY()

	/*
	* Draw a line for each rotation point
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	bool bHidePointsVisualization = false;

	/*
	* Hide distance text and rotation angles (in degrees)
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	bool bHideTextInfo = false;

	/*
	* Length of rotation point debug lines
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	float LineLength = 50;

	/*
	* Color of non-selected rotation point
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	FLinearColor PointColor = FLinearColor::Blue;

	/*
	* Color of selected rotation point
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	FLinearColor SelectedPointColor = FLinearColor::Yellow;

	/*
	* Debug point size
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	float PointSize = 20.f;

	/*
	* Debug lines thickness
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow)
	float LineThickness = 3.f;
};