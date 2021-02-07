// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFSpeedCurve.h"
#include "Components/SplineComponent.h"
#include "Components/PFPathFollowerComponent.h"
#include "Distributions/DistributionFloatConstant.h"
#include "Log/Log.h"

DEFINE_LOG_CATEGORY(PFSpeedCurve)


float FPFSpeedCurve::GetSpeedAtDistance(const FInterpCurveFloat& Curve, float Distance)
{
	float speed = Curve.Eval(Distance, 0);
	
	return speed;
}

//void FPFSpeedCurve::Dump()
//{
//	PF_LOG_VERB(PFSpeedCurve, "Dump():");
//	PF_LOG_VERB(PFSpeedCurve, "PointsNum: %i", _speedCurve.Points.Num());
//  for (auto& point : _speedCurve.Points)
//  {
//		PF_LOG_VERB(PFSpeedCurve, "Distance: %f, Speed: %f", point.InVal, point.OutVal);
//  }
//}

TTuple<float, float> FPFSpeedCurve::GetMinMaxSpeed(const FInterpCurveFloat& Curve)
{
	if (Curve.Points.Num() == 0)
	{
		return TTuple<float, float>(0, 0);
	}

	if (Curve.Points.Num() == 1)
	{
		float Speed = Curve.Points[0].OutVal;
		return TTuple<float, float>(Speed, Speed);
	}

	auto SpeedPoints = Curve.Points; //copy
	
	SpeedPoints.Sort([](const FInterpCurvePoint<float>& Max, const FInterpCurvePoint<float>& Item) 
	{
		return Max.OutVal < Item.OutVal;
	});

	float MinSpeed = SpeedPoints[0].OutVal;
	float MaxSpeed = SpeedPoints.Last().OutVal;
	PF_LOG_VERB(PFSpeedCurve, "MinSpeed: %f, MaxSpeed: %f", MinSpeed, MaxSpeed);
	
	return TTuple<float, float>(MinSpeed, MaxSpeed);
}

//void FPFSpeedCurve::Clear()
//{
//	_speedCurve.Points.Reset();
//}

FPFSpeedCurve::FPFSpeedCurve()
{
	
}

FPFSpeedPointsDrawConfig::FPFSpeedPointsDrawConfig()
{
	_SpeedPointSpriteTexture = LoadObject<UTexture2D>(NULL, TEXT("/PathFollow/S_Speed.S_Speed"));
}
