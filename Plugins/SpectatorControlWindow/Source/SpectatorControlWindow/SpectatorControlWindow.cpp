// Copyright 2016-2018 dfab BV, Rights Reserved.

#include "SpectatorControlWindow.h"

#define LOCTEXT_NAMESPACE "FSpectatorControlWindowModule"

void FSpectatorControlWindowModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FSpectatorControlWindowModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpectatorControlWindowModule, SpectatorControlWindow)