// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Bot（AI 玩家）控制器：继承 ModularAIController，实现队伍接口。
 * 它负责同步 Bot 的 PlayerState、队伍态度与 AI 感知系统。
 */
#include "ModularAIController.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerBotController.generated.h"

namespace ETeamAttitude { enum Type : int; }
struct FGenericTeamId;

class APlayerState;
class UAIPerceptionComponent;
class UObject;
struct FFrame;

// 本项目使用的 Bot 控制器
/**
 * ALyraPlayerBotController
 *
 *	The controller class used by player bots in this project.
 */
UCLASS(Blueprintable)
class ALyraPlayerBotController : public AModularAIController, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:
	// 构造
	ALyraPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 队伍接口
	//~ILyraTeamAgentInterface interface
	// 设置队伍 ID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 取队伍 ID
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 取队伍变化委托
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	// 对另一个 Actor 的态度（友好/中立/敌对）
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	//~End of ILyraTeamAgentInterface interface

	// 请求重生
	// Attempts to restart this controller (e.g., to respawn it)
	void ServerRestartController();

	// 更新 AI 感知系统的队伍态度
	//Update Team Attitude for the AI
	UFUNCTION(BlueprintCallable, Category = "Lyra AI Player Controller")
	void UpdateTeamAttitude(UAIPerceptionComponent* AIPerception);

	// 释放 Pawn
	virtual void OnUnPossess() override;


private:
	// PlayerState 换队伍
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// PlayerState 设置或清除时
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	// 广播 PlayerState 变化
	void BroadcastOnPlayerStateChanged();

protected:	
	// AController 接口
	//~AController interface
	// 初始化 PlayerState
	virtual void InitPlayerState() override;
	// 清理 PlayerState
	virtual void CleanupPlayerState() override;
	// PlayerState 复制到客户端
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

private:
	// 队伍变化委托
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 上次看到的 PlayerState
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;
};
