// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"

class ULocalPlayer;

//--------------------------------------
// FGameSettingDataSource
//--------------------------------------

/**
 * FGameSettingDataSource —— 设置项的"数据源"抽象。
 *
 * 它把"值存在哪、怎么读写"从设置项里剥离出来，统一成 字符串 的读写接口。
 * 这样同一个设置项类（如 GameSettingValueDiscreteDynamic）就能挂不同的数据源。
 */
class GAMESETTINGS_API FGameSettingDataSource : public TSharedFromThis<FGameSettingDataSource>
{
public:
	virtual ~FGameSettingDataSource() { }

	/**
	 * Some settings may take an async amount of time to finish initializing.  The settings system will wait
	 * for all settings to be ready before showing the setting.
	 */
	virtual void Startup(ULocalPlayer* InLocalPlayer, FSimpleDelegate StartupCompleteCallback) { StartupCompleteCallback.ExecuteIfBound(); }

	/** 解析数据源：确认当前玩家上下文下这个数据源确实可取（失败则该设置项不显示）。 */
	virtual bool Resolve(ULocalPlayer* InContext) = 0;

	/** 把当前值以字符串形式读出。 */
	virtual FString GetValueAsString(ULocalPlayer* InContext) const = 0;

	/** 把字符串形式的值写回去。 */
	virtual void SetValue(ULocalPlayer* InContext, const FString& Value) = 0;

	virtual FString ToString() const = 0;
};
