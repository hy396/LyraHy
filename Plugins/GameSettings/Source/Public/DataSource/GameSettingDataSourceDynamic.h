// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingDataSource.h"
#include "PropertyPathHelpers.h"

class ULocalPlayer;

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

/**
 * FGameSettingDataSourceDynamic —— 基于【属性路径】的动态数据源。
 *
 * 给一串属性名路径（如 "LocalPlayer→GameSettings→Resolution"），
 * 运行时用反射沿路径找到实际属性并读写，无需为每项写专门的 C++ 代码。
 * 代价是靠字符串定位，重命名属性会静默失效，改名字时务必同步检查。
 */
class GAMESETTINGS_API FGameSettingDataSourceDynamic : public FGameSettingDataSource
{
public:
	FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath);

	virtual bool Resolve(ULocalPlayer* InLocalPlayer) override;

	virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const override;

	virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) override;

	virtual FString ToString() const override;

private:
	/** 缓存的属性路径（缓存是为了避免每次读写都重新解析一遍路径）。 */
	FCachedPropertyPath DynamicPath;
};
