// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFPathRoller.h"
#include "Components/SplineComponent.h"
#include "Components/PFPathFollowerComponent.h"
#include "Components/PFPathComponent.h"
#include "Distributions/DistributionFloatConstant.h"
#include "Runtime/Launch/Resources/Version.h"

DEFINE_LOG_CATEGORY(PFRoller)

/**
* Min of Array
* @param	Array of templated type
* @param	Optional pointer for returning the index of the minimum element, if multiple minimum elements the first index is returned
* @return	The min value found in the array or default value if the array was empty
*/
template< class T, class U >
static FORCEINLINE T Min(const TArray<T, U>& Values, int32* MinIndex = NULL)
{
	if (Values.Num() == 0)
	{
		if (MinIndex)
		{
			*MinIndex = INDEX_NONE;
		}
		return T();
	}

	T CurMin = Values[0];
	int32 CurMinIndex = 0;
	for (int32 v = 1; v < Values.Num(); ++v)
	{
		const T Value = Values[v];
		if (Value < CurMin)
		{
			CurMin = Value;
			CurMinIndex = v;
		}
	}

	if (MinIndex)
	{
		*MinIndex = CurMinIndex;
	}
	return CurMin;
}

/**
* Max of Array
* @param	Array of templated type
* @param	Optional pointer for returning the index of the maximum element, if multiple maximum elements the first index is returned
* @return	The max value found in the array or default value if the array was empty
*/
template< class T, class U >
static FORCEINLINE T Max(const TArray<T,U>& Values, int32* MaxIndex = NULL)
{
	if (Values.Num() == 0)
	{
		if (MaxIndex)
		{
			*MaxIndex = INDEX_NONE;
		}
		return T();
	}

	T CurMax = Values[0];
	int32 CurMaxIndex = 0;
	for (int32 v = 1; v < Values.Num(); ++v)
	{
		const T Value = Values[v];
		if (CurMax < Value)
		{
			CurMax = Value;
			CurMaxIndex = v;
		}
	}

	if (MaxIndex)
	{
		*MaxIndex = CurMaxIndex;
	}
	return CurMax;
}


FRotator FPFPathRoller::GetRollRotationAtDistance(const USplineComponent* Spline, float Distance, bool UsePitchAndYawFromPath) const
{
	check(Spline);

	auto EulerAngles = _rollAnglesCurve.Eval(Distance, FVector::ZeroVector);
	FRotator Rotation(FQuat::MakeFromEuler(EulerAngles));

	if (UsePitchAndYawFromPath)
	{
		auto SplineRotation = Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		Rotation.Pitch = SplineRotation.Pitch;
		Rotation.Yaw = SplineRotation.Yaw;
	}

	if (auto* PathComp = Cast<UPFPathComponent>(Spline))
	{
		bool MirrorAroundX = PathComp->MirrorAroundX;
		if (MirrorAroundX)
		{
			Rotation.Roll = -Rotation.Roll;
			Rotation.Yaw = -Rotation.Yaw;
		}
	}

	return Rotation;
}

FRotator FPFPathRoller::GetRotationAtDistanceStableLinear(float Distance, const USplineComponent* Spline) const
{
	int32 NumPoints = _rollAnglesCurve.Points.Num();
	auto& Points = _rollAnglesCurve.Points;
	FQuat KeyQuat;
	//If no point in curve, return the Default value we passed in.
	if (NumPoints == 0)
	{
		KeyQuat = FQuat::Identity;
	}
	//If only one point, or before the first point in the curve, return the first points value.
	else if (NumPoints < 2 || (Distance <= Points[0].InVal))
	{
		FVector OutEulerRot = Points[0].OutVal;
		KeyQuat = FQuat::MakeFromEuler(OutEulerRot);
	}
	//If beyond the last point in the curve, return its value.
	else if (Distance >= Points[NumPoints - 1].InVal)
	{
		FVector OutEulerRot = Points.Last().OutVal;
		KeyQuat = FQuat::MakeFromEuler(OutEulerRot);
	}
	//Somewhere with curve range - linear search to find value.
	else
	{
		for (int32 KeyIdx = 1; KeyIdx < NumPoints; KeyIdx++)
		{
			if (Distance < Points[KeyIdx].InVal)
			{
				float Delta = Points[KeyIdx].InVal - Points[KeyIdx - 1].InVal;
				float Alpha = FMath::Clamp((Distance - Points[KeyIdx - 1].InVal) / Delta, 0.f, 1.f);
				FVector CurrentRot = Points[KeyIdx].OutVal;
				FVector PrevRot = Points[KeyIdx - 1].OutVal;
				FQuat Key1Quat = FQuat::MakeFromEuler(PrevRot);
				FQuat Key2Quat = FQuat::MakeFromEuler(CurrentRot);
				KeyQuat = FQuat::Slerp(Key1Quat, Key2Quat, Alpha);
				
				break;
			}
		}
	}

	FRotator Rotation(KeyQuat);
	if (auto* PathComp = Cast<UPFPathComponent>(Spline))
	{
		bool MirrorAroundX = PathComp->MirrorAroundX;
		if (MirrorAroundX)
		{
			Rotation.Roll = -Rotation.Roll;
			Rotation.Yaw = -Rotation.Yaw;
		}
	}

	return Rotation;
}

void FPFPathRoller::GenerateRollAngles(const FPFRotationComputeContext& Context)
{
  ENTER_EXIT_LOG;

	if (!Context.Spline)
		return;
	
	TFloatArray Angles, Distances;
	ComputeAngles(Context.Spline, Context.ComputeOnSplineControlPoints, Context.RollStepsNum, Context.RollSampleLength, /*out*/Angles, /*out*/Distances);
	if(Context.ClampAngles)
		NormalizeAngles(Angles, Context.MinAngle, Context.MaxAngle);
	SetRollCurvePoints(Context.Spline, Angles, Distances);
	SetInterpolationType(Context.RollInterpType, Context.Spline);
	
  PF_LOG_DIS(PFRoller, "Rotation points:");
	for (int i = 0; i < _rollAnglesCurve.Points.Num(); ++i)
	{
		PF_LOG_DIS(PFRoller, "distance: %f euler: %s", _rollAnglesCurve.Points[i].InVal, *FRotator::MakeFromEuler(_rollAnglesCurve.Points[i].OutVal).ToCompactString());
	}
}

void FPFPathRoller::UpdateRollAngles(const FPFRotationComputeContext& Context)
{
	if (!Context.Spline)
		return;

	float SampleLen = CalcRotationSampleLen(Context.RollSampleLength, Context.Spline->GetSplineLength(), Context.RollStepsNum);

	TFloatArray Angles;
	for (auto& RotationPoint : _rollAnglesCurve.Points)
	{
		float Distance = RotationPoint.InVal;
		float Angle = CalcRollAngleAtDistance( Context.Spline, Distance, SampleLen);
		Angles.Add(Angle);
	}

	if (Context.ClampAngles)
		NormalizeAngles(Angles, Context.MinAngle, Context.MaxAngle);

	check(_rollAnglesCurve.Points.Num() == Angles.Num());
	for (int i = 0; i < _rollAnglesCurve.Points.Num(); ++i)
	{
		auto& RotationPoint = _rollAnglesCurve.Points[i];
		auto Rotation = Context.Spline->GetRotationAtDistanceAlongSpline(RotationPoint.InVal, ESplineCoordinateSpace::Local);
		Rotation.Roll += Angles[i];
		RotationPoint.OutVal = Rotation.Euler();	
	}

	SetInterpolationType(Context.RollInterpType, Context.Spline);
}

float FPFPathRoller::CalcRotationSampleLen(float RollSampleLength, float SplineLen, int StepsNum)
{
	if (RollSampleLength < 0)
	{
		float samplingStepLen = SplineLen / (float)StepsNum;
		return samplingStepLen / (float)StepsNum;
	}

	return RollSampleLength;
}

void FPFPathRoller::NormalizeAngles(TFloatArray& RollAngles, float MinAngle, float MaxAngle)
{
	float CurveMinAngle = Min(RollAngles);
	float CurveMaxAngle = Max(RollAngles);

	float PositiveNormalizeVal = MaxAngle / CurveMaxAngle;
	float NegativeNormalizeVal = MinAngle / CurveMinAngle;

	for (float& RollAngle : RollAngles)
	{
		if (FMath::IsNegativeFloat(RollAngle))
		{
			RollAngle *= NegativeNormalizeVal;
		}
		else
		{
			RollAngle *= PositiveNormalizeVal;
		}
	}
}

void FPFPathRoller::SetRollCurvePoints(USplineComponent* Spline, const TFloatArray& Angles, const TFloatArray& Distances)
{
	_rollAnglesCurve.Points.Reset();
	check(Angles.Num() == Distances.Num());
	for (int i = 0; i < Angles.Num(); ++i)
	{
		auto Rotation = Spline->GetRotationAtDistanceAlongSpline(Distances[i], ESplineCoordinateSpace::Local);
		Rotation.Roll += Angles[i];
		_rollAnglesCurve.AddPoint(Distances[i], Rotation.Euler());
	}
}

void FPFPathRoller::SetInterpolationType(EPFRollInterpType IntepolationType, USplineComponent* Spline)
{
	EInterpCurveMode interpMode;
	switch (IntepolationType)
	{
	case EPFRollInterpType::Constant:
		interpMode = CIM_Constant;
		break;
	case EPFRollInterpType::Linear:
		interpMode = CIM_Linear;
		break;
	case EPFRollInterpType::Cubic:
		interpMode = CIM_CurveAuto;
		break;
	case EPFRollInterpType::CubicClamped:
		interpMode = CIM_CurveAutoClamped;
		break;
	default:
    interpMode = CIM_CurveAuto;
		break;
	}
	
	for (auto& point : _rollAnglesCurve.Points)
	{
		point.InterpMode = interpMode;
	}

	if (_rollAnglesCurve.Points.Num() > 1)
	{
		auto& point = _rollAnglesCurve.Points[0];
		auto RelativeSpaceEuler = point.OutVal;

		int32 AdjacentKeyIndex = 1;
		const FVector AdjacentEuler = _rollAnglesCurve.Points[AdjacentKeyIndex].OutVal;

		// Try to minimize differences in curves
		const FVector EulerDiff = RelativeSpaceEuler - AdjacentEuler;
		if (EulerDiff.X > 180.0f)
		{
			RelativeSpaceEuler.X -= 360.0f;
		}
		else if (EulerDiff.X < -180.0f)
		{
			RelativeSpaceEuler.X += 360.0f;
		}
		if (EulerDiff.Y > 180.0f)
		{
			RelativeSpaceEuler.Y -= 360.0f;
		}
		else if (EulerDiff.Y < -180.0f)
		{
			RelativeSpaceEuler.Y += 360.0f;
		}
		if (EulerDiff.Z > 180.0f)
		{
			RelativeSpaceEuler.Z -= 360.0f;
		}
		else if (EulerDiff.Z < -180.0f)
		{
			RelativeSpaceEuler.Z += 360.0f;
		}

		point.OutVal = RelativeSpaceEuler;
	}

	for (int i = 1; i < _rollAnglesCurve.Points.Num(); ++i)
	{
		auto& point = _rollAnglesCurve.Points[i];
		auto RelativeSpaceEuler = point.OutVal;

		int32 AdjacentKeyIndex = i - 1;
		const FVector AdjacentEuler = _rollAnglesCurve.Points[AdjacentKeyIndex].OutVal;

		// Try to minimize differences in curves
		const FVector EulerDiff = RelativeSpaceEuler - AdjacentEuler;
		if (EulerDiff.X > 180.0f)
		{
			RelativeSpaceEuler.X -= 360.0f;
		}
		else if (EulerDiff.X < -180.0f)
		{
			RelativeSpaceEuler.X += 360.0f;
		}
		if (EulerDiff.Y > 180.0f)
		{
			RelativeSpaceEuler.Y -= 360.0f;
		}
		else if (EulerDiff.Y < -180.0f)
		{
			RelativeSpaceEuler.Y += 360.0f;
		}
		if (EulerDiff.Z > 180.0f)
		{
			RelativeSpaceEuler.Z -= 360.0f;
		}
		else if (EulerDiff.Z < -180.0f)
		{
			RelativeSpaceEuler.Z += 360.0f;
		}

		point.OutVal = RelativeSpaceEuler;
	}
	
	_rollAnglesCurve.AutoSetTangents(0.f, Spline->bStationaryEndpoints);
}

void FPFPathRoller::ComputeAngles(USplineComponent* SplineComp, bool ComputeOnSplineControlPoints, int32 RollStepsNum, float RollSampleLength, TFloatArray& Angles, TFloatArray& Distances)
{
	check(SplineComp);

	float SplineLen = SplineComp->GetSplineLength();
	float SamplingStepLen = SplineLen / (float)RollStepsNum;
	RollSampleLength = CalcRotationSampleLen(RollSampleLength, SplineLen, RollStepsNum);

	RollStepsNum++;

#if ENGINE_MINOR_VERSION < 13
	const int SplineControlPointsNum = SplineComp->SplineInfo.Points.Num();
#else
	const int SplineControlPointsNum = SplineComp->SplineCurves.Position.Points.Num();
#endif

	const auto ActualStepsNum = ComputeOnSplineControlPoints ? SplineControlPointsNum : RollStepsNum;
	check(ActualStepsNum > 1);
	for (int i = 0; i < ActualStepsNum; ++i)
	{
		float currDist;
		if (ComputeOnSplineControlPoints)
		{
			currDist = SplineComp->GetDistanceAlongSplineAtSplinePoint(i);
		}
		else
		{
			currDist = i * SamplingStepLen;
		}
		
		float angle = CalcRollAngleAtDistance(SplineComp, currDist, RollSampleLength);
		Angles.Add(angle);
		Distances.Add(currDist);
	}

	//if closed loop ensure last roll angle is the same as the first one
	if (SplineComp->IsClosedLoop())
	{
		Angles.Last() = Angles[0];
	}

  UE_LOG(PFRoller, Verbose, TEXT("ComputeAngles:"));
  for (int i = 0; i < Angles.Num(); ++i)
  {
    UE_LOG(PFRoller, Verbose, TEXT("distance: %f angle: %f"), Distances[i], Angles[i]);
  }
}

float FPFPathRoller::CalcRollAngleAtDistance(USplineComponent* Spline, float Distance, float SampleLength)
{
	FVector startDir = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
	FVector endDir = startDir;
	float endDist = Distance + SampleLength;
	float SplineLen = Spline->GetSplineLength();
	if (endDist > SplineLen)
	{
		if (Spline->IsClosedLoop())
			endDir = Spline->GetDirectionAtDistanceAlongSpline(SampleLength, ESplineCoordinateSpace::Local);
		else
		{
			startDir = Spline->GetDirectionAtDistanceAlongSpline(SplineLen - 0.0001f - SampleLength, ESplineCoordinateSpace::Local);
			endDir = Spline->GetDirectionAtDistanceAlongSpline(SplineLen - 0.0001f, ESplineCoordinateSpace::Local);
		}
	}
	else
	{
		endDir = Spline->GetDirectionAtDistanceAlongSpline(endDist, ESplineCoordinateSpace::Local);
	}

	float angle = endDir.Rotation().Yaw - startDir.Rotation().Yaw;
	return angle;
}

void FPFPathRoller::Clear()
{
	_rollAnglesCurve.Points.Reset();
}

void FPFPathRoller::Dump()
{
  UE_LOG(PFRoller, Display, TEXT("Dump():"));
  UE_LOG(PFRoller, Display, TEXT("PointsNum: %i"), _rollAnglesCurve.Points.Num());
  for (auto& point : _rollAnglesCurve.Points)
  {
    UE_LOG(PFRoller, Display, TEXT("Distance: %f, Rotation: %s"), point.InVal, *FRotator::MakeFromEuler( point.OutVal).ToCompactString() );
  }
}



