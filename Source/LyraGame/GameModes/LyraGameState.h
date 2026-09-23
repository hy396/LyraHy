// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的游戏状态基类。
 *
 * 除了标准 GameState，它额外挂了两个组件：
 *   - ULyraExperienceManagerComponent：负责加载与管理本局的体验；
 *   - ULyraAbilitySystemComponent：全局 ASC，主要用于播放与具体角色无关的 GameplayCue。
 */
#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "LyraGameState.generated.h"

struct FLyraVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class ULyraExperienceManagerComponent;
class UObject;
struct FFrame;

// 本项目使用的游戏状态基类
/**
 * ALyraGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class LYRAGAME_API ALyraGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	ALyraGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 组件初始化前：创建 ExperienceManagerComponent 等子组件
	//~AActor interface
	// 组件初始化后
	virtual void PreInitializeComponents() override;
	// 结束时清理
	virtual void PostInitializeComponents() override;
	// 每帧：统计并更新服务端 FPS
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	//~End of AActor interface

	// 玩家状态加入
	//~AGameStateBase interface
	// 玩家状态移除
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	// 无缝旅行过渡检查点
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	//~End of AGameStateBase interface

	// IAbilitySystemInterface：返回全局 ASC
	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	// 取全局 ASC（用于游戏级 GameplayCue，例如全场播报）
	// Gets the ability system component used for game wide things
	UFUNCTION(BlueprintCallable, Category = "Lyra|GameState")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const { return AbilitySystemComponent; }

	// 不可靠多播：给所有客户端发一条消息（击杀播报、加入提示等，丢了也没关系）
	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Lyra|GameState")
	void MulticastMessageToClients(const FLyraVerbMessage Message);

	// 可靠多播：给所有客户端发一条消息（不能丢的场景才用）
	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Lyra|GameState")
	void MulticastReliableMessageToClients(const FLyraVerbMessage Message);

	// 取服务端 FPS（已复制到客户端）
	// Gets the server's FPS, replicated to clients
	float GetServerFPS() const;

	// 标记哪个 PlayerState 正在录制回放
	// Indicate the local player state is recording a replay
	void SetRecorderPlayerState(APlayerState* NewPlayerState);

	// 取录制回放的 PlayerState，无效时返回 nullptr
	// Gets the player state that recorded the replay, if valid
	APlayerState* GetRecorderPlayerState() const;

	// 录制回放的 PlayerState 变化时触发
	// Delegate called when the replay player state changes
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRecorderPlayerStateChanged, APlayerState*);
	FOnRecorderPlayerStateChanged OnRecorderPlayerStateChangedEvent;

private:
	// 负责加载与管理当前体验的组件
	// Handles loading and managing the current gameplay experience
	UPROPERTY()
	TObjectPtr<ULyraExperienceManagerComponent> ExperienceManagerComponent;

	// 全局 ASC 子组件，主要用于游戏级别的 GameplayCue
	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "Lyra|GameState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;

protected:
	// 服务端 FPS，复制到客户端
	UPROPERTY(Replicated)
	float ServerFPS;

	// 录制回放的 PlayerState；只在回放流里设置，正常运行不复制
	// The player state that recorded a replay, it is used to select the right pawn to follow
	// This is only set in replay streams and is not replicated normally
	UPROPERTY(Transient, ReplicatedUsing = OnRep_RecorderPlayerState)
	TObjectPtr<APlayerState> RecorderPlayerState;

	// RecorderPlayerState 的 OnRep 回调
	UFUNCTION()
	void OnRep_RecorderPlayerState();

};
