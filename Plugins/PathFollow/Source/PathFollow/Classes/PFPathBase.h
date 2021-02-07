// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PFPathBase.generated.h"

UCLASS()
class PATHFOLLOW_API APFPathBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APFPathBase();

  UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=PathFollow)
  class UPFPathComponent* PathToFollow;
};
