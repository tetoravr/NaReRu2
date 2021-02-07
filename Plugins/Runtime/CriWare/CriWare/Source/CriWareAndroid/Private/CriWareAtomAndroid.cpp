/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2019 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : Adx2 Atom for Android
 * File     : CriWareAtomAndroid.cpp
 *
 ****************************************************************************/

#include "CriWareAtomAndroid.h"
// CriWare
#include "CriWareApi.h"
#include "CriWareInitializer.h"
// Unreal
#include "Android/AndroidJNI.h"

FCriWareAtomAndroid::FCriWareAtomAndroid()
	: ICriWarePlatformAtom()
{
	// Register this class to CriWareRuntime module for Mana platform specific.
	UCriWareInitializer::Platform.RegisterPlatformAtomInterface(this);
}
FCriWareAtomAndroid::~FCriWareAtomAndroid()
{
	UCriWareInitializer::Platform.UnregisterPlatformAtomInterface();
}

void FCriWareAtomAndroid::InitializeAtomLibrary() 
{
#if defined(XPT_TGT_ANDROID)
	criAtom_SetJavaVM_ANDROID(GJavaVM);
#endif
}

void FCriWareAtomAndroid::FinalizeAtomLibrary() 
{
}
