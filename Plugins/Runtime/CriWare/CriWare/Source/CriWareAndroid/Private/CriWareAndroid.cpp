/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2019 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : IModuleInterface Class for CriWareAndroid module
 * File     : CriWareAndroid.cpp
 *
 ****************************************************************************/

#include "CriWareAndroidPrivate.h"

#include "CriWareAtomAndroid.h"
#include "CriWareManaAndroid.h"

#include "Engine/Engine.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogCriWareAndroid);


class FCriWareAndroidModule : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		// Check if CriWareRuntime is enabled
		bool IsCriWareRuntimeEnabled = false;
		auto AllPlugins = IPluginManager::Get().GetEnabledPlugins();
		for (auto& Plugin : AllPlugins) {
			if (Plugin->GetName() == "CriWare") {
				IsCriWareRuntimeEnabled = true;
			}
		}

		if (IsCriWareRuntimeEnabled) {
			AtomAndroid = MakeShareable(new FCriWareAtomAndroid());
			ManaAndroid = MakeShareable(new FCriWareManaAndroid());
		} else {
			UE_LOG(LogCriWareAndroid, Error, TEXT("CriWare Plugin is not Enabled."));
		}
	}

	virtual void ShutdownModule() override
	{
		AtomAndroid.Reset();
		ManaAndroid.Reset();
	}

private:
	TSharedPtr<FCriWareAtomAndroid> AtomAndroid;
	TSharedPtr<FCriWareManaAndroid> ManaAndroid;
};

IMPLEMENT_MODULE(FCriWareAndroidModule, CriWareAndroid)
