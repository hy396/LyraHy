// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "UObject/WeakInterfacePtr.h"

#include "LoadingScreenManager.generated.h"

template <typename InterfaceType> class TScriptInterface;

class FSubsystemCollectionBase;
class IInputProcessor;
class ILoadingProcessInterface;
class SWidget;
class UObject;
class UWorld;
struct FFrame;
struct FWorldContext;

/**
 * ULoadingScreenManager —— 加载界面的总控（GameInstance 子系统 + 可 Tick 对象）。
 *
 * 它【每帧】问一遍"现在该显示加载界面吗"，然后据此显示或隐藏。
 * 判断依据有两类：
 *   1) 引擎层面：是否正处于 PreLoadMap → PostLoadMap 之间（切关卡）
 *   2) 游戏层面：有没有任何"加载处理器"（ILoadingProcessInterface）说自己还在忙
 *
 * 这种"每帧轮询 + 谁都能举手说我在忙"的设计，让任意系统都能临时挂起加载界面，
 * 而不必互相知道对方的存在。
 */
UCLASS()
class COMMONLOADINGSCREEN_API ULoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

	/** 【调试用】取当前"为什么显示/不显示加载界面"的原因文字。 */
	UFUNCTION(BlueprintCallable, Category=LoadingScreen)
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const
	{
		return DebugReasonForShowingOrHidingLoadingScreen;
	}

	/** 加载界面当前是否正在显示。 */
	bool GetLoadingScreenDisplayStatus() const
	{
		return bCurrentlyShowingLoadingScreen;
	}

	/** 加载界面显示/隐藏状态发生变化时广播（参数 true=显示，false=隐藏）。 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	/** 注册一个"加载处理器"：只要它说自己还在忙，加载界面就不会消失。 */
	void RegisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface);

	/** 注销一个加载处理器。任务完成后必须调用，否则加载界面会一直挂着。 */
	void UnregisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface);
	
private:
	/** 引擎开始加载地图 —— 置位"正在切关卡"，之后加载界面会被强制显示。 */
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);

	/** 地图加载完成 —— 清掉"正在切关卡"标记。 */
	void HandlePostLoadMap(UWorld* World);

	/** 每帧调用：比对"当前是否在显示"与"是否应该显示"，不一致就切换。 */
	void UpdateLoadingScreen();

	/** 询问所有加载处理器：有谁还在忙吗？同时把原因写进调试字符串。 */
	bool CheckForAnyNeedToShowLoadingScreen();

	/** 最终决定：把"是否真的需要"和"是否被强行要求显示（调试开关）"合并起来。 */
	bool ShouldShowLoadingScreen();

	/** 是否正处于"引擎启动加载"阶段——那时由 PreLoadScreen 负责，本界面不该插手。 */
	bool IsShowingInitialLoadingScreen() const;

	/** 显示加载界面：创建控件、加到视口、开始屏蔽输入、调整性能设置。 */
	void ShowLoadingScreen();

	/** 隐藏加载界面：销毁控件、恢复输入、恢复性能设置。 */
	void HideLoadingScreen();

	/** Removes the widget from the viewport */
	void RemoveWidgetFromViewport();

	/** 屏蔽玩家输入：加载期间不应让玩家操作游戏。 */
	void StartBlockingInput();

	/** 恢复玩家输入。 */
	void StopBlockingInput();

	/** 加载期间临时放宽性能限制（如提高线程优先级）以加快加载，结束后恢复。 */
	void ChangePerformanceSettings(bool bEnabingLoadingScreen);

private:
	/** Delegate broadcast when the loading screen visibility changes */
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	/** 当前正在显示的加载界面控件（没有则为 nullptr）。 */
	TSharedPtr<SWidget> LoadingScreenWidget;

	/** 加载期间用来吃掉所有输入的前置处理器。 */
	TSharedPtr<IInputProcessor> InputPreProcessor;

	/** 外部注册的加载处理器列表（弱接口指针，宿主销毁后自动失效）。 */
	TArray<TWeakInterfacePtr<ILoadingProcessInterface>> ExternalLoadingProcessors;

	/** The reason why the loading screen is up (or not) */
	FString DebugReasonForShowingOrHidingLoadingScreen;

	/** 加载界面开始显示的时刻。 */
	double TimeLoadingScreenShown = 0.0;

	/** 最近一次"想要关闭"的时刻。可能因为还没到最短显示时长而实际上仍开着。 */
	double TimeLoadingScreenLastDismissed = -1.0;

	/** 距离下一次"心跳日志"还有多久（用于排查加载界面卡住）。 */
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	/** 是否正处于 PreLoadMap 与 PostLoadMap 之间（正在切关卡）。 */
	bool bCurrentlyInLoadMap = false;

	/** 加载界面当前是否正在显示。 */
	bool bCurrentlyShowingLoadingScreen = false;
};
