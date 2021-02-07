// Copyright 2016-2018 dfab BV, Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ControlWindow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnControlWindowActivated );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnControlWindowDeActivated );

class SWindow;
class UGameViewportClient;
class FSceneViewport;
/**
 * 
 */
UCLASS()
class UControlWindow : public UUserWidget
{
	GENERATED_BODY()
	UControlWindow( const FObjectInitializer& ObjectInitializer );

public:

    /**
     * Set the control window title
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ControlWindow Settings")
		FText ControlWindowTitle;	

    /**
     * Set the control window size
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ControlWindow Settings")
		FVector2D ControlWindowSize;

    /**
     * Set the control window position ( from the top left of the screen )
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ControlWindow Settings")
		FVector2D ControlWindowPosition;
	
    /** 
     * Wil enable the player inputs again immediately after interaction with the widget.
     * If you need to input text with a keyboard, keep this disabled and click the game window after
     * input to return input control to the user.
     */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ControlWindow Settings")
		bool bSetFocusToPlayerDirectlyAfterInput = false;
	
	UPROPERTY()
		bool bControlWindowCreated;
	
	UFUNCTION(BLueprintCallable, Category = "ControlWindow")
		bool CreateControlWindow();

	UFUNCTION(BLueprintCallable, Category = "ControlWindow")
		void RebuildControlWindow();

	UFUNCTION(BlueprintCallable, Category = "ControlWindow")
		void DestroyControlWindow();

    UFUNCTION(BlueprintCallable, Category = "ControlWindow")
        void ResizeWindow(FVector2D NewSize);


    TSharedPtr<SWindow> ControlWindow = nullptr;

    virtual FReply NativeOnKeyUp( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;
    virtual FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

    UPROPERTY(BlueprintAssignable, Category = "ControlWindow")
        FOnControlWindowActivated OnControlWindowActivated;
        
    UPROPERTY(BlueprintAssignable, Category = "ControlWindow")
        FOnControlWindowDeActivated OnControlWindowDeActivated;

protected:

	EWindowMode::Type WindowMode;

	UFUNCTION(BlueprintCallable, Category="ControlWindow")
		void SetIsStandAloneGame( bool bIsStandAlone );

private:

	bool bIsStandAloneGame  = false;
    bool bIsEditor = false;

    UGameViewportClient* SpectatorViewportClient;
	TSharedPtr<FSceneViewport> SpectatorWindowSceneViewport;

	void CreateSlateWindow();

    void OnControlWindowClosed( const TSharedRef<SWindow>& WindowBeingClosed );

    void HandleWindowActivated();
    void HandleWindowDeActivated();

};
