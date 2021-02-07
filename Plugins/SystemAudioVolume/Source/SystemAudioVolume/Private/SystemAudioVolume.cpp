// Copyright 2020 Andrew Bindraw. All Rights Reserved.

#include "SystemAudioVolume.h"
#include "SystemAudioManager.h"

#define LOCTEXT_NAMESPACE "FSystemAudioVolumeModule"

void FSystemAudioVolumeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FSystemAudioVolumeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FSystemAudioManager::DestroyInstance();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSystemAudioVolumeModule, SystemAudioVolume)