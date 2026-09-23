// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Containers/Ticker.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

#include "GameSettingPanel.generated.h"

// Note: FCompiledToken used to be forward declared here, but UE 5.6 declares it as an alias
// (using FCompiledToken = TCompiledToken<TCHAR>) in Misc/ExpressionParserTypesFwd.h, so the
// forward declaration conflicts with it.

class UGameSetting;
class UGameSettingDetailView;
class UGameSettingListView;
class UGameSettingRegistry;
class UObject;
struct FFocusEvent;
struct FGeometry;

// 多选广播：当前"获得焦点/被悬停"的设置项发生变化时触发（用于右侧详情面板跟随刷新）。
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusedSettingChanged, UGameSetting*)

/**
 * UGameSettingPanel —— 设置面板：左侧列表 + 右侧详情的组合体。
 *
 * 它把注册表里的设置项按当前过滤状态筛成"可见列表"喂给 ListView，
 * 并监听悬停/选中来驱动右侧详情视图；同时还维护一个"导航栈"以支持二级页面进出。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UGameSettingPanel();
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Focus transitioning to subwidgets for the gamepad
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	/**  */
	void SetRegistry(UGameSettingRegistry* InRegistry);

	/** 设置过滤状态，限制当前可见的设置项。bClearNavigationStack 为真时同时清空导航栈。 */
	void SetFilterState(const FGameSettingFilterState& InFilterState, bool bClearNavigationStack = true);

	/** Gets the currently visible and available settings based on the filter state. */
	TArray<UGameSetting*> GetVisibleSettings() const { return VisibleSettings; }

	/** Can we pop the current navigation stack */
	bool CanPopNavigationStack() const;

	/** Pop the navigation stack */
	void PopNavigationStack();

	/** 取得本界面上"可能被重置"的设置项集合。
	 *  注意：结果【可能包含当前不可见的项】，且【不包含二级子页面里的项】。 */
	TArray<UGameSetting*> GetSettingsWeCanResetToDefault() const;

	void SelectSetting(const FName& SettingDevName);
	UGameSetting* GetSelectedSetting() const;

	void RefreshSettingsList();

	FOnFocusedSettingChanged OnFocusedSettingChanged;

protected:
	void RegisterRegistryEvents();
	void UnregisterRegistryEvents();
	
	void HandleSettingItemHoveredChanged(UObject* Item, bool bHovered);
	void HandleSettingItemSelectionChanged(UObject* Item);
	void FillSettingDetails(UGameSetting* InSetting);
	void HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag);
	void HandleSettingNavigation(UGameSetting* Setting);
	void HandleSettingEditConditionsChanged(UGameSetting* Setting);

private:

	UPROPERTY(Transient)
	TObjectPtr<UGameSettingRegistry> Registry;

	/** 当前过滤后可见的设置项列表。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> VisibleSettings;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> LastHoveredOrSelectedSetting;

	UPROPERTY(Transient)
	FGameSettingFilterState FilterState;

	/** 导航栈：每进入一层子页面就压一个过滤状态，退出时弹出。 */
	UPROPERTY(Transient)
	TArray<FGameSettingFilterState> FilterNavigationStack;

	FName DesiredSelectionPostRefresh;

	bool bAdjustListViewPostRefresh = true;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingListView> ListView_Settings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingDetailView> Details_Settings;

private:
	/** 蓝图版：某个"具名动作"被执行时广播（对应 UGameSettingAction 的 NamedAction）。 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecuteNamedActionBP, UGameSetting*, Setting, FGameplayTag, Action);
	UPROPERTY(BlueprintAssignable, Category = Events, meta = (DisplayName = "On Execute Named Action"))
	FOnExecuteNamedActionBP BP_OnExecuteNamedAction;

private:
	/** 列表刷新的延迟句柄：把多次刷新合并到下一帧执行一次，避免重复重建。 */
	FTSTicker::FDelegateHandle RefreshHandle;
};
