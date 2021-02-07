// Copyright 2020 Andrew Bindraw. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SystemAudioManager.h"
#include "SystemAudioVolumeBPLibrary.generated.h"

UCLASS()
class USystemAudioVolumeBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	UFUNCTION(BlueprintCallable, Category = "SystemAudioVolume", DisplayName = "Set System Audio Default Device")
	static bool SetDefaultDevice(const FString &DeviceId);

	UFUNCTION(BlueprintPure, Category = "SystemAudioVolume", DisplayName = "Get System Audio Default Device Name")
	static FString GetDefaultDeviceName();

	UFUNCTION(BlueprintPure, Category = "SystemAudioVolume", DisplayName = "Get System Audio Default Device Id")
	static FString GetDefaultDeviceId();

	UFUNCTION(BlueprintCallable, Category = "SystemAudioVolume", DisplayName = "Get System Audio Device Id From Name")
	static void GetDeviceIdFromName(const FString &InDeviceName, FString &OutDeviceId);

	UFUNCTION(BlueprintCallable, Category = "SystemAudioVolume", DisplayName = "Get System Audio Device Name From Id")
	static void GetDeviceNameFromId(const FString &InDeviceId, FString &OutDeviceName);

	UFUNCTION(BlueprintPure, Category = "SystemAudioVolume", DisplayName = "Get System Audio Active Devices")
	static TMap<FString, FString> GetActiveDevices();


	UFUNCTION(BlueprintCallable, Category = "SystemAudioVolume", DisplayName = "Set System Audio Master Volume")
	static void SetMasterVolume(float Value);

	UFUNCTION(BlueprintPure, Category = "SystemAudioVolume", DisplayName = "Get System Audio Master Volume")
	static float GetMasterVolume();


	UFUNCTION(BlueprintCallable, Category = "SystemAudioVolume", DisplayName = "Set System Audio Master Mute")
	static void SetMasterMute(bool IsMuted);

	UFUNCTION(BlueprintPure, Category = "SystemAudioVolume", DisplayName = "Get System Audio Master Mute")
	static bool GetMasterMute();
};
