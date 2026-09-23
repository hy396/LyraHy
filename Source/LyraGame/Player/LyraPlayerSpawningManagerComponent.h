// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 玩家出生管理组件：挂在 GameState 上，接管 LyraGameMode 的出生点选择逻辑。
 *
 * 它维护一份缓存的 PlayerStart 列表，按「未占用 -> 随机 -> 距离最近」策略选点，
 * 并处理重生完成后的回调。每个体验可以通过覆盖虚函数来自定义重生规则。
 */
#include "Components/GameStateComponent.h"

#include "LyraPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class ALyraPlayerStart;
class AActor;

// 玩家出生管理组件
/**
 * @class ULyraPlayerSpawningManagerComponent
 */
UCLASS()
class LYRAGAME_API ULyraPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// 构造：默认设置
	ULyraPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	// 组件初始化：缓存当前地图上的所有 PlayerStart
	/** UActorComponent */
	virtual void InitializeComponent() override;
	// 每帧：检查是否有未占用的出生点需要释放
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */

protected:
	// Utility
	// 在未占用的出生点里随机挑一个
	APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<ALyraPlayerStart*>& FoundStartPoints) const;
	
	// 子类可覆盖：自定义选点逻辑
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts) { return nullptr; }
	// 子类可覆盖：重生完成后的回调
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	// 蓝图可实现的版本
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

	// 由 LyraGameMode 代理调用：选出生点
	/** We proxy these calls from ALyraGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	AActor* ChoosePlayerStart(AController* Player);
	// 由 LyraGameMode 代理调用：该控制器能否重生
	bool ControllerCanRestart(AController* Player);
	// 由 LyraGameMode 代理调用：重生完成
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class ALyraGameMode;
	/** ~ALyraGameMode */

	// 缓存的 PlayerStart 列表
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ALyraPlayerStart>> CachedPlayerStarts;

private:
	// 关卡动态加载时更新缓存
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	// 新 Actor 生成时检查是否是 PlayerStart
	void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	// 编辑器下：找 PlayFromHere 出生点
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};
