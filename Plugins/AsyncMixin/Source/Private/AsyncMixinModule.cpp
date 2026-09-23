// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FAsyncMixinModule —— 本插件的模块入口。
 * 启动与关闭阶段都没有需要初始化/清理的东西，因此两个函数均为空实现。
 */
class FAsyncMixinModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FAsyncMixinModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FAsyncMixinModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
	
// 注册本模块。注意模块名必须与 .uplugin 里声明的一致。
IMPLEMENT_MODULE(FAsyncMixinModule, AsyncMixin)
