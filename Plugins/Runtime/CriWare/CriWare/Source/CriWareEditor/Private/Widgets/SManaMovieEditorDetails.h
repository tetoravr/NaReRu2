/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : CriWareEdior
 * File     : ManaMovieEditorDetails.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Styling/ISlateStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class UManaMovie;

/**
 * Implements the details panel of the ManaMovie asset editor.
 */
class SManaMovieEditorDetails
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SManaMovieEditorDetails) { }
	SLATE_END_ARGS()

public:

	/**
	 * Construct this widget
	 *
	 * @param InArgs The declaration data for this widget.
	 * @param InManaMovie The ManaMovie asset to show the details for.
	 * @param InStyleSet The style set to use.
	 */
	void Construct(const FArguments& InArgs, UManaMovie& InManaMovie);

private:

	/** Pointer to the ManaMovie asset that is being viewed. */
	UManaMovie* ManaMovie;
};

