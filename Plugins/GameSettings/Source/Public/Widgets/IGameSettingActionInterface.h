// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "IGameSettingActionInterface.generated.h"

class UGameSetting;
class UObject;
struct FFrame;

UINTERFACE(MinimalAPI, meta = (BlueprintType))
class UGameSettingActionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * IGameSettingActionInterface —— 具名动作的处理接口。
 *
 * 让任意 UObject（通常是界面或子系统）都能接住 UGameSettingAction 抛出的
 * GameplayTag 动作。好处是设置项不必知道谁会处理它，只管抛标签。
 */
class GAMESETTINGS_API IGameSettingActionInterface
{
	GENERATED_BODY()

public:
	/** 处理一个具名动作。返回 true 表示已处理，不再继续向别处分发。 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ExecuteActionForSetting(FGameplayTag ActionTag, UGameSetting* InSetting);
};

