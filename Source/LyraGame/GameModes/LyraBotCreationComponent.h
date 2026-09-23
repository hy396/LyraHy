// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Bot（AI 玩家）创建组件：挂在 GameState 上，等体验加载完成后批量创建 AI 玩家。
 * 基类是 Abstract，实际用的子类由体验里的 GameFeature 提供（例如 ShooterCore 的 Bot 组件）。
 */
#include "Components/GameStateComponent.h"

#include "LyraBotCreationComponent.generated.h"

class ULyraExperienceDefinition;
class ULyraPawnData;
class AAIController;

// Bot 创建组件基类（抽象，蓝图可继承）
UCLASS(Blueprintable, Abstract)
class ULyraBotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	ULyraBotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 开始：订阅体验加载完成事件
	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	// 体验加载完成后才开始造 Bot
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

protected:
	// 要创建的 Bot 数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	int32 NumBotsToCreate = 5;

	// Bot 使用的 AI 控制器类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	TSubclassOf<AAIController> BotControllerClass;

	// 随机 Bot 名字池
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Gameplay)
	TArray<FString> RandomBotNames;

	// 还没被用掉的 Bot 名字，避免重名
	TArray<FString> RemainingBotNames;

protected:
	// 已生成的 Bot 控制器列表
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

	// 只创建一个 Bot
	/** Always creates a single bot */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Gameplay)
	virtual void SpawnOneBot();

	// 尽可能删除最近创建的一个 Bot
	/** Deletes the last created bot if possible */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Gameplay)
	virtual void RemoveOneBot();

	// 批量创建 Bot，直到数量达到 NumBotsToCreate
	/** Spawns bots up to NumBotsToCreate */
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category=Gameplay)
	void ServerCreateBots();

#if WITH_SERVER_CODE
public:
	// 作弊命令：加一个 Bot
	void Cheat_AddBot() { SpawnOneBot(); }
	// 作弊命令：删一个 Bot
	void Cheat_RemoveBot() { RemoveOneBot(); }

	// 按索引生成一个 Bot 名字
	FString CreateBotName(int32 PlayerIndex);
#endif
};
