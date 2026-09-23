// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// 本模块没有自定义启动/关闭逻辑，直接用引擎的默认模块实现即可。
// 真正的工作都在 ULoadingScreenManager 这个 GameInstance 子系统里。
IMPLEMENT_MODULE(FDefaultModuleImpl, CommonLoadingScreen)