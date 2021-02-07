// Copyright 2020 Andrew Bindraw. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmeapi.h>
#include "PolicyConfig.h"
#include "Windows/HideWindowsPlatformTypes.h"

class FSystemAudioManager
{
	private:
		FSystemAudioManager();
		~FSystemAudioManager();
		
		static FSystemAudioManager *Instance;

		IAudioEndpointVolume    *AudioEndpointVolume;
		IMMDevice               *DefaultDevice;
		IMMDeviceEnumerator     *DeviceEnumerator;
		IMMDeviceCollection     *DevicesCollection;
		IPropertyStore          *PropertyStore;

		IPolicyConfigVista      *PolicyConfigVista;
		IPolicyConfig           *PolicyConfig;

	public:
		static FSystemAudioManager *Get();
		
		static void DestroyInstance();
		
		void SetMasterVolume(float Value);
		float GetMasterVolume();
		
		bool SetDefaultDevice(const FString &Id);

		void SetMasterMute(bool IsMuted);
		bool GetMasterMute();

		TMap<FString, FString> GetActiveDevices();

		FString GetDefaultDeviceId();
		FString GetDefaultDeviceName();

		FString GetDeviceIdFromName(const FString &DeviceName);

		FString GetDeviceNameFromId(const FString &DeviceId);

	private:
		FORCEINLINE float GetScalarFromValue(int32 Value);

		FORCEINLINE float GetValueFromScalar(float Value);
};