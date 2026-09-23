// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"

class ULocalPlayer;


/**
 * FWhenPlayingAsPrimaryPlayer —— 编辑条件：只有【主玩家】才可见/可编辑。
 *
 * 用于分屏场景：第二个及以后的本地玩家不该改某些影响全局的设置。
 * 非主玩家时该项会被彻底杀掉（隐藏 + 不可重置 + 排除出数据分析）。
 */
class GAMESETTINGS_API FWhenPlayingAsPrimaryPlayer : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlayingAsPrimaryPlayer> Get();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override;
};
