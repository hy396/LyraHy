// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonRotator.h"

#include "GameSettingRotator.generated.h"

class UObject;

/**
 * UGameSettingRotator —— 带"默认值标记"的左右切换器。
 *
 * 在 UCommonRotator 基础上多一个能力：可以标出"哪一个是默认值"，
 * 让 UI 在该项上显示一个小圆点之类的提示，玩家就知道当前是不是出厂设置。
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UGameSettingRotator(const FObjectInitializer& Initializer);

	/** 指定第几个选项是"默认值"（传 INDEX_NONE 表示没有默认值）。 */
	void SetDefaultOption(int32 DefaultOptionIndex);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = Events, meta = (DisplayName = "On Default Option Specified"))
	void BP_OnDefaultOptionSpecified(int32 DefaultOptionIndex);
};
