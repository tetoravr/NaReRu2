// Copyright 2020 Andrew Bindraw. All Rights Reserved.

#include "SystemAudioManager.h"

FSystemAudioManager *FSystemAudioManager::Instance = nullptr;

FSystemAudioManager::FSystemAudioManager() : 
	AudioEndpointVolume(nullptr),
	DefaultDevice(nullptr),
	DeviceEnumerator(nullptr),
	DevicesCollection(nullptr),
	PropertyStore(nullptr),
	PolicyConfigVista(nullptr),
	PolicyConfig(nullptr)
{
	FWindowsPlatformMisc::CoInitialize();

	CoCreateInstance(__uuidof(CPolicyConfigVistaClient), nullptr, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&PolicyConfigVista);

	// For Win10
	HRESULT Result = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig2, (LPVOID *)&PolicyConfig);
	if (Result != S_OK)
	{
		Result = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig1, (LPVOID *)&PolicyConfig);
	}

	// For Win Vista, 7, 8, 8.1
	if (Result != S_OK)
	{
		Result = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_INPROC, IID_IPolicyConfig0, (LPVOID *)&PolicyConfig);
	}

	CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&DeviceEnumerator);
}

FSystemAudioManager *FSystemAudioManager::Get()
{
	if(Instance == nullptr)
	{
		Instance = new FSystemAudioManager;
	}
	
	return Instance;
}

FSystemAudioManager::~FSystemAudioManager()
{
	FWindowsPlatformMisc::CoUninitialize();
}

void FSystemAudioManager::DestroyInstance()
{
	if(Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}


bool FSystemAudioManager::SetDefaultDevice(const FString &Id)
{
	if (GetDefaultDeviceId() == Id)
	{
		return false;
	}

	if (!PolicyConfigVista)
	{
		return false;
	}

	PolicyConfigVista->SetDefaultEndpoint(reinterpret_cast<wchar_t *>(TCHAR_TO_UTF16(*Id)), eConsole);
	PolicyConfigVista->SetDefaultEndpoint(reinterpret_cast<wchar_t *>(TCHAR_TO_UTF16(*Id)), eMultimedia);
	PolicyConfigVista->SetDefaultEndpoint(reinterpret_cast<wchar_t *>(TCHAR_TO_UTF16(*Id)), eCommunications);

	return true;
}

FString FSystemAudioManager::GetDefaultDeviceName()
{
	return GetDeviceNameFromId(GetDefaultDeviceId());
}

FString FSystemAudioManager::GetDefaultDeviceId()
{
	HRESULT Result = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &DefaultDevice);
	if (Result != S_OK)
	{
		return FString(TEXT(""));
	}

	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd371405(v=vs.85).aspx, see Return value!
	Result = DefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (PVOID *)&AudioEndpointVolume);
	if (Result != S_OK)
	{
		return FString(TEXT(""));
	}

	WCHAR *swId = reinterpret_cast<wchar_t *>(TCHAR_TO_UTF16(*FString(TEXT(""))));
	Result = DefaultDevice->GetId(&swId);
	if (Result != S_OK)
	{
		return FString(TEXT(""));
	}

	return FString(UTF16_TO_TCHAR(swId));
}

FString FSystemAudioManager::GetDeviceNameFromId(const FString &DeviceId)
{
	FString DeviceName;

	for (TPair<FString, FString> &Device : GetActiveDevices())
	{
		if (Device.Key == DeviceId)
		{
			DeviceName = Device.Value;
		}
	}

	return DeviceName;
}

FString FSystemAudioManager::GetDeviceIdFromName(const FString &DeviceName)
{
	FString DeviceId;

	for (TPair<FString, FString> &Device : GetActiveDevices())
	{
		if (Device.Value == DeviceName)
		{
			DeviceId = Device.Key;
		}
	}

	return DeviceId;
}

TMap<FString, FString> FSystemAudioManager::GetActiveDevices()
{
	TMap<FString, FString> ActiveDevices;

	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd371400(v=vs.85).aspx, see Return value!
	HRESULT Result = DeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &DevicesCollection);
	if (Result != S_OK)
	{
		return TMap<FString, FString>();
	}

	UINT count = 0;
	DevicesCollection->GetCount(&count);
	LPWSTR pwszID = nullptr;

	for (UINT i = 0; i < count; i++)
	{
		DevicesCollection->Item(i, &DefaultDevice);
		DefaultDevice->GetId(&pwszID);
		DefaultDevice->OpenPropertyStore(STGM_READ, &PropertyStore);

		PROPVARIANT nameDevice;
		PropVariantInit(&nameDevice);
		PropertyStore->GetValue(PKEY_Device_FriendlyName, &nameDevice);

		ActiveDevices.Add(FString(UTF16_TO_TCHAR(pwszID)));
		ActiveDevices[FString(UTF16_TO_TCHAR(pwszID))] = FString(UTF16_TO_TCHAR(nameDevice.pwszVal));

		CoTaskMemFree(pwszID);
		pwszID = nullptr;
	}

	if (PropertyStore)
	{
		PropertyStore->Release();
		PropertyStore = nullptr;
	}

	if (DefaultDevice)
	{
		DefaultDevice->Release();
		DefaultDevice = nullptr;
	}

	if (DevicesCollection)
	{
		DevicesCollection->Release();
		DevicesCollection = nullptr;
	}

	return ActiveDevices;
}


void FSystemAudioManager::SetMasterVolume(float Value)
{
	float MasterVolume = this->GetScalarFromValue(Value);

	HRESULT Result = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &DefaultDevice);
	if (Result != S_OK)
	{
		return;
	}

	Result = DefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (PVOID *)&AudioEndpointVolume);
	if (Result != S_OK)
	{
		return;
	}

	Result = AudioEndpointVolume->SetMasterVolumeLevelScalar(MasterVolume, nullptr);
	if (Result != S_OK)
	{
		return;
	}

	if (AudioEndpointVolume)
	{
		AudioEndpointVolume->Release();
		AudioEndpointVolume = nullptr;
	}

	if (DefaultDevice)
	{
		DefaultDevice->Release();
		DefaultDevice = nullptr;
	}
}

float FSystemAudioManager::GetMasterVolume()
{
	HRESULT Result = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &DefaultDevice);
	if (Result != S_OK)
	{
		return 0.0f;
	}

	Result = DefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (PVOID *)&AudioEndpointVolume);
	if (Result != S_OK)
	{
		return 0.0f;
	}

	float MasterVolume = 0.0f;
	Result = AudioEndpointVolume->GetMasterVolumeLevelScalar(&MasterVolume);
	if (Result != S_OK)
	{
		return 0.0f;
	}

	if (AudioEndpointVolume)
	{
		AudioEndpointVolume->Release();
		AudioEndpointVolume = nullptr;
	}

	if (DefaultDevice)
	{
		DefaultDevice->Release();
		DefaultDevice = nullptr;
	}

	return GetValueFromScalar(MasterVolume);
}


void FSystemAudioManager::SetMasterMute(bool IsMuted)
{
	HRESULT Result = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &DefaultDevice);
	if (Result != S_OK)
	{
		return;
	}

	Result = DefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (PVOID *)&AudioEndpointVolume);
	if (Result != S_OK)
	{
		return;
	}

	Result = AudioEndpointVolume->SetMute(IsMuted, nullptr);
	if (Result != S_OK)
	{
		return;
	}

	if (AudioEndpointVolume)
	{
		AudioEndpointVolume->Release();
		AudioEndpointVolume = nullptr;
	}

	if (DefaultDevice)
	{
		DefaultDevice->Release();
		DefaultDevice = nullptr;
	}
}

bool FSystemAudioManager::GetMasterMute()
{
	HRESULT Result = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &DefaultDevice);
	if (Result != S_OK)
	{
		return false;
	}

	Result = DefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (PVOID *)&AudioEndpointVolume);
	if (Result != S_OK)
	{
		return false;
	}

	BOOL bIsMuted = 0;
	Result = AudioEndpointVolume->GetMute(&bIsMuted);
	if (Result != S_OK)
	{
		return false;
	}

	if (AudioEndpointVolume)
	{
		AudioEndpointVolume->Release();
		AudioEndpointVolume = nullptr;
	}

	if (DefaultDevice)
	{
		DefaultDevice->Release();
		DefaultDevice = nullptr;
	}

	return bIsMuted;
}


float FSystemAudioManager::GetScalarFromValue(int32 Value)
{
	return FMath::Abs(Value) >= 100.0f ? 1.0f : Value / 100.0f;
}

float FSystemAudioManager::GetValueFromScalar(float Value)
{
	return FMath::RoundToFloat(FMath::Abs(Value) > 0.0f ? Value * 100.0f : 0.0f);
}
