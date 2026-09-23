// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "GameSettingVisualData.generated.h"

class FName;
class UGameSetting;
class UGameSettingDetailExtension;
class UGameSettingListEntryBase;
class UObject;

/** 按【设置项类型】配置的详情扩展列表。 */
USTRUCT(BlueprintType)
struct FGameSettingClassExtensions
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Extensions)
	TArray<TSoftClassPtr<UGameSettingDetailExtension>> Extensions;
};

/** 按【设置项 DevName】配置的详情扩展列表；可开关是否同时继承按类型配的那批。 */
USTRUCT(BlueprintType)
struct FGameSettingNameExtensions
{
	GENERATED_BODY()

public:
	/** 是否同时应用"按类型"配置的扩展（默认否，即按名字的配置会替换掉按类型的）。 */
	UPROPERTY(EditAnywhere, Category = Extensions)
	bool bIncludeClassDefaultExtensions = false;

	UPROPERTY(EditAnywhere, Category = Extensions)
	TArray<TSoftClassPtr<UGameSettingDetailExtension>> Extensions;
};

/**
 * UGameSettingVisualData —— 设置界面的"外观配置资产"。
 *
 * 它把"数据"与"表现"解耦：同一个设置项换一份 VisualData 就能换一套长相，
 * 无需改 C++。两张核心映射表：
 *   - EntryWidgetForClass / EntryWidgetForName —— 决定用哪个条目控件渲染
 *   - ExtensionsForClasses / ExtensionsForName —— 决定详情区挂哪些扩展控件
 */
UCLASS(BlueprintType)
class GAMESETTINGS_API UGameSettingVisualData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 取某个设置项该用哪个条目控件类（先按名字找，找不到再按类型找）。 */
	TSubclassOf<UGameSettingListEntryBase> GetEntryForSetting(UGameSetting* InSetting);

	/** 汇总某个设置项该挂哪些详情扩展控件（按名字 + 按类型 合并）。 */
	virtual TArray<TSoftClassPtr<UGameSettingDetailExtension>> GatherDetailExtensions(UGameSetting* InSetting);
	
protected:
	virtual TSubclassOf<UGameSettingListEntryBase> GetCustomEntryForSetting(UGameSetting* InSetting);

protected:
	UPROPERTY(EditDefaultsOnly, Category = ListEntries, meta = (AllowAbstract))
	TMap<TSubclassOf<UGameSetting>, TSubclassOf<UGameSettingListEntryBase>> EntryWidgetForClass;

	UPROPERTY(EditDefaultsOnly, Category = ListEntries, meta = (AllowAbstract))
	TMap<FName, TSubclassOf<UGameSettingListEntryBase>> EntryWidgetForName;

	UPROPERTY(EditDefaultsOnly, Category = Extensions, meta = (AllowAbstract))
	TMap<TSubclassOf<UGameSetting>, FGameSettingClassExtensions> ExtensionsForClasses;

	UPROPERTY(EditDefaultsOnly, Category = Extensions)
	TMap<FName, FGameSettingNameExtensions> ExtensionsForName;
};
