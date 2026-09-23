// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

// 本插件专用的日志分类。使用时：UE_LOG(LogUIExtension, Warning, TEXT("..."));
// 注册失败、句柄无效、扩展被契约过滤掉等情况都会打到这个分类下。
DECLARE_LOG_CATEGORY_EXTERN(LogUIExtension, Log, All);
