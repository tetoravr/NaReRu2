// Copyright 2016-2018 dfab BV, Rights Reserved.

#include "ControlWindow.h"
#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Slate/SceneViewport.h"
#include "Slate/SGameLayerManager.h"
#include "Runtime/Slate/Public/Widgets/Layout/SConstraintCanvas.h"

// Contstructor, Initialize ControlWindow
UControlWindow::UControlWindow( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
    WindowMode = EWindowMode::Windowed;

    SetVisibility(ESlateVisibility::Visible);

    //Init window properties
    ControlWindowTitle = FText::FromString ( TEXT( "Control Window" ) );
    ControlWindowSize = FVector2D(300, 400);
    ControlWindowPosition = FVector2D( 0 ,0 );
}

void UControlWindow::RebuildControlWindow()
{
    RebuildWidget();
}

void UControlWindow::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    ControlWindow.Reset();
}

//Creat Control Window
bool UControlWindow::CreateControlWindow()
{
    bControlWindowCreated = false;
#if WITH_EDITOR
    bIsEditor = true;
#endif

        // Create and register a new Slate window
        CreateSlateWindow();

        // Get reference to Slate application
        FSlateApplication& SlateApplication = FSlateApplication::Get();

        // Add a new seperate window to the SlateApplication
        SlateApplication.AddWindow( ControlWindow.ToSharedRef(), true );

        TSharedPtr<SWidget> UserSlateWidget = TakeWidget();

        ControlWindow->SetContent(UserSlateWidget.ToSharedRef());
        ControlWindow->ShowWindow();

        ControlWindow->SetOnWindowClosed( FOnWindowClosed::CreateUObject( this, &UControlWindow::OnControlWindowClosed ) );

        // Set game Type
        SetIsStandAloneGame(GetWorld()->WorldType == EWorldType::Game);
    // }

    return bControlWindowCreated;

}

/**
 * void CreateSlateWindow()
 * 
 * Creates a new Slate window and sets it properties
 */
 void UControlWindow::CreateSlateWindow()
{
    ControlWindow = SNew( SWindow )
        .Title( ControlWindowTitle )
        .ScreenPosition( ControlWindowPosition )
        .ClientSize( ControlWindowSize )
        .SizingRule( ESizingRule::FixedSize )
        .AutoCenter( EAutoCenter::None )
        .FocusWhenFirstShown( false )
        .UseOSWindowBorder( false )
        .CreateTitleBar( true )
        .SupportsTransparency( EWindowTransparency::PerWindow )
        .InitialOpacity( 1.0f )
        .SupportsMaximize( false )
        .HasCloseButton( false )
        .ActivationPolicy(EWindowActivationPolicy::FirstShown )
        .IsTopmostWindow( true );

    ControlWindow->SetViewportSizeDrivenByWindow( false );

    ControlWindow->GetOnWindowActivatedEvent().AddUObject(this, &UControlWindow::HandleWindowActivated);
    ControlWindow->GetOnWindowDeactivatedEvent().AddUObject(this, &UControlWindow::HandleWindowDeActivated);

}

void UControlWindow::HandleWindowActivated()
{
    OnControlWindowActivated.Broadcast();
}
void UControlWindow::HandleWindowDeActivated()
{
    OnControlWindowDeActivated.Broadcast();
}

void UControlWindow::SetIsStandAloneGame(bool bIsStandAlone)
{
	bIsStandAloneGame = bIsStandAlone;
}

void UControlWindow::OnControlWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed )
{
    DestroyControlWindow();
}

void UControlWindow::DestroyControlWindow()
{
    // Destroy window
	if (ControlWindow.Get() != nullptr)
	{
        ControlWindow->HideWindow();
        ControlWindow->DestroyWindowImmediately();
	}
    ControlWindow.Reset();

    bControlWindowCreated = false;
}

void UControlWindow::ResizeWindow(FVector2D NewSize)
{
    ControlWindowSize = NewSize;
    ControlWindowPosition = ControlWindow->GetPositionInScreen();
    DestroyControlWindow();
    CreateControlWindow();
}

// Override function to keep inputs active in the Game window
FReply UControlWindow::NativeOnKeyUp( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent )
{
    Super::NativeOnKeyUp(InGeometry, InKeyEvent);

    if(bSetFocusToPlayerDirectlyAfterInput)
    {
        //Set focus to game viewport after UMG interaction
        FSlateApplication::Get().SetAllUserFocusToGameViewport();
    }
    
    return FReply::Handled();
}

// Override function to keep inputs active in the Game window
FReply UControlWindow::NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
    Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

    if(bSetFocusToPlayerDirectlyAfterInput)
    {
        //Set focus to game viewport after UMG interaction
        FSlateApplication::Get().SetAllUserFocusToGameViewport();
    }
    return FReply::Handled();
}