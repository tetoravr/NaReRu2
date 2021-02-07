// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFPathComponent.h"
#include "SceneManagement.h"
#include "PFPathFollowerComponent.h"
#include "Runtime/Launch/Resources/Version.h"

UPFPathComponent::UPFPathComponent()
{
	bWantsInitializeComponent = true;
}

void UPFPathComponent::InitializeComponent()
{
	Super::InitializeComponent();

	EventPoints.Init();
}

FBoxSphereBounds UPFPathComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FVector min;
	FVector max;
#if ENGINE_MINOR_VERSION < 13
	SplineInfo.CalcBounds(min, max, FVector::ZeroVector);
#else
	SplineCurves.Position.CalcBounds(min, max, FVector::ZeroVector);
#endif
	FBoxSphereBounds NewBounds(FBox(min, max));
	FTransform Trans = LocalToWorld;
	if (MirrorAroundX)
	{
		Trans.Mirror(EAxis::None, EAxis::Y);
		//auto Location = Trans.GetLocation();
		//Location.Y
	}

	return NewBounds.TransformBy(Trans);
}

FVector UPFPathComponent::GetLocationAtDistanceAlongSplineMirrored(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	if (MirrorAroundX)
	{
		FVector Location = GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		Location.Y = -Location.Y;

		if (CoordinateSpace == ESplineCoordinateSpace::World)
		{
			Location = GetComponentTransform().TransformPosition(Location);
		}

		return Location;
	}
	else
	{
		return GetLocationAtDistanceAlongSpline(Distance, CoordinateSpace);
	}
}

FVector UPFPathComponent::GetLocationAtSplinePointMirrored(int32 PointIdx, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	if (MirrorAroundX)
	{
		FVector Location = GetLocationAtSplinePoint(PointIdx, ESplineCoordinateSpace::Local);
		Location.Y = -Location.Y;

		if (CoordinateSpace == ESplineCoordinateSpace::World)
		{
			Location = GetComponentTransform().TransformPosition(Location);
		}

		return Location;
	}
	else
	{
		return GetLocationAtSplinePoint(PointIdx, CoordinateSpace);
	}
}

FVector UPFPathComponent::GetLocationAtSplineInputKeyMirrored(float Key, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	if (MirrorAroundX)
	{
		FVector Location = GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::Local);
		Location.Y = -Location.Y;

		if (CoordinateSpace == ESplineCoordinateSpace::World)
		{
			Location = GetComponentTransform().TransformPosition(Location);
		}

		return Location;
	}
	else
	{
		return GetLocationAtSplineInputKey(Key, CoordinateSpace);
	}
}
#if WITH_EDITOR
void UPFPathComponent::OnFollowerSelected()
{
	_FollowerSelected = true;
}

void UPFPathComponent::OnFollowerDeselected()
{
	_FollowerSelected = false;
}
#endif

FPrimitiveSceneProxy* UPFPathComponent::CreateSceneProxy()
{
#if WITH_EDITOR
  class FTRPathComponentSceneProxy : public FPrimitiveSceneProxy
  {
  public:
    FTRPathComponentSceneProxy(UPFPathComponent* debugDraw)
      : FPrimitiveSceneProxy(debugDraw)
      , _pathComp(debugDraw)
    {
      bWillEverBeLit = false;
      check(_pathComp);
    }

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				FPrimitiveDrawInterface* PDI;
				const FSceneView* View;
				if (VisibilityMap & (1 << ViewIndex))
				{
					PDI = Collector.GetPDI(ViewIndex);
					View = Views[ViewIndex];
				}
				else
				{
					continue;
				}

				DrawPath(PDI);
				if (!_pathComp || !_pathComp->_FollowerSelected)
				{
					DrawRotationCurve(PDI);
					DrawEventPoints(PDI);
					DrawSpeedPoints(PDI);
				}
			}
    }

    virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
    uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

#if ENGINE_MINOR_VERSION > 10
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
#else
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
#endif
	{
		bool Visible = false;
		if (IsSelected())
			Visible = _pathComp->DrawerConfig.bDrawIfSelected;
		else
			Visible = _pathComp->DrawerConfig.bDrawIfNotSelected;

		if (_pathComp)
			Visible = Visible || _pathComp->_FollowerSelected;

		if (_pathComp && _pathComp->GetWorld() && _pathComp->GetWorld()->IsGameWorld())
		{
			Visible = false;
		}

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = Visible && IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	private:
		void DrawRotationCurve(FPrimitiveDrawInterface* PDI) const
		{
			auto& DebugConfig = _pathComp->AutoRollDebugConfig;
			if (!DebugConfig.bHidePointsVisualization)
			{
				auto& rollCurve = _pathComp->GetPathRoller().GetRollCurve();
				for (int32 i = 0; i < rollCurve.Points.Num(); ++i)
				{
					auto& point = rollCurve.Points[i];
					auto startPos = _pathComp->GetLocationAtDistanceAlongSplineMirrored(point.InVal, ESplineCoordinateSpace::World);
					auto Rotation = _pathComp->GetComponentQuat() * _pathComp->GetPathRoller().GetRollRotationAtDistance(_pathComp, point.InVal, false).Quaternion();
					PDI->DrawPoint(startPos, DebugConfig.PointColor, DebugConfig.PointSize, SDPG_Foreground);
					DrawCoordinateSystem(PDI, startPos, Rotation.Rotator(), DebugConfig.LineLength, SDPG_Foreground, DebugConfig.LineThickness);
				}
			}
		}

		void DrawEventPoints(FPrimitiveDrawInterface* PDI) const
		{
			check(_pathComp);

			auto& EventViz = _pathComp->EventPointsVisualization;
			if (!EventViz.bHideEventPoints)
			{
				FColor EventPointColor = EventViz.EventPointsColor;
				float Size = EventViz.EventPointHitProxySize;
				auto* EventPointTex = EventViz._EventPointSpriteTexture;
				for (auto& EventPoint : _pathComp->EventPoints.Points)
				{
					auto Location = _pathComp->GetLocationAtDistanceAlongSplineMirrored(EventPoint.Distance, ESplineCoordinateSpace::World);
					if (EventPointTex)
					{
						PDI->DrawSprite(Location,
							Size,
							Size,
							EventPointTex->Resource,
							EventPointColor,
							SDPG_Foreground,
							0, EventPointTex->Resource->GetSizeX(),
							0, EventPointTex->Resource->GetSizeY(),
							SE_BLEND_Masked);
					}
					else
					{
						PDI->DrawPoint(Location, EventPointColor, Size, SDPG_Foreground);
					}
				}
			}
		}

		void DrawSpeedPoints(FPrimitiveDrawInterface* PDI) const
		{
			check(_pathComp);

			if (_pathComp->SpeedPointDrawConfig.bHideSpeedPoints)
				return;

			FColor SpeedPointColor = _pathComp->SpeedPointDrawConfig.SpeedPointsColor;
			float Size = _pathComp->SpeedPointDrawConfig.SpeedPointHitProxySize;
			auto& SpeedPoints = _pathComp->GetSpeedCurve().GetCurve().Points;
			auto* SpeedPointTex = _pathComp->SpeedPointDrawConfig._SpeedPointSpriteTexture;
			for (auto& SpeedPoint : SpeedPoints)
			{
				auto PointLocation = _pathComp->GetLocationAtDistanceAlongSplineMirrored(SpeedPoint.InVal, ESplineCoordinateSpace::World);
				if (SpeedPointTex)
				{
					PDI->DrawSprite(PointLocation,
						Size,
						Size,
						SpeedPointTex->Resource,
						SpeedPointColor,
						SDPG_Foreground,
						0, SpeedPointTex->Resource->GetSizeX(),
						0, SpeedPointTex->Resource->GetSizeY(),
						SE_BLEND_AlphaComposite);
				}
				else
				{
					PDI->DrawPoint(PointLocation, SpeedPointColor, Size, SDPG_Foreground);
				}
			}
		}
    
		void DrawPath(FPrimitiveDrawInterface* PDI) const
		{
			check(_pathComp);

			auto& DrawConfig = _pathComp->DrawerConfig;

			const uint8 DepthPriorityGroup = SDPG_Foreground;

			FVector OldKeyPos(0);
			float OldKeyTime = 0.f;
			

#if ENGINE_MINOR_VERSION < 13
			const int32 NumPoints = _pathComp->SplineInfo.Points.Num();
#else
			const int32 NumPoints = _pathComp->SplineCurves.Position.Points.Num();
#endif

			int NumSegments = _pathComp->IsClosedLoop() ? NumPoints + 1 : NumPoints;
			for (int32 KeyIdx = 0; KeyIdx < NumSegments; KeyIdx++)
			{
				FVector NewKeyPos = _pathComp->GetLocationAtSplinePointMirrored(KeyIdx, ESplineCoordinateSpace::World);
				const FColor KeyColor = FColor(255, 128, 0);// (SplineComp == EditedSplineComp && KeyIdx == SelectedKeyIndex) ? SelectedColor : NormalColor;
				if (KeyIdx < NumPoints)
				{
					PDI->DrawPoint(NewKeyPos, KeyColor, 20, DepthPriorityGroup);
				}

				// If not the first keypoint, draw a line to the previous keypoint.
				if (KeyIdx > 0)
				{
					//const int32 NumSteps = FMath::CeilToInt((NewKeyTime - OldKeyTime) * 32.0f);
					//const float DrawSubstep = (NewKeyTime - OldKeyTime) / NumSteps;
					const int32 NumSteps = 30;

					// Find position on first keyframe.
					FVector OldPos = OldKeyPos;

					// Then draw a line for each substep.
					for (int32 StepIdx = 1; StepIdx < NumSteps + 1; StepIdx++)
					{
						const float Key = (KeyIdx - 1) + (StepIdx / static_cast<float>(NumSteps));
						FVector NewPos = _pathComp->GetLocationAtSplineInputKeyMirrored(Key, ESplineCoordinateSpace::World);
						FColor PathColor = DrawConfig.PathColor;
						bool VisualizeSpeedPoints = _pathComp->_FollowerComponent ? _pathComp->_FollowerComponent->SpeedPointDrawConfig.bVisualizeSpeed : _pathComp->SpeedPointDrawConfig.bVisualizeSpeed;
						if (VisualizeSpeedPoints)
						{
							 PathColor = GetSpeedColor(DrawConfig.PathColor, Key / NumSegments);
						}
						
						PDI->DrawLine(OldPos, NewPos, PathColor, DepthPriorityGroup, DrawConfig.Thickness);

						OldPos = NewPos;
					}

				}

				OldKeyPos = NewKeyPos;
			}

			

		}

		FColor GetSpeedColor(const FColor DefaultColor, float Key) const
		{
			check(_pathComp);

			const float Distance = Key * _pathComp->GetSplineLength();

			auto& SpeedCurve = _pathComp->_FollowerComponent ? _pathComp->_FollowerComponent->GetSpeedCurve() : _pathComp->GetSpeedCurve().GetCurve();
			auto MinMaxSpeed = FPFSpeedCurve::GetMinMaxSpeed(SpeedCurve);
			const float CurrentSpeed = _pathComp->_FollowerComponent ? _pathComp->_FollowerComponent->GetSpeedAtDistance(Distance) : FPFSpeedCurve::GetSpeedAtDistance(_pathComp->GetSpeedCurve().GetCurve(), Distance);


			const float SpeedColorScale = MinMaxSpeed.Get<1>() - MinMaxSpeed.Get<0>();

			if (SpeedColorScale < 0.001f)
			{
				return DefaultColor;
			}

			const float Alpha = FMath::Min((CurrentSpeed - MinMaxSpeed.Get<0>()) / SpeedColorScale, 1.f);
			auto& SpeedConfig = _pathComp->_FollowerComponent ? _pathComp->_FollowerComponent->SpeedPointDrawConfig : _pathComp->SpeedPointDrawConfig;
			FLinearColor LowSpeedColor = SpeedConfig.LowSpeedColor;
			FLinearColor HighSpeedColor = SpeedConfig.HighSpeedColor;

			const auto FinalColor = FMath::Lerp(SpeedConfig.LowSpeedColor, SpeedConfig.HighSpeedColor, Alpha);
			return FinalColor.ToRGBE();
		}

	private:
    UPFPathComponent* _pathComp;
  };

  return new FTRPathComponentSceneProxy(this);
#else
	return nullptr;
#endif
}
