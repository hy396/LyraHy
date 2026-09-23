// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CommonUser : ModuleRules
{
	public CommonUser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// 在线层版本开关：true 使用 OnlineSubsystem(OSS v1)，false 使用 OnlineServices(OSS v2)
		// 这个开关会写入下面的 COMMONUSER_OSSV1 宏，源码里大量 #if COMMONUSER_OSSV1 依赖它
		bool bUseOnlineSubsystemV1 = true;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreOnline",
				"GameplayTags",
				// ... add other public dependencies that you statically link with here ...
			}
			);

		// 按开关把对应的在线模块加入公共依赖
		if (bUseOnlineSubsystemV1)
		{
			PublicDependencyModuleNames.Add("OnlineSubsystem");
		}
		else
		{
			PublicDependencyModuleNames.Add("OnlineServicesInterface");
		}
		PrivateDependencyModuleNames.Add("OnlineSubsystemUtils");
		// 把开关导出成编译宏，供头/源文件做条件编译
		PublicDefinitions.Add("COMMONUSER_OSSV1=" + (bUseOnlineSubsystemV1 ? "1" : "0"));

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreOnline",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"InputCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
