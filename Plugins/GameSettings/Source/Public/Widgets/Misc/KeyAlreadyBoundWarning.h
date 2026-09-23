// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingPressAnyKey.h"
#include "KeyAlreadyBoundWarning.generated.h"

class UTextBlock;

/**
 * UKeyAlreadyBoundWarning —— 带"按键已被占用"提示的选键界面。
 *
 * 在 UGameSettingPressAnyKey 基础上多了两行文字：警告文本与取消提示文本。
 * 改键位时如果新按的键已经被别的动作占用，就用它提示玩家。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UKeyAlreadyBoundWarning : public UGameSettingPressAnyKey
{
	GENERATED_BODY()

public:
	void SetWarningText(const FText& InText);

	void SetCancelText(const FText& InText);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> WarningText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UTextBlock> CancelText;
};
