/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : Build Settings of CriWareAndroid Module
 * File     : CriWareAndroid.Build.cs
 *
 ****************************************************************************/
namespace UnrealBuildTool.Rules
{
	public class CriWareAndroid : ModuleRules
	{
		public CriWareAndroid(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateIncludePaths.Add("CriWareAndroid/Private");

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Engine",
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Projects",
					"RenderCore",
					"RHI",
                    "Launch",
                    "CriWareRuntime",
				}
			);
		}
	}
}
