// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// CommonStartupLoadingScreen 模块的构建规则。
// 加载阶段为 PreLoadingScreen（极早期），且仅客户端需要（ClientOnly）。
public class CommonStartupLoadingScreen : ModuleRules
{
	public CommonStartupLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
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
		//   CoreUObject / Engine —— 基础
		//   Slate / SlateCore    —— 只能用纯 Slate（此时 UMG 还不可用）
		//   MoviePlayer          —— 播放启动影片（预留能力）
		//   PreLoadScreen        —— 引擎的早期加载画面机制，本模块就是接在它上面
		//   DeveloperSettings    —— 配置读取
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"MoviePlayer",
				"PreLoadScreen",
				"DeveloperSettings"
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
