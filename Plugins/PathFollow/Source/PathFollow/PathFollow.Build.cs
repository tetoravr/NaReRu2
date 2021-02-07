// Copyright 2015-2016 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

namespace UnrealBuildTool.Rules
{
	public class PathFollow : ModuleRules
	{
		public PathFollow(ReadOnlyTargetRules Target) : base(Target)
		{

            PCHUsage = PCHUsageMode.NoSharedPCHs;
            PrivatePCHHeaderFile = "Private/PathFollowPrivatePCH.h";
            
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
                    ModuleDirectory
                }
				);

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
	                "CoreUObject",
					"Engine",
	                "InputCore"
				
					// ... add other public dependencies that you statically link with here ...
				}
				);
					
	        PrivateDependencyModuleNames.AddRange(
	            new string[]
	            {
	                // ... add private dependencies that you statically link with here ...	
	            }
	            );
        
	        if (Target.bBuildEditor == true)
	        {
	            //Needed for PathFollowerComponentVizualizer
	            PrivateDependencyModuleNames.Add("UnrealEd");
	            PrivateDependencyModuleNames.Add("LevelEditor");
	            PrivateDependencyModuleNames.Add("DistCurveEditor");
	            PrivateDependencyModuleNames.Add("PropertyEditor");
	            PrivateDependencyModuleNames.Add("EditorStyle");
	            PrivateDependencyModuleNames.Add("Slate");
	            PrivateDependencyModuleNames.Add("SlateCore"); 
	        }
		
		
			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
				
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}
