// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// AsyncMixin 模块的构建规则。
// 本插件是纯 C++ 运行时模块，不依赖任何其它插件，也不含内容资产。
public class AsyncMixin : ModuleRules
{
	public AsyncMixin(ReadOnlyTargetRules Target) : base(Target)
	{
		// 公共依赖说明：
		//   Core / CoreUObject —— UE 基础
		//   Engine            —— 需要 UAssetManager / FStreamableManager 来做真正的异步加载
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
            }
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
