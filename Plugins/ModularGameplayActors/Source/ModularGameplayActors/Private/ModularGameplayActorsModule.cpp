// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// 本插件不提供自定义模块逻辑（没有 StartupModule/ShutdownModule 要跑），
// 因此直接用引擎的默认模块实现 FDefaultModuleImpl 即可。
IMPLEMENT_MODULE(FDefaultModuleImpl, ModularGameplayActors);
