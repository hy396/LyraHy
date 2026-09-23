// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameSubtitles : ModuleRules
{
	// GameSubtitles 模块的构建规则。本插件是纯 C++ 运行时模块。
	public GameSubtitles(ReadOnlyTargetRules Target) : base(Target)
	{
		// 公共依赖说明：
		//   Core          —— 基础
		//   Overlay       —— 提供 UOverlays / FOverlayItem（字幕资源的数据结构）
		//   UMG           —— 字幕控件走 UMG 包装
		//   MediaAssets   —— 提供 UMediaPlayer（字幕时间轴以它为准）
		//   MediaUtils    —— 媒体播放相关工具
		//   GameplayTags  —— 模块启动时会注册插件自带的标签目录
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"Overlay",
                "UMG",
				"MediaAssets",
				"MediaUtils",
				"GameplayTags"
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
                "UMG",
            }
        );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
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
