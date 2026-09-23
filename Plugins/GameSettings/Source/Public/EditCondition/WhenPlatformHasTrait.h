// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

class ULocalPlayer;

//////////////////////////////////////////////////////////////////////
// FWhenPlatformHasTrait

// 编辑条件：检查 CommonUI 的"平台特性标签"来决定某个设置项要不要显示。
// 例如手柄相关设置只在支持手柄的平台显示。
// 四个工厂方法分别是：
//   KillIfMissing     —— 缺少该特性就彻底杀掉（隐藏 + 不可重置 + 不上报）
//   DisableIfMissing  —— 缺少该特性就禁用（仍可见，给玩家看禁用理由）
//   KillIfPresent     —— 拥有该特性就彻底杀掉
//   DisableIfPresent  —— 拥有该特性就禁用
class GAMESETTINGS_API FWhenPlatformHasTrait : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlatformHasTrait> KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	static TSharedRef<FWhenPlatformHasTrait> KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason);
	static TSharedRef<FWhenPlatformHasTrait> DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	//~FGameSettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
	//~End of FGameSettingEditCondition interface

private:
	FGameplayTag VisibilityTag;
	bool bTagDesired;
	FString KillReason;
	FText DisableReason;
};
