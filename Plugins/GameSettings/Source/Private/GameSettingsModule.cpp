// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FGameSettingsModule —— 本插件的模块入口。
 * 启动与关闭阶段都没有需要初始化/清理的东西（设置项都是用时才创建），
 * 因此两个函数均为空实现。
 */
class FGameSettingsModule : public IModuleInterface
{
public:
	FGameSettingsModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

};


FGameSettingsModule::FGameSettingsModule()
{
}

void FGameSettingsModule::StartupModule()
{
}

void FGameSettingsModule::ShutdownModule()
{
}

// 注册本模块。注意模块名必须与 .uplugin 里声明的一致。
IMPLEMENT_MODULE(FGameSettingsModule, GameSettings);
