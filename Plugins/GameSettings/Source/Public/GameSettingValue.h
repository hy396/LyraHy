// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingValue.generated.h"

class UObject;

//--------------------------------------
// UGameSettingValue
//--------------------------------------

/**
 * UGameSettingValue —— 所有"带数值"的设置项的基类。
 *
 * 相比基类多出三件事：记录初始值、可重置为默认值、可还原为初始值。
 * 三个都是纯虚函数，子类必须实现：
 *   - StoreInitial()      打开设置界面时记下"初始值"，用于之后撤销
 *   - ResetToDefault()    恢复到游戏出厂默认值
 *   - RestoreToInitial()  恢复到打开界面时的初始值（撤销本次改动）
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValue : public UGameSetting
{
	GENERATED_BODY()

public:
	UGameSettingValue();

	/** 记录"初始值"。初始化时会调用一次；此外"应用"之后也应该再调一次，
	 *  否则用户应用完再改就会撤销到旧值上。 */
	virtual void StoreInitial() PURE_VIRTUAL(, );

	/** Resets the property to the default. */
	virtual void ResetToDefault() PURE_VIRTUAL(, );

	/** Restores the setting to the initial value, this is the value when you open the settings before making any tweaks. */
	virtual void RestoreToInitial() PURE_VIRTUAL(, );

protected:
	virtual void OnInitialized() override;
};
