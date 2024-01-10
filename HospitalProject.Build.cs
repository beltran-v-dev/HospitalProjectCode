// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HospitalProject : ModuleRules
{
	public HospitalProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "NavigationSystem", "AIModule", "Niagara", "GameplayCameras", "GameplayTasks", "PhysicsCore"  });

    }
}
