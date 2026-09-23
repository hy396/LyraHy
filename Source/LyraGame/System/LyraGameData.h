// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 全局游戏数据资产（不可变）：集中存放全项目共用的默认 GameplayEffect 等配置，
 * 由 ULyraAssetManager 在启动时加载，之后通过 ULyraGameData::Get() 访问。
 */
#include "Engine/DataAsset.h"

#include "LyraGameData.generated.h"

class UGameplayEffect;
class UObject;

// 全局游戏数据资产，运行期只读
/**
 * ULyraGameData
 *
 *	Non-mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Lyra Game Data", ShortTooltip = "Data asset containing global game data."))
class ULyraGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	ULyraGameData();

	// 取已加载的全局游戏数据（内部转发给资产管理器）
	// Returns the loaded game data.
	static const ULyraGameData& Get();

public:

	// 造成伤害用的 GE，伤害数值通过 SetByCaller 传入
	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// 治疗用的 GE，治疗量通过 SetByCaller 传入
	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// 增删动态标签用的 GE（把标签挂在 GE 上，通过增删 GE 来增删标签）
	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};
