// Copyright 2016-2018 dfab BV, Rights Reserved.

#include "ControlWindowActor.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
//#include "SlateBasics.h"
#include "ControlWindow.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
AControlWindowActor::AControlWindowActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AControlWindowActor::BeginPlay()
{
	Super::BeginPlay();
	
    // If a Control Window Widget Blueprint is set create the ControlWindowWidget
    if(ControlWindowWidget != nullptr)
    {
        ControlWindow = CreateWidget<UControlWindow>(GetWorld(), ControlWindowWidget);
    }

    // If a ControlWindowWidget is created, call the CreateControlWindow to setup the Slate window viewport
    if(ControlWindow !=nullptr )
    {
        ControlWindow->CreateControlWindow();
    }

    // Set reference to player controller
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    PlayerController->bShowMouseCursor = true;

}

void AControlWindowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Destroy Window, only if widget exists
	if (ControlWindow != nullptr)
	{
		ControlWindow->DestroyControlWindow();
	}

	Super::EndPlay(EndPlayReason);
	
}
