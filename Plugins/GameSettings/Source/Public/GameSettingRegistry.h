// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"
#include "Templates/Casts.h"

#include "GameSettingRegistry.generated.h"

struct FGameplayTag;

//--------------------------------------
// UGameSettingRegistry
//--------------------------------------

class ULocalPlayer;
struct FGameSettingFilterState;

enum class EGameSettingChangeReason : uint8;

/**
 * UGameSettingRegistry —— 设置注册表：某一类设置界面的"总清单"。
 *
 * 它是所有设置项的根节点，负责：持有顶层设置项、按过滤条件产出可见列表、
 * 转发各项的变更事件给 UI，并在保存时把改动统一落地。
 * 项目里通常每种设置界面配一个子类（如 LyraGameSettingRegistry_Video）。
 */
UCLASS(Abstract, BlueprintType)
class GAMESETTINGS_API UGameSettingRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** 任意一个受管设置项发生变更时广播（UI 据此显示"有未保存改动"）。 */
	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingChanged, UGameSetting*, EGameSettingChangeReason);

	/** 任意一个受管设置项的编辑条件变化时广播。 */
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnSettingEditConditionChanged, UGameSetting*);

	FOnSettingChanged OnSettingChangedEvent;
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;

	DECLARE_EVENT_TwoParams(UGameSettingRegistry, FOnSettingNamedActionEvent, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnSettingNamedActionEvent OnSettingNamedActionEvent;

	/** 请求"导航进入"某个设置项的子项时广播（用于打开二级设置页面）。 */
	DECLARE_EVENT_OneParam(UGameSettingRegistry, FOnExecuteNavigation, UGameSetting* /*Setting*/);
	FOnExecuteNavigation OnExecuteNavigationEvent;

public:
	UGameSettingRegistry();

	void Initialize(ULocalPlayer* InLocalPlayer);

	virtual void Regenerate();

	virtual bool IsFinishedInitializing() const;

	virtual void SaveChanges();
	
	void GetSettingsForFilter(const FGameSettingFilterState& FilterState, TArray<UGameSetting*>& InOutSettings);

	UGameSetting* FindSettingByDevName(const FName& SettingDevName);

	template<typename T = UGameSetting>
	T* FindSettingByDevNameChecked(const FName& SettingDevName)
	{
		T* Setting = Cast<T>(FindSettingByDevName(SettingDevName));
		check(Setting);
		return Setting;
	}

protected:
	/** 子类在这里构建整棵设置树：创建各项、设置编辑条件与依赖、注册进来。 */
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) PURE_VIRTUAL(, )

	virtual void OnSettingApplied(UGameSetting* Setting) { }
	
	/** 注册一个顶层设置项，并递归注册它的所有子项。 */
	void RegisterSetting(UGameSetting* InSetting);

	/** 递归注册某个设置项的内部子项（含 Action 内部隐藏的设置项）。 */
	void RegisterInnerSettings(UGameSetting* InSetting);

	// Internal event handlers.
	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);
	void HandleSettingApplied(UGameSetting* Setting);
	void HandleSettingEditConditionsChanged(UGameSetting* Setting);
	void HandleSettingNamedAction(UGameSetting* Setting, FGameplayTag GameSettings_Action_Tag);
	void HandleSettingNavigation(UGameSetting* Setting);

	/** 顶层设置项（设置界面的第一级列表）。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> TopLevelSettings;

	/** 所有已注册的设置项（含各级子项），用于按 DevName 查找与统一遍历。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameSetting>> RegisteredSettings;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;
};
