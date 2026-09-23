// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 蓝图异步节点：等待「GameState 就绪 且 体验加载完成」。
 *
 * 这是 Lyra 里 UI 和游戏逻辑最常用的等待入口——因为很多系统（HUD、背包、武器）
 * 都必须等 Experience 加载完、能力集注入之后才能初始化。
 * 如果体验已经就绪，OnReady 会立刻触发，不会卡住。
 */
#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_ExperienceReady.generated.h"

class AGameStateBase;
class ULyraExperienceDefinition;
class UWorld;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

// 等待 GameState 有效且体验加载完毕的异步节点
/**
 * Asynchronously waits for the game state to be ready and valid and then calls the OnReady event.  Will call OnReady
 * immediately if the game state is valid already.
 */
UCLASS()
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	// 入口：创建一个等待节点；游戏状态已经就绪时立刻回调
	// Waits for the experience to be determined and loaded
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	// 节点激活：开始分步等待流程
	virtual void Activate() override;

public:

	// 体验已确定并加载完成时广播
	// Called when the experience has been determined and is ready/loaded
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;

private:
	// 第 1 步：等 GameState 出现
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	// 第 2 步：订阅 Experience 加载完成事件（若已加载则直接进入下一步）
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	// 第 3 步：体验加载完成的回调
	void Step3_HandleExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience);
	// 第 4 步：广播 OnReady
	void Step4_BroadcastReady();

	// 弱引用世界，避免异步等待期间世界已销毁导致悬空
	TWeakObjectPtr<UWorld> WorldPtr;
};
