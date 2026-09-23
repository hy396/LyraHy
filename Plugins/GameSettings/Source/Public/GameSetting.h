// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Components/SlateWrapperTypes.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

#include "GameSetting.generated.h"

class ULocalPlayer;
class UGameSettingRegistry;

//--------------------------------------
// UGameSetting
//--------------------------------------

// 动态详情回调：构建详情面板时会被调用一次，用来生成那段"随时变化"的说明文字
//（例如剩余退款次数、账号 ID）。注意这段文本不参与搜索。
DECLARE_DELEGATE_RetVal_OneParam(FText, FGetGameSettingsDetails, ULocalPlayer& /*InLocalPlayer*/);

/**
 * UGameSetting —— 所有设置项的基类。
 *
 * 一个设置项 = 一条可在设置界面里显示的条目。它本身只是"描述 + 状态"，
 * 真正带数值的由子类 UGameSettingValue 系列承载，纯按钮类的由 UGameSettingAction 承载，
 * 分组用的由 UGameSettingCollection 承载。
 *
 * 三个核心机制：
 *   1) 编辑条件（EditCondition）—— 决定本项当前是否可见 / 可编辑，以及不可用的原因；
 *   2) 依赖（EditDependency）—— 别的设置项变了，本项要重新评估自己的编辑条件；
 *   3) 变更广播（OnSettingChangedEvent / OnSettingAppliedEvent）—— 让 UI 跟着刷新。
 */
UCLASS(Abstract, BlueprintType)
class GAMESETTINGS_API UGameSetting : public UObject
{
	GENERATED_BODY()

public:
	UGameSetting() { }

public:
	/** 本设置项的值发生变化时广播（参数带变更原因：改值 / 依赖变化 / 重置默认 / 还原初始）。 */
	DECLARE_EVENT_TwoParams(UGameSetting, FOnSettingChanged, UGameSetting* /*InSetting*/, EGameSettingChangeReason /*InChangeReason*/);

	/** 本设置项被"应用"（真正落地生效）时广播。 */
	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingApplied, UGameSetting* /*InSetting*/);

	/** 本设置项的编辑条件发生变化时广播：可能是变为不可见、变为禁用，或者可选项变了。 */
	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingEditConditionChanged, UGameSetting* /*InSetting*/);

	FOnSettingChanged OnSettingChangedEvent;
	FOnSettingApplied OnSettingAppliedEvent;
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;

public:

	/**
	 * Gets the non-localized developer name for this setting.  This should remain constant, and represent a 
	 * unique identifier for this setting inside this settings registry.
	 */
	UFUNCTION(BlueprintCallable)
	FName GetDevName() const { return DevName; }
	void SetDevName(const FName& Value) { DevName = Value; }

	bool GetAdjustListViewPostRefresh() const { return bAdjustListViewPostRefresh; }
	void SetAdjustListViewPostRefresh(const bool Value) { bAdjustListViewPostRefresh = Value; }

	UFUNCTION(BlueprintCallable)
	FText GetDisplayName() const { return DisplayName; }
	void SetDisplayName(const FText& Value) { DisplayName = Value; }
#if !UE_BUILD_SHIPPING
	void SetDisplayName(const FString& Value) { SetDisplayName(FText::FromString(Value)); }
#endif
	UFUNCTION(BlueprintCallable)
	ESlateVisibility GetDisplayNameVisibility() { return DisplayNameVisibility; }
	void SetNameDisplayVisibility(ESlateVisibility InVisibility) { DisplayNameVisibility = InVisibility; }

	UFUNCTION(BlueprintCallable)
	FText GetDescriptionRichText() const { return DescriptionRichText; }
	void SetDescriptionRichText(const FText& Value) { DescriptionRichText = Value; InvalidateSearchableText(); }
#if !UE_BUILD_SHIPPING
	/** This version is for cheats and other non-shipping items, that don't need to localize their text.  We don't permit this in shipping to prevent unlocalized text being introduced. */
	void SetDescriptionRichText(const FString& Value) { SetDescriptionRichText(FText::FromString(Value)); }
#endif

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetTags() const { return Tags; }
	void AddTag(const FGameplayTag& TagToAdd) { Tags.AddTag(TagToAdd); }

	void SetRegistry(UGameSettingRegistry* InOwningRegistry) { OwningRegistry = InOwningRegistry; }

	/** Gets the searchable plain text for the description. */
	const FString& GetDescriptionPlainText() const;

	/** Initializes the setting, giving it the owning local player.  Containers automatically initialize settings added to them. */
	void Initialize(ULocalPlayer* InLocalPlayer);

	/** Gets the owning local player for this setting - which all initialized settings will have. */
	ULocalPlayer* GetOwningLocalPlayer() const { return LocalPlayer; }
	
	/** Set the dynamic details callback, we query this when building the description panel.  This text is not searchable.*/
	void SetDynamicDetails(const FGetGameSettingsDetails& InDynamicDetails) { DynamicDetails = InDynamicDetails; }

	/**
	 * Gets the dynamic details about this setting.  This may be information like, how many refunds are remaining 
	 * on their account, or the account number.
	 */
	UFUNCTION(BlueprintCallable)
	FText GetDynamicDetails() const;

	UFUNCTION(BlueprintCallable)
	FText GetWarningRichText() const { return WarningRichText; }
	void SetWarningRichText(const FText& Value) { WarningRichText = Value; InvalidateSearchableText(); }
#if !UE_BUILD_SHIPPING
	/** This version is for cheats and other non-shipping items, that don't need to localize their text.  We don't permit this in shipping to prevent unlocalized text being introduced. */
	void SetWarningRichText(const FString& Value) { SetWarningRichText(FText::FromString(Value)); }
#endif

	/**
	 * Gets the edit state of this property based on the current state of its edit conditions as well as any additional
	 * filter state.
	 */
	const FGameSettingEditableState& GetEditState() const { return EditableStateCache; }

	/** 添加一个"编辑条件"：它能在运行时决定本设置项是否可见、是否可编辑。
	 *  例如"只有主玩家才能改""只有支持该特性的平台才显示"。 */
	void AddEditCondition(const TSharedRef<FGameSettingEditCondition>& InEditCondition);

	/** 声明依赖：这些设置项一变化，本项就会重新评估自己的编辑条件。
	 *  典型用法："分辨率"变了 → "刷新率"的可选项要跟着变。 */
	void AddEditDependency(UGameSetting* DependencySetting);

	/** The parent object that owns the setting, in most cases the collection, but for top level settings the registry. */
	void SetSettingParent(UGameSetting* InSettingParent);
	UGameSetting* GetSettingParent() const { return SettingParent; }

	/** 是否把本设置项上报到数据分析（默认否，只有 GameSettingValue 系列默认会上报）。 */
	bool GetIsReportedToAnalytics() const { return bReportAnalytics; }
	void SetIsReportedToAnalytics(bool bReport) { bReportAnalytics = bReport; }

	/** Gets the analytics value for this setting. */
	virtual FString GetAnalyticsValue() const { return TEXT(""); }

	/**
	 * Some settings may take an async amount of time to finish initializing.  The settings system will wait
	 * for all settings to be ready before showing the setting.
	 */
	bool IsReady() const { return bReady; }

	/**
	 * Any setting can have children, this is so we can allow for the possibility of "collections" or "actions" that
	 * are not directly visible to the user, but are set by some means and need to have initial and restored values.
	 * In that case, you would likely have internal settings inside an action subclass that is set on another screen,
	 * but never directly listed on the settings panel.
	 */
	virtual TArray<UGameSetting*> GetChildSettings() { return TArray<UGameSetting*>(); }

	/**
	 * Refresh the editable state of the setting and notify that the state has changed so that any UI currently
	 * examining this setting is updated with the new options, or whatever.
	 */
	void RefreshEditableState(bool bNotifyEditConditionsChanged = true);

	/**
	 * We expect settings to change the live value immediately, but occasionally there are special settings
	 * that go are immediately stored to a temporary location but we don't actually apply them until later
	 * like selecting a new resolution.
	 */
	void Apply();

	/** Gets the current world of the local player that owns these settings. */
	virtual UWorld* GetWorld() const override;

protected:
	/**  */
	virtual void Startup();
	void StartupComplete();

	/** 初始化完成后的钩子，子类在这里绑定数据源、记录初始值等。 */
	virtual void OnInitialized();

	/** 应用时的钩子，子类在这里把值真正写到游戏/平台配置里。 */
	virtual void OnApply();

	/** 收集本项自己的编辑状态（可见/可编辑/不可重置等），子类重写来追加条件。 */
	virtual void OnGatherEditState(FGameSettingEditableState& InOutEditState) const;

	/** 依赖项发生变化后的钩子。 */
	virtual void OnDependencyChanged();

	/**  */
	virtual FText GetDynamicDetailsInternal() const;

	/** */
	void HandleEditDependencyChanged(UGameSetting* DependencySetting, EGameSettingChangeReason Reason);
	void HandleEditDependencyChanged(UGameSetting* DependencySetting);

	/** Regenerates the plain searchable text if it has been dirtied. */
	void RefreshPlainText() const;
	void InvalidateSearchableText() { bRefreshPlainSearchableText = true; }

	/** Notify that the setting changed */
	void NotifySettingChanged(EGameSettingChangeReason Reason);
	virtual void OnSettingChanged(EGameSettingChangeReason Reason);

	/** Notify that the settings edit conditions changed.  This may mean it's now invisible, or disabled, or possibly that the options have changed in some meaningful way. */
	void NotifyEditConditionsChanged();
	virtual void OnEditConditionsChanged();

	/**  */
	FGameSettingEditableState ComputeEditableState() const;

protected:

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> SettingParent;

	UPROPERTY(Transient)
	TObjectPtr<UGameSettingRegistry> OwningRegistry;

	FName DevName;
	FText DisplayName;
	ESlateVisibility DisplayNameVisibility = ESlateVisibility::SelfHitTestInvisible;
	FText DescriptionRichText;
	FText WarningRichText;

	/** A collection of tags for the settings.  These can just be arbitrary flags used by the UI to do different things. */
	FGameplayTagContainer Tags;

	FGetGameSettingsDetails DynamicDetails;

	/** Any edit conditions for this setting. */
	TArray<TSharedRef<FGameSettingEditCondition>> EditConditions;

	class FStringCultureCache
	{
		FStringCultureCache(TFunction<FString()> InStringGetter);

		void Invalidate();

		FString Get() const;

	private:
		mutable FString StringCache;
		mutable FCultureRef Culture;
		TFunction<FString()> StringGetter;
	};

	/** When the text changes, we invalidate the searchable text. */
	mutable bool bRefreshPlainSearchableText = true;
	/** When we set the rich text for a setting, we automatically generate the plain text. */
	mutable FString AutoGenerated_DescriptionPlainText;

	/** Report as part of analytics, by default no setting reports, except GameSettingValues. */
	bool bReportAnalytics = false;

private:

	/** 是否已就绪。多数设置项立刻就绪；少数需要异步初始化（如读平台配置），
	 *  设置界面会等所有项都就绪后才显示出来。 */
	bool bReady = false;

	/** 防止"设置已变更"广播过程中的重入（回调里再改值导致无限递归）。 */
	bool bOnSettingChangedEventGuard = false;

	/** Prevent re-entrancy problems when announcing a setting has changed edit conditions. */
	bool bOnEditConditionsChangedEventGuard = false;

	/**  */
	bool bAdjustListViewPostRefresh = true;

	/** 编辑状态缓存：只在状态变化时重算，而不是每次用到都算一遍。 */
	FGameSettingEditableState EditableStateCache;
};
