/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2019 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : Adx2 Atom for Android
 * File     : CriWareAtomAndroid.h
 *
 ****************************************************************************/

#pragma once

#include "CriWarePlatform.h"

class FCriWareAtomAndroid : ICriWarePlatformAtom {

public:
	FCriWareAtomAndroid();
	virtual ~FCriWareAtomAndroid();

	virtual void InitializeAtomLibrary();
	virtual void FinalizeAtomLibrary();
};