// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// CommonLoadingScreen 模块的构建规则。
// 本插件由两个模块组成：
//   CommonLoadingScreen           —— 运行时加载界面（Default 阶段加载）
//   CommonStartupLoadingScreen    —— 引擎启动画面（PreLoadingScreen 阶段加载，ClientOnly）
public class CommonLoadingScreen : ModuleRules
{
	public CommonLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		// 私有依赖说明：
		//   CoreUObject / Engine —— 子系统、世界上下文
		//   Slate / SlateCore    —— 加载界面控件
		//   UMG                  —— 加载界面用 UMG 用户控件
		//   InputCore            —— 加载期间屏蔽玩家输入
		//   PreLoadScreen        —— 与引擎的 PreLoadScreen 机制对接
		//   RenderCore           —— 加载期间的性能设置调整
		//   DeveloperSettings    —— 配置类 UCommonLoadingScreenSettings 需要
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"PreLoadScreen",
				"RenderCore",
				"DeveloperSettings",
				"UMG"
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
