// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#if WITH_EDITOR

#include "PathFollowPrivatePCH.h"
#include "PFPathFollowerVisualizer.h"
#include "Components/PFPathFollowerComponent.h"
#include "Components/SplineComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SVectorInputBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogFollowerVizualizer, Display, Display);

FPFPathFollowerVisulalizer::FPFPathFollowerVisulalizer()
{


}

FPFPathFollowerVisulalizer::~FPFPathFollowerVisulalizer()
{

}

const UPFPathFollowerComponent* FPFPathFollowerVisulalizer::GetEditedPathFollower(const UActorComponent* Component)
{
	return Cast<const UPFPathFollowerComponent>(Component);
}

UPFPathFollowerComponent* FPFPathFollowerVisulalizer::GetEditedPathFollower()
{
	return Cast<UPFPathFollowerComponent>(FollowerPropertyPath.GetComponent());
}

const UPFPathFollowerComponent* FPFPathFollowerVisulalizer::GetEditedPathFollower() const
{
	return const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
}

void FPFPathFollowerVisulalizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	auto* FollowerComponent = Cast<const UPFPathFollowerComponent>(Component);
	if (!FollowerComponent)
		return;

	auto* SplineToFollow = FollowerComponent->GetSplineToFollow();
	if (!SplineToFollow)
	{
		return;
	}

	DrawRollCurveVisualization(SplineToFollow, PDI, FollowerComponent);

	auto& EventViz = FollowerComponent->EventPointsVisualization;
	if (!EventViz.bHideEventPoints)
	{
		FColor EventPointColor = EventViz.EventPointsColor;
		float HitProxySize = EventViz.EventPointHitProxySize;
		auto* EventPointTex = EventViz._EventPointSpriteTexture;
		auto& EventPoints = FollowerComponent->GetEventPoints();
		auto& Points = EventPoints.Points;
		for (int i = 0; i < Points.Num(); ++i)
		{
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HEventPointKeyProxy(Component, i));

			auto Location = FollowerComponent->GetLocationAtDistance(Points[i].Distance, ESplineCoordinateSpace::World);
			if (EventPointTex)
			{
				PDI->DrawSprite(Location,
					HitProxySize,
					HitProxySize,
					EventPointTex->Resource,
					EventPointColor,
					SDPG_Foreground,
					0, EventPointTex->Resource->GetSizeX(),
					0, EventPointTex->Resource->GetSizeY(),
					SE_BLEND_Masked);
			}
			else
			{
				PDI->DrawPoint(Location, EventPointColor, HitProxySize, SDPG_Foreground);
			}
			PDI->SetHitProxy(NULL);
		}
	}

	if (!FollowerComponent->SpeedPointDrawConfig.bHideSpeedPoints)
	{
		const FColor SpeedPointColor = FollowerComponent->SpeedPointDrawConfig.SpeedPointsColor;
		const float HitProxySize = FollowerComponent->SpeedPointDrawConfig.SpeedPointHitProxySize;
		auto& SpeedPoints = FollowerComponent->GetSpeedCurve().Points;
		auto* SpeedPointTex = FollowerComponent->SpeedPointDrawConfig._SpeedPointSpriteTexture;
		for (int i = 0; i < SpeedPoints.Num(); ++i)
		{
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HSpeedPointKeyProxy(Component, i));

			const auto SpeedPointDistance = FollowerComponent->TimeBased ? SpeedPoints[i].OutVal : SpeedPoints[i].InVal;
			auto Location = FollowerComponent->GetLocationAtDistance(SpeedPointDistance, ESplineCoordinateSpace::World);
			if (SpeedPointTex)
			{
				PDI->DrawSprite(Location,
					HitProxySize,
					HitProxySize,
					SpeedPointTex->Resource,
					SpeedPointColor,
					SDPG_Foreground,
					0, SpeedPointTex->Resource->GetSizeX(),
					0, SpeedPointTex->Resource->GetSizeY(),
					SE_BLEND_Masked);
			}
			else
			{
				PDI->DrawPoint(Location, SpeedPointColor, HitProxySize, SDPG_Foreground);
			}

			PDI->SetHitProxy(NULL);
		}
	}
}


void FPFPathFollowerVisulalizer::DrawRollCurveVisualization(USplineComponent * path, FPrimitiveDrawInterface* PDI, const UPFPathFollowerComponent* FollowerComponent)
{
	check(FollowerComponent);
	check(path);

	auto& DebugConfig = FollowerComponent->AutoRollDebugConfig;
	if (!DebugConfig.bHidePointsVisualization)
	{
		auto& rollCurve = FollowerComponent->GetPathRoller().GetRollCurve();
		for (int32 i = 0; i < rollCurve.Points.Num(); ++i)
		{
			auto& point = rollCurve.Points[i];
			auto startPos = FollowerComponent->GetLocationAtDistance(point.InVal, ESplineCoordinateSpace::World);
			FRotator Rotation = FollowerComponent->GetRotationAtDistance(point.InVal, ESplineCoordinateSpace::World);
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HRollAngleKeyProxy(FollowerComponent, i));
			bool IsSelected = _SelectedRotationPointIndex.Contains(i);
			FLinearColor PointColor = IsSelected ? DebugConfig.SelectedPointColor : DebugConfig.PointColor;
			PDI->DrawPoint(startPos, PointColor, DebugConfig.PointSize, SDPG_Foreground);
			PDI->SetHitProxy(NULL);

			if (_SelectedRotationPointIndex.Num() == 0 || i != _SelectedRotationPointIndex.Last() || !_bHideCurrentRotationPointWidget)
			{
				DrawCoordinateSystem(PDI, startPos, Rotation, DebugConfig.LineLength, SDPG_Foreground, DebugConfig.LineThickness);
			}
		}
	}
}


void FPFPathFollowerVisulalizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	auto* FollowerComponent = GetEditedPathFollower(Component);
	if (!FollowerComponent)
		return;

	auto* SplineToFollow = FollowerComponent->GetSplineToFollow();
	if (!SplineToFollow)
	{
		return;
	}

	auto* font = GEngine->GetLargeFont();

	if (!FollowerComponent->bHidePathInfoText)
	{
		auto TimeDistances = GetTimeDistanceArray(FollowerComponent, SplineToFollow);
		auto pointsNum = SplineToFollow->GetNumberOfSplinePoints();
		check(pointsNum == TimeDistances.Num());
		for (int i = 0; i < pointsNum; ++i)
		{
			auto point = FollowerComponent->GetLocationAtInputKey(static_cast<float>(i), ESplineCoordinateSpace::World);
			FVector2D ScreenPos;
			if (View->WorldToPixel(point, ScreenPos))
			{
				auto& tmp = TimeDistances[i];
				auto FormatText = FText::FromString("Time: {0} Distance: {1}");
				auto text = FText::Format(FormatText, FText::AsNumber(tmp.Time), FText::AsNumber(tmp.Distance));
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 45, text, font, FLinearColor::Red);
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 60, FText::FromString(point.ToCompactString()), font, FLinearColor::Red);
			}
		}
	}

	if (!FollowerComponent->SpeedPointDrawConfig.bHideSpeedPointInfoText)
	{
		FColor SpeedPointColor = FollowerComponent->SpeedPointDrawConfig.SpeedPointsColor;
		auto& SpeedPoints = FollowerComponent->GetSpeedCurve().Points;
		for (int i = 0; i < SpeedPoints.Num(); ++i)
		{
			auto SpeedPointDistance = FollowerComponent->TimeBased ? SpeedPoints[i].OutVal : SpeedPoints[i].InVal;
			auto point = FollowerComponent->GetLocationAtDistance(SpeedPointDistance, ESplineCoordinateSpace::World);
			FVector2D ScreenPos;
			if (View->WorldToPixel(point, ScreenPos))
			{
				auto FormatText = FollowerComponent->TimeBased ? FText::FromString("Time: {0}") : FText::FromString("Speed: {0}");
				auto text = FText::Format(FormatText, FText::AsNumber(FollowerComponent->TimeBased ? SpeedPoints[i].InVal : SpeedPoints[i].OutVal));
				//todo: measure size       
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 30, text, font, SpeedPointColor.ReinterpretAsLinear());

				int DistanceTextYOffset;
				if (FollowerComponent->SpeedFactorOperation != EPFFactorOperation::None)
				{
					FormatText = FText::FromString("Modulated: {0}");
					text = FText::Format(FormatText, FText::AsNumber(FollowerComponent->GetSpeedAtSpeedPoint(i)));
					Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 15, text, font, SpeedPointColor.ReinterpretAsLinear());
					DistanceTextYOffset = 0;
				}
				else
				{
					DistanceTextYOffset = 15;
				}

				FormatText = FText::FromString("Distance: {0}");
				text = FText::Format(FormatText, FText::AsNumber(SpeedPointDistance));
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - DistanceTextYOffset, text, font, SpeedPointColor.ReinterpretAsLinear());
			}
		}
	}

	if (!FollowerComponent->AutoRollDebugConfig.bHideTextInfo)
	{

		auto& RotationPoints = FollowerComponent->GetPathRoller().GetRollCurve().Points;
		for (int i = 0; i < RotationPoints.Num(); ++i)
		{
			auto point = FollowerComponent->GetLocationAtDistance(RotationPoints[i].InVal, ESplineCoordinateSpace::World);
			FVector2D ScreenPos;
			if (View->WorldToPixel(point, ScreenPos))
			{
				auto FormatText = FText::FromString("Rotation: {0}");
				auto text = FText::Format(FormatText, FText::FromString(FRotator::MakeFromEuler(RotationPoints[i].OutVal).ToCompactString()));
				Canvas->DrawShadowedText(ScreenPos.X + 20, ScreenPos.Y - 30, text, font, FollowerComponent->AutoRollDebugConfig.PointColor);

				FormatText = FText::FromString("Distance: {0}");
				text = FText::Format(FormatText, FText::AsNumber(RotationPoints[i].InVal));
				Canvas->DrawShadowedText(ScreenPos.X + 20, ScreenPos.Y - 15, text, font, FollowerComponent->AutoRollDebugConfig.PointColor);
			}
		}
	}

	if (!FollowerComponent->EventPointsVisualization.bHideEventPointInfoText)
	{
		auto& EventPoints = FollowerComponent->GetEventPoints().Points;
		for (int i = 0; i < EventPoints.Num(); ++i)
		{
			auto point = FollowerComponent->GetLocationAtDistance(EventPoints[i].Distance, ESplineCoordinateSpace::World);
			FVector2D ScreenPos;
			if (View->WorldToPixel(point, ScreenPos))
			{
				auto FormatText = FText::FromString("Name: {0}  Idx: {1}");
				auto text = FText::Format(FormatText, FText::FromName(EventPoints[i].Name), FText::AsNumber(i));
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 30, text, font, FollowerComponent->EventPointsVisualization.EventPointsColor);

				FormatText = FText::FromString("Distance: {0}");
				text = FText::Format(FormatText, FText::AsNumber(EventPoints[i].Distance));
				Canvas->DrawShadowedText(ScreenPos.X, ScreenPos.Y - 15, text, font, FollowerComponent->EventPointsVisualization.EventPointsColor);
			}
		}
	}
}

TArray<FPFPathFollowerVisulalizer::FTimeDistance> FPFPathFollowerVisulalizer::GetTimeDistanceArray(const UPFPathFollowerComponent* Follower, USplineComponent* Spline)
{
	float len = Spline->GetSplineLength();
	if (FMath::IsNearlyZero(len))
		return TArray<FTimeDistance>();

	TArray<FVector> SamplesCenters;
	TArray<float> SamplesRadius;
	TArray<FTimeDistance> SamplesTimeDistance;

	int stepNum = 20;
	float distanceDelta = len / (float)stepNum;
	float time = 0.f;
	for (int i = 0; i < stepNum; ++i)
	{
		float currDistance = i*distanceDelta;
		float speed = Follower->GetSpeedAtDistance(currDistance);
		UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("CurrentSpeed: %f"), speed);
		float delatT = distanceDelta / FMath::Max(speed, 0.1f);//t = s/v;
		UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("DeltaT: %f"), delatT);

		FTimeDistance tmp;
		tmp.Distance = currDistance;
		tmp.Time = time;
		SamplesTimeDistance.Add(tmp);

		FVector currDistanceLocation = Spline->GetLocationAtDistanceAlongSpline(currDistance, ESplineCoordinateSpace::World);
		float nextDistance = currDistance + distanceDelta;
		if (nextDistance > len)
			nextDistance = len;
		FVector nextDistanceLocation = Spline->GetLocationAtDistanceAlongSpline(nextDistance, ESplineCoordinateSpace::World);
		auto diff = nextDistanceLocation - currDistanceLocation;
		FVector center = currDistanceLocation + diff*0.5f;
		SamplesCenters.Add(center);
		SamplesRadius.Add(diff.Size() *0.5f);

		time += delatT;
	}

	UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("Total time: %f"), time);

	TArray<FTimeDistance> TimeDistArr;

	auto SplinePointsNum = Spline->GetNumberOfSplinePoints();

	//first spline point
	FTimeDistance TimeDist;
	TimeDist.Distance = 0;
	TimeDist.Time = 0;
	TimeDistArr.Add(TimeDist);

	for (int i = 1; i < SplinePointsNum - 1; ++i)
	{

		FTimeDistance SplinePointTimeDistance;
		SplinePointTimeDistance.Distance = Spline->GetDistanceAlongSplineAtSplinePoint(i);
		auto SplinePointLocation = Spline->GetWorldLocationAtSplinePoint(i);
		for (int k = 0; k < SamplesRadius.Num() - 1; ++k)
		{

			if ((SplinePointLocation - SamplesCenters[k]).Size() < SamplesRadius[k])
			{
				SplinePointTimeDistance.Time = SamplesTimeDistance[k].Time;// +(SamplesTimeDistance[k + 1].Time - SamplesTimeDistance[k].Time)*0.5f;
				break;
			}
		}

		TimeDistArr.Add(SplinePointTimeDistance);
	}

	//last spline point
	TimeDist.Distance = len;
	TimeDist.Time = time;
	TimeDistArr.Add(TimeDist);

	return TimeDistArr;
}


void FPFPathFollowerVisulalizer::ClearSelectionIndexes()
{
	_SelectedRotationPointIndex.Empty();
	_selectedEventPointIndex.Empty();
	_selectedSpeedPointIndex.Empty();
}

template<typename TProxyType>
bool FPFPathFollowerVisulalizer::HandleSelection(HComponentVisProxy* VisProxy, const FViewportClick& Click, TIndexArray& AddTo)
{
	if (VisProxy->IsA(TProxyType::StaticGetType()))
	{
		auto* Proxy = static_cast<TProxyType*>(VisProxy);
		if (!Click.IsControlDown())
		{
			ClearSelectionIndexes();
		}
		AddTo.Add(Proxy->KeyIndex);
		return true;
	}

	return false;
}

#if ENGINE_MINOR_VERSION < 13
bool FPFPathFollowerVisulalizer::VisProxyHandleClick(FLevelEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
#else
bool FPFPathFollowerVisulalizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
#endif
{
	ENTER_EXIT_LOG;

	if (!VisProxy || !VisProxy->Component.IsValid())
		return false;

	const UPFPathFollowerComponent* FollowerComp = Cast<const UPFPathFollowerComponent>(VisProxy->Component.Get());

	AActor* OldFollowerOwningActor = FollowerPropertyPath.GetParentOwningActor();
	FollowerPropertyPath = FComponentPropertyPath(FollowerComp);
	AActor* FollowerOwningActor = FollowerPropertyPath.GetParentOwningActor();
	
	if (FollowerPropertyPath.IsValid())
	{
		if (OldFollowerOwningActor != FollowerOwningActor)
		{
			ClearSelectionIndexes();
		}

		if (HandleSelection<HRollAngleKeyProxy>(VisProxy, Click, _SelectedRotationPointIndex))
			return true;
		if (HandleSelection<HEventPointKeyProxy>(VisProxy, Click, _selectedEventPointIndex))
			return true;
		if (HandleSelection<HSpeedPointKeyProxy>(VisProxy, Click, _selectedSpeedPointIndex))
			return true;
	}
	
	return false;
}

bool FPFPathFollowerVisulalizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	ENTER_EXIT_LOG;

	auto* _FollowerComponent = GetEditedPathFollower();
	if (!_FollowerComponent)
		return false;

	auto* path = _FollowerComponent->GetSplineToFollow();
	if (path == nullptr)
	{
		PF_LOG_WARN(LogFollowerVizualizer, "_FollowerComponent->SplineToFollow == nullptr");
		return false;
	}

	if (_SelectedRotationPointIndex.Num() > 0)
	{
		auto& rollCurve = _FollowerComponent->GetPathRoller().GetRollCurve();
		if (_SelectedRotationPointIndex.Last() >= rollCurve.Points.Num())
		{
			PF_LOG_WARN(LogFollowerVizualizer, "index outside of the array");
			return false;
		}
		auto& point = rollCurve.Points[_SelectedRotationPointIndex.Last()];
		OutLocation = path->GetWorldLocationAtDistanceAlongSpline(point.InVal);
		UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("OutLocation: %s"), *OutLocation.ToString());
		return true;
	}

	if (_selectedEventPointIndex.Num() > 0)
	{
		if (_selectedEventPointIndex.Last() >= _FollowerComponent->GetEventPoints().Points.Num())
		{
			PF_LOG_WARN(LogFollowerVizualizer, "index outside of the array");
			return false;
		}
		auto& point = _FollowerComponent->GetEventPoints().Points[_selectedEventPointIndex.Last()];
		OutLocation = path->GetWorldLocationAtDistanceAlongSpline(point.Distance);
		UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("OutLocation: %s"), *OutLocation.ToString());
		return true;
	}

	if (_selectedSpeedPointIndex.Num() > 0)
	{
		auto& SpeedPoints = _FollowerComponent->GetSpeedCurve().Points;
		if (_selectedSpeedPointIndex.Last() >= SpeedPoints.Num())
		{
			PF_LOG_WARN(LogFollowerVizualizer, "index outside of the array");
			return false;
		}
		auto& point = SpeedPoints[_selectedSpeedPointIndex.Last()];
		OutLocation = path->GetWorldLocationAtDistanceAlongSpline(point.InVal);
		UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("OutLocation: %s"), *OutLocation.ToString());
		return true;
	}

	return false;
}

bool FPFPathFollowerVisulalizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	auto* FollowerComponent = GetEditedPathFollower();
	if (!FollowerComponent)
		return false;

	if (ViewportClient->GetWidgetCoordSystemSpace() == COORD_Local)
	{
		auto CurrMode = ViewportClient->GetWidgetMode();

		if (_SelectedRotationPointIndex.Num() > 0)
		{
			_bHideCurrentRotationPointWidget = CurrMode != FWidget::WM_Rotate;

			FVector SelectedRotationPointEuler = FollowerComponent->GetPathRoller().GetRollCurve().Points[_SelectedRotationPointIndex.Last()].OutVal;
			FRotator RotationPointLocalRotation = FRotator::MakeFromEuler(SelectedRotationPointEuler);
			auto* Spline = FollowerComponent->GetSplineToFollow();
			check(Spline);
			FRotator WorldRotation = Spline->GetComponentRotation() + RotationPointLocalRotation;


			if (CurrMode == FWidget::WM_Rotate && _WidgetMode != FWidget::WM_Rotate)
			{
				_StartRotation = WorldRotation;
			}

			if (_WidgetMode == FWidget::WM_Rotate)
			{
				OutMatrix = FRotationMatrix::Make(_StartRotation);
			}
			else
			{
				OutMatrix = FRotationMatrix::Make(WorldRotation);
			}

			_WidgetMode = CurrMode;
			return true;
		}

		if (_selectedSpeedPointIndex.Num() > 0 && CurrMode == FWidget::WM_Translate)
		{
			auto& point = FollowerComponent->GetSpeedCurve().Points[_selectedSpeedPointIndex.Last()];
			auto* Spline = FollowerComponent->GetSplineToFollow();
			if (!Spline)
				return false;
			auto Rotation = Spline->GetRotationAtDistanceAlongSpline(point.InVal, ESplineCoordinateSpace::World);
			OutMatrix = FRotationMatrix::Make(Rotation);
			return true;
		}

		if (_selectedEventPointIndex.Num() > 0 && CurrMode == FWidget::WM_Translate)
		{
			auto& point = FollowerComponent->GetEventPoints().Points[_selectedEventPointIndex.Last()];
			auto* Spline = FollowerComponent->GetSplineToFollow();
			if (!Spline)
				return false;
			auto Rotation = Spline->GetRotationAtDistanceAlongSpline(point.Distance, ESplineCoordinateSpace::World);
			OutMatrix = FRotationMatrix::Make(Rotation);
			return true;
		}
	}

	if (ViewportClient->GetWidgetCoordSystemSpace() == COORD_World)
	{
		_bHideCurrentRotationPointWidget = false;
	}

	return false;
}

bool FPFPathFollowerVisulalizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
	ENTER_EXIT_LOG;

	auto* FollowerComponent = GetEditedPathFollower();
	if (!FollowerComponent || !FollowerComponent->GetSplineToFollow())
		return false;

	if (_SelectedRotationPointIndex.Num() > 0)
	{
		auto& rollCurve = FollowerComponent->GetPathRoller().GetRollCurve();
		int32 LastIndex = _SelectedRotationPointIndex.Last();
		auto& LastRollPoint = rollCurve.Points[LastIndex];
		auto DistanceDelta = GetNewDistance(LastRollPoint.InVal, DeltaTranslate) - LastRollPoint.InVal;

		for (int i = 0; i < _SelectedRotationPointIndex.Num(); ++i)
		{
			int32 Index = _SelectedRotationPointIndex[i];

			auto& RotationPoint = rollCurve.Points[Index];
			auto Euler = RotationPoint.OutVal;
			auto Rotation = FRotator::MakeFromEuler(Euler);

			auto WorldSpaceRotation = FollowerComponent->GetSplineToFollow()->GetComponentQuat() * Rotation.Quaternion();
			WorldSpaceRotation = DeltaRotate.Quaternion() * WorldSpaceRotation; // apply world-space rotation
			auto NewLocalRotation = FollowerComponent->GetSplineToFollow()->GetComponentQuat().Inverse() * WorldSpaceRotation; // convert world-space rotation to local-space

			RotationPoint.OutVal = NewLocalRotation.Euler();
			if (!DeltaRotate.IsNearlyZero())
			{
				PF_LOG_VERB(LogFollowerVizualizer, "Euler: %s", *Euler.ToCompactString());
				PF_LOG_VERB(LogFollowerVizualizer, "Rotation: %s", *Rotation.ToCompactString());
				PF_LOG_VERB(LogFollowerVizualizer, "DeltaRotate: %s", *DeltaRotate.ToString());
				PF_LOG_VERB(LogFollowerVizualizer, "New Euler: %s", *RotationPoint.OutVal.ToCompactString());
			}
			auto PointDistanceAlongCurve = RotationPoint.InVal;
			_SelectedRotationPointIndex[i] = rollCurve.MovePoint(Index, PointDistanceAlongCurve + DistanceDelta);
		}

		FollowerComponent->GetPathRoller().SetInterpolationType(FollowerComponent->RollIntepType, FollowerComponent->GetSplineToFollow());
	}


	for (int32 Index : _selectedEventPointIndex)
	{
		auto& EventPoints = FollowerComponent->GetEventPoints().Points;
		auto& EventPoint = EventPoints[Index];
		auto NewDistance = GetNewDistance(EventPoint.Distance, DeltaTranslate);
		EventPoint.Distance = NewDistance;
	}

	for (int i = 0; i < _selectedSpeedPointIndex.Num(); ++i)
	{
		int32 Index = _selectedSpeedPointIndex[i];
		auto& curve = FollowerComponent->GetSpeedCurve();
		auto& speedPoints = curve.Points;
		auto& SpeedPoint = speedPoints[Index];
		auto NewDistance = GetNewDistance(SpeedPoint.InVal, DeltaTranslate);
		_selectedSpeedPointIndex[i] = curve.MovePoint(Index, NewDistance);
	}

	return true;
}

bool FPFPathFollowerVisulalizer::HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		_WidgetMode = FWidget::WM_None;
	}

	return false;
}

float FPFPathFollowerVisulalizer::GetNewDistance(float CurrentDistance/*location*/, FVector DeltaTranslate)
{
	ENTER_EXIT_LOG;

	if (DeltaTranslate.IsNearlyZero())
		return CurrentDistance;

	auto* FollowerComponent = GetEditedPathFollower();

	FVector tanAtDistance = FollowerComponent->GetSplineToFollow()->GetWorldDirectionAtDistanceAlongSpline(CurrentDistance);
	auto dotProduct = FVector::DotProduct(tanAtDistance, DeltaTranslate.GetSafeNormal());
	auto NewDistance = CurrentDistance + (dotProduct*DeltaTranslate.Size());
	UE_LOG(LogFollowerVizualizer, VeryVerbose, TEXT("NewDistance: %f"), NewDistance);

	NewDistance = FMath::Clamp(NewDistance, 0.f, FollowerComponent->GetSplineToFollow()->GetSplineLength());

	return NewDistance;
}

TSharedPtr<SWidget> FPFPathFollowerVisulalizer::GenerateContextMenu() const
{
	auto* FollowerComponent = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
	if (!FollowerComponent)
		return TSharedPtr<SWidget>();

	if (_selectedSpeedPointIndex.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, /*global commands*/nullptr);
		{
			MenuBuilder.BeginSection(NAME_None, FText::FromString("Speed Point properties"));
			{
				auto& SpeedPoint = FollowerComponent->GetSpeedCurve().Points[_selectedSpeedPointIndex.Last()];
				auto& Speed = FollowerComponent->TimeBased ? SpeedPoint.InVal : SpeedPoint.OutVal;
				TSharedRef<SWidget> SpeedWidget = SNew(SNumericEntryBox<float>)
					.Value_Lambda([&]()
				{
					return Speed;
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					for (int32 SpeedPointIndex : _selectedSpeedPointIndex)
					{
						auto& Point = Follower->GetSpeedCurve().Points[SpeedPointIndex];
						float& Value = Follower->TimeBased ? Point.InVal : Point.OutVal;
						Value = NewValue;
					}
				});
				MenuBuilder.AddWidget(SpeedWidget, FText::FromString(FollowerComponent->TimeBased ? "Time" : "Speed"));

				SpeedPoint = FollowerComponent->GetSpeedCurve().Points[_selectedSpeedPointIndex.Last()];
				auto& Distance = FollowerComponent->TimeBased ? SpeedPoint.OutVal : SpeedPoint.InVal;
				TSharedRef<SWidget> DistanceWidget = SNew(SNumericEntryBox<float>)
					.Value_Lambda([&]()
				{
					return Distance;
				})
				.OnValueChanged_Lambda([this](float NewValue)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					for (int32 SpeedPointIndex : _selectedSpeedPointIndex)
					{
						auto& Point = Follower->GetSpeedCurve().Points[SpeedPointIndex];
						float& Value = Follower->TimeBased ? Point.OutVal : Point.InVal;
						Value = NewValue;
					}
				});
				MenuBuilder.AddWidget(DistanceWidget, FText::FromString("Distance"));
			}
			MenuBuilder.EndSection();

			MenuBuilder.BeginSection(NAME_None, FText::FromString("Actions"));
			{

				TSharedRef<SWidget> DeleteButton = SNew(SButton)
					.OnClicked_Raw(const_cast<FPFPathFollowerVisulalizer*>(this), &FPFPathFollowerVisulalizer::DeleteSpeedPoints)
					.Text(FText::FromString(TEXT("Delete All Selected Speed Points")));

				MenuBuilder.AddWidget(DeleteButton, FText::GetEmpty());

			}
			MenuBuilder.EndSection();

		}

		return MenuBuilder.MakeWidget();
	}

	if (_SelectedRotationPointIndex.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, /*global commands*/nullptr);
		{
			MenuBuilder.BeginSection(NAME_None, FText::FromString("Roll Curve Point properties"));
			{
				auto& RollAngle = FollowerComponent->GetPathRoller().GetRollCurve().Points[_SelectedRotationPointIndex.Last()].OutVal;
				TSharedRef<SWidget> RollWidget = SNew(SVectorInputBox)
					.AllowResponsiveLayout(true)
					.X(RollAngle.X)
					.Y(RollAngle.Y)
					.Z(RollAngle.Z)
					.OnXCommitted_Lambda([this, RollAngle](float X, ETextCommit::Type type)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto Current = RollAngle;
					auto& RollCurve = Follower->GetPathRoller().GetRollCurve();
					for (int32 RotPointIndex : _SelectedRotationPointIndex)
					{
						RollCurve.Points[RotPointIndex].OutVal = FVector(X, Current.Y, Current.Z);
					}

				})
					.OnYCommitted_Lambda([this, RollAngle](float Y, ETextCommit::Type type)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto Current = RollAngle;
					auto& RotationPoints = Follower->GetPathRoller().GetRollCurve().Points;
					for (int32 RotPointIndex : _SelectedRotationPointIndex)
					{
						if (RotationPoints.IsValidIndex(RotPointIndex))
						{
							RotationPoints[RotPointIndex].OutVal = FVector(Current.X, Y, Current.Z);
						}
					}
				})
					.OnZCommitted_Lambda([this, RollAngle](float Z, ETextCommit::Type type)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto Current = RollAngle;
					auto& RotationPoints = Follower->GetPathRoller().GetRollCurve().Points;
					for (int32 RotPointIndex : _SelectedRotationPointIndex)
					{
						if (RotationPoints.IsValidIndex(RotPointIndex))
						{
							RotationPoints[RotPointIndex].OutVal = FVector(Current.X, Current.Y, Z);
						}
					}
				});
				MenuBuilder.AddWidget(RollWidget, FText::FromString("Rotation"));

				auto& Distance = FollowerComponent->GetPathRoller().GetRollCurve().Points[_SelectedRotationPointIndex.Last()].InVal;
				TSharedRef<SWidget> DistanceWidget = SNew(SNumericEntryBox<float>)
					.Value_Lambda([&]()
				{
					return Distance;
				})
					.OnValueChanged_Lambda([this, &Distance](float NewValue)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto& RotationPoints = Follower->GetPathRoller().GetRollCurve().Points;
					for (int32 RotPointIndex : _SelectedRotationPointIndex)
					{
						if (RotationPoints.IsValidIndex(RotPointIndex))
						{
							RotationPoints[RotPointIndex].InVal = NewValue;
						}
					}

					Distance = NewValue;
				});
				MenuBuilder.AddWidget(DistanceWidget, FText::FromString("Distance"));
			}

			MenuBuilder.BeginSection(NAME_None, FText::FromString("Actions"));
			{

				TSharedRef<SWidget> DeleteButton = SNew(SButton)
					.OnClicked_Raw(const_cast<FPFPathFollowerVisulalizer*>(this), &FPFPathFollowerVisulalizer::DeleteRotationPoint)
					.Text(FText::FromString(TEXT("Delete Rotation Point")));

				MenuBuilder.AddWidget(DeleteButton, FText::GetEmpty());

			}
			MenuBuilder.EndSection();

		}

		return MenuBuilder.MakeWidget();
	}

	if (_selectedEventPointIndex.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, /*global commands*/nullptr);
		{
			MenuBuilder.BeginSection(NAME_None, FText::FromString("Event Point properties"));
			{
				auto& Name = FollowerComponent->GetEventPoints().Points[_selectedEventPointIndex.Last()].Name;
				TSharedRef<SWidget> NameWidget = SNew(SEditableTextBox)
					.Text_Lambda([&]()
				{
					return FText::FromName(Name);
				})
					.OnTextChanged_Lambda([this](const FText& NewValue)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto& EventPoints = Follower->GetEventPoints().Points;
					for (int32 EventPointIndex : _selectedEventPointIndex)
					{
						EventPoints[EventPointIndex].Name = FName(*NewValue.ToString());
					}
				});
				MenuBuilder.AddWidget(NameWidget, FText::FromString("Name"));

				auto& Distance = FollowerComponent->GetEventPoints().Points[_selectedEventPointIndex.Last()].Distance;
				TSharedRef<SWidget> DistanceWidget = SNew(SNumericEntryBox<float>)
					.Value_Lambda([&]()
				{
					return Distance;
				})
					.OnValueChanged_Lambda([this](float NewValue)
				{
					auto* Follower = const_cast<FPFPathFollowerVisulalizer*>(this)->GetEditedPathFollower();
					if (!Follower)
						return;
					auto& EventPoints = Follower->GetEventPoints().Points;
					for (int32 EventPointIndex : _selectedEventPointIndex)
					{
						EventPoints[EventPointIndex].Distance = NewValue;
					}
				});
				MenuBuilder.AddWidget(DistanceWidget, FText::FromString("Distance"));
			}
		}

		return MenuBuilder.MakeWidget();
	}

	return TSharedPtr<SWidget>();
}

FReply FPFPathFollowerVisulalizer::DeleteSpeedPoints()
{
	auto* FollowerComponent = GetEditedPathFollower();

	//todo: _selectedSpeedPointIndex.Num() is always 1 or 0. how's that possible? was working once
	TArray<int32> tmp = _selectedSpeedPointIndex;
	if (tmp.Num() > 0)
	{
		if (tmp.Num() > 1)
		{
			auto Msg = FText::FromString("More than one speed points selected. Operation will delete all of them. Continue?");
			auto Title = FText::FromString("Delete all selected");
			auto Return = FMessageDialog::Open(EAppMsgType::YesNo, Msg, &Title);
			if (Return == EAppReturnType::No)
				return FReply::Handled();
		}


		tmp.Sort();
		for (auto index : tmp)
		{
			FollowerComponent->GetSpeedCurve().Points.RemoveAt(index, 1, false);
		}

		_selectedSpeedPointIndex.Empty();

		if (FollowerComponent->GetSpeedCurve().Points.Num() == 0)
		{
			FollowerComponent->bUseSpeedCurve = false;
		}
	}

	return FReply::Handled();
}

FReply FPFPathFollowerVisulalizer::DeleteRotationPoint()
{
	auto* FollowerComponent = GetEditedPathFollower();

	TArray<int32> tmp = _SelectedRotationPointIndex;
	if (tmp.Num() > 0)
	{
		if (tmp.Num() > 1)
		{
			auto Msg = FText::FromString("More than one speed points selected. Operation will delete all of them. Continue?");
			auto Title = FText::FromString("Delete all selected");
			auto Return = FMessageDialog::Open(EAppMsgType::YesNo, Msg, &Title);
			if (Return == EAppReturnType::No)
				return FReply::Handled();
		}


		tmp.Sort();
		//for (int i = tmp.Num()-1; i >= 0; --i)
		for (auto index : tmp)
		{
			//_FollowerComponent->GetSpeedCurve().GetCurve().Points.RemoveAt(tmp[i], 1, false);
			FollowerComponent->GetPathRoller().GetRollCurve().Points.RemoveAt(index, 1, false);
		}

		_selectedSpeedPointIndex.Empty();

		if (FollowerComponent->GetPathRoller().GetRollCurve().Points.Num() == 0)
		{
			FollowerComponent->bUseRotationCurve = false;
		}
	}

	return FReply::Handled();
}

void FPFPathFollowerVisulalizer::EndEditing()
{
	ClearSelectionIndexes();
}

#endif
