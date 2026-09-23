// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FCommonGameModule —— 本插件的模块入口。
 * 启动与关闭阶段都没有需要初始化/清理的东西，因此两个函数均为空实现。
 */
class FCommonGameModule : public IModuleInterface
{
public:
	FCommonGameModule();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

};


FCommonGameModule::FCommonGameModule()
{
}

void FCommonGameModule::StartupModule()
{
}

void FCommonGameModule::ShutdownModule()
{
}

// 注册本模块。注意模块名必须与 .uplugin 里声明的一致。
IMPLEMENT_MODULE(FCommonGameModule, CommonGame);
