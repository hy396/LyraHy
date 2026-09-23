// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 开发期专用工具：只在 PIE 里生效的快捷开关，以及作弊控制台要用的类查找。
 * 正式构建里这些函数基本恒返回「正常流程」的值。
 */
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "LyraDevelopmentStatics.generated.h"

class UClass;
class UObject;
class UWorld;
struct FAssetData;
struct FFrame;

// 开发期工具函数库
UCLASS()
class ULyraDevelopmentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 是否跳过热身/等待玩家等阶段直接进战斗；只有 PIE 且开发者设置里关掉完整流程时才为 true
	// Should game logic skip directly to gameplay (skipping any match warmup / waiting for players / etc... aspects)
	// Will always return false except when playing in the editor and bTestFullGameFlowInPIE (in Lyra Developer Settings) is false
	UFUNCTION(BlueprintCallable, Category="Lyra")
	static bool ShouldSkipDirectlyToGameplay();

	// 是否加载装饰性背景；只有 PIE 且勾选跳过时才为 false
	// Should game logic load cosmetic backgrounds in the editor?
	// Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in Lyra Developer Settings) is true
	UFUNCTION(BlueprintCallable, Category = "Lyra", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool ShouldLoadCosmeticBackgrounds();

	// Bot 是否允许攻击（PIE 调试用开关，上方英文注释是复制粘贴的，实际含义以函数名为准）
	// Should game logic load cosmetic backgrounds in the editor?
	// Will always return true except when playing in the editor and bSkipLoadingCosmeticBackgroundsInPIE (in Lyra Developer Settings) is true
	UFUNCTION(BlueprintCallable, Category = "Lyra")
	static bool CanPlayerBotsAttack();

	// 找最适合跑服务端作弊命令的 PIE 世界（单机时就是唯一那个，否则是监听服或专用服）
	// Finds the most appropriate play-in-editor world to run 'server' cheats on
	//   This might be the only world if running standalone, the listen server, or the dedicated server
	static UWorld* FindPlayInEditorAuthorityWorld();

	// 按短名猜一个 UClass，带若干启发式规则，方便在作弊控制台里敲类名
	// Tries to find a class by a short name (with some heuristics to improve the usability when done via a cheat console)
	static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

	// 模板版本，自动把目标基类填成 DesiredClass
	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
	{
		return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
	}

private:
	// 扫描并缓存所有蓝图资产
	static TArray<FAssetData> GetAllBlueprints();
	// 在蓝图里按名字找类
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};
