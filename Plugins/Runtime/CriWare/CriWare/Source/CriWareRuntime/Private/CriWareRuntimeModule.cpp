/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2013-2016 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : CriWareRuntime Module Implementation
* File     : CriWareRuntimeModule.cpp
*
****************************************************************************/

#include "CriWareRuntimePrivatePCH.h"
#include "ICriWareRuntime.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"


/**
* CriWareRuntime module implementation (private)
*/

class FCriWareRuntimeModule : public ICriWareRuntime
{
public:
	
	//~ Start of IModuleInterface
	
	virtual void StartupModule() override
	{
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
		// ここで作られる RealShaderSourceDir は CriWare.uplugin と同じディレクトリ
		// Prior to 4.21, plugin shaders was automatically loaded, from 4.21 we need to do this manually.
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 21
		FString RealShaderSourceDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("CriWare"))->GetBaseDir(), TEXT("Shaders"));
		FString VirtualShaderSourceDir = FString(TEXT("/Plugin/CriWare"));
		UE_LOG(LogCriWareRuntime, Log,
			TEXT("Mapping shader source directory %s to virtual directory %s"),
			*RealShaderSourceDir, *VirtualShaderSourceDir);
		AddShaderSourceDirectoryMapping(VirtualShaderSourceDir, RealShaderSourceDir);
#endif
#endif	/* </cri_delete_if_LE> */
	}

	virtual void ShutdownModule() override
	{
	}

	//~ End of IModuleInterface
};

IMPLEMENT_MODULE(FCriWareRuntimeModule, CriWareRuntime);
