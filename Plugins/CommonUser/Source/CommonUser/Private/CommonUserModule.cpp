// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUserModule.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FCommonUserModule"

// 模块启动回调：目前无需额外初始化，子系统由引擎自动创建
void FCommonUserModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

// 模块关闭回调：无需额外清理
void FCommonUserModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
// 把 FCommonUserModule 注册为名为 CommonUser 的模块实现
IMPLEMENT_MODULE(FCommonUserModule, CommonUser)