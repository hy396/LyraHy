// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的游戏模式基类。
 *
 * 它在标准 AGameMode 之上主要做了两件事：
 *   1) 决定并加载本局要用哪个 Experience（见 HandleMatchAssignmentIfNotExpectingOne 的优先级链）；
 *   2) 把玩家出生、Pawn 选择、重生的时机全部推迟到「体验加载完成之后」。
 */
#include "ModularGameMode.h"

#include "LyraGameMode.generated.h"

class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class ULyraExperienceDefinition;
class ULyraPawnData;
class UObject;
struct FFrame;
struct FPrimaryAssetId;
enum class ECommonSessionOnlineMode : uint8;

// 玩家（含 Bot）初始化完成时触发；无缝/非无缝 Travel 之后也会触发
/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLyraGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

// 本项目使用的游戏模式基类
/**
 * ALyraGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ALyraGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:

	ALyraGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取指定控制器应该使用的 Pawn 数据（通常来自 PlayerState 或体验默认值）
	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	const ULyraPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase interface
	// 初始化本局游戏：下一帧再去决定用哪个体验
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	// 取该控制器默认生成的 Pawn 类
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	// 在指定位置生成默认 Pawn
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	// 是否直接用 PlayerStart 上的 StartSpot 标签定位
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	// 新玩家加入时的处理入口
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	// 选择出生点：优先交给 LyraPlayerSpawningManagerComponent
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	// 玩家重生成完成
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	// 玩家能否重生（体验没加载完、或正在等待重生时返回 false）
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	// 初始化 GameState（会创建 ExperienceManagerComponent）
	virtual void InitGameState() override;
	// 传送门切换时更新出生点
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	// 通用的玩家初始化：在这里广播 OnGameModePlayerInitialized
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	// 重生失败时的处理
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~End of AGameModeBase interface

	// 请求下一帧重生指定玩家/Bot；bForceReset 为真会立刻放弃当前控制的 Pawn
	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// 与玩家类型无关的重生条件判断（玩家和 Bot 通用）
	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	virtual bool ControllerCanRestart(AController* Controller);

	// 玩家初始化完成委托，见上方 FOnLyraGameModePlayerInitialized 说明
	// Delegate called on player initialization, described above 
	FOnLyraGameModePlayerInitialized OnGameModePlayerInitialized;

protected:	
	// 体验加载完成回调：此时才允许玩家出生
	void OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);
	// 当前体验是否已完全加载
	bool IsExperienceLoaded() const;

	// 拿到「比赛分配」的体验 ID 后调用，正式开始加载体验
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

	// 没有外部指定体验时，按优先级链自己挑一个
	void HandleMatchAssignmentIfNotExpectingOne();

	// 专用服务器尝试做一次在线登录；返回 true 表示已进入专用服务器开房流程
	bool TryDedicatedServerLogin();
	// 以专用服务器身份开一场比赛
	void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode);

	UFUNCTION()
	// 专用服务器在线登录完成的回调
	void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);
};
