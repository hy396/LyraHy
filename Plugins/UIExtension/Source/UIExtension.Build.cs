// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UIExtension : ModuleRules
{
	// UIExtension 模块的构建规则。
	// 本插件是纯 C++ 运行时模块（CanContainContent=false）。
	public UIExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		// 公共依赖说明：
		//   Core / CoreUObject / Engine —— UE 基础三件套
		//   SlateCore / Slate / UMG    —— 控件与 UI 底层
		//   CommonUI                   —— Lyra/CommonGame 体系的 UI 基础
		//   CommonGame                 —— 提供 UCommonLocalPlayer（用于分玩家的上下文）
		//   GameplayTags               —— 扩展点标签体系的基础
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"SlateCore",
				"Slate",
				"UMG",
				"CommonUI",
				"CommonGame",
				"GameplayTags"
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
            }
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
			}
		);
	}
}
