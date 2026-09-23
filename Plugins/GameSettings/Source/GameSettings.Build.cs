// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// GameSettings 模块的构建规则。
// 本插件是纯 C++ 运行时模块（CanContainContent=false），不依赖任何内容资产。
public class GameSettings : ModuleRules
{
	public GameSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		// 公共依赖说明：
		//   Core / CoreUObject / Engine —— UE 基础三件套
		//   InputCore     —— 处理按键选择（改键位界面）
		//   Slate / SlateCore / UMG —— UI 底层与 UMG 控件
		//   CommonInput / CommonUI —— Lyra/CommonGame 体系的 UI 与输入基础（平台特性标签也来自 CommonUI）
		//   GameplayTags —— 设置项的标签与具名动作标签
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
				"GameplayTags"
			}
		);
			
		
		// 私有依赖说明：
		//   ApplicationCore —— 窗口/输入等平台层能力
		//   PropertyPath    —— FGameSettingDataSourceDynamic 用它按属性路径做反射读写
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"PropertyPath"
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
