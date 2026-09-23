// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameSettingRegistry.h"
#include "GameSettingRegistryChangeTracker.h"

#include "GameSettingScreen.generated.h"

class UGameSetting;
class UGameSettingCollection;
class UGameSettingPanel;
class UObject;
class UWidget;
struct FFrame;

enum class EGameSettingChangeReason : uint8;

/**
 * UGameSettingScreen —— 设置界面基类（一个可激活的 UI 界面）。
 *
 * 它是整个设置界面的最外层：持有注册表与改动跟踪器，提供"应用/取消"两个动作，
 * 并在界面关闭时负责把未保存的改动还原掉。
 * 子类必须实现 CreateRegistry() 来给出具体的设置清单。
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** 按 DevName 跳转并选中某个设置项（常用于从外部直接定位到某项）。 */
	UFUNCTION(BlueprintCallable)
	void NavigateToSetting(FName SettingDevName);
	
	/** 批量跳转：依次进入多层子页面，最终选中最后一项。 */
	UFUNCTION(BlueprintCallable)
	void NavigateToSettings(const TArray<FName>& SettingDevNames);

	UFUNCTION(BlueprintNativeEvent)
	void OnSettingsDirtyStateChanged(bool bSettingsDirty);
	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) { }

	/** 尝试退出当前子页面回到上一层；已在最顶层时返回 false（交由上层关闭界面）。 */
	UFUNCTION(BlueprintCallable)
	bool AttemptToPopNavigation();

	UFUNCTION(BlueprintCallable)
	UGameSettingCollection* GetSettingCollection(FName SettingDevName, bool& HasAnySettings); 

protected:
	/** 子类必须实现：创建并返回本界面对应的设置注册表。 */
	virtual UGameSettingRegistry* CreateRegistry() PURE_VIRTUAL(, return nullptr;);

	template <typename GameSettingRegistryT = UGameSettingRegistry>
	GameSettingRegistryT* GetRegistry() const { return Cast<GameSettingRegistryT>(const_cast<UGameSettingScreen*>(this)->GetOrCreateRegistry()); }

	/** 取消：把所有改动还原到打开界面时的初始值。 */
	UFUNCTION(BlueprintCallable)
	virtual void CancelChanges();

	/** 应用：把改动真正落地保存。 */
	UFUNCTION(BlueprintCallable)
	virtual void ApplyChanges();

	UFUNCTION(BlueprintCallable)
	/** 是否有尚未保存的改动（界面据此显示"保存/取消"按钮）。 */
	bool HaveSettingsBeenChanged() const { return ChangeTracker.HaveSettingsBeenChanged(); }

	void ClearDirtyState();

	void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	/** 改动跟踪器：记录哪些项被改过，支撑应用/取消。 */
	FGameSettingRegistryChangeTracker ChangeTracker;

private:
	UGameSettingRegistry* GetOrCreateRegistry();

private:	// Bound Widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameSettingPanel> Settings_Panel;

	UPROPERTY(Transient)
	mutable TObjectPtr<UGameSettingRegistry> Registry;
};
