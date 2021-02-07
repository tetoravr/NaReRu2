// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#if WITH_EDITOR

#include "Editor/PropertyEditor/Public/IDetailCustomization.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/DistCurveEditor/Public/IDistCurveEditor.h"
//#include "PFPathFollowerCustomization.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PFPathFollowerCustomization, Warning, Warning);

/**
 * Provide event point creation and roll angles curve features
 */
class PATHFOLLOW_API FPFPathFollowerCustomization : public IDetailCustomization
{
public:
	FPFPathFollowerCustomization();
	~FPFPathFollowerCustomization();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
  static TSharedRef<IDetailCustomization> MakeInstance();

  /** IDetailCustomization interface */
  virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	



	TSharedRef<class IDistributionCurveEditor> CreateCurveEditor();

private:
	FReply AddNewEventPoint();
	bool Can_AddEventPoint_BeExecuted() const;
	TSharedRef<SDockTab> OnSpawnCurveEditor(const FSpawnTabArgs& SpawnTabArgs);
	bool IsOpenCurveEditorEnabled() const;
	FReply OpenRollCurveEditor();
	bool IsAlwaysOpenRollEditorEnabled() const;
	FReply GenerateRollAngles();
	class UPFPathFollowerComponent* _PathFollower;
	static const FName CurveTabId;

	void OnSplineSelectionChanged(TSharedPtr<FString> SelectedText, ESelectInfo::Type SelectInfo);
	TSharedPtr<FString> GetSelectedSplineName();
	void RefreshSplineNames();
	TArray< TSharedPtr<FString> > SplineNames;
	
};

#endif
