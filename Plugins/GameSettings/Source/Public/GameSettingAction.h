// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingAction.generated.h"

//--------------------------------------
// UGameSettingAction
//--------------------------------------

class ULocalPlayer;

// 自定义动作回调："点这个设置项时要干什么"由它决定。
DECLARE_DELEGATE_TwoParams(UGameSettingCustomAction, UGameSetting* /*Setting*/, ULocalPlayer* /*LocalPlayer*/)

/**
 * UGameSettingAction —— "按钮型"设置项：没有值，点了就执行一个动作。
 *
 * 例如"恢复默认设置""查看制作人员名单""解绑按键"。
 * 触发方式有两种，二选一：
 *   - NamedAction：只给一个 GameplayTag，由上层监听者决定做什么（便于 UI 与逻辑解耦）
 *   - CustomAction：直接绑一个 C++ 委托
 */
UCLASS()
class GAMESETTINGS_API UGameSettingAction : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingAction();

public:

	/** 执行"具名动作"时广播，带一个 GameplayTag 告诉上层具体是哪个动作。 */
	DECLARE_EVENT_TwoParams(UGameSettingAction, FOnExecuteNamedAction, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);
	FOnExecuteNamedAction OnExecuteNamedActionEvent;

public:

	FText GetActionText() const { return ActionText; }
	void SetActionText(FText Value) { ActionText = Value; }
#if !UE_BUILD_SHIPPING
	void SetActionText(const FString& Value) { SetActionText(FText::FromString(Value)); }
#endif

	FGameplayTag GetNamedAction() const { return NamedAction; }
	void SetNamedAction(FGameplayTag Value) { NamedAction = Value; }

	bool HasCustomAction() const { return CustomAction.IsBound(); }
	void SetCustomAction(UGameSettingCustomAction InAction) { CustomAction = InAction; }
	void SetCustomAction(TFunction<void(ULocalPlayer*)> InAction);

	/** 默认情况下，执行动作【不会】把设置标记为"已改动"，因为多数动作要么不可撤销，
	 *  要么只是打开制作人员名单、用户协议之类的页面。
	 *  如果你的动作确实改动了设置，把这个开关打开，触发时才会广播变更事件。 */
	void SetDoesActionDirtySettings(bool Value) { bDirtyAction = Value; }

	virtual void ExecuteAction();

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;

protected:
	FText ActionText;
	FGameplayTag NamedAction;
	UGameSettingCustomAction CustomAction;
	bool bDirtyAction = false;
};
