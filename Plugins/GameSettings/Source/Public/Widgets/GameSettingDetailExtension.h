// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"

#include "GameSettingDetailExtension.generated.h"

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UObject;

/**
 * UGameSettingDetailExtension —— 详情面板里的"扩展区块"。
 *
 * 想在某个设置项的详情区里额外显示自定义内容（比如按键绑定的冲突提示、
 * 一个"重置此项"的小按钮），就做一个它的子类，并在 UGameSettingVisualData 里
 * 按"设置项类型"或"设置项 DevName"配上去。
 */
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class GAMESETTINGS_API UGameSettingDetailExtension : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSetting(UGameSetting* InSetting);
	
protected:
	virtual void NativeSettingAssigned(UGameSetting* InSetting);
	virtual void NativeSettingValueChanged(UGameSetting* InSetting, EGameSettingChangeReason Reason);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingAssigned(UGameSetting* InSetting);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSettingValueChanged(UGameSetting* InSetting);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> Setting;
};
