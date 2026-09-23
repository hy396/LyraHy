// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameData.h"
#include "LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameData)

// 构造：默认设置
ULyraGameData::ULyraGameData()
{
}

// 取全局游戏数据（转发给资产管理器）
const ULyraGameData& ULyraGameData::ULyraGameData::Get()
{
	return ULyraAssetManager::Get().GetGameData();
}
