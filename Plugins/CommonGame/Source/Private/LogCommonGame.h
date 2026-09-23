// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

// 本插件专用的日志分类。用法：UE_LOG(LogCommonGame, Warning, TEXT("..."));
// UI 策略切换失败、图层未注册、对话框无法显示等情况都会打到这个分类下。
DECLARE_LOG_CATEGORY_EXTERN(LogCommonGame, Log, All);
