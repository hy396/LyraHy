// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"

#include "GameSettingListEntry.generated.h"

class FGameSettingEditableState;
enum class EGameSettingChangeReason : uint8;

class UAnalogSlider;
class UCommonButtonBase;
class UCommonTextBlock;
class UGameSetting;
class UGameSettingAction;
class UGameSettingCollectionPage;
class UGameSettingRotator;
class UGameSettingValueDiscrete;
class UGameSettingValueScalar;
class UObject;
class UPanelWidget;
class UUserWidget;
class UWidget;
struct FFocusEvent;
struct FFrame;
struct FGeometry;

//////////////////////////////////////////////////////////////////////////
// UAthenaChallengeListEntry
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntryBase —— 列表条目控件的【基类】。
 *
 * 它实现 IUserObjectListEntry，负责把 ListView 传来的 UGameSetting 绑到控件上，
 * 并统一处理"设置项被禁用/隐藏"时的视觉状态。所有自定义条目控件都该从它派生。
 */
UCLASS(Abstract, NotBlueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntryBase : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting);
	virtual void SetDisplayNameOverride(const FText& OverrideName);

protected:
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged();
	virtual void HandleEditConditionChanged(UGameSetting* InSetting);
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState);
	
protected:
	// Focus transitioning to subwidgets for the gamepad
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UFUNCTION(BlueprintImplementableEvent)
	UWidget* GetPrimaryGamepadFocusWidget();

protected:
	bool bSuspendChangeUpdates = false;

	UPROPERTY()
	TObjectPtr<UGameSetting> Setting;

	FText DisplayNameOverride = FText::GetEmpty();

private:
	void HandleSettingChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UUserWidget> Background;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntry_Setting
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntry_Setting —— 通用设置项的条目控件（显示名称 + 右侧交互区）。
 *
 * 它是下面 Discrete / Scalar / Action / Navigation 四种具体条目的共同父类，
 * 负责左侧名称文字、整体可交互性、选中态等公共表现。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntry_Setting : public UGameSettingListEntryBase
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;
	
private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Discrete
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntrySetting_Discrete —— 离散值设置项的条目（左右切换器 / 下拉选择）。
 * 对应 UGameSettingValueDiscrete：在若干固定选项之间切换，并显示"哪项是默认值"。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Discrete : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;

	void HandleOptionDecrease();
	void HandleOptionIncrease();
	void HandleRotatorChangedValue(int32 Value, bool bUserInitiated);

	void Refresh();
	virtual void OnSettingChanged() override;
	virtual void HandleEditConditionChanged(UGameSetting* InSetting) override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingValueDiscrete> DiscreteSetting;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingRotator> Rotator_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Decrease;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Increase;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Scalar
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntrySetting_Scalar —— 连续值设置项的条目（滑块）。
 * 对应 UGameSettingValueScalar：滑块统一在 0~1 的归一化坐标上工作，
 * 由本控件负责在"归一化值"与"真实值"之间来回换算并显示格式化文本。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Scalar : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	void Refresh();
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;

	UFUNCTION()
	void HandleSliderValueChanged(float Value);
	UFUNCTION()
	void HandleSliderCaptureEnded();

	UFUNCTION(BlueprintImplementableEvent)
	void OnValueChanged(float Value);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDefaultValueChanged(float DefaultValue);

	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingValueScalar> ScalarSetting;

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UPanelWidget> Panel_Value;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UAnalogSlider> Slider_SettingValue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_SettingValue;
};


//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Action
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntrySetting_Action —— 按钮型设置项的条目。
 * 对应 UGameSettingAction：没有值，点一下就执行动作（如"恢复默认"）。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Action : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

	void HandleActionButtonClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(const FText& ActionText);

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingAction> ActionSetting;

private:	// Bound Widgets

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Action;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingListEntrySetting_Navigation
//////////////////////////////////////////////////////////////////////////

/**
 * UGameSettingListEntrySetting_Navigation —— 子页面入口条目。
 * 对应 UGameSettingCollectionPage：点它会导航进入该子页面内部的设置项列表。
 */
UCLASS(Abstract, Blueprintable, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingListEntrySetting_Navigation : public UGameSettingListEntry_Setting
{
	GENERATED_BODY()

public:
	virtual void SetSetting(UGameSetting* InSetting) override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void RefreshEditableState(const FGameSettingEditableState& InEditableState) override;

	void HandleNavigationButtonClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(const FText& ActionText);

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingCollectionPage> CollectionSetting;

private:	// Bound Widgets

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UCommonButtonBase> Button_Navigate;
};
