// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 复制图的默认设置。继承 UDeveloperSettingsBackedByCVars，
 * 所以每个字段都同时是一个控制台变量（Lyra.RepGraph.*），运行时可直接调。
 */
#include "CoreTypes.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "LyraReplicationGraphTypes.h"
#include "LyraReplicationGraphSettings.generated.h"

// 复制图默认设置
/**
 * Default settings for the Lyra replication graph
 */
UCLASS(config=Game, MinimalAPI)
class ULyraReplicationGraphSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	ULyraReplicationGraphSettings();

public:

	// 总开关：为 true 时完全不走复制图，退回引擎默认的复制方式
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	bool bDisableReplicationGraph = true;

	// 要使用的复制图类（必须是 ULyraReplicationGraph 子类）
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph, meta = (MetaClass = "/Script/LyraGame.LyraReplicationGraph"))
	FSoftClassPath DefaultReplicationGraphClass;

	// 是否启用 FastShared 路径（移动更新的快速通道）
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Lyra.RepGraph.EnableFastSharedPath"))
	bool bEnableFastSharedPath = true;

	// FastShared 移动更新可用带宽；独立于 NetDriver 的总带宽配额
	// How much bandwidth to use for FastShared movement updates. This is counted independently of the NetDriver's target bandwidth.
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ForceUnits=Kilobytes, ConsoleVariable = "Lyra.RepGraph.TargetKBytesSecFastSharedPath"))
	int32 TargetKBytesSecFastSharedPath = 10;

	// FastShared 的剔除距离百分比
	UPROPERTY(EditAnywhere, Category = FastSharedPath, meta = (ConsoleVariable = "Lyra.RepGraph.FastSharedPathCullDistPct"))
	float FastSharedPathCullDistPct = 0.80f;

	// 销毁信息（DestructionInfo）复制的最大距离，超出就不发给该客户端
	UPROPERTY(EditAnywhere, Category = DestructionInfo, meta = (ForceUnits = cm, ConsoleVariable = "Lyra.RepGraph.DestructInfo.MaxDist"))
	float DestructionInfoMaxDist = 30000.f;

	// 空间网格的单格边长
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.CellSize"))
	float SpatialGridCellSize = 10000.0f;

	// 网格的初始 Min X；若有 Actor 超出这个范围，系统会自动重置网格
	// Essentially "Min X" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.SpatialBiasX"))
	float SpatialBiasX = -200000.0f;

	// 网格的初始 Min Y，同上
	// Essentially "Min Y" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta=(ForceUnits=cm, ConsoleVariable = "Lyra.RepGraph.SpatialBiasY"))
	float SpatialBiasY = -200000.0f;

	// 是否禁用空间网格重建
	UPROPERTY(EditAnywhere, Category=SpatialGrid, meta = (ConsoleVariable = "Lyra.RepGraph.DisableSpatialRebuilds"))
	bool bDisableSpatialRebuilds = true;

	// 把动态空间化 Actor 摊到多少个桶里；桶越多，单个 Actor 的有效复制频率越低
	// 这一层在 Actor 自身的 NetUpdateFrequency 判定之前生效
	// How many buckets to spread dynamic, spatialized actors across.
	// High number = more buckets = smaller effective replication frequency.
	// This happens before individual actors do their own NetUpdateFrequency check.
	UPROPERTY(EditAnywhere, Category = DynamicSpatialFrequency, meta = (ConsoleVariable = "Lyra.RepGraph.DynamicActorFrequencyBuckets"))
	int32 DynamicActorFrequencyBuckets = 3;

	// 针对特定类的自定义复制设置列表
	// Array of Custom Settings for Specific Classes 
	UPROPERTY(config, EditAnywhere, Category = ReplicationGraph)
	TArray<FRepGraphActorClassSettings> ClassSettings;
};
