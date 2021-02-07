/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : ManaPlayerEditorDetails.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class UManaPlayer;

/**
 * Implements the details panel of the ManaPlayer asset editor.
 */
class SManaPlayerEditorDetails
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SManaPlayerEditorDetails) { }
	SLATE_END_ARGS()

public:

	/**
	 * Construct this widget
	 *
	 * @param InArgs The declaration data for this widget.
	 * @param InManaPlayer The ManaPlayer asset to show the details for.
	 * @param InStyleSet The style set to use.
	 */
	void Construct(const FArguments& InArgs, UManaPlayer& InManaPlayer);

private:

	/** Pointer to the ManaPlayer asset that is being viewed. */
	UManaPlayer* ManaPlayer;
};
