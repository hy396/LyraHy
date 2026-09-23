// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraReplicationGraphSettings.h"
#include "Misc/App.h"
#include "System/LyraReplicationGraph.h"

// 构造：默认配置值
ULyraReplicationGraphSettings::ULyraReplicationGraphSettings()
{
	CategoryName = TEXT("Game");
	DefaultReplicationGraphClass = ULyraReplicationGraph::StaticClass();
}