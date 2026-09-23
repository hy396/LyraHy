// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的玩家控制器基类。
 *
 * 除了标准 APlayerController，它主要做：
 *   - 持有队伍 ID、自动奔跑状态、作弊命令转发；
 *   - 控制摄像机穿透目标时的辅助回调；
 *   - 客户端回放录制开关。
 */
#include "Camera/LyraCameraAssistInterface.h"
#include "CommonPlayerController.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPlayerController.generated.h"

struct FGenericTeamId;

class ALyraHUD;
class ALyraPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class ULyraAbilitySystemComponent;
class ULyraSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

// 本项目使用的玩家控制器基类
/**
 * ALyraPlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class LYRAGAME_API ALyraPlayerController : public ACommonPlayerController, public ILyraCameraAssistInterface, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:

	// 构造：默认设置
	ALyraPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取 Lyra 的 PlayerState
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	ALyraPlayerState* GetLyraPlayerState() const;

	// 取 Lyra 的 ASC
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;

	// 取 Lyra 的 HUD
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	ALyraHUD* GetLyraHUD() const;

	// 尝试开始录制客户端回放（若 ShouldRecordClientReplay 返回 true）
	// Call from game state logic to start recording an automatic client replay if ShouldRecordClientReplay returns true
	UFUNCTION(BlueprintCallable, Category = "Lyra|PlayerController")
	bool TryToRecordClientReplay();

	// 是否应录制客户端回放；子类可覆盖
	// Call to see if we should record a replay, subclasses could change this
	virtual bool ShouldRecordClientReplay();

	// 在服务端执行作弊命令（仅开发构建有效）
	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// 在服务端对所有玩家执行作弊命令
	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);

	// AActor 接口
	//~AActor interface
	// 组件初始化前
	virtual void PreInitializeComponents() override;
	// 开始
	virtual void BeginPlay() override;
	// 结束
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// 复制属性
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	// AController 接口
	//~AController interface
	// 占据 Pawn
	virtual void OnPossess(APawn* InPawn) override;
	// 释放 Pawn
	virtual void OnUnPossess() override;
	// 初始化 PlayerState
	virtual void InitPlayerState() override;
	// 清理 PlayerState
	virtual void CleanupPlayerState() override;
	// PlayerState 复制到客户端
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	// APlayerController 接口
	//~APlayerController interface
	// 玩家初始化完成
	virtual void ReceivedPlayer() override;
	// 每帧 Tick
	virtual void PlayerTick(float DeltaTime) override;
	// 设置玩家
	virtual void SetPlayer(UPlayer* InPlayer) override;
	// 添加作弊命令
	virtual void AddCheats(bool bForce) override;
	// 更新力反馈
	virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	// 更新隐藏组件（例如穿墙时隐藏阻挡摄像机视线的物体）
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	// 输入处理前
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	// 输入处理后
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	// 摄像机辅助接口：摄像机穿透目标时调用
	//~ILyraCameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override;
	//~End of ILyraCameraAssistInterface interface
	
	// 队伍接口
	//~ILyraTeamAgentInterface interface
	// 设置队伍 ID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 取队伍 ID
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 取队伍变化委托
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	// 设置自动奔跑
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	void SetIsAutoRunning(const bool bEnabled);

	// 是否正在自动奔跑
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	bool GetIsAutoRunning() const;

private:
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

private:
	// PlayerState 换队伍
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// PlayerState 设置或清除时调用
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	// 广播 PlayerState 变化
	void BroadcastOnPlayerStateChanged();

protected:

	//~APlayerController interface

	//~End of APlayerController interface

	// 共享设置变更回调
	void OnSettingsChanged(ULyraSettingsShared* Settings);
	
	// 开始自动奔跑
	void OnStartAutoRun();
	// 结束自动奔跑
	void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	void K2_OnEndAutoRun();

	// 下一帧是否需要隐藏被视角目标 Pawn（用于调试摄像机）
	bool bHideViewTargetPawnNextFrame = false;
};


// A player controller used for replay capture and playback
UCLASS()
class ALyraReplayPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
	virtual bool ShouldRecordClientReplay() override;

	// Callback for when the game state's RecorderPlayerState gets replicated during replay playback
	void RecorderPlayerStateUpdated(APlayerState* NewRecorderPlayerState);

	// Callback for when the followed player state changes pawn
	UFUNCTION()
	void OnPlayerStatePawnSet(APlayerState* ChangedPlayerState, APawn* NewPlayerPawn, APawn* OldPlayerPawn);

	// The player state we are currently following */
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> FollowedPlayerState;
};
