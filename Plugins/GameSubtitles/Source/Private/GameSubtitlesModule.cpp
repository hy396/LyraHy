// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTagsManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

/**
 * FGameSubtitlesModule —— 本插件的模块入口。
 * 启动阶段唯一要紧的事：把本插件的 Config/Tags 目录注册为 GameplayTag 的 ini 搜索路径，
 * 这样插件自带的字幕相关标签才能被引擎发现并加载。
 */
class FGameSubtitlesModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FGameSubtitlesModule::StartupModule()
{
	// 注册本插件自带的 GameplayTag 配置目录，否则插件里的标签不会被加载。
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("GameSubtitles/Config/Tags"));
}

void FGameSubtitlesModule::ShutdownModule()
{
}
	
IMPLEMENT_MODULE(FGameSubtitlesModule, GameSubtitles)
