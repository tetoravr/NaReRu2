// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#if WITH_EDITOR

#include "ComponentVisualizer.h"
#include "PFVisualizerProxies.h"
#include "Runtime/Launch/Resources/Version.h"

/**
 * PathFollowerVizulalizer provides visualization for auto roll feature.
 * It draws a line for each roll sample. User can specify color, thickness, size/length of a line.
 */
class PATHFOLLOW_API FPFPathFollowerVisulalizer : public FComponentVisualizer
{
	struct FTimeDistance
	{
		float Distance = -1.f;
		float Time = -1.f;
	};

public:
	FPFPathFollowerVisulalizer();
	~FPFPathFollowerVisulalizer();
	

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
#if ENGINE_MINOR_VERSION < 13
	virtual bool VisProxyHandleClick(FLevelEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
#else
	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;
#endif
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual TSharedPtr<SWidget> GenerateContextMenu() const override;
	FReply DeleteSpeedPoints();
	FReply DeleteRotationPoint();

	//gets called if there was a hit to any hitproxy of this visualizer 
	//it's not called on selection status change
	virtual void EndEditing() override;
protected:

private:
	const class UPFPathFollowerComponent* GetEditedPathFollower(const UActorComponent* Component);
	class UPFPathFollowerComponent* GetEditedPathFollower();
	const class UPFPathFollowerComponent* GetEditedPathFollower() const;
	void DrawRollCurveVisualization(class USplineComponent * path, FPrimitiveDrawInterface* PDI, const class UPFPathFollowerComponent* Component);

	float GetNewDistance(float CurrentDictance/*location*/, FVector DeltaTranslate);
	TArray<FTimeDistance> GetTimeDistanceArray(const class UPFPathFollowerComponent* Follower, USplineComponent* Spline);
	void ClearSelectionIndexes();
	template <class TProxyType>
	bool HandleSelection(HComponentVisProxy* VisProxy, const FViewportClick& Click, TArray<int32>& AddTo);
	//TWeakObjectPtr<class UPFPathFollowerComponent> _FollowerComponent;
	using TIndexArray = TArray<int32>;
	TIndexArray _SelectedRotationPointIndex;
	TIndexArray _selectedEventPointIndex;
	TIndexArray _selectedSpeedPointIndex;
	mutable bool _bHideCurrentRotationPointWidget = false;
	mutable FWidget::EWidgetMode _WidgetMode = FWidget::WM_None;
	mutable FRotator _StartRotation;
	
	FComponentPropertyPath FollowerPropertyPath;
};

#endif
