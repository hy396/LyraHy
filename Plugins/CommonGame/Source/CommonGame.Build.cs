// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// CommonGame 模块的构建规则。
// 本插件是纯 C++ 运行时模块（CanContainContent=false）。
public class CommonGame : ModuleRules
{
	public CommonGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		// 公共依赖说明：
		//   Core / CoreUObject / Engine —— UE 基础三件套
		//   InputCore     —— 键位提示控件需要
		//   Slate / SlateCore / UMG —— UI 底层与 UMG 控件
		//   CommonInput / CommonUI —— UI 与输入体系的基础
		//   CommonUser    —— 提供 UCommonUserInfo / 权限 / 会话（GameInstance 的消息处理依赖它）
		//   GameplayTags  —— UI 图层标签（UI.Layer.*）
		//   ModularGameplayActors —— ACommonPlayerController 派生自 AModularPlayerController
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"CommonInput",
				"CommonUI",
				"CommonUser",
				"GameplayTags",
				"ModularGameplayActors",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
