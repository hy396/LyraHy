// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/TextFilterExpressionEvaluator.h"

#include "UObject/ObjectPtr.h"
#include "GameSettingFilterState.generated.h"

class ULocalPlayer;
class UGameSetting;
class UGameSettingCollection;

/** 设置项为什么会变化？区分类别是为了让上层 UI 决定要不要提示、要不要存盘等。 */
enum class EGameSettingChangeReason : uint8
{
	/** 用户直接改了值。 */
	Change,

	/** 它依赖的另一个设置项变了，导致它被连带改变。 */
	DependencyChanged,

	/** 被重置为默认值。 */
	ResetToDefault,

	/** 被还原为打开设置界面时的初始值（相当于撤销本次所有改动）。 */
	RestoreToInitial,
};

/**
 * FGameSettingFilterState —— 过滤状态：决定设置界面当前该显示哪些项。
 * 它同时管三件事：搜索文字、要不要包含隐藏/禁用项、以及可选的"白名单/根列表"。
 */
USTRUCT()
struct GAMESETTINGS_API FGameSettingFilterState
{
	GENERATED_BODY()

public:

	FGameSettingFilterState();

	UPROPERTY()
	bool bIncludeDisabled = true;

	UPROPERTY()
	bool bIncludeHidden = false;

	UPROPERTY()
	bool bIncludeResetable = true;

	UPROPERTY()
	bool bIncludeNestedPages = false;

public:
	/** 设置搜索关键字，交给内部的文本过滤表达式求值器处理。 */
	void SetSearchText(const FString& InSearchText);

	/** 判断某个设置项是否通过了当前过滤条件（可见性 + 搜索 + 白名单）。 */
	bool DoesSettingPassFilter(const UGameSetting& InSetting) const;

	void AddSettingToRootList(UGameSetting* InSetting);
	void AddSettingToAllowList(UGameSetting* InSetting);

	bool IsSettingInAllowList(const UGameSetting* InSetting) const
	{
		return SettingAllowList.Contains(InSetting);
	}
	
	const TArray<UGameSetting*>& GetSettingRootList() const { return SettingRootList; }
	bool IsSettingInRootList(const UGameSetting* InSetting) const
	{
		return SettingRootList.Contains(InSetting);
	}

private:
	FTextFilterExpressionEvaluator SearchTextEvaluator;

	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingRootList;

	/** 白名单：非空时，只有列在这里的设置项才允许通过过滤。 */
	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingAllowList;
};

/**
 * FGameSettingEditableState —— 某个设置项"当前能不能被用户动"的状态快照。
 *
 * 不只是一个 bool，而是同时记录原因，这样 UI 可以直接把"为什么禁用"显示给用户。
 * 它由各个 EditCondition 在 GatherEditState 里往里塞，最后由 UGameSetting 缓存起来。
 */
class GAMESETTINGS_API FGameSettingEditableState
{
public:
	FGameSettingEditableState()
		: bVisible(true)
		, bEnabled(true)
		, bResetable(true)
		, bHideFromAnalytics(false)
	{
	}

	bool IsVisible() const { return bVisible; }
	bool IsEnabled() const { return bEnabled; }
	bool IsResetable() const { return bResetable; }
	bool IsHiddenFromAnalytics() const { return bHideFromAnalytics; }
	const TArray<FText>& GetDisabledReasons() const { return DisabledReasons; }

#if !UE_BUILD_SHIPPING
	const TArray<FString>& GetHiddenReasons() const { return HiddenReasons; }
#endif

	const TArray<FString>& GetDisabledOptions() const { return DisabledOptions; }

	/** 隐藏该项。不必给玩家看的理由，但必须填一个给开发者看的理由（便于排查）。 */
	void Hide(const FString& DevReason);

	/** 禁用该项（仍可见但灰掉）。必须给出面向玩家的理由文本。 */
	void Disable(const FText& Reason);

	/** 只隐藏离散选项中的某一个（例如家长控制禁止了"成人内容"这一档）。 */
	void DisableOption(const FString& Option);

	template<typename EnumType>
	void DisableEnumOption(EnumType InEnumValue)
	{
		DisableOption(StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue));
	}

	/**
	 * Prevents the setting from being reset if the user resets the settings on the screen to their defaults.
	 */
	void UnableToReset();

	/**
	 * Hide from analytics, you may want to do this if for example, we just want to prevent noise, such as platform
	 * specific edit conditions where it doesn't make sense to report settings for platforms where they don't exist.
	 */
	void HideFromAnalytics() { bHideFromAnalytics = true; }

	/** 彻底"杀掉"该项：既隐藏、又标记为不可重置、还排除出数据分析上报。
	 *  用于"在当前平台/当前账号下这项根本不该存在"的场景。 */
	void Kill(const FString& DevReason)
	{
		Hide(DevReason);
		HideFromAnalytics();
		UnableToReset();
	}

private:
	uint8 bVisible : 1;
	uint8 bEnabled : 1;
	uint8 bResetable : 1;
	uint8 bHideFromAnalytics : 1;

	TArray<FString> DisabledOptions;

	TArray<FText> DisabledReasons;

#if !UE_BUILD_SHIPPING
	TArray<FString> HiddenReasons;
#endif
};

/**
 * FGameSettingEditCondition —— 编辑条件：动态决定某个设置项的可见性与可编辑性。
 *
 * 它可以监视游戏状态、平台特性、玩家身份，甚至别的设置项的值。
 * 每当需要重新评估时，引擎会调用 GatherEditState，由它往 InOutEditState 里写结论。
 */
class GAMESETTINGS_API FGameSettingEditCondition : public TSharedFromThis<FGameSettingEditCondition>
{
public:
	FGameSettingEditCondition() { }
	virtual ~FGameSettingEditCondition() { }

	DECLARE_EVENT_OneParam(FGameSettingEditCondition, FOnEditConditionChanged, bool);
	FOnEditConditionChanged OnEditConditionChangedEvent;

	/** 主动广播"我这个条件变了"，迫使宿主设置项重新评估编辑状态。
	 *  条件依赖的是外部异步状态（比如平台特性刚查询完）时用这个。 */
	void BroadcastEditConditionChanged()
	{
		OnEditConditionChangedEvent.Broadcast(true);
	}

	/** 设置项初始化时调用，条件可以在此绑定监听、预取状态。 */
	virtual void Initialize(const ULocalPlayer* InLocalPlayer)
	{
	}

	/** Called when the setting is 'applied'. */
	virtual void SettingApplied(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting) const
	{
	}

	/** Called when the setting is changed. */
	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting, EGameSettingChangeReason Reason) const
	{
	}

	/**
	 * Called when the setting needs to re-evaluate edit state. Usually this is in response to a 
	 * dependency changing, or if this edit condition emits an OnEditConditionChangedEvent.
	 */
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
	{
	}

	/** 生成便于调试的文本描述。当"这项为什么没显示"时靠它排查。 */
	virtual FString ToString() const { return TEXT(""); }
};
