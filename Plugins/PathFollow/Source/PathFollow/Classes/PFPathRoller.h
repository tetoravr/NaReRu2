// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "PFRollInterpType.h"
#include "PFPathRoller.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PFRoller, Display, Display);

struct FPFRotationComputeContext
{
	class USplineComponent* Spline = nullptr;
	bool ComputeOnSplineControlPoints = false;
	int32 RollStepsNum = 1;
	float RollSampleLength;
	bool ClampAngles = false;
	float MinAngle = -360;
	float MaxAngle = 360;
	EPFRollInterpType RollInterpType = EPFRollInterpType::Linear;
};

/**
 * 
 */
USTRUCT(BlueprintType, Blueprintable)
struct PATHFOLLOW_API FPFPathRoller
{
public:
  GENERATED_BODY()


public:
	FRotator GetRollRotationAtDistance(const USplineComponent* Spline, float Distance, bool UsePitchAndYawFromPath) const;
	FRotator GetRotationAtDistanceStableLinear(float Distance, const USplineComponent* Spline) const;
	void GenerateRollAngles(const FPFRotationComputeContext& Context);
	void UpdateRollAngles(const FPFRotationComputeContext& Context);

	void Clear();

  void Dump();

  const FInterpCurveVector& GetRollCurve() const
  {
    return _rollAnglesCurve;
  }
  
	FInterpCurveVector& GetRollCurve()
  {
    return _rollAnglesCurve;
  }

	void SetInterpolationType(EPFRollInterpType IntepolationType, class USplineComponent* Spline);
  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	FInterpCurveVector _rollAnglesCurve;

protected:

private:
	typedef TArray<float, TInlineAllocator<16>> TFloatArray;
		
	float CalcRotationSampleLen(float RollSampleLength, float SplineLen, int StepsNum);
	void ComputeAngles(USplineComponent* SplineComp, bool ComputeOnSplineControlPoints, int32 RollStepsNum, float RollSampleLength, TFloatArray& Angles, TFloatArray& Distances);
	float CalcRollAngleAtDistance(USplineComponent* Spline, float Distance, float SampleLength);
	void NormalizeAngles(TFloatArray& Angles, float MinAngle, float MaxAngle);
  void SetRollCurvePoints(USplineComponent* Spline, const TFloatArray& Angles, const TFloatArray& Distances);
	
	UPROPERTY()
 	TArray<UObject*> _curves;
};
