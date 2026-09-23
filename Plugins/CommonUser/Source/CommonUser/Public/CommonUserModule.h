// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

// CommonUser 模块实现：只在引擎加载/卸载模块时被调用，
// 插件真正的业务逻辑都在 UCommonUserSubsystem、UCommonSessionSubsystem 等子系统里
class FCommonUserModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	// 模块加载：把本模块注册进引擎
	virtual void StartupModule() override;
	// 模块卸载：清理模块级资源
	virtual void ShutdownModule() override;
};
