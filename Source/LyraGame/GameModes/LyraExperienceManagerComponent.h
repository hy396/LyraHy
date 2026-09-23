// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 体验管理组件：挂在 GameState 上，是 Lyra 整个「体验加载」流程的中枢。
 *
 * 完整流水线见 ELyraExperienceLoadState：
 *   Unloaded -> Loading(加载资产) -> LoadingGameFeatures(加载并激活插件)
 *   -> [LoadingChaosTestingDelay] -> ExecutingActions(执行动作) -> Loaded(广播) -> Deactivating
 *
 * 它还实现 ILoadingProcessInterface：加载没完成时返回 true，让 CommonLoadingScreen 保持显示。
 */
#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"

#include "LyraExperienceManagerComponent.generated.h"

namespace UE::GameFeatures { struct FResult; }

class ULyraExperienceDefinition;

// 体验加载完成委托，参数就是加载好的体验定义
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLyraExperienceLoaded, const ULyraExperienceDefinition* /*Experience*/);

// 体验加载状态机
enum class ELyraExperienceLoadState
{
	// 尚未加载
	Unloaded,
	// 正在加载体验相关资产
	Loading,
	// 正在加载并激活 GameFeature 插件
	LoadingGameFeatures,
	// 混沌测试用的随机延迟（lyra.chaos.ExperienceDelay 控制），正常构建不会经过
	LoadingChaosTestingDelay,
	// 正在执行体验里配置的各个 UGameFeatureAction
	ExecutingActions,
	// 已完全加载，可以开始游戏
	Loaded,
	// 正在反激活（清理已执行的动作）
	Deactivating
};

// 体验管理组件：加载体验、激活插件、执行动作、广播完成
UCLASS()
class ULyraExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	// 构造：开启网络复制
	ULyraExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	// ILoadingProcessInterface：体验没加载完时返回 true，让加载界面保持显示
	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

	// 设置并加载指定的体验（服务端权威，客户端通过 OnRep 跟随）
	// Tries to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// 注册「体验加载完成」回调——高优先级，比普通回调先触发（给需要提前搭环境的子系统用）
	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// 注册「体验加载完成」回调——普通优先级
	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// 注册「体验加载完成」回调——低优先级，最后触发
	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	// 取当前体验；若尚未加载完会直接断言失败，调用前先用 IsExperienceLoaded 判断
	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	const ULyraExperienceDefinition* GetCurrentExperienceChecked() const;

	// 体验是否已完全加载
	// Returns true if the experience is fully loaded
	bool IsExperienceLoaded() const;

private:
	// 客户端收到服务端复制下来的体验 ID 后开始加载
	UFUNCTION()
	void OnRep_CurrentExperience();

	// 第 1 步：异步加载体验及其动作集的资产包
	void StartExperienceLoad();
	// 第 2 步：资产加载完，收集并加载要激活的 GameFeature 插件
	void OnExperienceLoadComplete();
	// 单个插件加载完成的回调，全部完成才进入下一步
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	// 第 3 步：插件就绪，按序执行所有 UGameFeatureAction，然后按优先级广播完成
	void OnExperienceFullLoadCompleted();

	// 单个动作反激活完成
	void OnActionDeactivationCompleted();
	// 所有动作都反激活完成，可以真正卸载了
	void OnAllActionsDeactivated();

private:
	// 当前体验（复制到客户端）
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;

	// 当前加载状态
	ELyraExperienceLoadState LoadState = ELyraExperienceLoadState::Unloaded;

	// 还在加载中的插件计数
	int32 NumGameFeaturePluginsLoading = 0;
	// 本次体验要激活的插件 URL 列表
	TArray<FString> GameFeaturePluginURLs;

	// 已观察到的「暂停者」数量
	int32 NumObservedPausers = 0;
	// 预期的「暂停者」数量
	int32 NumExpectedPausers = 0;

	// 高优先级完成委托（最先广播，供需要提前准备的子系统使用）
	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnLyraExperienceLoaded OnExperienceLoaded_HighPriority;

	// 普通优先级完成委托
	/** Delegate called when the experience has finished loading */
	FOnLyraExperienceLoaded OnExperienceLoaded;

	// 低优先级完成委托（最后广播）
	/** Delegate called when the experience has finished loading */
	FOnLyraExperienceLoaded OnExperienceLoaded_LowPriority;
};
