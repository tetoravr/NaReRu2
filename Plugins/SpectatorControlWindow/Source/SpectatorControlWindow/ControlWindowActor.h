// Copyright 2016-2018 dfab BV, Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ControlWindowActor.generated.h"

class UControlWindow;

UCLASS()
class AControlWindowActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AControlWindowActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called whenever this actor is being removed from a level
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Set a property to access it from a Blueprint
    UPROPERTY(BlueprintReadOnly, Category = "ControlWindow")
    UControlWindow* ControlWindow;

    /** Reference to blueprint class*/
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "ControlWindow")
	TSubclassOf<UControlWindow> ControlWindowWidget;

private:
    APlayerController* PlayerController;
};
