// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的复制图实现。
 *
 * 核心思路：给每个 Actor 类指定一个 EClassRepNodeMapping，据此把它路由到
 *   - AlwaysRelevant 节点（对所有连接都相关，例如 GameState）
 *   - 2D 空间网格节点（按距离做剔除，绝大多数 Actor 走这里）
 *   - 专用节点（PlayerState 走限流节点，见下方）
 * 更详细的说明在 LyraReplicationGraph.cpp 顶部。
 */
#include "ReplicationGraph.h"
#include "LyraReplicationGraphTypes.h"
#include "LyraReplicationGraph.generated.h"

class AGameplayDebuggerCategoryReplicator;

// 复制图日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogLyraRepGraph, Display, All);

// 复制图实现主体
/** Lyra Replication Graph implementation. See additional notes in LyraReplicationGraph.cpp! */
UCLASS(transient, config=Engine)
class ULyraReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	ULyraReplicationGraph();

	// 重置世界状态：清空网格与各类节点里的 Actor
	virtual void ResetGameWorldState() override;

	// 初始化「类 -> 复制策略」的全局设置（读配置 + 代码里显式指定的部分）
	virtual void InitGlobalActorClassSettings() override;
	// 创建全局节点：网格节点、AlwaysRelevant 节点等
	virtual void InitGlobalGraphNodes() override;
	// 为每条连接创建专属节点
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	// 把新加入网络的 Actor 按策略路由到对应节点（复制图的核心分派函数）
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	// 从各节点移除该 Actor
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	// 始终对所有连接相关的类列表
	UPROPERTY()
	TArray<TObjectPtr<UClass>>	AlwaysRelevantClasses;
	
	// 2D 空间网格节点
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	// 始终相关节点
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	// 按流关（Streaming Level）分组的、始终相关的 Actor
	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

#if WITH_GAMEPLAY_DEBUGGER
	// GameplayDebugger 归属变化时把调试器 Actor 挪到对应连接
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	// 打印当前所有类的路由策略（调试用）
	void PrintRepNodePolicies();

private:
	// 把一个类的路由策略写进映射表
	void AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping);
	// 为一个类登记路由策略（会向上查找父类继承）
	void RegisterClassRepNodeMapping(UClass* Class);
	// 取一个类的路由策略
	EClassRepNodeMapping GetClassNodeMapping(UClass* Class) const;

	// 为一个类登记复制信息
	void RegisterClassReplicationInfo(UClass* Class);
	// 按需初始化一个类的复制信息，返回是否成功
	bool ConditionalInitClassReplicationInfo(UClass* Class, FClassReplicationInfo& ClassInfo);
	// 填充 FClassReplicationInfo（是否空间化会影响剔除距离等）
	void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool Spatialize) const;

	// 推导一个类的路由策略（结合配置与继承）
	EClassRepNodeMapping GetMappingPolicy(UClass* Class);

	// 是否属于「空间化」策略：枚举值 >= Spatialize_Static 的都算
	bool IsSpatialized(EClassRepNodeMapping Mapping) const { return Mapping >= EClassRepNodeMapping::Spatialize_Static; }

	// 类 -> 路由策略 的映射表
	TClassMap<EClassRepNodeMapping> ClassRepNodePolicies;

	// 由代码显式设置过复制策略的类（区别于从配置读来的）
	/** Classes that had their replication settings explictly set by code in ULyraReplicationGraph::InitGlobalActorClassSettings */
	TArray<UClass*> ExplicitlySetClasses;
};

// 连接专属的 AlwaysRelevant 节点：额外处理流关可见性
UCLASS()
class ULyraReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode_AlwaysRelevant_ForConnection
{
	GENERATED_BODY()

public:
	// 这几个通知被刻意留空：本节点自己维护列表，不走通用的增删流程
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	// 同上
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }
	// 同上
	virtual void NotifyResetAllNetworkActors() override { }

	// 为这条连接收集要复制的 Actor 列表
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// 输出调试信息
	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	// 客户端某个流关变为可见：把该流关里始终相关的 Actor 加入待复制队列
	void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);
	// 流关不再可见
	void OnClientLevelVisibilityRemove(FName LevelName);

	// 重置本节点状态
	void ResetGameWorldState();

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator* GameplayDebugger = nullptr;
#endif

private:
	// 待复制的流关名列表
	TArray<FName, TInlineAllocator<64> > AlwaysRelevantStreamingLevelsNeedingReplication;

	// PlayerState 是否已初始化（只处理一次）
	bool bInitializedPlayerState = false;
};

// PlayerState 专用限流节点：跟踪所有 PlayerState，但每帧只交一部分给复制驱动，
// 这是为大量连接做的优化，不是必需项
/** 
	This is a specialized node for handling PlayerState replication in a frequency limited fashion. It tracks all player states but only returns a subset of them to the replication driver each frame. 
	This is an optimization for large player connection counts, and not a requirement.
*/
// PlayerState 频率限制节点
UCLASS()
class ULyraReplicationGraphNode_PlayerStateFrequencyLimiter : public UReplicationGraphNode
{
	GENERATED_BODY()

	ULyraReplicationGraphNode_PlayerStateFrequencyLimiter();

	// 通知留空：本节点自己管理列表
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	// 同上
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }
	// 同上
	virtual bool NotifyActorRenamed(const FRenamedReplicatedActorInfo& Actor, bool bWarnIfNotFound=true) override { return false; }

	// 每帧只返回 TargetActorsPerFrame 个 PlayerState
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// 每帧复制前准备：轮转桶，决定本帧放出哪一批
	virtual void PrepareForReplication() override;

	// 输出调试信息
	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	// 每帧希望交给复制驱动的 Actor 数量；不会压制 ForceNetUpdate 的 Actor
	/** How many actors we want to return to the replication driver per frame. Will not suppress ForceNetUpdate. */
	int32 TargetActorsPerFrame = 2;

private:
	
	// 分桶后的各批 PlayerState 列表
	TArray<FActorRepListRefView> ReplicationActorLists;
	// 被 ForceNetUpdate 的 PlayerState，永远优先复制
	FActorRepListRefView ForceNetUpdateReplicationActorList;
};