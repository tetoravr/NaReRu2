// Copyright 2015-2016 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "PFRollInterpType.h"
#include "PFPathRoller.h"
#include "PFSpeedCurve.h"
#include "PFEventPoints.h"
#include "Classes/PFAutoRollVisualConfig.h"
#include "Classes/Easing.h"
#include "PFPathFollowerComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PF, Verbose, Verbose);

class EnterExitLog
{
public:
	EnterExitLog(const char* func)
	{
		_func = func;
		UE_LOG(PF, VeryVerbose, TEXT("[ENTER] %s"), *_func);
	}

	~EnterExitLog()
	{
		UE_LOG(PF, VeryVerbose, TEXT("[EXIT] %s"), *_func);
	}

private:
	FString _func;
};
#define ENTER_EXIT_LOG EnterExitLog _logger(__FUNCTION__);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FReachedEndSignature, class UPFPathFollowerComponent*, followerComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FReachedStartSignature, class UPFPathFollowerComponent*, followerComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartPathSignature, class UPFPathFollowerComponent*, followerComp);


/** Looping type for PathFollower component.*/
UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPFLoopType : uint8
{
	Replay UMETA(DisplayName = "Replay"),
	ReplayFromStart UMETA(DisplayName = "Replay From Start Distance"),
	PingPong UMETA(DisplayName = "Ping Pong")
};

/** Speed or Rotation factor operation */
UENUM(BlueprintType)
enum class EPFFactorOperation : uint8
{
	None UMETA(DisplayName = "None"),
	Add UMETA(DisplayName = "Add"),
	Mul UMETA(DisplayName = "Multiply")
};



/*
* Allows to follow location, rotation and automatic rolling on a spline.
* Supports number of easing and looping types.
* One can also specify look at scene component so that follower will always rotate towards that component.
* Just add to an actor and set PathOwner actor with spline component (first will be used if there is more than one).
*/
UCLASS(BlueprintType, Blueprintable, ClassGroup = (PF), meta = (BlueprintSpawnableComponent))
class PATHFOLLOW_API UPFPathFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = PathFollow)
	FReachedEndSignature OnReachedEnd;

	/* Fire when we reached start point - could happen in looping scenarios or bReverse is true
	*/
	UPROPERTY(BlueprintAssignable, Category = PathFollow)
	FReachedStartSignature OnReachedStart;	//Tony; Bug Fix, each should have unique signature.

	/** Ent fired everytime we start following a path. */
	UPROPERTY(BlueprintAssignable, Category = PathFollow)
	FStartPathSignature OnStartPath;
	
public:
	UPFPathFollowerComponent();

	/** Starts following when game begins if path is set and bStartAtPlay is true */
	virtual void BeginPlay() override;

	const FPFEventPoints& GetEventPoints() const;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	FPFEventPoints& GetEventPoints();

	const class UPFPathComponent* GetPathToFollow() const;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	class UPFPathComponent* GetPathToFollow();

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void SetPathToFollow(class USplineComponent* Spline);

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	bool HasPath() const;

	void InitializeComponent() override;
	void PostLoad() override;

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* Main simulation routine.
	* Process event points, update location, rotation according to speed settings, etc.
	* Could be called manually to gain control over simulation.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void FollowPath(float FollowStep);

	/**
	* Start following a path.
	* @note Helper for delayed start. Calls Start implementation after specified delay in seconds.
	* @param StartDelay delay amount in seconds.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void Start(float StartDelay = 0);


	/**
	* Pause following. Following could be resumed using Start().
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void Pause();

	/**
	* Stop following and rewind.
	* @note Does not affects current location/rotation.
	* @note starts from beginning on next call to Start()
	* @note if bLookAtEvenIfNotStarted is true Stop() call will not stop LookAt rotation
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void Stop();

	/**
	* Set an actor which has SplineComponent.
	* @note First spline will be used if there is more than one.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void SetPathOwner(class AActor* PathOwner);

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	class AActor* GetPathOwner();
	
private:
	/**
	* Actor that owns spline component to follow.
	* To chnage path during runtime use SetPathOwner() method.
	* You can choose which path of pathOwner to follow (there could be more than one path in pathOwner) 
	* via combo box which is shown once pathOwner is set
	*/
	UPROPERTY(EditAnywhere, Category = PathFollow, meta = (DisplayName = "Path Owner"))
	TWeakObjectPtr<AActor> pathOwner;

public:
	/** If true no overlap events are fired during follower move */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bTeleportPhysics = false;

	/**
	* Time interval between each simulation tick
	* Value of zero (0.f) mean no tick interval applied i.e. tick every frame
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	float TickInterval = 0.f;

	/**
	* Show/hide path control points info text - location, time, etc.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	bool bHidePathInfoText = false;

	/**
	* Speed of location following
	* @note Speed could be changed during runtime if not using the speed points feature
	* @see bUseSpeedCurve
	* @note name of the member not changed directly becaue of backward compatibility
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (DisplayName = "Speed", EditCondition = "!bUseSpeedCurve"))
	float SpeedDuration = 50;

	/**
	* If true a Speed point defines time and distance. The Speed point says: at this time be at location which is distance along a path.
	* This allows for full and smooth stop on a path during following.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	bool TimeBased = false;

	/**
	* If true rotation follows directly rotation defined by path/spline followed
	* I.e. generated/added rotation points are NOT used
	* Uncheck 'Use Rotation Curve' to edit this property (only one of 'Follow Path Rotation' and 'Use Rotation Curve' could be checked at any time)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (DisplayName = "Follow Path Rotation", EditCondition = "!bUseRotationCurve"))
	bool bFollowRotation = true;

	/**
	* Should follow speed defined by speed curve?
	* @note This disables use of easing. Speed property is ignored if true.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	bool bUseSpeedCurve = false;

	/**
	* Speed points are stored on a path by default.
	* Check this to override speed points of the path.
	* Follower component speed points will be used instead.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	bool bOverridePathSpeedPoints = false;

	/**
	* Speed Curve modulation operation. Add or Multiply currently supported.
	* Speed Values defined by speed points are modulated by SpeedFactor.
	* Check Use Speed Curve to edit the value.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (EditCondition = "bUseSpeedCurve"))
	EPFFactorOperation SpeedFactorOperation = EPFFactorOperation::None;

	/**
	* Value to be used with specified modulation operation (Speed Factor Operation)
	* Check Use Speed Curve to edit the value.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (EditCondition = "bUseSpeedCurve"))
	float SpeedFactor = 1.f;

	//DEPRECATED, ONLY FOR BACKWARD COMPATIBILITY
	UPROPERTY(meta = (DeprecatedProperty))
	FPFSpeedCurve _SpeedCurve;

	/**
	* Holds speed curve points.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (DisplayName = Curve, EditCondition = bOverridePathSpeedPoints))
	FInterpCurveFloat _InterpSpeedCurve;

	/**
	* Easing of movement. 
	* @note Use only if bUseSpeedCurve is false.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (EditCondition = "!bUseSpeedCurve"))
	EEasingType EasingType = EEasingType::Linear;

	/**
	* Curve exponent. Used if EasingType is EaseIn/Out/InOut.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Speed, meta = (EditCondition = "!bUseSpeedCurve"))
	float CurveDegree = 2;

	/**
	* Turns looping on/off. Useful also for debugging in simulation mode.
	* @see ELoopType
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bLoop = false;

	/**
	* Looping type. bLoop must be set.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	EPFLoopType LoopType = EPFLoopType::Replay;

	/**
	* Should we start following at begin play?
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bStartAtPlay = true;

	/**
	* Delay first start in seconds
	* Used only on first time start if bStartAtPlay is checked
	* i.e. all other calls to Start() method will use it's StartDelay param
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow, meta = ( EditCondition = "bStartAtPlay"))
	float StartDelay = 0;

	/**
	* From where to start from following.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow, meta = (UIMin = 0, ClampMin = 0))
	float StartDistance = 0.f;

	/**
	* Reverse following i.e. start from the other end of path.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	bool bReverse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bInvertRotationOnReverse = false;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void Reverse(bool Reverse);

	/**
	* Rotation is computed from spline tangent on current location.
	* This says: set rotation as if we're in: location = (current location) + (LookAhead distance along a spline/path)
	* Useful e.g. for water ships, rally car steering
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float LookAhead = 0;

	/**
	* Turns on Rotation curve feature.
	* Rotation is precomputed in editor.
	* Stored rotations are interpolated (according to interpolation mode) during runtime.
	* @see PFRollIntepType
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = "!bFollowRotation"))
	bool bUseRotationCurve = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = "bUseRotationCurve"))
	bool bUsePathPitchAndYaw = false;

	/**
	* Recompute rotation curve points if path changes.
	* Does NOT change rotation points distance on a path nor rotation points count. I.e. just recompute/update rotation at each (already defined) point.
	* Warning! All manual changes made to rotation points will be lost
	*/
	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool bAutoRecompute = false;

	/**
	* Keep relative distances of rotation points when recomputing on path changes.
	*/
	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool bKeepRelativeDistances = false;

	/** Sampling rate for rotation curve. RotationPointSteps+1 rotation points will be generated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (EditCondition = "!GenerateOnSplineControlPoints"))
	int32 RotationPointSteps = 10;

	/**
	* Generate rotation points on spline's controls points location.
	* If true RotationPointSteps number is ignored
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool GenerateOnSplineControlPoints = false;

	/** 
	* Distance between two points on a spline which will be used to compute one roll angle. 
	* -1 for automatic setup 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float RollSampleDistance = -1;

	/**
	* Generated roll angles will be clamped to desired angles range if enabled.
	* See RollAnglesClampMax
	* See RollAnglesClampMin
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	bool bClampGeneratedAngles = false;

	/**
	* Generated roll angles are clamped to fit with range of RollAnglesClampMax and RollAnglesClampMin.
	* ClampGeneratedAngles must be set. Ignored otherwise.
	* Roll angles greater than or equal zero will be scaled to range [0, RollAnglesClampMax].
	* This is to maintain original direction in reference (relative) to spline default up vector.	
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float RollAnglesClampMax = 90;

	/**
	* Generated roll angles are clamped to fit with range of RollAnglesClampMax and RollAnglesClampMin.
	* ClampGeneratedAngles must be set. Ignored otherwise.
	* Roll angles less than zero will be scaled to range [RollAnglesClampMin, 0].
	* This is to maintain original direction in reference (relative) to spline default up vector.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float RollAnglesClampMin = -90;

	/**
	* Rotation curve interpolation type.
	* CubicClamped provides smooth rotation across curve.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (DisplayName = "Rotation Interpolation Mode"))
	EPFRollInterpType RollIntepType = EPFRollInterpType::CubicClamped;

	/**
	* Speed Curve modulation operation. Add or Multiply currently supported.
	* Speed Values defined by speed points are modulated by SpeedFactor.
	* Check Use Speed Curve to edit the value.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	EPFFactorOperation RotationFactorOperation = EPFFactorOperation::None;

	/**
	* Value to be used with specified modulation operation (Speed Factor Operation)
	* Check Use Speed Curve to edit the value.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FRotator RotationFactor = FRotator(1.f);

	/** Rotation mask will be applied in local space. I.e. in space independent on whole path rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow, meta = (DisplayName = "Apply Rotation Mask in Local Space"))
	bool RotationMaskLocal = true;

	/**
	* Specify which rotation axis to update in world space.
	* Set vector components only to 0 or 1 for correct behavior.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	FIntVector RotationUpdateMask = FIntVector(1, 1, 1);

	/**
	* Specify which location axis to update in world space.
	* Set vector components only to 0 or 1 for correct behavior.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	FIntVector LocationUpdateMask = FIntVector(1, 1, 1);

	/**
	* Rotation points are stored on a path by default.
	* Check this to override rotation points of the path.
	* Follower component rotation points will be used instead.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	bool bOverrideRotationCurve = false;

	/**
	* Per follower instance rotation curve points.
	* Check 'Override Rotation Curve' to enable.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = bOverrideRotationCurve, DisplayName = "Rotation Points"))
	FPFPathRoller _PathRoller;

	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (DisplayName = "Visualization"))
	FPFAutoRollVisualConfig AutoRollDebugConfig;
#endif
	    
	/**
	* Component which location is changing as we follow a path.
	* Could be freely changed during runtime.
	*/
	UPROPERTY(BlueprintReadWrite, Category = PathFollow)
	class USceneComponent* LocationComponent = nullptr;

	/**
	* Component which rotation is changing as we follow a path.
	* Could be freely changed during runtime.
	*/
	UPROPERTY(BlueprintReadWrite, Category = PathFollow)
	class USceneComponent* RotationComponent = nullptr;

	/**
	* Component which rolls as we follow a path.
	* Only Roll angle is set. Rotation of this component is set in local space (i.e. relative to it's parent).
	*/
	UPROPERTY(BlueprintReadWrite, Category = PathFollow)
	class USceneComponent* RollComponent = nullptr;

	/**
	* Point/look at to a target
	* If set (not nullptr) then bFollowRotation and bAutoRoll is ignored.
	  * Could be freely changed during runtime.
	*/
	UPROPERTY(BlueprintReadWrite, Category = PathFollow)
	class USceneComponent* LookAtComponent = nullptr;

	/**
	* Normally LookAtComponent tracking is updated only when we Start() follow.
	* If this is true than LookAtComponent tracking gets updated whether or not we're started actual following of a path.
	* This covers the case where we want to update look at rotation before we start to follow path location (i.e. LookAtComponent is already moving)
	* or after we reached end of path (i.e. LookAtComponent is still moving).
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bLookAtEvenIfNotStarted = false;

	  /** True if following was started and its following or paused (have not reached end in case looping is off or Stop() was called)  */
	UPROPERTY(BlueprintReadOnly, Category = PathFollow)
	bool bStarted = false;

	/**
	* True if Pause() was called
	* Call Start() to unpause
	*/
	UPROPERTY(BlueprintReadOnly, Category = PathFollow)
	bool bPause = false;

	/**
	* If true align follower to start location/rotation
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PathFollow)
	bool bAlignToPathStart = true;

	/**
	* Retrieve event delegate holder wchich than could be used to execute script when passing the event point
	* Nullptr if event point with this name does not exist.
	* If OverrideEventPoints is true follower component event points are searched. If false path's event points are searched.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	UEventPointDelegateHolder* GetEventPointDelegateByName(const FName& Name);

	/**
	* Retrieve event delegate holder which than could be used to execute script when passing the event point
	* If OverrideEventPoints is true follower component event points are used. If false path's event points are used.
	* Returns nullptr if index is not valid
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	UEventPointDelegateHolder* GetEventPointDelegateByIndex(int32 Index);

	/**
	* Retrieve event delegate holder which than could be used to execute script when passing every event point.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	UEventPointDelegateHolder* GetEventPointDelegateAll();


	/**
	* Retrieve event point by name.
	* If OverrideEventPoints is true follower component event points are searched. If false path's event points are searched.
	* Warning! If event point is not found assertion is triggered. Invalid (empty) event point is returned in shipping builds.
	* Please use EventPointExistByName() to check whether the point exists.
	*/
	UFUNCTION(BlueprintCallable, Category = Events)
	FEventPoint& GetEventPointByName(const FName& Name);

	/*
	* Checks whether event point with specified name exists in currently used event points curve (either path's or follower component)
	* If OverrideEventPoints is true follower component event points are searched. If false path's event points are searched.
	*/
	UFUNCTION(BlueprintCallable, Category = Events)
	bool EventPointExistByName(const FName& Name);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Speed, meta = (DisplayName = "Visualization"))
	FPFSpeedPointsDrawConfig SpeedPointDrawConfig;

	UPROPERTY(EditAnywhere, Category = Events, meta = (DisplayName = "Visualization"))
	FPFEventPointsVisualization EventPointsVisualization;
#endif

	FVector GetLocationAtInputKey(float Key, ESplineCoordinateSpace::Type CoordinateSpace) const;

	/*
	* Get speed a follower will have at specified distance along a path
	* Returns final speed including possible modulation.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
  float GetSpeedAtDistance(float Distance) const;

	/*
	* Get speed a follower will have at specified speed point.
	* Returns final speed including possible modulation. Zero if no speed points are defined or index is no valid.
	*/
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	float GetSpeedAtSpeedPoint(int PointIndex) const;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	FRotator GetRotationAtDistance(float Distance, ESplineCoordinateSpace::Type Coord) const;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	FVector GetLocationAtDistance(float Distance, ESplineCoordinateSpace::Type Coord) const;

	UFUNCTION(BlueprintCallable, Category = PathFollow)
	void SetCurrentDistance(float NewDistance);

	UPROPERTY(BlueprintReadOnly, Interp, Category = PathFollow)
	float CurrentDistanceOnPath = 0;


	/** If true follower component event points are used in simulation instead of path's event points.	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Events)
	bool bOverridePathEventPoints = false;

	/** Follower component event points. Used in simulation if OverridePathEventPoints is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Events, meta = (EditCondition = bOverridePathEventPoints))
	FPFEventPoints EventPoints;

	/** Returns current normalized vector representing last location change direction */
	UFUNCTION(BlueprintCallable, Category = PathFollow)
	FVector GetMoveDirection() const;

	UFUNCTION(BlueprintNativeEvent, Category = PathFollow)
	FVector ModifyFinalLocation(const FVector& NewLocation);

	UFUNCTION(BlueprintNativeEvent, Category = PathFollow)
	FRotator ModifyFinalRotation(const FRotator& NewRotation);

	UFUNCTION(BlueprintNativeEvent, Category = PathFollow)
	FRotator ComputeLookAtRotation(USceneComponent* TargetComponent, const FVector& FollowerLocation);

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UObject* GetRollAnglesDistribution() const;

	UObject* GetSpeedDistribution() const;

	void FixAutoRollAndFollowRotationClash();

	void InformPathAboutSelection();

	void MonitorSpline();
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	bool bAlwaysOpenRollCurveEditor = false;
#endif

	UFUNCTION()
	void ComputeAutoRotationPoints();

	UFUNCTION()
	void UpdateAutoRotationPoints();

	/**
	 * Get currently used rotation curve
	 * If OverrideRotationCurve is true follower component rotation curve is returned. Path's rotation curve is returned otherwise.
	 */
	const FPFPathRoller& GetPathRoller() const;
	FPFPathRoller& GetPathRoller();

	/**
	* Get currently used speed curve
	* If OverrideSpeedCurve is true follower component speed curve is returned. Path's speed curve is returned otherwise.
	*/
	const FInterpCurveFloat& GetSpeedCurve() const;
	FInterpCurveFloat& GetSpeedCurve();

	/**
	* Retrieve currently followed spline
	* Nullptr if SplineToFollow is nullptr and there is no PathOwner with at least one spline component set.
	*/
	USplineComponent* GetSplineToFollow() const;

protected:
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

private:
	void Init();

	float MeasurePathDuration() const;

	void InitCurrentDistance(USplineComponent* Spline);

	void UpdateCurrentDistanceOnPath(float DeltaTime);

	void SetupUpdatedComponents();

	void HandleEndOfPath();

	UFUNCTION()
	void StartImpl();

	float DistanceToTime(float Distance);

	void ConfigurePath();

	FVector ComputeNewLocation(float currDistance);

	FRotator ComputeNewRotation(float currDistance);

	void UpdateLastMoveDirection(const FVector& NewLocation);

	void AlignToCurrentDistance();

	void LookAt();

	void HandleLoopingType();

	void ProcessEvents(float currDistance);

	float UpdateSpeed(float Distance);

	ETeleportType GetTeleportType() const;

	FORCEINLINE FVector MaskLocation(const FVector& CurrentLocation, const FVector& ComputedNewLocation);
	
	FORCEINLINE FRotator MaskRotation(const FRotator& CurrentRotation, const FRotator& ComputedNewRotation);
	
	FRotator MaskRotation(const FRotator & Rotation);

	FPFRotationComputeContext CreateRotationComputeContext();

	template<typename TValue, typename TFactor>
	TValue Modulate(TValue ValueToModulate, EPFFactorOperation ModulateOperation, const TFactor& ModulateFactor) const;
	
	float ModulateSpeed(float Speed) const;
	FRotator ModulateRotation(const FRotator& Rotation) const;

	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;

	/** Spline we're currently following. */
	UPROPERTY()
	mutable TWeakObjectPtr<class USplineComponent> SplineToFollow;

	UPROPERTY()
	float _pathDuration = 0;

	UPROPERTY()
	float _currSplineLength = 0.f;

	UPROPERTY()
	float _ElapsedTime = 0;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FInterpCurveVector _LastSplineInfo;

	UPROPERTY()
	FTransform _LastTransform = FTransform::Identity;

	UPROPERTY()
	float LastLength = -1;

	UPROPERTY()
	float LastCurrentDistance = -1;
#endif

	UPROPERTY()
	FTimerHandle _DelayTimer;

	UPROPERTY()
	float _lastTickTime = 0.f;

	FVector _LastTargetLocation = FVector::ZeroVector;
	
	FVector _LastMoveDirection = FVector::ZeroVector;

	UPROPERTY()
	int32 _LastPassedEventIndex = -1;

	friend struct FPathFollowerInstanceData;
};

USTRUCT()
struct FPathFollowerInstanceData : public FActorComponentInstanceData
{
	GENERATED_BODY()
public:
	FPathFollowerInstanceData() = default;
	explicit FPathFollowerInstanceData(const UPFPathFollowerComponent* SourceComponent);

	virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;


	FPFPathRoller PathRoller;
	FInterpCurveFloat SpeedCurve;
	FPFEventPoints EventPoints;
#if WITH_EDITORONLY_DATA
	bool bAlwaysOpenRollCurveEditor;
	FInterpCurveVector _LastInfo;
#endif
};