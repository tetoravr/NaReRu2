// Copyright 2015-2016 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "PFEventPoints.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPFEventPoints, Display, All);

/**
* Event firing mode
*/
UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPFEventFireMode : uint8
{
	Forward UMETA(DisplayName = "Forward", ToolTip = "Only if we're following on forward direction i.e. bReverse is false"),
	Reverse UMETA(DisplayName = "Reverse", ToolTip = "bReverse is true"),
	Always UMETA(DisplayName = "Always", ToolTip = "Fire the event while traveling in any direction")

};

USTRUCT(BlueprintType)
struct FEventPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	float Distance = 0;

	/**
	 * Any custom class with user data.
	 * Default object of this class will be passed into event handelers when passing this event point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	TSubclassOf<UObject> UserData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	EPFEventFireMode FireMode = EPFEventFireMode::Always;

	/**
	* The event will be fired at most FireCount times.
	* -1 means always
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PathFollow)
	int32 FireCount = -1;

	UPROPERTY(BlueprintReadOnly, Category = PathFollow)
	int32 Index = -1;

	static FEventPoint Invalid;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEventPointReachedSignature, class UPFPathFollowerComponent*, FollowerComp, float, Distance, UObject*, ExtraData);

UCLASS()
class PATHFOLLOW_API UEventPointDelegateHolder : public UObject
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable, Category = PathFollow)
	FEventPointReachedSignature OnEventPointReached;
};

USTRUCT(BlueprintType)
struct PATHFOLLOW_API FPFEventPointsVisualization
{
	GENERATED_BODY()

	FPFEventPointsVisualization()
	{
		_EventPointSpriteTexture = LoadObject<UTexture2D>(NULL, TEXT("/PathFollow/event_sprite.event_sprite"));
	}

	UPROPERTY(EditAnywhere, Category = "Events")
	bool bHideEventPoints = false;

	UPROPERTY(EditAnywhere, Category = "Events")
	bool bHideEventPointInfoText = false;

	UPROPERTY(EditAnywhere, Category = "Events")
	FColor EventPointsColor = FColor::Blue;

	

	UPROPERTY(EditAnywhere, Category = "Events")
	float EventPointHitProxySize = 20.f;

	UPROPERTY(EditAnywhere, Category = "Events")
	UTexture2D* _EventPointSpriteTexture = nullptr;
};


USTRUCT(BlueprintType)
struct PATHFOLLOW_API FPFEventPoints
{
	GENERATED_BODY()

	FPFEventPoints()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Events)
	TArray<FEventPoint> Points;

	void Init();
	void Reset(float CurrentDistanceOnPath, bool bReverse, int32& LastPassedEventIndex);
	void ProcessEvents(float CurrentDistance, class UPFPathFollowerComponent* Follower, bool bReverse, int32& LastPassedEventIndex);
	UEventPointDelegateHolder* GetEventPointDelegateByName(const FName& Name);
	UEventPointDelegateHolder* GetEventPointDelegateByIndex(int32 Index);
	//Returns holder which is executed when passing any event point
	//Could be nullptr if there are no event points defined
	UEventPointDelegateHolder* GetEventPointDelegateAll() const;
	

private:
	int32 FindPassedEventPointIndex(float CurrentDistance, bool bReverse);
	void BroadcastEventPointReached(FEventPoint& EventPoint, class UPFPathFollowerComponent* Follower);

	void SortPointsByDistance();
	
	TArray<FEventPoint> DistanceSorted;

	UPROPERTY()
	UEventPointDelegateHolder* AllEventHolder = nullptr;

	UPROPERTY()
	TArray<UEventPointDelegateHolder*> Holders;
};
