// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingFilterState.h"

/**
 * FWhenCondition —— 内联编辑条件。
 *
 * 不想为一次性小条件专门建个类时，直接把一段 lambda 塞进来即可：
 *   Setting->AddEditCondition(MakeShared<FWhenCondition>([](const ULocalPlayer* LP, FGameSettingEditableState& State){ ... }));
 */
class FWhenCondition : public FGameSettingEditCondition
{
public:
	FWhenCondition(TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState&)>&& InInlineEditCondition)
		: InlineEditCondition(InInlineEditCondition)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const override
	{
		InlineEditCondition(InLocalPlayer, InOutEditState);
	}

	virtual FString ToString() const override
	{
		return TEXT("Inline Edit Condition");
	}

private:
	TFunction<void(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState)> InlineEditCondition;
};
