// Copyright 2015-2016 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#include "PFEventPoints.h"
#include "Components/SplineComponent.h"
#include "Components/PFPathFollowerComponent.h"
#include "Distributions/DistributionFloatConstant.h"

DEFINE_LOG_CATEGORY(LogPFEventPoints);

FEventPoint FEventPoint::Invalid = FEventPoint();

void FPFEventPoints::Init()
{
	auto EventPointsNum = Points.Num();
	if (EventPointsNum == 0)
		return;

	AllEventHolder = NewObject<UEventPointDelegateHolder>();

	Holders.Reset(EventPointsNum);
	for (int i = 0; i < EventPointsNum; ++i)
	{
		Points[i].Index = i;
		auto* Holder = NewObject<UEventPointDelegateHolder>();
		Holders.Add(Holder);
	}

	SortPointsByDistance();

	for (auto& EventPoint : DistanceSorted)
	{
		UE_LOG(LogPFEventPoints, Verbose, TEXT("distance: %f"), EventPoint.Distance);
	}
}

void FPFEventPoints::SortPointsByDistance()
{
	DistanceSorted = Points;

	DistanceSorted.Sort([](const FEventPoint& max, const FEventPoint& item)
	{
		return item.Distance > max.Distance;
	});
}

UEventPointDelegateHolder* FPFEventPoints::GetEventPointDelegateByName(const FName& Name)
{
	for (int i = 0; i < Points.Num(); ++i)
	{
		if (Points[i].Name == Name)
		{
			check(i < Holders.Num() && Holders[i] != nullptr);
			return Holders[i];
		}
	}

	return nullptr;
}

UEventPointDelegateHolder* FPFEventPoints::GetEventPointDelegateByIndex(int32 Index)
{
	if (!Points.IsValidIndex(Index))
		return nullptr;
	
	check(Holders.IsValidIndex(Index) && Holders[Index] != nullptr);
	return Holders[Index];
}

UEventPointDelegateHolder* FPFEventPoints::GetEventPointDelegateAll() const
{
	return AllEventHolder;
}

void FPFEventPoints::Reset(float CurrentDistanceOnPath, bool bReverse, int32& LastPassedEventIndex)
{
	LastPassedEventIndex = FindPassedEventPointIndex(CurrentDistanceOnPath, bReverse);
}

int32 FPFEventPoints::FindPassedEventPointIndex(float CurrentDistance, bool bReverse)
{
	if (!bReverse)
	{
		for (int i = DistanceSorted.Num() - 1; i >= 0; --i)
		{
			auto& EventPoint = DistanceSorted[i];
			auto pointDistance = EventPoint.Distance;
			if (CurrentDistance > pointDistance)
			{
				return i;
			}
		}
	}
	else
	{
		for (int i = 0; i < DistanceSorted.Num(); ++i)
		{
			auto pointDistance = DistanceSorted[i].Distance;
			if (CurrentDistance < pointDistance)
			{
				return i;
			}
		}
	}

	return -1;
}


bool CanBroadcastEventPoint(EPFEventFireMode FireMode, bool bReverse)
{
	bool FireAlways = FireMode == EPFEventFireMode::Always;
	bool FireInReverse = bReverse && FireMode == EPFEventFireMode::Reverse;
	bool FireForward = !bReverse && FireMode == EPFEventFireMode::Forward;

	return FireAlways || FireForward || FireInReverse;
}


void FPFEventPoints::ProcessEvents(float CurrentDistance, class UPFPathFollowerComponent* Follower, bool bReverse, int32& LastPassedEventIndex)
{
	int IndexPassed = FindPassedEventPointIndex(CurrentDistance, bReverse);
	if (IndexPassed != LastPassedEventIndex  && IndexPassed != -1)
	{
		LastPassedEventIndex = IndexPassed;

		auto& EventPoint = DistanceSorted[IndexPassed];
		UE_LOG(PF, Verbose, TEXT("Reached event point: %s"), *EventPoint.Name.ToString());

		if (CanBroadcastEventPoint(EventPoint.FireMode, bReverse))
		{
			BroadcastEventPointReached(EventPoint, Follower);
		}
	}

}

void FPFEventPoints::BroadcastEventPointReached(FEventPoint& EventPoint, UPFPathFollowerComponent* Follower)
{
	if (EventPoint.FireCount == 0)
		return;
	if (EventPoint.FireCount > 0)
		--EventPoint.FireCount;

	int32 PointIdx = EventPoint.Index;
	check(PointIdx < Holders.Num() && Holders[PointIdx] != nullptr);
	auto* holder = Holders[PointIdx];
	if (holder->OnEventPointReached.IsBound())
		holder->OnEventPointReached.Broadcast(Follower, EventPoint.Distance, EventPoint.UserData.GetDefaultObject());

	if (AllEventHolder != nullptr)
	{
		if (AllEventHolder->OnEventPointReached.IsBound())
			AllEventHolder->OnEventPointReached.Broadcast(Follower, EventPoint.Distance, EventPoint.UserData.GetDefaultObject());
	}
}