// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"

#include "GameSettingPressAnyKey.generated.h"

struct FKey;

class UObject;

/**
 * UGameSettingPressAnyKey —— "请按任意键"界面（改键位时用）。
 *
 * 它会在激活时挂一个输入预处理器，抢在其它 UI 之前吃掉第一次按键，
 * 然后把按下的键通过 OnKeySelected 抛出去，或按 Esc 时抛 OnKeySelectionCanceled。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingPressAnyKey : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UGameSettingPressAnyKey(const FObjectInitializer& Initializer);

	/** 用户按下了一个有效键（参数是那个键）。 */
	DECLARE_EVENT_OneParam(UGameSettingPressAnyKey, FOnKeySelected, FKey);
	FOnKeySelected OnKeySelected;

	/** 用户取消了选键（通常按了 Esc）。 */
	DECLARE_EVENT(UGameSettingPressAnyKey, FOnKeySelectionCanceled);
	FOnKeySelectionCanceled OnKeySelectionCanceled;

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void HandleKeySelected(FKey InKey);
	void HandleKeySelectionCanceled();

	/** 关闭本界面，并在关闭完成后执行回调（用于把结果交回调用方）。 */
	void Dismiss(TFunction<void()> PostDismissCallback);

private:
	/** 是否已经选到了键（防止一次按键被处理两遍）。 */
	bool bKeySelected = false;

	/** 抢在其它 UI 之前拦截按键的输入预处理器。 */
	TSharedPtr<class FSettingsPressAnyKeyInputPreProcessor> InputProcessor;
};
