// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.
#if WITH_EDITOR

#include "PathFollowPrivatePCH.h"
#include "PFPathFollowerCustomization.h"
#include "Components/PFPathFollowerComponent.h"
#include "Components/SplineComponent.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "Editor/DistCurveEditor/Public/DistCurveEditorModule.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

struct CurveDistPair
{
	CurveDistPair(FInterpCurveFloat& C, const UObject* D)
		: Curve(C)
		, Distrib(D)
	{
	}

	FInterpCurveFloat& Curve;
	const UObject* Distrib;
};
using CurveDistributionPairArray = TArray<CurveDistPair>;

class FCurveEdNotify : public FCurveEdNotifyInterface
{
public:
	FCurveEdNotify(FInterpCurveFloat& SpeedCurve, FInterpCurveVector& RotationCurve)
		: _SpeedCurve(SpeedCurve)
		, _RotationCurve(RotationCurve)
	{
	}

	virtual ~FCurveEdNotify() {}

	virtual void PreEditCurve(TArray<UObject*> CurvesAboutToChange) override
	{
		_EdittedDistributions = CurvesAboutToChange;
	}

	virtual void PostEditCurve() override
	{
		for (auto& Distribution : _EdittedDistributions)
		{
			auto* EdittedDistr = Cast<UDistributionFloatConstantCurve>(Distribution);
			if (EdittedDistr)
			{
				_SpeedCurve = EdittedDistr->ConstantCurve;
			}

			auto* RollDistr = Cast<UDistributionVectorConstantCurve>(Distribution);
			if (RollDistr)
			{
				_RotationCurve.Reset();
				for (auto& VectorPoint : RollDistr->ConstantCurve.Points)
				{
					int index = _RotationCurve.AddPoint(VectorPoint.InVal, VectorPoint.OutVal);
					_RotationCurve.Points[index].InterpMode = VectorPoint.InterpMode;
					_RotationCurve.Points[index].ArriveTangent = VectorPoint.ArriveTangent;
					_RotationCurve.Points[index].LeaveTangent = VectorPoint.LeaveTangent;
				}


			}
		}
	}

private:
	FInterpCurveFloat& _SpeedCurve;
	FInterpCurveVector& _RotationCurve;
	TArray<UObject*> _EdittedDistributions;
};


DEFINE_LOG_CATEGORY(PFPathFollowerCustomization)

#define LOCTEXT_NAMESPACE "FPFPathFollowerCustomization"

const FName FPFPathFollowerCustomization::CurveTabId("PathFollow");

FPFPathFollowerCustomization::FPFPathFollowerCustomization()
{

}


FPFPathFollowerCustomization::~FPFPathFollowerCustomization()
{
	if (FGlobalTabmanager::Get()->HasTabSpawner(CurveTabId))
	{
		auto RollCurveEditorTab = FGlobalTabmanager::Get()->TryInvokeTab(CurveTabId);
		RollCurveEditorTab->RequestCloseTab();
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CurveTabId);
	}
}


TSharedRef<IDetailCustomization> FPFPathFollowerCustomization::MakeInstance()
{
  return MakeShareable(new FPFPathFollowerCustomization);
}

void FPFPathFollowerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
  TArray< TWeakObjectPtr<UObject>> OutObjects;
  DetailBuilder.GetObjectsBeingCustomized(OutObjects);
  check(OutObjects.Num() > 0);
  _PathFollower = CastChecked<UPFPathFollowerComponent>(OutObjects[0].Get());

  IDetailCategoryBuilder& PathFollowCategory = DetailBuilder.EditCategory("PathFollow", FText::FromString("General"), ECategoryPriority::Important);
  
	IDetailCategoryBuilder& EventsCategory = DetailBuilder.EditCategory("Events", FText::FromString("Events"), ECategoryPriority::Important);
  EventsCategory.AddCustomRow(FText::FromString("EventPointRow"))
    .WholeRowContent()
    [
			SNew(SButton)
			.IsEnabled_Raw(this, &FPFPathFollowerCustomization::Can_AddEventPoint_BeExecuted)
      .Text(FText::FromString("Create new event point")).OnClicked_Raw(this, &FPFPathFollowerCustomization::AddNewEventPoint)
    ];


	IDetailCategoryBuilder& SpeedCategory = DetailBuilder.EditCategory("Speed", FText::FromString("Speed"), ECategoryPriority::Important);
	SpeedCategory.AddCustomRow(FText::FromString("SpeedPointRow"))
		.WholeRowContent()
		[
			SNew(SButton)
			.IsEnabled_Lambda([this](){ return _PathFollower->bUseSpeedCurve && _PathFollower->HasPath(); })
			.Text(FText::FromString("Create new speed point")).OnClicked_Lambda([&]()
			{
				check(_PathFollower);
				if (auto* Spline = _PathFollower->GetSplineToFollow())
				{
					const float Distance = Spline->GetSplineLength() / 5;
					auto& SpeedCurve = _PathFollower->GetSpeedCurve();
					SpeedCurve.AddPoint(Distance, 0);
					const bool FirstSpeedPoint = SpeedCurve.Points.Num() == 1;
					if (FirstSpeedPoint)
					{
						_PathFollower->bUseSpeedCurve = true;
					}
				}
				return FReply::Handled();
			})
		];


	PathFollowCategory.AddCustomRow(FText::FromString("OpenCurveEditorRow"))
		.NameContent()
		[
			SNew(SButton)
			.IsEnabled_Raw(this, &FPFPathFollowerCustomization::IsOpenCurveEditorEnabled)
			.Text(FText::FromString("Open curve editor")).OnClicked_Raw(this, &FPFPathFollowerCustomization::OpenRollCurveEditor)
			.ToolTipText(FText::FromString("Click to open curve editor to fine tune rotation and speed"))
		]
		.ValueContent()
		[
			SNew(SGridPanel)
			.ToolTipText(FText::FromString("Check this to automatically open curve editor everytime the PFPathFollowerComponent gets selected"))
      + SGridPanel::Slot(0, 0).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Always open"))
			]
			+ SGridPanel::Slot(1, 0)
			.Padding(5)
				[
					SNew(SCheckBox)
					.IsChecked(_PathFollower->bAlwaysOpenRollCurveEditor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					.IsEnabled_Raw(this, &FPFPathFollowerCustomization::IsAlwaysOpenRollEditorEnabled)
					.OnCheckStateChanged_Lambda([&](ECheckBoxState NewState){
						_PathFollower->bAlwaysOpenRollCurveEditor = NewState == ECheckBoxState::Checked;
					})
					
				]
		];

		RefreshSplineNames();
		if (SplineNames.Num() > 0)
		{
			auto SelectedPathName = GetSelectedSplineName();

			PathFollowCategory.AddCustomRow(FText::FromString("PathSelectionRow"))
				.NameContent()
				[
					SNew(STextBlock).Text(FText::FromString("Path selection"))
				]
			.ValueContent()
				[
					SNew(STextComboBox)
					.OptionsSource(&SplineNames)
					.InitiallySelectedItem(SelectedPathName)
				.OnSelectionChanged(this, &FPFPathFollowerCustomization::OnSplineSelectionChanged)
				];
		}
		else if (_PathFollower->GetPathOwner())
		{
			//show msg box, clear path owner value
			auto MsgTitle = FText::FromString("No PFPath component found");
			_PathFollower->SetPathOwner(nullptr);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("PathOwner does not contain any PFPath/Spline component. Please choose different actor."), &MsgTitle);
		}
					

		if (_PathFollower->bAlwaysOpenRollCurveEditor && (_PathFollower->bUseRotationCurve || _PathFollower->bUseSpeedCurve))
		{
			OpenRollCurveEditor();
		} 
			

		IDetailCategoryBuilder& RotationCategory = DetailBuilder.EditCategory("Rotation", FText::FromString("Rotation"), ECategoryPriority::Important);
		RotationCategory.AddCustomRow(FText::FromString("RotationPointRow"))
			.WholeRowContent()
			[
				SNew(SButton)
				.IsEnabled_Lambda([&]() { return _PathFollower->bUseSpeedCurve && _PathFollower->HasPath(); })
			.Text(FText::FromString("Create new rotation point")).OnClicked_Lambda([&]()
		{
			check(_PathFollower);
			auto* Spline = _PathFollower->GetSplineToFollow();
			if (Spline)
			{
				float Distance = Spline->GetSplineLength() / 5;
				auto Rotation = Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
				_PathFollower->GetPathRoller().GetRollCurve().AddPoint(Distance, Rotation.Euler() );
				if (_PathFollower->GetPathRoller().GetRollCurve().Points.Num() == 1)
				{
					_PathFollower->bUseSpeedCurve = true;
				}
			}
			return FReply::Handled();
		})
			];

		//IDetailCategoryBuilder& AutoRollCategory = DetailBuilder.EditCategory("AutoRoll", FText::FromString("Auto Roll"), ECategoryPriority::Important);
		RotationCategory.AddCustomRow(FText::FromString("GenerateRotationPointsRow"))
      .WholeRowContent()
      [
        SNew(SButton)
        .Text(FText::FromString("Generate Rotation Points"))
        .ToolTipText(FText::FromString("Pressing this will regenerate rotation points. All custom changes to rotation points will be lost"))
        .OnClicked_Raw(this, &FPFPathFollowerCustomization::GenerateRollAngles)
        .IsEnabled_Lambda([&](){
					return _PathFollower && (_PathFollower->HasPath() || _PathFollower->GetPathOwner());
        })
      ];
}

void FPFPathFollowerCustomization::RefreshSplineNames()
{
	check(_PathFollower);

	SplineNames.Reset();

	if (auto* PathOwner = _PathFollower->GetPathOwner())
	{
		TArray<USplineComponent*> Splines;
		PathOwner->GetComponents(Splines);
		for (auto* Spline : Splines)
		{
			SplineNames.Add(MakeShareable(new FString(Spline->GetName())));
		}
	}
}

TSharedPtr<FString> FPFPathFollowerCustomization::GetSelectedSplineName()
{
	check(_PathFollower);
	

	TSharedPtr<FString> SelectedPathName;
	if (auto* SplineToFollow = _PathFollower->GetSplineToFollow())
	{
		auto SplineName = SplineToFollow->GetName();
		TSharedPtr<FString>* FoundName = SplineNames.FindByPredicate([SplineName](TSharedPtr<FString> ExistingSplineName)
		{
			return *ExistingSplineName == SplineName;
		});

		if (FoundName)
		{
			check(FoundName->IsValid());
			SelectedPathName = *FoundName;
		}
		else
		{
			SelectedPathName = SplineNames[0];
		}
	}
	
	return SelectedPathName;
}

void FPFPathFollowerCustomization::OnSplineSelectionChanged(TSharedPtr<FString> SelectedText, ESelectInfo::Type SelectInfo)
{
	check(_PathFollower && _PathFollower->GetPathOwner());
	TArray<USplineComponent*> Splines;
	_PathFollower->GetPathOwner()->GetComponents(Splines);
	USplineComponent** SelectedSpline = Splines.FindByPredicate([SelectedText](USplineComponent* Spline)
	{
		return Spline && Spline->GetName() == *SelectedText;
	});
	
	if (*SelectedSpline)
	{
		_PathFollower->SetPathToFollow(*SelectedSpline);
	}
	
}

TSharedRef<SDockTab>  FPFPathFollowerCustomization::OnSpawnCurveEditor(const FSpawnTabArgs& SpawnTabArgs)
{
	auto CurveEditor = CreateCurveEditor();
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(10, 5)
			[
				CurveEditor
			]
		];
}

FReply FPFPathFollowerCustomization::AddNewEventPoint()
{
  FEventPoint newPoint;
  newPoint.Distance = 10;
	check(_PathFollower)
  _PathFollower->GetEventPoints().Points.Add(newPoint);
  return FReply::Handled();
}
bool FPFPathFollowerCustomization::Can_AddEventPoint_BeExecuted() const
{
	return _PathFollower && _PathFollower->HasPath();
}

FReply FPFPathFollowerCustomization::OpenRollCurveEditor()
{
	if (!FGlobalTabmanager::Get()->HasTabSpawner(CurveTabId))
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CurveTabId, FOnSpawnTab::CreateRaw(this, &FPFPathFollowerCustomization::OnSpawnCurveEditor))
			.SetDisplayName(LOCTEXT("FPFPathFollowerCustomization", "PathFollow Curve editor"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	FGlobalTabmanager::Get()->TryInvokeTab(CurveTabId);
	return FReply::Handled();
}

bool FPFPathFollowerCustomization::IsOpenCurveEditorEnabled() const
{
	return IsAlwaysOpenRollEditorEnabled() && _PathFollower->bUseRotationCurve;
}

bool FPFPathFollowerCustomization::IsAlwaysOpenRollEditorEnabled() const
{
	return _PathFollower!=nullptr;
}

FReply FPFPathFollowerCustomization::GenerateRollAngles()
{
	check(_PathFollower);

	auto* SplineToFollow = _PathFollower->GetSplineToFollow();
	if (SplineToFollow == nullptr)
	{
		auto msgtitle = FText::FromString("No path assigned!");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please assign path owner before generating rotation points."), &msgtitle);
	}
	else
	{
		_PathFollower->ComputeAutoRotationPoints();
		_PathFollower->bFollowRotation = false;
		_PathFollower->bUseRotationCurve = true;
	}

	return FReply::Handled();
}

TSharedRef<IDistributionCurveEditor> FPFPathFollowerCustomization::CreateCurveEditor()
{
	auto* CurveEdSetup = NewObject<UInterpCurveEdSetup>(_PathFollower, NAME_None, RF_Transactional);
	IDistributionCurveEditorModule* CurveEditorModule = &FModuleManager::LoadModuleChecked<IDistributionCurveEditorModule>("DistCurveEditor");

	auto* RollCurveDistribution = _PathFollower->GetRollAnglesDistribution();
	auto* SpeedCurveDistribution = _PathFollower->GetSpeedDistribution();
	CurveDistributionPairArray Pairs;
	//Pairs.Add(CurveDistPair(_PathFollower->GetPathRoller().GetRollCurve(), RollCurveDistribution));
	//Pairs.Add(CurveDistPair(_PathFollower->GetSpeedCurve().GetCurve(), SpeedCurveDistribution));
	//auto CurveEditor = CurveEditorModule->CreateCurveEditorWidget(CurveEdSetup, new FCurveEdNotify(Pairs));
	auto* Notify = new FCurveEdNotify(_PathFollower->GetSpeedCurve(), _PathFollower->GetPathRoller().GetRollCurve());
	auto CurveEditor = CurveEditorModule->CreateCurveEditorWidget(CurveEdSetup, Notify);
	CurveEdSetup->AddCurveToCurrentTab(RollCurveDistribution, TEXT("Rotation Curve"), FColor::Red);
	CurveEdSetup->AddCurveToCurrentTab(SpeedCurveDistribution, TEXT("Speed Curve"), FColor::Green);
	CurveEditor->FitViewHorizontally();
	CurveEditor->FitViewVertically();
	CurveEditor->RefreshViewport();

	return CurveEditor;
}

#undef LOCTEXT_NAMESPACE

#endif
