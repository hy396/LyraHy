// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的玩家状态基类。
 *
 * 它是 ASC 的「真正持有者」：PlayerState 上挂着 ASC 子组件，
 * 角色（Pawn）只是作为 AvatarActor 被注册进去。
 * 这样做的好处是：换角色（重生/换职业）时 ASC 与属性不会丢。
 * 同时它也管理队伍、小队、连接类型、统计标签栈与观战视角旋转。
 */
#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerState.generated.h"

struct FLyraVerbMessage;

class AController;
class ALyraPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceDefinition;
class ULyraPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

// 客户端连接类型
/** Defines the types of client connected */
UENUM()
// 连接类型枚举
enum class ELyraPlayerConnectionType : uint8
{
	// An active player
	// 活跃玩家
	Player = 0,

	// Spectator connected to a running game
	// 实时观战者
	LiveSpectator,

	// Spectating a demo recording offline
	// 回放观战者
	ReplaySpectator,

	// A deactivated player (disconnected)
	// 已断开的不活跃玩家
	InactivePlayer
};

// 本项目使用的玩家状态基类
/**
 * ALyraPlayerState
 *
 *	Base player state class used by this project.
 */
UCLASS(Config = Game)
class LYRAGAME_API ALyraPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:
	ALyraPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取 Lyra 的 PlayerController
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerState")
	ALyraPlayerController* GetLyraPlayerController() const;

	// 取 Lyra 的 ASC（直接返回缓存的组件）
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerState")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const { return AbilitySystemComponent; }
	// IAbilitySystemInterface：返回 ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 取 PawnData（模板，调用方指定类型）
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 设置 PawnData；设置后会给 ASC 授予对应能力集
	void SetPawnData(const ULyraPawnData* InPawnData);

	// AActor 接口
	//~AActor interface
	// 组件初始化前
	virtual void PreInitializeComponents() override;
	// 组件初始化后
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	// APlayerState 接口
	//~APlayerState interface
	// 重置
	virtual void Reset() override;
	// 客户端初始化
	virtual void ClientInitialize(AController* C) override;
	// 复制属性到另一个 PlayerState（无缝旅行时）
	virtual void CopyProperties(APlayerState* PlayerState) override;
	// 被反激活（断开连接）
	virtual void OnDeactivated() override;
	// 被重新激活
	virtual void OnReactivated() override;
	//~End of APlayerState interface

	// 队伍接口
	//~ILyraTeamAgentInterface interface
	// 设置队伍 ID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 取队伍 ID
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 取队伍变化委托
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	// 事件名常量：能力已就绪
	static const FName NAME_LyraAbilityReady;

	// 设置连接类型
	void SetPlayerConnectionType(ELyraPlayerConnectionType NewType);
	// 取连接类型
	ELyraPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	// 取小队 ID
	/** Returns the Squad ID of the squad the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	// 取队伍 ID（转成整数）
	/** Returns the Team ID of the team the player belongs to. */
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}

	// 设置小队 ID
	void SetSquadID(int32 NewSquadID);

	// 增加统计标签栈
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 减少统计标签栈
	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 取统计标签栈数量
	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Teams)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// 是否含有指定统计标签
	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Teams)
	bool HasStatTag(FGameplayTag Tag) const;

	// 不可靠客户端 RPC：向该玩家发送一条消息（成就、任务提示等，可丢）
	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Lyra|PlayerState")
	void ClientBroadcastMessage(const FLyraVerbMessage Message);

	// 取复制的视角旋转（用于观战）
	// Gets the replicated view rotation of this player, used for spectating
	FRotator GetReplicatedViewRotation() const;

	// 设置复制的视角旋转（仅服务端有效）
	// Sets the replicated view rotation, only valid on the server
	void SetReplicatedViewRotation(const FRotator& NewRotation);

// 体验加载完成回调：在这里按体验配置授予能力
private:
	void OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);

protected:
	// PawnData 复制到客户端
	UFUNCTION()
	void OnRep_PawnData();

protected:

	// PawnData（复制到客户端）
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const ULyraPawnData> PawnData;

private:

	// ASC 子组件，由玩家状态持有
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Lyra|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;

	// 生命值属性集
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraHealthSet> HealthSet;
	// 战斗属性集
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraCombatSet> CombatSet;

	// 连接类型（复制到客户端）
	UPROPERTY(Replicated)
	ELyraPlayerConnectionType MyPlayerConnectionType;

	// 队伍变化委托
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 队伍 ID（复制到客户端）
	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// 小队 ID（复制到客户端）
	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	// 统计标签栈（复制到客户端）
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	// 复制的视角旋转（复制到客户端）
	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;

private:
	// 队伍 ID 的 OnRep
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	// 小队 ID 的 OnRep
	UFUNCTION()
	void OnRep_MySquadID();
};
