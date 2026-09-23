// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 复制图用到的类型定义：路由策略枚举 + 可在配置里逐类覆盖的复制设置结构。
 */
#include "ReplicationGraphTypes.h"
#include "LyraReplicationGraphTypes.generated.h"

// 核心枚举：决定一个 Actor 类被路由到哪个复制节点（每个类映射到一个枚举值）
// This is the main enum we use to route actors to the right replication node. Each class maps to one enum.
UENUM()
enum class EClassRepNodeMapping : uint32
{
	// 不路由到任何节点，交给专用节点处理（例如 PlayerState 限流节点）
	NotRouted,						// Doesn't map to any node. Used for special case actors that handled by special case nodes (ULyraReplicationGraphNode_PlayerStateFrequencyLimiter)
	// 对所有连接都相关：路由到 AlwaysRelevant 节点
	RelevantAllConnections,			// Routes to an AlwaysRelevantNode or AlwaysRelevantStreamingLevelNode node

	// 分界线：此枚举以下全部都是「空间化」的，见 ULyraReplicationGraph::IsSpatialized
	// ONLY SPATIALIZED Enums below here! See ULyraReplicationGraph::IsSpatialized

	// 静态空间化：放在网格节点里，不会移动，不需要每帧更新
	Spatialize_Static,				// Routes to GridNode: these actors don't move and don't need to be updated every frame.
	// 动态空间化：会频繁移动，每帧更新
	Spatialize_Dynamic,				// Routes to GridNode: these actors mode frequently and are updated once per frame.
	// 休眠空间化：休眠期间按静态处理，被唤醒时按动态处理（适用于「不休眠时才会动」的 Actor）
	Spatialize_Dormancy,			// Routes to GridNode: While dormant we treat as static. When flushed/not dormant dynamic. Note this is for things that "move while not dormant".
};

// 可直接配给某个类的复制设置（也能映射成引擎的 FRepGraphActorTemplateSettings）
// Actor Class Settings that can be assigned directly to a Class.  Can also be mapped to a FRepGraphActorTemplateSettings 
USTRUCT()
struct FRepGraphActorClassSettings
{
	GENERATED_BODY()

	FRepGraphActorClassSettings() = default;

	// 要应用这些设置的类
	// Name of the Class the settings will be applied to
	UPROPERTY(EditAnywhere)
	FSoftClassPath ActorClass;

	// 是否把这个类的 RepInfo 加进 ClassRepNodePolicies 映射表
	// If we should add this Class' RepInfo to the ClassRepNodePolicies Map
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddClassRepInfoToMap  = true;

	// 加进映射表时使用哪个路由策略
	// What ClassNodeMapping we should use when adding Class to ClassRepNodePolicies Map
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddClassRepInfoToMap"))
	EClassRepNodeMapping ClassNodeMapping = EClassRepNodeMapping::NotRouted;

	// 是否把该类登记到「RPC 多播自动开通道」映射表
	// Should we add this to the RPC_Multicast_OpenChannelForClass map
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddToRPC_Multicast_OpenChannelForClassMap = false;

	// 已登记到上面的表里时，是否真的为它开通道
	// If this is added to RPC_Multicast_OpenChannelForClass map then should we actually open a channel or not
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddToRPC_Multicast_OpenChannelForClassMap"))
	bool bRPC_Multicast_OpenChannelForClass = true;

	// 把配置里的软类路径解析成真正的 UClass（蓝图类会同步加载）
	UClass* GetStaticActorClass() const
	{
		UClass* StaticActorClass = nullptr;
		const FString ActorClassNameString = ActorClass.ToString();

		if (FPackageName::IsScriptPackage(ActorClassNameString))
		{
			StaticActorClass = FindObject<UClass>(nullptr, *ActorClassNameString, true);

			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Find Static Class for %s"), *ActorClassNameString);
			}
		}
		else
		{
			// Allow blueprints to be used for custom class settings
			StaticActorClass = (UClass*)StaticLoadObject(UClass::StaticClass(), nullptr, *ActorClassNameString);
			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Load Static Class for %s"), *ActorClassNameString);
			}
		}

		return StaticActorClass;
	}
};