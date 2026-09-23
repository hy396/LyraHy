// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FUIExtensionModule —— 本插件的模块入口。
 * 启动与关闭阶段都没有需要初始化/清理的东西，因此两个函数都是空的，
 * 但保留了完整的 IModuleInterface 实现，便于将来扩展。
 */
class FUIExtensionModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FUIExtensionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUIExtensionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
	
IMPLEMENT_MODULE(FUIExtensionModule, UIExtension)
