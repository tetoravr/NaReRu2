// Copyright 2015-2016 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFPathFollowerComponent.h"
#include "PFPathComponent.h"
#include "Runtime/Launch/Resources/Version.h"

DEFINE_LOG_CATEGORY(PF)

// Sets default values for this component's properties
UPFPathFollowerComponent::UPFPathFollowerComponent()
{
	ENTER_EXIT_LOG;

	bWantsInitializeComponent = true;
	bTickInEditor = true;
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITOR	
	PrimaryComponentTick.bStartWithTickEnabled = true;
#else
	PrimaryComponentTick.bStartWithTickEnabled = false;
#endif
}


// Called when the game starts
void UPFPathFollowerComponent::InitializeComponent()
{
	ENTER_EXIT_LOG;

	if (!FMath::IsNearlyZero(TickInterval))
		PrimaryComponentTick.TickInterval = TickInterval;

	Super::InitializeComponent();

	EventPoints.Init();
	Init();

	
}

void UPFPathFollowerComponent::PostLoad()
{
	Super::PostLoad();

	if (_SpeedCurve._speedCurve.Points.Num() > 0 && _InterpSpeedCurve.Points.Num() == 0)
	{
		_InterpSpeedCurve = _SpeedCurve._speedCurve;
	}
}

void UPFPathFollowerComponent::Init()
{
	ENTER_EXIT_LOG;

	SetupUpdatedComponents();

	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return;

	InitCurrentDistance(SplineToFollow.Get());
	ConfigurePath();
	//todo: _elapsedTime setting? 
	GetEventPoints().Reset(CurrentDistanceOnPath, bReverse, _LastPassedEventIndex);

	if (bAlignToPathStart)
		AlignToCurrentDistance();
}

float UPFPathFollowerComponent::MeasurePathDuration() const
{
	check(SplineToFollow.IsValid());

	const int StepsCnt = 20;//todo: could be made configurable
	const float StepSize = 1.f / (float)StepsCnt;
	float AverageEase = 0;
	float Alpha = 0;
	for (int i = 0; i <= StepsCnt; ++i, Alpha += StepSize)
	{
		AverageEase += Ease(EasingType, Alpha, CurveDegree);
	}
	AverageEase /= (StepsCnt + 1);
	float AverageSpeed = SpeedDuration * AverageEase;
	float SplineLength = SplineToFollow->GetSplineLength();
	float Duration = SplineLength / AverageSpeed;

	return Duration;
}

void UPFPathFollowerComponent::BeginPlay()
{
	ENTER_EXIT_LOG;

	Super::BeginPlay();

	//redundant? are properties initialized before InitializeComponent is called?
	Init();

	if (bStartAtPlay)
		Start(StartDelay);
}

FPFEventPoints& UPFPathFollowerComponent::GetEventPoints()
{
	if (bOverridePathEventPoints)
	{
		return EventPoints;
	}
	else
	{
		auto* Path = GetPathToFollow();
		if (Path)
		{
			return Path->EventPoints;
		}
		else
		{
			return EventPoints;
		}
	}
}

const FPFEventPoints& UPFPathFollowerComponent::GetEventPoints() const
{
	return ((UPFPathFollowerComponent*)this)->GetEventPoints();
}

UPFPathComponent* UPFPathFollowerComponent::GetPathToFollow()
{
	return const_cast<UPFPathComponent*>(static_cast<const UPFPathFollowerComponent*>(this)->GetPathToFollow());
}

const UPFPathComponent* UPFPathFollowerComponent::GetPathToFollow() const
{
	return Cast<UPFPathComponent>(GetSplineToFollow());
}



void UPFPathFollowerComponent::SetPathToFollow(class USplineComponent* Spline)
{
	SplineToFollow = Spline;
	Init();
}

void UPFPathFollowerComponent::Start(float CurrentStartDelay/*=0*/)
{
	ENTER_EXIT_LOG;

	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return;

	if (FMath::IsNearlyZero(CurrentStartDelay))
	{
		StartImpl();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(_DelayTimer, this, &UPFPathFollowerComponent::StartImpl, CurrentStartDelay, false);
	}
}

void UPFPathFollowerComponent::StartImpl()
{
	ENTER_EXIT_LOG;

	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return;

	if (bPause)
	{
		bPause = false;
	}

	if (!bStarted)//Stop() was called or first Start()
	{
		InitCurrentDistance(SplineToFollow.Get());
		GetEventPoints().Reset(CurrentDistanceOnPath, bReverse, _LastPassedEventIndex);
		_ElapsedTime = DistanceToTime(CurrentDistanceOnPath);
		bStarted = true;
		Activate();

		OnStartPath.Broadcast(this);
	}
}

void UPFPathFollowerComponent::Pause()
{
	ENTER_EXIT_LOG;
	bPause = true;
}

void UPFPathFollowerComponent::Stop()
{
	ENTER_EXIT_LOG;

	bStarted = false;
	if (_DelayTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(_DelayTimer);
		_DelayTimer.Invalidate();
	}

#if !WITH_EDITOR	//deactivate will stop ticking which we do not want in editor because of spline monitoring
	if (!bLookAtEvenIfNotStarted)
		Deactivate();
#endif
}

void UPFPathFollowerComponent::SetPathOwner(class AActor* PathOwner)
{
	ENTER_EXIT_LOG;
	pathOwner = PathOwner;
	SplineToFollow = nullptr;//invalidate
	Init();
}

class AActor* UPFPathFollowerComponent::GetPathOwner()
{
	return pathOwner.Get();
}

void UPFPathFollowerComponent::Reverse(bool Reverse)
{
	bReverse = Reverse;
	GetEventPoints().Reset(CurrentDistanceOnPath, bReverse, _LastPassedEventIndex);
}

void SetUpdatedComponent(USceneComponent*& ComponentToSet, USceneComponent* DefaultComp)
{
	if (!ComponentToSet)
	{
		ComponentToSet = DefaultComp;
	}
}

void UPFPathFollowerComponent::SetupUpdatedComponents()
{
	if (GetOwner())
	{
		auto* OwnerRootComp = GetOwner()->GetRootComponent();
		SetUpdatedComponent(LocationComponent, OwnerRootComp);
		SetUpdatedComponent(RotationComponent, OwnerRootComp);
		SetUpdatedComponent(RollComponent, RotationComponent);
	}
}


// Called every frame
void UPFPathFollowerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	QUICK_SCOPE_CYCLE_COUNTER(STAT_PathFollowerComponent_Tick);

	if (!FMath::IsNearlyZero(TickInterval))
	{
		auto currTime = GetWorld()->GetTimeSeconds();
		DeltaTime = currTime - _lastTickTime;
		_lastTickTime = currTime;
	}

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::PIE)
	{
		MonitorSpline();
	}

	InformPathAboutSelection();
	FixAutoRollAndFollowRotationClash();

#endif

	if (bLookAtEvenIfNotStarted && LookAtComponent)
		LookAt();

	SplineToFollow = GetSplineToFollow();
	if (!bStarted || bPause || !SplineToFollow.IsValid() || FMath::IsNearlyZero(SplineToFollow->GetSplineLength()))
		return;

	FollowPath(DeltaTime);
}

void UPFPathFollowerComponent::FollowPath(float FollowStep)
{
	check((bFollowRotation != bUseRotationCurve) || (!bFollowRotation && !bUseRotationCurve));

	check(SplineToFollow.IsValid());

	SetupUpdatedComponents();

	check(LocationComponent && RotationComponent && RollComponent);
	UpdateCurrentDistanceOnPath(FollowStep);
	ProcessEvents(CurrentDistanceOnPath);

	if (!SplineToFollow.IsValid())
		return;

	FVector NewLocation = ComputeNewLocation(CurrentDistanceOnPath);
	FRotator NewRotation = FRotator::ZeroRotator;
	if (!LookAtComponent)
	{
		NewRotation = ComputeNewRotation(CurrentDistanceOnPath);
	}
	else if (!bLookAtEvenIfNotStarted)
	{
		NewRotation = ComputeLookAtRotation(LookAtComponent, NewLocation);
	}

	NewLocation = ModifyFinalLocation(NewLocation);
	NewRotation = ModifyFinalRotation(NewRotation);

	if (!SplineToFollow.IsValid())
		return;

	auto CurrLocation = LocationComponent->GetComponentToWorld().GetLocation();
	auto CurrRotation = RotationComponent->GetComponentToWorld().GetRotation().Rotator();
	auto FinalLocation = MaskLocation(CurrLocation, NewLocation);

	auto TeleportType = GetTeleportType();
	bool UpdatingSameComponent = (LocationComponent == RotationComponent) && (RotationComponent == RollComponent);
	bool UpdateRotation = bFollowRotation || bUseRotationCurve || (LookAtComponent && !bLookAtEvenIfNotStarted);

	if (UpdatingSameComponent && UpdateRotation)
	{
		FRotator FinalRotation = NewRotation;
		if (!RotationMaskLocal)
		{
			FinalRotation = MaskRotation(CurrRotation, NewRotation);
		}
		UE_LOG(PF, VeryVerbose, TEXT("Location: %s, Rotation: %s"), *FinalLocation.ToCompactString(), *FinalRotation.ToCompactString());
		LocationComponent->SetWorldLocationAndRotation(FinalLocation, FinalRotation, false, nullptr, TeleportType);
	}
	else
	{
		LocationComponent->SetWorldLocation(FinalLocation, false, nullptr, TeleportType);

		if (UpdateRotation)
		{
			FRotator FinalRotation = NewRotation;
			if (!RotationMaskLocal)
			{
				FinalRotation = MaskRotation(CurrRotation, NewRotation);
			}

			if (RotationComponent == RollComponent)
			{
				RotationComponent->SetWorldRotation(FinalRotation, false, nullptr, TeleportType);
			}
			else
			{
				FRotator RotationWithoutRoll(FinalRotation.Pitch, FinalRotation.Yaw, 0.f);
				RotationComponent->SetWorldRotation(RotationWithoutRoll, false, nullptr, TeleportType);
				FRotator Roll(0.f, 0.f, FinalRotation.Roll);
				RollComponent->SetRelativeRotation(Roll, false, nullptr, TeleportType);
			}
		}
	}

	UpdateLastMoveDirection(NewLocation);
	_LastTargetLocation = NewLocation;

	HandleEndOfPath();
}

void UPFPathFollowerComponent::HandleEndOfPath()
{
	if (!SplineToFollow.IsValid())
		return;

	bool EndPointReached;
	bool DistancePastPathEnd = CurrentDistanceOnPath > SplineToFollow->GetSplineLength();
	bool DistanceBeforeStart = CurrentDistanceOnPath < 0;
	if (bUseSpeedCurve)
	{
		EndPointReached = DistancePastPathEnd || DistanceBeforeStart;
	}
	else
	{
		bool DurationElapsed = _ElapsedTime > SplineToFollow->Duration;
		bool ElapsedBeforeStart = _ElapsedTime < 0;
		EndPointReached = DistancePastPathEnd || DistanceBeforeStart || DurationElapsed || ElapsedBeforeStart;
	}
	if (EndPointReached)//we've reached start or end point of path
	{
		if (bReverse)
		{
			OnReachedStart.Broadcast(this);
		}
		else
		{
			OnReachedEnd.Broadcast(this);
		}

		if (bLoop)
		{
			HandleLoopingType();
		}
		else
		{
			bStarted = false;
		}
	}
}


void UPFPathFollowerComponent::UpdateCurrentDistanceOnPath(float DeltaTime)
{
	float currLength = SplineToFollow->GetSplineLength();
	float ReverseSign = bReverse ? -1 : 1;
	if (bUseSpeedCurve && !TimeBased)
	{
		float NewSpeed = UpdateSpeed(CurrentDistanceOnPath);
		NewSpeed = FMath::Max(1.f, NewSpeed);
		CurrentDistanceOnPath += ReverseSign * NewSpeed * DeltaTime;
	}
	else if (bUseSpeedCurve && TimeBased)
	{
		_ElapsedTime += ReverseSign * DeltaTime;
		CurrentDistanceOnPath = FPFSpeedCurve::GetSpeedAtDistance(GetSpeedCurve(), _ElapsedTime);
	}
	else
	{
		if (EasingType == EEasingType::Linear)
		{
			CurrentDistanceOnPath += ReverseSign * DeltaTime * ModulateSpeed(SpeedDuration);
		}
		else
		{
			_ElapsedTime += ReverseSign * DeltaTime;
			float CurrAlpha = FMath::Clamp(_ElapsedTime / _pathDuration, 0.f, 1.f);
			CurrAlpha = Ease(EasingType, CurrAlpha, CurveDegree);
			CurrentDistanceOnPath = currLength*CurrAlpha;
		}
	}
}

#if WITH_EDITOR
void UPFPathFollowerComponent::InformPathAboutSelection()
{
	if (GetOwner())
	{
		bool CurrSelected = GetOwner()->IsSelected();;

		if (auto* PathToFollow = GetPathToFollow())
		{
			if (CurrSelected)
			{
				PathToFollow->OnFollowerSelected();
			}
			else
			{
				PathToFollow->OnFollowerDeselected();
			}
		}
	}
}

//in past versions bAutoRoll && bFollowRotation could both be set
//not possible anymore, favor AutoRoll
void UPFPathFollowerComponent::FixAutoRollAndFollowRotationClash()
{
	if (bUseRotationCurve && bFollowRotation)
	{
		UE_LOG(PF, Warning, TEXT("bAutoRoll && bFollowRotation! bFollowRotation turned off. actor: %s"), *GetOwner()->GetName());
		bFollowRotation = false;
	}
}

#endif

ETeleportType UPFPathFollowerComponent::GetTeleportType() const
{
	ETeleportType TeleportType = bTeleportPhysics ? ETeleportType::TeleportPhysics : ETeleportType::None;
	return TeleportType;
}

//todo: space handling, would be nice to support relative/local masks instead of just world axis mask
FVector UPFPathFollowerComponent::MaskLocation(const FVector& CurrentLocation, const FVector& ComputedNewLocation)
{
	FVector InvertedMask(!LocationUpdateMask.X, !LocationUpdateMask.Y, !LocationUpdateMask.Z);
	auto MaskedCurrLocation = CurrentLocation * InvertedMask;
	auto MaskedNewLocation = ComputedNewLocation * FVector(LocationUpdateMask);
	return MaskedNewLocation + MaskedCurrLocation;
}

FRotator UPFPathFollowerComponent::MaskRotation(const FRotator& CurrentRotation, const FRotator& ComputedNewRotation)
{
	auto MaskedCurrRotation = FRotator(CurrentRotation.Pitch * !RotationUpdateMask.X, CurrentRotation.Yaw * !RotationUpdateMask.Y, CurrentRotation.Roll * !RotationUpdateMask.Z);
	auto MaskedNewRotation = MaskRotation(ComputedNewRotation);
	return MaskedNewRotation + MaskedCurrRotation;
}

FRotator UPFPathFollowerComponent::MaskRotation(const FRotator& Rotation)
{
	auto MaskedNewRotation = FRotator(Rotation.Pitch * RotationUpdateMask.X, Rotation.Yaw * RotationUpdateMask.Y, Rotation.Roll* RotationUpdateMask.Z);
	return MaskedNewRotation;
}

FVector UPFPathFollowerComponent::ComputeNewLocation(float currDistance)
{
	check(SplineToFollow.IsValid());

	auto CoordSpace = ESplineCoordinateSpace::World;

	FVector NewLocation = FVector::ZeroVector;
	if (auto* PathComp = Cast<UPFPathComponent>(SplineToFollow.Get()))
	{
		NewLocation = PathComp->GetLocationAtDistanceAlongSplineMirrored(currDistance, CoordSpace);
	}
	else
	{
		NewLocation = SplineToFollow->GetLocationAtDistanceAlongSpline(currDistance, CoordSpace);
	}

	return NewLocation;
}

FRotator UPFPathFollowerComponent::ComputeNewRotation(float currDistance)
{
	check(SplineToFollow.IsValid());

	FRotator NewRotation(0.f);
	const float RotationCurveDistance = FMath::Clamp(CurrentDistanceOnPath + LookAhead, 0.f, SplineToFollow->GetSplineLength());
	
	if (bFollowRotation)
	{
		NewRotation = SplineToFollow->GetRotationAtDistanceAlongSpline(RotationCurveDistance, ESplineCoordinateSpace::Local);
	}
	else if (bUseRotationCurve)
	{
		NewRotation = GetRotationAtDistance(RotationCurveDistance, ESplineCoordinateSpace::Local);
		PF_LOG_VERYVERB(PF, "Rotation cuver NewRotation: %s", *NewRotation.ToCompactString());
	}

	NewRotation = ModulateRotation(NewRotation);

	if (RotationMaskLocal)
	{
		NewRotation = MaskRotation(NewRotation);
	}

	auto NewQuat = NewRotation.Quaternion();
	if (bReverse && bInvertRotationOnReverse)
	{
		NewQuat = NewQuat * FQuat(NewQuat.GetUpVector(), PI);
	}

	auto WorldSpaceRotation = SplineToFollow->GetComponentQuat() * NewQuat;
	return WorldSpaceRotation.Rotator();
}

void UPFPathFollowerComponent::LookAt()
{
	check(LookAtComponent);
	check(RotationComponent);
	check(LocationComponent);
	FRotator LookAtRotation = ComputeLookAtRotation(LookAtComponent, LocationComponent->GetComponentLocation());
	RotationComponent->SetWorldRotation(LookAtRotation, false, nullptr, GetTeleportType());
}

FRotator UPFPathFollowerComponent::ComputeLookAtRotation_Implementation(USceneComponent* TargetComponent, const FVector& FollowerLocation)
{
	check(TargetComponent);
	auto target = TargetComponent->GetComponentToWorld().GetLocation();
	auto LookAtRot = (target - FollowerLocation).Rotation();
	return LookAtRot;
}

#if WITH_EDITOR

void UPFPathFollowerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	ENTER_EXIT_LOG;

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UPFPathFollowerComponent, pathOwner))
	{
		SplineToFollow = nullptr;
		Init();
	}

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UPFPathFollowerComponent, StartDistance))
	{
		Init();
	}

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UPFPathFollowerComponent, EasingType))
	{
		Init();
	}

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UPFPathFollowerComponent, RollIntepType))
	{
		if (auto* Spline = GetSplineToFollow())
		{
			GetPathRoller().SetInterpolationType(RollIntepType, Spline);
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UPFPathFollowerComponent::MonitorSpline()
{
	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return;

	PF_LOG_VERYVERB(PF, "Got spline");

#if ENGINE_MINOR_VERSION < 13
	const bool TrajectoryEqual = _LastSplineInfo == SplineToFollow->SplineInfo;
#else
	const bool TrajectoryEqual = _LastSplineInfo == SplineToFollow->SplineCurves.Position;
#endif

	//this will detect change in path location/rotation
	//needs to be tracked since we want to align follower to start (if bAlignToStart) 
	//when path location changes
	const bool TransformEqual = _LastTransform.Equals(SplineToFollow->GetComponentToWorld());

	if (TrajectoryEqual && TransformEqual)
	{
		if (LastCurrentDistance == CurrentDistanceOnPath)
		{
			return;
		}
	}

	if (GetWorld()->WorldType != EWorldType::PIE)
	{
		PF_LOG_DIS(PF, "Spline change detected");
	}

	ConfigurePath();

	//this needs to be done for align in both cases - trajectory changed, location/rotation of path changed
	if (bAlignToPathStart && GetWorld()->WorldType != EWorldType::PIE)
	{
		InitCurrentDistance(SplineToFollow.Get());
	}

	AlignToCurrentDistance();
	LastCurrentDistance = CurrentDistanceOnPath;

	_LastTransform = SplineToFollow->GetComponentToWorld();
	
#if ENGINE_MINOR_VERSION < 13
	_LastSplineInfo = SplineToFollow->SplineInfo;
#else
	_LastSplineInfo = SplineToFollow->SplineCurves.Position;
#endif
	
	if (!TrajectoryEqual)	//Path trajectory was changed
	{
		if (bKeepRelativeDistances)
		{
			auto CurrLen = SplineToFollow->GetSplineLength();
			if (LastLength < 0)
			{
				LastLength = CurrLen;
			}
			else
			{
				auto Factor = CurrLen / LastLength;
				LastLength = CurrLen;
				auto& RotPoints = GetPathRoller().GetRollCurve().Points;
				for (auto& RotPoint : RotPoints)
				{
					RotPoint.InVal *= Factor;
				}
			}
		}

		if (bAutoRecompute)
		{
			UpdateAutoRotationPoints();
		}
	}
}

#endif

void UPFPathFollowerComponent::ConfigurePath()
{
	ENTER_EXIT_LOG;

	SplineToFollow = GetSplineToFollow();
	if (!SplineToFollow.IsValid())
	{
		PF_LOG_WARN(PF, "No path given");
		_pathDuration = 0;
		_currSplineLength = 0.f;
		return;
	}

	if (FMath::IsNearlyZero(SpeedDuration))
		_pathDuration = 0;
	else
		_pathDuration = MeasurePathDuration();


	SplineToFollow->Duration = _pathDuration;
	_currSplineLength = SplineToFollow->GetSplineLength();
}



void UPFPathFollowerComponent::ComputeAutoRotationPoints()
{
	ENTER_EXIT_LOG;
	if (auto* PathToFollow = GetSplineToFollow())
	{
		PF_LOG_DIS(PF, "Recomputing auto rotation points ...");
		auto& Roller = GetPathRoller();
		FPFRotationComputeContext Ctx = CreateRotationComputeContext();
		Roller.GenerateRollAngles(Ctx);
		Roller.Dump();
	}
}

FPFRotationComputeContext UPFPathFollowerComponent::CreateRotationComputeContext()
{
	FPFRotationComputeContext Ctx;
	Ctx.Spline = GetSplineToFollow();
	Ctx.RollStepsNum = RotationPointSteps;
	Ctx.RollSampleLength = RollSampleDistance;
	Ctx.ClampAngles = bClampGeneratedAngles;
	Ctx.MinAngle = RollAnglesClampMin;
	Ctx.MaxAngle = RollAnglesClampMax;
	Ctx.RollInterpType = RollIntepType;
	Ctx.ComputeOnSplineControlPoints = GenerateOnSplineControlPoints;

	return Ctx;
}

void UPFPathFollowerComponent::UpdateAutoRotationPoints()
{
	ENTER_EXIT_LOG;
	if (auto* PathToFollow = GetSplineToFollow())
	{
		PF_LOG_DIS(PF, "Recomputing auto rotation points ...");
		FPFRotationComputeContext Ctx = CreateRotationComputeContext();
		auto& Roller = GetPathRoller();
		Roller.UpdateRollAngles(Ctx);
		Roller.Dump();
	}
}

void UPFPathFollowerComponent::AlignToCurrentDistance()
{
	ENTER_EXIT_LOG;

	FollowPath(0.f);
}

void UPFPathFollowerComponent::HandleLoopingType()
{
	check(SplineToFollow.IsValid());


	switch (LoopType)
	{
	case EPFLoopType::Replay:
		CurrentDistanceOnPath = bReverse ? SplineToFollow->GetSplineLength() : 0;
		_ElapsedTime = DistanceToTime(CurrentDistanceOnPath);
		break;
	case EPFLoopType::ReplayFromStart:
		InitCurrentDistance(SplineToFollow.Get());
		_ElapsedTime = DistanceToTime(CurrentDistanceOnPath);
		break;
	case EPFLoopType::PingPong:
		Reverse(!bReverse);
		//necessary to avoid multiple trigger of ReachedStart/End events
		CurrentDistanceOnPath = bReverse ? SplineToFollow->GetSplineLength() - 0.01f : 0;
		_ElapsedTime = bReverse ? SplineToFollow->Duration - 0.01f : 0;
		break;
	default:
		break;
	}

	GetEventPoints().Reset(CurrentDistanceOnPath, bReverse, _LastPassedEventIndex);
}

void UPFPathFollowerComponent::ProcessEvents(float currDistance)
{
	GetEventPoints().ProcessEvents(currDistance, this, bReverse, _LastPassedEventIndex);
}

float UPFPathFollowerComponent::ModulateSpeed(float Speed) const
{
	return Modulate(Speed, SpeedFactorOperation, SpeedFactor);
}

FRotator UPFPathFollowerComponent::ModulateRotation(const FRotator & Rotation) const
{
	auto ModulatedEuler = Modulate(Rotation.Euler(), RotationFactorOperation, RotationFactor.Euler());
	return FRotator::MakeFromEuler(ModulatedEuler);
}

TStructOnScope<FActorComponentInstanceData> UPFPathFollowerComponent::GetComponentInstanceData() const
{
	ENTER_EXIT_LOG;
	
	return MakeStructOnScope<FActorComponentInstanceData, FPathFollowerInstanceData>(this);
}

FVector UPFPathFollowerComponent::ModifyFinalLocation_Implementation(const FVector& NewLocation)
{
	return NewLocation;
}

bool UPFPathFollowerComponent::HasPath() const
{
	return SplineToFollow.IsValid();
}

void UPFPathFollowerComponent::InitCurrentDistance(USplineComponent* Spline)
{
	check(Spline);
	float InitDistance = bReverse ? Spline->GetSplineLength() - StartDistance : StartDistance;
	CurrentDistanceOnPath = FMath::Clamp(InitDistance, 0.f, Spline->GetSplineLength());
}

const FPFPathRoller& UPFPathFollowerComponent::GetPathRoller() const
{
	return ((UPFPathFollowerComponent*)this)->GetPathRoller();
}

FPFPathRoller & UPFPathFollowerComponent::GetPathRoller()
{
	if (bOverrideRotationCurve)
	{
		return _PathRoller;
	}
	else
	{
		if (auto* path = GetPathToFollow())
		{
			return path->GetPathRoller();
		}
		else
		{
			return _PathRoller;
		}
	}
}

const FInterpCurveFloat& UPFPathFollowerComponent::GetSpeedCurve() const
{
	if (bOverridePathSpeedPoints)
	{
		return _InterpSpeedCurve;
	}
	else
	{
		if (auto* path = GetPathToFollow())
		{
			return path->GetSpeedCurve().GetCurve();
		}
		else
		{
			return _InterpSpeedCurve;
		}
	}
}

FInterpCurveFloat& UPFPathFollowerComponent::GetSpeedCurve()
{
	return const_cast<FInterpCurveFloat&>(static_cast<const UPFPathFollowerComponent*>(this)->GetSpeedCurve());
}

#if WITH_EDITOR
UObject* UPFPathFollowerComponent::GetSpeedDistribution() const
{
	ENTER_EXIT_LOG;
	auto* speedDistribution = NewObject<UDistributionFloatConstantCurve>((UPFPathFollowerComponent*)this, FName("SpeedCurveDistribution"));
	speedDistribution->ConstantCurve = GetSpeedCurve();
	return speedDistribution;
}


UObject* UPFPathFollowerComponent::GetRollAnglesDistribution() const
{
	ENTER_EXIT_LOG;
	auto* rollDistribution = NewObject<UDistributionVectorConstantCurve>((UPFPathFollowerComponent*)this, FName("RollAnglesCurveDistribution"));
	for (auto& point : GetPathRoller().GetRollCurve().Points)
	{
		int index = rollDistribution->ConstantCurve.AddPoint(point.InVal, point.OutVal);
		auto& DistPoint = rollDistribution->ConstantCurve.Points[index];
		DistPoint.ArriveTangent = point.ArriveTangent;
		DistPoint.LeaveTangent = point.LeaveTangent;
		DistPoint.InterpMode = point.InterpMode;
	}

	return rollDistribution;
}
#endif


float UPFPathFollowerComponent::DistanceToTime(float Distance)
{
	if (!FMath::IsNearlyZero(Distance) && Distance > 0.f)
	{
		return Distance / _currSplineLength * _pathDuration;
	}

	return 0.f;
}

UEventPointDelegateHolder* UPFPathFollowerComponent::GetEventPointDelegateByName(const FName& Name)
{
	ENTER_EXIT_LOG;
	return GetEventPoints().GetEventPointDelegateByName(Name);
}

UEventPointDelegateHolder* UPFPathFollowerComponent::GetEventPointDelegateByIndex(int32 Index)
{
	ENTER_EXIT_LOG;
	return GetEventPoints().GetEventPointDelegateByIndex(Index);
}

UEventPointDelegateHolder*  UPFPathFollowerComponent::GetEventPointDelegateAll()
{
	ENTER_EXIT_LOG;
	return GetEventPoints().GetEventPointDelegateAll();
}

FEventPoint& UPFPathFollowerComponent::GetEventPointByName(const FName& Name)
{
	ENTER_EXIT_LOG;
	auto& Points = GetEventPoints().Points;
	for (int i = 0; i < Points.Num(); ++i)
	{
		if (Points[i].Name == Name)
		{
			return Points[i];
		}
	}

	check(false && "No event point with name");
	return FEventPoint::Invalid;
}

bool UPFPathFollowerComponent::EventPointExistByName(const FName& Name)
{
	ENTER_EXIT_LOG;
	auto& Points = GetEventPoints().Points;
	for (int i = 0; i < Points.Num(); ++i)
	{
		if (Points[i].Name == Name)
		{
			return true;
		}
	}

	return false;
}



float UPFPathFollowerComponent::UpdateSpeed(float Distance)
{
	ENTER_EXIT_LOG;
	check(bUseSpeedCurve);
	float Speed = FPFSpeedCurve::GetSpeedAtDistance(GetSpeedCurve(), Distance);
	UE_LOG(PF, VeryVerbose, TEXT("newSpeed: %f"), Speed);
	Speed = Modulate(Speed, SpeedFactorOperation, SpeedFactor);
	return Speed;
}



float UPFPathFollowerComponent::GetSpeedAtDistance(float Distance) const
{
	float Speed = 0.f;
	if (bUseSpeedCurve && (GetSpeedCurve().Points.Num() > 0))
	{
		Speed = FPFSpeedCurve::GetSpeedAtDistance(GetSpeedCurve(), Distance);
		Speed = ModulateSpeed(Speed);
	}
	else
	{
		/*auto* Spline = GetSplineToFollow();
		if (!Spline)
		return 0;

		float len = Spline->GetSplineLength();
		float ease = 1.f;
		ease = Easing(FMath::Clamp(Distance / len, 0.f, 1.f));
		return Speed*ease;*/
		Speed = SpeedDuration;
	}


	return Speed;
}

float UPFPathFollowerComponent::GetSpeedAtSpeedPoint(int PointIndex) const
{
	float Speed = 0.f;

	auto& SpeedCurve = GetSpeedCurve();
	if (SpeedCurve.Points.IsValidIndex(PointIndex))
	{
		Speed = SpeedCurve.Points[PointIndex].OutVal;
		Speed = ModulateSpeed(Speed);
	}

	return Speed;
}

template<typename TValue, typename TFactor>
TValue UPFPathFollowerComponent::Modulate(TValue ValueToModulate, EPFFactorOperation ModulateOperation, const TFactor& ModulateFactor) const
{
	switch (ModulateOperation)
	{
	case EPFFactorOperation::None:
		break;
	case EPFFactorOperation::Add:
		ValueToModulate += ModulateFactor;
		break;
	case EPFFactorOperation::Mul:
		ValueToModulate *= ModulateFactor;
		break;
	};

	return ValueToModulate;
}

FRotator UPFPathFollowerComponent::GetRotationAtDistance(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return FRotator::ZeroRotator;

	FRotator Rotation;
	if (RollIntepType == EPFRollInterpType::LinearStable)
	{
		Rotation = GetPathRoller().GetRotationAtDistanceStableLinear(Distance, SplineToFollow.Get());
	}
	else
	{
		Rotation = GetPathRoller().GetRollRotationAtDistance(SplineToFollow.Get(), Distance, bUsePathPitchAndYaw);
	}

	if (CoordinateSpace == ESplineCoordinateSpace::World)
	{
		Rotation = (SplineToFollow->GetComponentQuat() * Rotation.Quaternion()).Rotator();
	}

	return Rotation;
}

FVector UPFPathFollowerComponent::GetLocationAtDistance(float Distance, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return FVector::ZeroVector;

	if (auto* PathComp = GetPathToFollow())
	{
		return PathComp->GetLocationAtDistanceAlongSplineMirrored(Distance, CoordinateSpace);
	}
	else
	{
		return SplineToFollow->GetLocationAtDistanceAlongSpline(Distance, CoordinateSpace);
	}
}

FVector UPFPathFollowerComponent::GetLocationAtInputKey(float Key, ESplineCoordinateSpace::Type CoordinateSpace) const
{
	GetSplineToFollow();
	if (!SplineToFollow.IsValid())
		return FVector::ZeroVector;

	if (auto* PathComp = Cast<UPFPathComponent>(SplineToFollow.Get()))
	{
		return PathComp->GetLocationAtSplineInputKeyMirrored(Key, CoordinateSpace);
	}
	else
	{
		return SplineToFollow->GetLocationAtSplineInputKey(Key, CoordinateSpace);
	}
}

void UPFPathFollowerComponent::SetCurrentDistance(float NewDistance)
{
	if (auto* Spline = GetSplineToFollow())
	{
		CurrentDistanceOnPath = FMath::Clamp(NewDistance, 0.f, Spline->GetSplineLength());
	}

	GetEventPoints().Reset(NewDistance, bReverse, _LastPassedEventIndex);

	if (!bStarted)
	{
		FollowPath(0.f);
	}
}

FVector UPFPathFollowerComponent::GetMoveDirection() const
{
	return _LastMoveDirection;
}

FRotator UPFPathFollowerComponent::ModifyFinalRotation_Implementation(const FRotator& NewRotation)
{
	return NewRotation;
}

void UPFPathFollowerComponent::UpdateLastMoveDirection(const FVector& NewLocation)
{
	if (NewLocation.Equals(_LastTargetLocation))
	{
		_LastMoveDirection = FVector::ZeroVector;
		return;
	}

	FVector LastLocationChange = NewLocation - _LastTargetLocation;
	_LastMoveDirection = LastLocationChange.GetSafeNormal();
}

USplineComponent* UPFPathFollowerComponent::GetSplineToFollow() const
{
	USplineComponent* Spline = nullptr;

	if (!SplineToFollow.IsValid())
	{
		PF_LOG_VERYVERB(PF, "SplineToFollow nullptr");
		if (pathOwner.IsValid())
		{
			PF_LOG_VERYVERB(PF, "Owner valid");
			Spline = Cast<USplineComponent>(pathOwner->GetComponentByClass(USplineComponent::StaticClass()));
			if (Spline)
			{
				PF_LOG_VERYVERB(PF, "Got spline from actor");
				SplineToFollow = Spline;
			}
			else
			{
				PF_LOG_VERYVERB(PF, "No spline found in actor");
			}
		}
		else
		{
			FString OwnerName;
			if (GetOwner())
			{
				OwnerName = GetOwner()->GetName();
			}
			else
			{
				OwnerName.Empty();
			}
			//UE_LOG(PF, Warning, TEXT("Owner nullptr && SplineToFollow nullptr. Owner: %s"), *OwnerName);
		}
	}
	else
	{
		PF_LOG_VERYVERB(PF, "SplineToFollow is valid");
		Spline = SplineToFollow.Get();
	}

	if (Spline)
	{
		if (auto* Path = Cast<UPFPathComponent>(Spline))
		{
			Path->_FollowerComponent = this;
		}
	}

	return Spline;
}

void UPFPathFollowerComponent::Activate(bool bReset /*= false*/)
{
	Super::Activate(bReset);

	if (bReset)
	{
		Init();
	}

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UPFPathFollowerComponent::Deactivate()
{
	Super::Deactivate();

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
}


FPathFollowerInstanceData::FPathFollowerInstanceData(const UPFPathFollowerComponent* SourceComponent)
	: FActorComponentInstanceData(SourceComponent)
{
	PathRoller = SourceComponent->_PathRoller;
	SpeedCurve = SourceComponent->_InterpSpeedCurve;
	EventPoints = SourceComponent->EventPoints;
#if WITH_EDITORONLY_DATA
	bAlwaysOpenRollCurveEditor = SourceComponent->bAlwaysOpenRollCurveEditor;
	_LastInfo = SourceComponent->_LastSplineInfo;
#endif
}

void FPathFollowerInstanceData::ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	FActorComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);
	auto* Follower = CastChecked<UPFPathFollowerComponent>(Component);

	ENTER_EXIT_LOG;
	Follower->_PathRoller = PathRoller;
	Follower->_InterpSpeedCurve = SpeedCurve;
	Follower->EventPoints = EventPoints;
#if WITH_EDITORONLY_DATA
	Follower->_LastSplineInfo = _LastInfo;
	Follower->bAlwaysOpenRollCurveEditor = bAlwaysOpenRollCurveEditor;
#endif
}