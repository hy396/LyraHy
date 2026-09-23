// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的出生点：在标准 PlayerStart 上加了「占用状态」与 GameplayTag，
 * 让 SpawningManager 能按标签筛选、按占用状态避免挤在一起。
 */
#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"

#include "LyraPlayerStart.generated.h"

class AController;
class UObject;

// 出生点占用状态
enum class ELyraPlayerStartLocationOccupancy
{
	// 空
	Empty,
	// 部分占用
	Partial,
	// 已满
	Full
};

// 带占用状态与标签的出生点
/**
 * ALyraPlayerStart
 * 
 * Base player starts that can be used by a lot of modes.
 */
UCLASS(Config = Game)
class LYRAGAME_API ALyraPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ALyraPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取出生点的标签
	const FGameplayTagContainer& GetGameplayTags() { return StartPointTags; }

	// 取当前占用状态（按碰撞检测判断）
	ELyraPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	// 是否已被某个控制器占用
	/** Did this player start get claimed by a controller already? */
	bool IsClaimed() const;

	// 尝试占用；已被占用时返回 false
	/** If this PlayerStart was not claimed, claim it for ClaimingController */
	bool TryClaim(AController* OccupyingController);

protected:
	// 定期检查是否已无人占用，是则释放
	/** Check if this PlayerStart is still claimed */
	void CheckUnclaimed();

	// 占用该出生点的控制器
	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	// 检查间隔（秒）
	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	// 出生点的标签，用于筛选
	/** Tags to identify this player start */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	// 检查定时器句柄
	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;
};
