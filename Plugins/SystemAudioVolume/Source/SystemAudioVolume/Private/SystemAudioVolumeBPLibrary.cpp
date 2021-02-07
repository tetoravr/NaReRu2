// Copyright 2020 Andrew Bindraw. All Rights Reserved.

#include "SystemAudioVolumeBPLibrary.h"
#include "SystemAudioVolume.h"

USystemAudioVolumeBPLibrary::USystemAudioVolumeBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	FSystemAudioManager::Get();
}


bool USystemAudioVolumeBPLibrary::SetDefaultDevice(const FString &DeviceId)
{
	return FSystemAudioManager::Get()->SetDefaultDevice(DeviceId);
}

FString USystemAudioVolumeBPLibrary::GetDefaultDeviceName()
{
	return FSystemAudioManager::Get()->GetDefaultDeviceName();
}

FString USystemAudioVolumeBPLibrary::GetDefaultDeviceId()
{
	return FSystemAudioManager::Get()->GetDefaultDeviceId();
}

void USystemAudioVolumeBPLibrary::GetDeviceIdFromName(const FString &InDeviceName, FString &OutDeviceId)
{
	OutDeviceId = FSystemAudioManager::Get()->GetDeviceIdFromName(InDeviceName);
}

void USystemAudioVolumeBPLibrary::GetDeviceNameFromId(const FString &InDeviceId, FString &OutDeviceName)
{
	OutDeviceName = FSystemAudioManager::Get()->GetDeviceNameFromId(InDeviceId);
}

TMap<FString, FString> USystemAudioVolumeBPLibrary::GetActiveDevices()
{
	return FSystemAudioManager::Get()->GetActiveDevices();
}


void USystemAudioVolumeBPLibrary::SetMasterVolume(float Value)
{
	FSystemAudioManager::Get()->SetMasterVolume(Value);
}

float USystemAudioVolumeBPLibrary::GetMasterVolume()
{
	return FSystemAudioManager::Get()->GetMasterVolume();
}


void USystemAudioVolumeBPLibrary::SetMasterMute(bool IsMuted)
{
	FSystemAudioManager::Get()->SetMasterMute(IsMuted);
}

bool USystemAudioVolumeBPLibrary::GetMasterMute()
{
	return FSystemAudioManager::Get()->GetMasterMute();
}