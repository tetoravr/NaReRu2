// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include <Components/SplineComponent.h>
#include "Classes/PFEventPoints.h"
#include "Classes/PFPathRoller.h"
#include "Classes/PFSpeedCurve.h"
#include "Classes/PFAutoRollVisualConfig.h"
#include "PFPathComponent.generated.h"

//#if WITH_EDITORONLY_DATA

USTRUCT()
struct PATHFOLLOW_API FDrawerData
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, Category = Vizualization)
  bool bDrawIfNotSelected = true;

  UPROPERTY(EditAnywhere, Category = Vizualization)
  bool bDrawIfSelected = false;

  UPROPERTY(EditAnywhere, Category = Vizualization)
  FColor PathColor = FColor::Blue;

  UPROPERTY(EditAnywhere, Category = Vizualization)
  float Thickness = 5;

  FDrawerData()
  {
  }
};

//#endif

/**
 * 
 */
UCLASS(ClassGroup = PF, meta = (BlueprintSpawnableComponent))
class PATHFOLLOW_API UPFPathComponent : public USplineComponent
{
public:
	GENERATED_BODY()

	UPFPathComponent();

	virtual void InitializeComponent() override;

	UPROPERTY(EditAnywhere, Category = Path)
	FDrawerData DrawerConfig;

	UPROPERTY(EditAnywhere, Category = "Events", meta = (DisplayName = "Visualization"))
	FPFEventPointsVisualization EventPointsVisualization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events", meta = (DisplayName = "Points"))
	FPFEventPoints EventPoints;

	FPFPathRoller& GetPathRoller()
	{
		return _PathRoller;
	}
	const FPFPathRoller& GetPathRoller() const
	{
		return _PathRoller;
	}

	const FPFSpeedCurve& GetSpeedCurve() const
	{
		return _SpeedCurve;
	}
	FPFSpeedCurve& GetSpeedCurve()
	{
		return _SpeedCurve;
	}

	/**
	* Holds roll angle curve for AutoRoll feature.
	*/
	UPROPERTY(EditAnywhere, Category=Rotation)
	FPFPathRoller _PathRoller;

	/**
	* Holds roll angle curve for AutoRoll feature.
	*/
	UPROPERTY(EditAnywhere, Category = Speed, meta = (DisplayName = "Curve"))
	FPFSpeedCurve _SpeedCurve;

	UPROPERTY(EditAnywhere, Category = Path)
	bool MirrorAroundX;

	/** Given a distance along the length of this spline, return the point in space where this puts you */
	UFUNCTION(BlueprintCallable, Category = Spline)
	FVector GetLocationAtDistanceAlongSplineMirrored(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const;

	/** Given a distance along the length of this spline, return the point in space where this puts you */
	UFUNCTION(BlueprintCallable, Category = Spline)
	FVector GetLocationAtSplinePointMirrored(int32 PointIdx, ESplineCoordinateSpace::Type CoordinateSpace) const;

	/** Get location along spline at the provided input key value mirrored along xz plane is MirrorAroundX is true*/
	UFUNCTION(BlueprintCallable, Category = Spline)
	FVector GetLocationAtSplineInputKeyMirrored(float Key, ESplineCoordinateSpace::Type CoordinateSpace) const;

//#if WITH_EDITORONLY_DATA
	/**
	*
	*/
	UPROPERTY(EditAnywhere, Category = Rotation, meta = (DisplayName = "Visualization"))
	FPFAutoRollVisualConfig AutoRollDebugConfig;

	/**
	*
	*/
	UPROPERTY(EditAnywhere, Category = Speed, meta = (DisplayName = "Visualization"))
	FPFSpeedPointsDrawConfig SpeedPointDrawConfig;

#if WITH_EDITOR
	void OnFollowerSelected();
	void OnFollowerDeselected();
#endif
public:
	/*
	* Valid if this component has a follower component which is following it. I.e. valid if it has been assigned as SplineToFollow to path follower
	*/
	UPROPERTY()
	const class UPFPathFollowerComponent* _FollowerComponent = nullptr;

//#endif

private:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

//#if WITH_EDITORONLY_DATA
	bool _FollowerSelected = false;
//#endif
};
