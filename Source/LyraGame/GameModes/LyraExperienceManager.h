// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 体验管理器（引擎子系统）。
 *
 * 它存在的唯一目的：在多 PIE 会话之间仲裁 GameFeature 插件的激活。
 * 同一台机器上开多个 PIE 窗口时，一个插件可能被多个会话同时请求激活，
 * 必须「最后一个退出的才真正卸载」，否则先关的窗口会把别人正在用的插件干掉。
 */
#include "Subsystems/EngineSubsystem.h"
#include "LyraExperienceManager.generated.h"

// 体验管理器，主要用于多 PIE 会话间的插件激活仲裁
/**
 * Manager for experiences - primarily for arbitration between multiple PIE sessions
 */
UCLASS(MinimalAPI)
class ULyraExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
// 每次 PIE 开始时清空请求计数
#if WITH_EDITOR
	LYRAGAME_API void OnPlayInEditorBegun();

	// 某个会话请求激活插件：计数 +1
	static void NotifyOfPluginActivation(const FString PluginURL);
	// 某个会话请求反激活插件：计数 -1，归零时才真正允许卸载
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif

private:
	// The map of requests to active count for a given game feature plugin
	// (to allow first in, last out activation management during PIE)
	// 插件 URL -> 当前请求激活它的会话数（配合上面的先进后出管理）
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
